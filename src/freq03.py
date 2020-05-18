import re
import time
from collections import defaultdict

start_time = time.time()

words = defaultdict(int)
pattern = re.compile(r'[a-zA-Z]+')

result = open('result.txt', 'w', encoding='utf-8', errors='ignore') 
with open('pg.txt', 'r', encoding='utf-8', errors='ignore') as f:
	for line in f: 
		str = pattern.findall(line)
		for w in str:
			words[w.lower()] += 1

for w in sorted(words, key=words.get, reverse=True):
	print(w, words[w], file=result)

print("--- %s seconds ---" % (time.time() - start_time))
