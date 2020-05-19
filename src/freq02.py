import string
import sys
from collections import Counter
from itertools import islice


def make_translation_table():
    ascii = bytes(range(0, 256))
    letters = string.ascii_letters.encode('ascii')
    intab = ascii
    space = b' '[0]
    outtab = bytearray(b in letters and b or space for b in ascii).lower()
    translation_table = bytearray.maketrans(intab, outtab)
    return translation_table


def bwords(stream, chunk_size, max_bword_len):
    table = make_translation_table()
    space_byte = b' '
    space = space_byte[0]
    buffer = bytearray(space for _ in range(max_bword_len + chunk_size))
    view = memoryview(buffer)
    head_view = view[:max_bword_len]
    tail_view = view[max_bword_len:]
    space_head = b''.rjust(max_bword_len)

    while True:
        n = stream.readinto(tail_view)
        if n == 0:
            break
        if n < chunk_size:
            tail_view[n:] = b''.rjust(len(tail_view) - n)

        words_and_spaces_only = buffer.translate(table)
        words = words_and_spaces_only.split()

        all_words = (bytes(w[:max_bword_len]) for w in words)

        if len(words) > 1:
            yield from islice(all_words, len(words) - 1)

        head_view[:] = space_head
        if words_and_spaces_only[-1] != 0:
            for w in all_words:
                head_view[-len(w) :] = w
        else:
            yield from all_words

    yield from bytes(head_view).split()


if __name__ == '__main__':
    if len(sys.argv) != 3:
        exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, 'rb') as f:
        counter = Counter(bwords(f, chunk_size=1024*1024, max_bword_len=256))
    with open(output_file, 'w') as f:
        for word, count in sorted(counter.items(), key=lambda kv: (-kv[1], kv[0])):
            f.write(str(count))
            f.write(' ')
            f.write(word.decode())
            f.write('\n')
