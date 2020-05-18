using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace freq01
{
    class freq01
    {
        private static readonly Dictionary<string, int> dict = new Dictionary<string, int>(500000);

        private static void Count(string filename)
        {
            var text = File.ReadAllText(filename);

            var word = new StringBuilder(16);
            var strWord = "";

            foreach (var ch in text)
            {
                if (ch >= 'a' && ch <= 'z')
                {
                    word.Append(ch);
                    continue;
                }

                if (ch >= 'A' && ch <= 'Z')
                {
                    word.Append((char)(ch + 32));
                    continue;
                }

                if (word.Length == 0)
                    continue;

                strWord = word.ToString();

                if (dict.ContainsKey(strWord))
                    dict[strWord]++;
                else
                    dict[strWord] = 1;

                word = new StringBuilder(16);
            }

            if (word.Length > 0)
            {
                strWord = word.ToString();

                if (dict.ContainsKey(strWord))
                    dict[strWord]++;
                else
                    dict[strWord] = 1;
            }
        }

        private static void SortAndDump(string filename)
        {
            var sorted = dict.OrderByDescending(kvp => kvp.Value).ThenBy(kvp => kvp.Key);
            File.WriteAllLines(filename, sorted.Select(kvp => $"{kvp.Value} {kvp.Key}"));
        }

        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: freq <input_file> <output_file>");
                return;
            }

            var stopwatch = new Stopwatch();
            stopwatch.Start();

            Count(args[0]);
            SortAndDump(args[1]);

            stopwatch.Stop();
            Console.WriteLine($"Elapsed time: {stopwatch.ElapsedMilliseconds/1000}s");
        }
    }
}
