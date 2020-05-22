import java.io.*;
import java.util.HashMap;

class freq01 {
    final static char[] ALPHABET = new char[256];

    static {
        for (char c = 'a'; c <= 'z'; c++) {
            ALPHABET[c & 0xFF] = c;
            ALPHABET[c ^ 0x20] = c;
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 2) {
            System.out.println("Usage: java -cp . freq01 in.txt out.txt");
            System.exit(1);
        }

        final HashMap<String, Integer> dict = new HashMap<>(256);
        final StringBuilder word = new StringBuilder(256);
        final byte[] buffer = new byte[16 * 1024];
        try (final BufferedInputStream bis = new BufferedInputStream(new FileInputStream(args[0]))) {
            for (int read = bis.read(buffer); read > 0; read = bis.read(buffer)) {
                for (int i = 0; i < read; i++) {
                    char c = ALPHABET[buffer[i] & 0xFF];
                    if (c != 0) {
                        word.append(c);
                    } else if (word.length() > 0) {
                        dict.merge(word.toString(), 1, Integer::sum);
                        word.setLength(0);
                    }
                }
            }
            if (word.length() > 0) {
                dict.merge(word.toString(), 1, Integer::sum);
            }
        }

        try (final BufferedWriter writer = new BufferedWriter(new FileWriter(args[1]))) {
            dict.entrySet()
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
