#!/usr/bin/env python

import re
import mmap
import sys
from collections import defaultdict

def usage():
    print(f'{sys.argv[0]} <input> <output>')
    sys.exit(1)

def main():
    if len(sys.argv) != 3:
        usage()

    xpr = re.compile(b'[a-z]+', re.I)
    with open(sys.argv[1]) as fh:
        fh.seek(0, 2)
        fsz = fh.tell()
        data = mmap.mmap(fh.fileno(), fsz, access=mmap.ACCESS_READ)
        words = xpr.finditer(data)

        freq = defaultdict(int)
        for w in words:
            freq[w[0].lower()] += 1

        with open(sys.argv[2], "w") as out:
            for k, v in sorted(freq.items(), key=lambda u: (-u[1], u[0])):
                out.write('%d %s\n' % (v, k.decode(encoding='UTF-8')))

if __name__ == '__main__':
    main()
