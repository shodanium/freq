import string
import sys
from collections import Counter

from itertools import chain


if len(sys.argv) != 3:
    print('No args')
    exit(1)


def sort_key(x):
    word, fq = x
    return -fq, word


non_alpha = bytes(ch for ch in range(256) if chr(ch) not in string.ascii_letters)


tab = bytes.maketrans(
    string.ascii_uppercase.encode() + non_alpha,
    string.ascii_lowercase.encode() + b' ' * len(non_alpha)
)


with open(sys.argv[1], 'rb') as in_file:
    # Replace any non-alphabetical symbols with spaces and split the string using space separator.
    translated = chain.from_iterable(
        line.translate(tab).split(b' ') for line in in_file
    )

    # Ignore empty strings.
    words = filter(None, translated)

    counts = sorted(Counter(words).items(), key=sort_key)

    result = b'\r\n'.join(
        b' '.join(
            (str(v).encode(), k)
        )
        for k, v in counts
    ) + b'\r\n'

with open(sys.argv[2], 'wb+') as out_file:
    out_file.write(result)
