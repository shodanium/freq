import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import static java.lang.Character.toLowerCase;

class freq01 {

    public static void main(String[] args) throws IOException {

        if (args.length < 3) {
            System.out.println("usage: freq <in.txt> <out.txt> [--prof]");
            return;
        }

        List<String> words = new ArrayList<>();
        try (BufferedReader fileBufferReader = new BufferedReader(new FileReader(args[1]))) {
            String line;
            StringBuilder sb = new StringBuilder();
            while ((line = fileBufferReader.readLine()) != null) {
                char[] s = line.toCharArray();
                for (char c: s) {
                    if (c >= 'a' && c <= 'z') {
                        sb.append(c);
                    } else if (c >= 'A' && c <= 'Z') {
                        sb.append(toLowerCase(c));
                    } else if (sb.length() > 0) {
                        words.add(sb.toString());
                        sb.setLength(0);
                    }
                }
                if (sb.length() > 0) {
                    words.add(sb.toString());
                    sb.setLength(0);
                }
            }
        }

        List<E> res = new freq01().freq(words);

        PrintWriter pw = new PrintWriter(new FileWriter(args[2]));
        StringBuilder sb = new StringBuilder();
        for (E e: res) {
            sb.setLength(0);
            pw.println(sb.append(e.freq).append(" ").append(e.s));
        }
        pw.close();
    }

    public List<E> freq(List<String> words) {
        Map<String, E> m = new HashMap<>(words.size() >> 2);
        for (String word: words) {
            m.compute(word, freq01::update);
        }

        return m.values()
                .stream()
                .sorted()
                .collect(Collectors.toList());
    }

    static E update(String k, E v) {
        if (v != null) {
            v.freq++;
            return v;
        } else {
            return new E(k);
        }
    }

    static class E implements Comparable<E> {
        final String s;
        int freq;

        E(String s) {
            this.s = s;
            this.freq = 1;
        }

        @Override
        public int compareTo(E other) {
            return freq != other.freq ? other.freq - freq : s.compareTo(other.s);
        }
    }
}
