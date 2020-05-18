import java.nio.charset.StandardCharsets

import java.io._
import java.nio.charset.StandardCharsets
import java.nio.file.Paths

import cats.data._
import cats.effect._
import cats.implicits._
import com.monovore.decline._
import com.monovore.decline.effect._
import fs2._
import it.unimi.dsi.fastutil.bytes.ByteArrayList
import it.unimi.dsi.fastutil.io.FastBufferedInputStream
import it.unimi.dsi.fastutil.objects._

import scala.collection.mutable
import scala.jdk.CollectionConverters._

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
            .through(words)
            .fold(new Object2IntOpenHashMap[String]()) { (dict, word) =>
              dict.put(word, dict.getOrDefault(word, 0) + 1)
              dict
            }
            .flatMap { dict =>
              val ww = new ObjectArrayList[(Int, String)](dict.size())

              dict.object2IntEntrySet().fastForEach { e =>
                val _ = ww.add(e.getIntValue -> e.getKey)
              }
              ww.unstableSort({
                case ((c1: Int, w1: String), (c2: Int, w2: String)) =>
                  val ints = c2 - c1
                  if (ints == 0) w1.compareTo(w2)
                  else ints
              })

              Stream.fromIterator[IO](ww.iterator().asScala)
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

    def words[F[_]]: Pipe[F, Byte, String] = { bytes =>
      def bal2String(bytes: ByteArrayList): String = {
        val arr = new Array[Byte](bytes.size())
        bytes.toArray(arr)
        bytes.clear()

        new String(arr, StandardCharsets.UTF_8)
      }

      def loop(s: Stream[F, Byte], rem: ByteArrayList): Pull[F, String, Unit] =
        s.pull.uncons.flatMap {
          case None if rem.isEmpty => Pull.done
          case None                => Pull.output1(bal2String(rem))
          case Some(chunk -> s) =>
            val words = mutable.ArrayBuilder.make[String]
            val word  = rem
            chunk.foreach { byte =>
              if (a <= byte && byte <= z) {
                val _ = word.add(byte)
              } else if (A <= byte && byte <= Z) {
                val _ = word.add((byte ^ 0x020).toByte)
              } else if (!word.isEmpty) {
                words += bal2String(word)
              }
            }

            Pull.output(Chunk.seq(words.result())) >> loop(s, word)
        }

      loop(bytes, new ByteArrayList(16)).stream
    }
  }

  final case class Args(in: Option[File], out: Option[File], chunkSize: Int, bufferSize: Option[Int]) {

    def input[F[_]](blocker: Blocker)(implicit F: Sync[F], CS: ContextShift[F]): Stream[F, Byte] =
      in.map { file =>
          val fis = F
            .catchNonFatal {
              bufferSize
                .map { size =>
                  new FastBufferedInputStream(new FileInputStream(file), size)
                }
                .getOrElse {
                  new FastBufferedInputStream(new FileInputStream(file))
                }
            }
            .widen[InputStream]
          io.readInputStream(fis, chunkSize, blocker)
        }
        .getOrElse {
          io.stdin(chunkSize, blocker)
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

}
