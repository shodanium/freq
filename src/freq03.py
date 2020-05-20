import string
import mmap
import sys
from collections import Counter
from itertools import islice, groupby


def bwords(f, max_bword_len):
    mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    byte_by_byte = islice(iter(mm.read_byte, None), mm.size())
    is_letter = lambda b: 65 <= b <= 90 or 97 <= b <= 122
    start = 0 if is_letter(mm[0]) else 1
    gby = groupby(byte_by_byte, is_letter)
    for k, group in islice(gby, start, None, 2):
        yield bytes(islice(group, max_bword_len)).lower()


if __name__ == '__main__':
    if len(sys.argv) != 3:
        exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, 'rb') as f:
        counter = Counter(bwords(f, max_bword_len=256))
    with open(output_file, 'w') as f:
        for word, count in sorted(counter.items(), key=lambda kv: (-kv[1], kv[0])):
            f.write(str(count))
            f.write(' ')
            f.write(word.decode())
            f.write('\n')
