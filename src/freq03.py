import re
import sys
from collections import defaultdict


if len(sys.argv) != 3:
	print("Example: " + sys.argv[0] + " in.txt out.txt");
	sys.exit(1)

words = defaultdict(int)
pattern = re.compile(r'[a-zA-Z]+')

with open(sys.argv[1], 'r', encoding='utf-8', errors='ignore') as f:
	for line in f: 
		str = pattern.findall(line)
		for w in str:
			words[w.lower()] += 1

with open(sys.argv[2], 'w', encoding='utf-8', errors='ignore') as result:
	for w, c in sorted(words.items(), key=lambda item: (-item[1], item[0])):
		result.write('%s %d\n' % (w, c))


