import java.io._
import java.nio.ByteBuffer
import java.nio.channels.FileChannel
import java.nio.charset.StandardCharsets
import java.nio.file.Paths
import java.util

import cats.data.{ NonEmptyList, ValidatedNel }
import cats.effect._
import cats.implicits._
import com.monovore.decline._
import com.monovore.decline.effect._
import fs2._
import it.unimi.dsi.fastutil.bytes.ByteArrayList
import it.unimi.dsi.fastutil.objects.ObjectArrays

import scala.annotation.tailrec

package freq01 {
  object App extends CommandIOApp("freq", "Counts '[a-zA-Z]+' words in input", version = "0.1.0") {
    private val a = 'a'.toByte
    private val z = 'z'.toByte
    private val A = 'A'.toByte
    private val Z = 'Z'.toByte

    def main: Opts[IO[ExitCode]] = Args.parse.map { args =>
      Blocker[IO]
        .use { blocker =>
          args
            .input[IO](blocker)
            .through(collect)
            .flatMap { dict =>
              Stream.fromIterator[IO](dict.drain)
            }
            .map {
              case (counter, word) => f"$counter%d $word%s%n"
            }
            .through(args.output(blocker))
            .compile
            .drain
        }
        .as(ExitCode.Success)
    }

    def collect[F[_]]: Pipe[F, ByteBuffer, FrequencyDict] = { buffers =>
      def loop(
          s: Stream[F, ByteBuffer],
          dict: FrequencyDict,
          lastHash: Int,
          lastWord: ByteArrayList
      ): Pull[F, FrequencyDict, Unit] =
        s.pull.uncons1.flatMap {
          case None =>
            if (lastWord.isEmpty) Pull.output1(dict)
            else Pull.output1(dict.register(lastHash, lastWord.toArray(new Array[Byte](lastWord.size()))))
          case Some(buffer -> nxt) =>
            var hash = lastHash
            val word = lastWord

            while (buffer.remaining() > 0) {
              var byte = buffer.get()
              if (a <= byte && byte <= z) {
                hash = Fnv1.next(hash, byte)
                word.add(byte)
              } else if (A <= byte && byte <= Z) {
                byte = (byte | 0x20).toByte
                hash = Fnv1.next(hash, byte)
                word.add(byte)
              } else if (!word.isEmpty) {
                dict.register(hash, word.toArray(new Array[Byte](word.size())))
                word.clear()
                hash = Fnv1.H
              }
            }

            loop(nxt, dict, hash, word)
        }

      loop(buffers, FrequencyDict(), Fnv1.H, new ByteArrayList(256)).stream
    }

  }

  final case class Args(in: Option[File], out: Option[File], chunkSize: Int, bufferSize: Option[Int]) {
    def input[F[_]](blocker: Blocker)(implicit F: Sync[F], CS: ContextShift[F]): Stream[F, ByteBuffer] =
      in.map { file =>
          Stream
            .bracket {
              blocker.delay(new FileInputStream(file).getChannel)
            } { channel =>
              blocker.delay(channel.close())
            }
            .flatMap { channel =>
              Stream.unfoldEval(0L -> math.min(channel.size(), Int.MaxValue.toLong)) {
                case (_, 0L) => F.pure(none[(ByteBuffer, (Long, Long))])
                case (p, sz) =>
                  val size = math.min(sz, Int.MaxValue.toLong)
                  blocker
                    .delay(channel.map(FileChannel.MapMode.READ_ONLY, p, size))
                    .widen[ByteBuffer]
                    .tupleRight((p + size) -> (channel.size() - p - size))
                    .map(_.some)
              }
            }
        }
        .getOrElse {
          io.stdin(chunkSize, blocker).chunks.map(_.toByteBuffer)
        }

    def output[F[_]](blocker: Blocker)(implicit F: Sync[F], CS: ContextShift[F]): Pipe[F, String, Unit] = { lines =>
      val bytes = lines.flatMap { str =>
        Stream.chunk(Chunk.array(str.getBytes(StandardCharsets.UTF_8)))
      }
      val writer = out
        .map { file =>
          val fos = F
            .catchNonFatal {
              new BufferedOutputStream(new FileOutputStream(file, false))
            }
            .widen[OutputStream]
          io.writeOutputStream(fos, blocker)
        }
        .getOrElse {
          io.stdout(blocker)
        }
      bytes.through(writer)
    }

  }

  object Args {
    def stringToFile(fileName: String): ValidatedNel[String, File] =
      Either
        .catchNonFatal {
          Paths.get(fileName).toFile
        }
        .toValidated
        .leftMap(e => s"Unable to open '$fileName': ${e.getMessage}")
        .toValidatedNel

    val in: Opts[Option[File]] = Opts
      .argument[String]("input.txt")
      .mapValidated {
        case "-" => none.validNel[String]
        case fnm =>
          stringToFile(fnm)
            .ensure(NonEmptyList.one(s"'$fnm' not exists"))(_.exists())
            .ensure(NonEmptyList.one(s"'$fnm' is not a file"))(_.isFile())
            .ensure(NonEmptyList.one(s"'$fnm' can not be read"))(_.canRead())
            .map(_.some)
      }

    val out: Opts[Option[File]] = Opts
      .argument[String]("output.txt")
      .mapValidated {
        case "-" => none.validNel[String]
        case fnm =>
          stringToFile(fnm)
            .ensure(NonEmptyList.one(s"'$fnm' can not be overwritten")) { f =>
              !f.exists() || f.canWrite
            }
            .map(_.some)
      }
      .withDefault(none)

    val bufferSize: Opts[Option[Int]] = Opts
      .option[Int]("buffer-size", "Length of read buffer.", short = "b")
      .orNone

    val chunkSize: Opts[Int] = Opts
      .option[Int]("chunk-size", "Length of one chunk when processing.", short = "c")
      .withDefault(16 * 1024)

    val parse: Opts[Args] = (in, out, chunkSize, bufferSize).mapN(Args(_, _, _, _))
  }

  object Fnv1 {
    val H: Int = 0x811c9dc5
    val P: Int = 0x01000193

    def next(hash: Int, value: Byte): Int = (hash ^ value) * P
  }

  final class FrequencyDict(initial: Int) {
    import FrequencyDict.{ LoadFactor, Value }

    private var capacity = initial
    private var length   = 0
    private var mask     = capacity - 1
    private var max      = (LoadFactor * capacity.toFloat).toInt

    private var hashes = new Array[Int](capacity)
    private var values = new Array[Value](capacity)

    def size: Int = length

    def drain: Iterator[(Int, String)] = {
      val data = values

      hashes = new Array[Int](initial)
      values = new Array[Value](initial)

      capacity = initial
      length = 0
      mask = capacity - 1
      max = (LoadFactor * capacity.toFloat).toInt

      // scalafix:off DisableSyntax.null; keeping buckets sparse for locality
      ObjectArrays.unstableSort(data, { (l: Value, r: Value) =>
        if ((l ne null) && (r ne null)) l.compareTo(r)
        else if (l ne null) -1
        else if (r ne null) 1
        else 0
      })
      //scalafix:on

      data.iterator
        .takeWhile(_ ne null) // scalafix:ok DisableSyntax.null
        .map(v => (v.value, v.key))
    }

    def register(hash: Int, key: Array[Byte]): this.type = {
      val hsh = if (hash == 0) Fnv1.H else hash
      @tailrec def loop(idx: Int): Unit = {
        val idxHash = hashes(idx)
        if (idxHash == 0) {
          hashes(idx) = hsh
          values(idx) = Value(key)
          length += 1

          if (length > max) ensureCapacity()
        } else if (idxHash != hsh || !values(idx).update(key)) {
          loop((idx + 1) & mask)
        }
      }

      loop(hsh & mask)
      this
    }

    private def ensureCapacity(): Unit = {
      while (length > max) {
        capacity *= 2
        mask = capacity - 1
        max = (LoadFactor * capacity.toFloat).toInt
      }

      val newHashes = new Array[Int](capacity)
      val newValues = new Array[Value](capacity)

      hashes.iterator.zipWithIndex.filter(_._1 != 0).foreach {
        case (hash, i) =>
          var idx = hash & mask
          while (newHashes(idx) != 0) idx = (idx + 1) & mask

          newHashes(idx) = hash
          newValues(idx) = values(i)
      }

      hashes = newHashes
      values = newValues
    }

  }

  object FrequencyDict {

    final class Value(private val bytes: Array[Byte]) extends Comparable[Value] {
      private var counter: Int = 1

      lazy val key: String = new String(bytes, StandardCharsets.UTF_8)
      def value: Int       = counter

      def update(arr: Array[Byte]): Boolean = {
        val same = util.Arrays.equals(bytes, arr)
        if (same) counter += 1
        same
      }

      def compareTo(that: Value): Int = {
        var r = that.counter - this.counter
        var i = 0
        while (r == 0 && i < this.bytes.length && i < that.bytes.length) {
          r = this.bytes(i) - that.bytes(i)
          i += 1
        }

        if (r == 0 && i < that.bytes.length) -1
        else if (r == 0 && i < this.bytes.length) 1
        else r
      }
    }

    object Value {
      def apply(bytes: Array[Byte]): Value =
        new Value(bytes)
    }

    val InitialCapacity = 128
    val LoadFactor      = 0.9f

    def apply(): FrequencyDict = new FrequencyDict(InitialCapacity)

  }

}
