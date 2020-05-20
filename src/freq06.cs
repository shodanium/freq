using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Policy;
using System.Threading;

namespace freq06
{
    public class Accountant
    {
        private const int CAPACITY = 15000000;

        public class node
        {
            public int PersonalndexStart;
            public int Count;
            public int Depth;
            public string Word;
        }

        public class ThreadStartInfo
        {
            public int Start;
            public int End;
            public byte[] Data;
        }

        private node _root = new node();
        private readonly char[] _enumerationWordBuf = new char[256];
        private readonly char[] _charmap = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
        private readonly int[] _charIndexLookup = new[]
        {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

        private volatile node[] _sparseNodes = new node[CAPACITY];
        public volatile int _nextFreeChunkIndex = 26;

        public IEnumerable<node> Enumerate()
        {
            return Enumerate(_root);
        }

        public IEnumerable<node> Enumerate(node root)
        {
            var realIndex = root.PersonalndexStart;
            for (int i = 0; i < 26; i++)
            {
                var child = _sparseNodes[realIndex];
                _enumerationWordBuf[root.Depth] = _charmap[i];

                if (child != null)
                {
                    if (child.Count > 0)
                    {
                        child.Word = new string(_enumerationWordBuf, 0, root.Depth + 1);
                        yield return child;
                    }

                    foreach (var child_node in Enumerate(child))
                    {
                        yield return child_node;
                    }
                }

                realIndex++;
            }
        }

        public void Count(byte[] text, int chunks)
        {
            var len = text.Length;
            var start = 0;
            var threads = new List<Thread>();
            
            for (int i = 1; i <= chunks; i++)
            {
                var end = (len / chunks) * i  + 5;
                if (end > len)
                    end = len;

                while (end < len && _charIndexLookup[text[end]] != -1)
                {
                    end++;
                }

                var thread = new Thread(Count);
                thread.Start(new ThreadStartInfo { Data = text, Start = start, End = end });
                threads.Add(thread);

                start = end + 1;
            }

            foreach (var thread in threads)
                thread.Join();
        }

        private void Count(object state)
        {
            var tsi = state as ThreadStartInfo;

            var node = _root;
            int index;

            for (int i = tsi.Start; i < tsi.End; i++)
            {
                index = _charIndexLookup[tsi.Data[i]];

                if (index != -1)
                {
                    var newnode = _sparseNodes[node.PersonalndexStart + index];
                    if (newnode == null)
                    {
                        lock (_sparseNodes)
                        {
                            newnode = _sparseNodes[node.PersonalndexStart + index];
                            if (newnode == null)
                            {
                                newnode = new node();
                                newnode.PersonalndexStart = _nextFreeChunkIndex;
                                newnode.Depth = node.Depth + 1;
                                _nextFreeChunkIndex += 26;
                                _sparseNodes[node.PersonalndexStart + index] = newnode;
                            }
                        }
                    }

                    node = newnode;
                    continue;
                }

                if (node == _root)
                    continue;

                Interlocked.Increment(ref node.Count);
                node = _root;
            }

            if (node != _root)
            {
                Interlocked.Increment(ref node.Count);
            }
        }
    }

    class freq06
    {
        private static void SortAndDump(Accountant accountant, string filename)
        {
            var enumerable = accountant.Enumerate();
            var sorted = enumerable.OrderByDescending(n => n.Count); // words are already sorted by traversal
            File.WriteAllLines(filename, sorted.Select(n => $"{n.Count} {n.Word}"));
        }

        static void Main(string[] args)
        {
            if (args.Length != 2 && args.Length != 3)
            {
                Console.WriteLine("Usage: freq <input_file> <output_file> [chunks (default=4)]");
                return;
            }

            int chunks = 4;
            if (args.Length == 3)
                chunks = Int32.Parse(args[2]);

            var text = File.ReadAllBytes(args[0]);
            var acc = new Accountant();

            GC.TryStartNoGCRegion(244 * 1000 * 1000, true);
            acc.Count(text, chunks);
            SortAndDump(acc, args[1]);
        }
    }
}
