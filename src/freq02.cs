using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace freq02
{
    class trie
    {
        public node Root = new node() { Word = "" };

        public IEnumerable<node> Enumerate(node root)
        {
            for (int i = 0; i < 26; i++)
            {
                var child = root.Children[i];
                if (child != null)
                {
                    if (child.Count > 0)
                        yield return root.Children[i];

                    foreach (var child_node in Enumerate(child))
                    {
                        yield return child_node;
                    }
                }
            }
        }

        public class node
        {
            public node[] Children = new node[26];
            public int Count;
            public string Word;

            public node WeNeedToGoDeeper(int index)
            {
                return Children[index] ?? (Children[index] = new node() {Word = Word + (char) (index + 97)});
            }
        }
    }

    class freq02
    {
        private static readonly trie _trie = new trie();
        private static void Count(string filename)
        {
            var text = File.ReadAllBytes(filename);

            trie.node node = _trie.Root;
            foreach (var ch in text)
            {
                if (ch >= 'a' && ch <= 'z')
                {
                    var index = ch - 97;
                    node = node.WeNeedToGoDeeper(index);
                    continue;
                }

                if (ch >= 'A' && ch <= 'Z')
                {
                    var index = ch - 65;
                    node = node.WeNeedToGoDeeper(index);
                    continue;
                }

                if (node == _trie.Root)
                    continue;

                node.Count++;
                node = _trie.Root;
            }

            if (node != _trie.Root)
            {
                node.Count++;
            }
        }

        private static void SortAndDump(string filename)
        {
            var enumerable = _trie.Enumerate(_trie.Root);
            var sorted = enumerable.OrderByDescending(n => n.Count); //.ThenBy(n => n.Word); words are already sorted during trie traversal
            File.WriteAllLines(filename, sorted.Select(n => $"{n.Count} {n.Word}"));
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
            Console.WriteLine($"Elapsed time: {(decimal) stopwatch.ElapsedMilliseconds / 1000}s");
        }
    }
}
