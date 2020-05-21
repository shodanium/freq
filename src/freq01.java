import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.util.*;

class freq01 {
  final static byte UPPER_A = (byte) 'A';
  final static byte UPPER_Z = (byte) 'Z';
  final static byte LOWER_A = (byte) 'a';
  final static byte LOWER_Z = (byte) 'z';

  public static void main(String[] args) throws Exception {
    if (args.length < 2) {
      System.out.println("Usage: java -cp . Main in.txt out.txt");
      System.exit(1);
    }

    HashMap<String, Integer> dict = new HashMap<>(256);
    try (FileChannel channel = new FileInputStream(args[0]).getChannel()) {
      MappedByteBuffer map = channel.map(FileChannel.MapMode.READ_ONLY, 0L, channel.size());

      byte[] word = new byte[256];
      int wordLength = 0;
      while (map.remaining() > 0) {
        byte b = map.get();
        if (LOWER_A <= b && b <= LOWER_Z) {
          word[wordLength] = b;
          wordLength += 1;
        } else if (UPPER_A <= b && b <= UPPER_Z) {
          word[wordLength]  = (byte) (b | 0x20);
          wordLength += 1;
        } else if (wordLength > 0) {
          String key = new String(Arrays.copyOf(word, wordLength), StandardCharsets.UTF_8);
          wordLength = 0;
          dict.merge(key, 1, Integer::sum);
        }
      }
      if (wordLength > 0) {
        String key = new String(Arrays.copyOf(word, wordLength), StandardCharsets.UTF_8);
        dict.merge(key, 1, Integer::sum);
      }
    }

    try (BufferedWriter writer = new BufferedWriter(new FileWriter(args[1]))) {
      dict
        .entrySet()
        .stream()
        .sorted((l, r) -> {
          int result = r.getValue().compareTo(l.getValue());
          if (result == 0) result = l.getKey().compareTo(r.getKey());
          return result;
        })
        .forEachOrdered((e) -> {
          try {
            writer.write(e.getValue().toString() + " " + e.getKey());
            writer.newLine();
          } catch (IOException ioException) {
            ioException.printStackTrace();
            System.exit(1);
          }
        });
    }
  }
}
