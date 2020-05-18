package freq

import java.nio.charset.StandardCharsets

import cats.effect._
import cats.implicits._
import com.monovore.decline._
import com.monovore.decline.effect._
import com.sun.xml.internal.fastinfoset.util.StringArray
import fs2._
import it.unimi.dsi.fastutil.bytes.ByteArrayList
import it.unimi.dsi.fastutil.ints.{Int2ObjectAVLTreeMap, Int2ObjectSortedMaps}
import it.unimi.dsi.fastutil.objects.{Object2IntMap, Object2IntMaps, Object2IntOpenHashMap, ObjectAVLTreeSet}

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
          .map { dict =>
            val words = new Int2ObjectAVLTreeMap[ObjectAVLTreeSet[String]]
            Object2IntMaps.fastForEach(dict, { e: Object2IntMap.Entry[String] =>
              val key   = e.getIntValue
              val value = e.getKey
              val _ = words
                .getOrDefault(key, new ObjectAVLTreeSet[String])
                .add(value)
            })
            words
          }
          .flatMap { counts =>
            Stream
              .unfold(Int2ObjectSortedMaps.fastIterator(counts)) { entries =>
                if (entries.hasPrevious) {
                  val prev = entries.previous()
                  (Stream.unfold(prev.getValue.iterator()) { values =>
                    if (values.hasNext) ((prev.getIntKey, values.next()) -> values).some
                    else none
                  } -> entries).some
                } else none
              }
              .flatten
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
          val words = new StringArray()
          val word  = rem
          chunk.foreach { byte =>
            if (a <= byte && byte <= z) {
              val _ = word.add(byte)
            } else if (A <= byte && byte <= Z) {
              val _ = word.add((byte ^ 0x020).toByte)
            } else if (!word.isEmpty) {
              val _ = words.add(bal2String(word))
            }
          }

          Pull.output(Chunk.seq(words.getArray)) >> loop(s, word)
      }

    loop(bytes, new ByteArrayList(16)).stream
  }
}
