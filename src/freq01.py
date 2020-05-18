import sys
from collections import defaultdict
from typing import List, Dict

CHUNK_SIZE = 1024 * 1024

def get_words(input_file: str) -> List[str]:
    words = []
    current = []
    with open(input_file, "rb") as f:
        chunk = f.read(CHUNK_SIZE)
        while chunk:
            for byte in chunk:
                if 65 <= byte <= 90 or 97 <= byte <= 122:
                    current.append(chr(byte))
                else:
                    if len(current) > 0:
                        words.append(''.join(current))
                        current = []
            chunk = f.read(CHUNK_SIZE)
        if len(current) > 0:
            words.append(''.join(current))
    return list(map(lambda w: w.lower()[:256], words))

def get_word_frequency(words: List[str]) -> Dict[int, List[str]]:
    word_count = defaultdict(int)
    for word in words:
        word_count[word] += 1
    word_frequency = defaultdict(list)
    for word, count in word_count.items():
        word_frequency[count].append(word)
    return word_frequency

def write_words(output_file: str, word_frequency: Dict[int, List[str]]) -> None:
    with open(output_file, 'w') as output:
        for count in sorted(word_frequency.keys(), reverse=True):
            for word in sorted(word_frequency[count]):
                output.write(output.write(str(count) + ' ' + word + '\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    words = get_words(input_file)
    word_frequency = get_word_frequency(words)
    write_words(output_file, word_frequency)
