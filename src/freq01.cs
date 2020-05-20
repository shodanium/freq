using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace freq01
{
    class freq01
    {
        private static readonly Dictionary<string, int> dict = new Dictionary<string, int>();

        private static void AddWord(StringBuilder word)
        {
            var strWord = word.ToString().ToLowerInvariant();

            if (dict.ContainsKey(strWord))
                dict[strWord]++;
            else
                dict[strWord] = 1;
        }

        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: freq <input_file> <output_file>");
                return;
            }

            using (StreamReader sr = new StreamReader(args[0]))
            {
                var word = new StringBuilder();
                while (sr.Peek() >= 0)
                {
                    var ch = (char)sr.Read();
                    if (Char.IsLetter(ch))
                    {
                        word.Append(ch);
                        continue;
                    }

                    if (word.Length == 0)
                        continue;

                    AddWord(word);
                    word = new StringBuilder();
                }

                if (word.Length > 0)
                    AddWord(word);
            }

            File.WriteAllLines(args[1], dict.OrderByDescending(kvp => kvp.Value)
                                                    .ThenBy(kvp => kvp.Key)
                                                    .Select(kvp => $"{kvp.Value} {kvp.Key}"));
        }
    }
}
