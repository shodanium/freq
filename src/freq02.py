import string
import sys
from collections import Counter

from itertools import chain


CHUNK_SIZE = 20 * 1024 * 1024


non_alpha = bytes(ch for ch in range(256) if chr(ch) not in string.ascii_letters)

translation_table = bytes.maketrans(
    string.ascii_uppercase.encode() + non_alpha,
    string.ascii_lowercase.encode() + b' ' * len(non_alpha)
)

tail = [b'']  # variable to store tail of previous chunk of data.


def get_words_with_empty_strings(line):
    """Replaces any non-alphabetical symbols with spaces and splits the string using space separator.

    Saves the last word into tails[0] to concat it with the first word of the next chunk.
    """
    result = line.translate(translation_table).split(b' ')
    result[0] = tail[0] + result[0]
    tail[0] = result[-1]
    result[-1] = b''
    return result


def sort_key(x):
    word, fq = x
    return -fq, word


def main(in_file_name, out_file_name):

    with open(in_file_name, 'rb') as in_file:
        def read_chunk():
            return in_file.read(CHUNK_SIZE)

        words_and_spaces = chain.from_iterable(
            get_words_with_empty_strings(line) for line in iter(read_chunk, b'')
        )

        # Ignore empty strings.
        words = filter(None, chain(words_and_spaces, tail[0]))

        counts = sorted(Counter(words).items(), key=sort_key)

        rows = (
            b' '.join((str(v).encode(), k))
            for k, v in counts
        )

        result = b'\r\n'.join(rows) + b'\r\n'

    with open(out_file_name, 'wb+') as out_file:
        out_file.write(result)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('No args')
        exit(1)

    main(sys.argv[1], sys.argv[2])
