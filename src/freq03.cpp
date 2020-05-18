#define _CRT_SECURE_NO_WARNINGS

#include <algorithm> // for std::sort
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include "ext/windows-mmap.h"
#define lseek64 _lseeki64
#pragma warning(disable : 4996)
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long long ullong;

struct entry {
	const char *key = NULL; // key pointer; NULL means unused; points to hash-managed pool
	uint hash = 0;			// hash code; storing is better than recomputing
	int value = 0;			// value counter; initialized with zero

	// used for sorting only; orders by freq desc, key asc
	bool operator<(const entry &b) const {
		if (value != b.value)
			return value > b.value;
		if (value)
			return strcmp(key, b.key) < 0;
		return false;
	}
};

struct keychunk {
	char *buffer;   // storage space
	char *curptr;   // current pointer
	char *maxptr;   // max current pointer, exclusive
	keychunk *next; // next chunk in the list

	explicit keychunk(int size, keychunk *anext) {
		buffer = new char[size];
		curptr = buffer;
		maxptr = buffer + size;
		next = anext;
	}

	~keychunk() {
		delete[] buffer;
		delete next;
	}
};

class strhash {
  private:
	const float LOAD_FACTOR = 0.9f;		 // even 0.99f works alright but let's stay safe
	const int INITIAL_SIZE = 512 * 1024; // MUST be a power of 2
	const int KEYS_CHUNK = 1024 * 1024;  // no power of 2 restrictions here

	entry *data;
	int capacity = INITIAL_SIZE;
	int used = 0;
	int maxused = int(LOAD_FACTOR * capacity);
	keychunk *keys;

  public:
	strhash() {
		data = new entry[capacity];
		keys = new keychunk(KEYS_CHUNK, NULL);
	}

	~strhash() {
		delete[] data;
		delete keys;
	}

	// the accessor; creates a new zero value for new keys
	int &operator[](const char *key);

	// sorts and dumps; AND LEAVES THE HASH UNUSABLE!
	void dump(FILE *out);

  private:
	int &add(int i, uint hash, const char *key);
	const char *dup4(const char *key);
	void grow();
};

uint myhash(const char *key) {
	uint h = 2166136261UL;
	while (uint c = *key++)
		h = (h ^ c) * 0x1000193;
	return h;
}

bool streq4(const char *aa, const char *bb) {
	const uint *a = (const uint *)aa;
	const uint *b = (const uint *)bb;
	while ((*a >> 24) && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

int &strhash::operator[](const char *key) {
	uint hash = myhash(key);
	int mask = capacity - 1;
	int i = hash & mask;
	while (data[i].key) {
		if (data[i].hash == hash && streq4(data[i].key, key))
			return data[i].value;
		i = (i + 1) & mask;
	}
	return add(i, hash, key);
}

int &strhash::add(int i, uint hash, const char *key) {
	assert(!data[i].key);
	data[i].key = dup4(key);
	data[i].hash = hash;
	data[i].value = 0;

	if (++used <= maxused)
		return data[i].value;

	assert(used > maxused);
	grow();
	return (*this)[key];
}

const char *strhash::dup4(const char *key) {
	int len = (int)strlen(key);
	int gap = 4 - (len & 3);
	int lg = len + gap;

	if (keys->curptr + lg > keys->maxptr)
		keys = new keychunk((lg > KEYS_CHUNK) ? lg : KEYS_CHUNK, keys);

	assert(keys->curptr + lg <= keys->maxptr);
	uint zero = 0;
	memcpy(keys->curptr, key, len);
	memcpy(keys->curptr + len, &zero, gap);
	keys->curptr += lg;
	return keys->curptr - lg;
}

void strhash::grow() {
	entry *newdata = new entry[2 * capacity];
	int mask = 2 * capacity - 1;

	for (int i = 0; i < capacity; i++)
		if (data[i].key) {
			int j = data[i].hash & mask;
			while (newdata[j].key)
				j = (j + 1) & mask;
			newdata[j] = data[i];
		}

	delete[] data;
	data = newdata;
	capacity *= 2;
	maxused = int(LOAD_FACTOR * capacity);
}

void strhash::dump(FILE *out) {
	// BOOM! and hash becomes unusable; trivial to sort a temp clone though
	std::sort(data, data + capacity);
	for (int i = 0; i < capacity && data[i].key; i++)
		fprintf(out, "%d %s\n", data[i].value, data[i].key);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("usage: freq <in.txt> <out.txt>\n");
		return 0;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("FATAL: failed to read %s", argv[1]);
		return 1;
	}
	FILE *fp2 = fopen(argv[2], "wb+");
	if (!fp2) {
		printf("FATAL: failed to write %s", argv[2]);
		return 1;
	}

	const ullong fsz = lseek64(fd, 0, SEEK_END);
	const byte *fbegin =
		(const byte *)mmap(NULL, fsz, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	static byte lc[256];
	memset(lc, 0, sizeof(lc));
	for (int i = 'a'; i <= 'z'; i++)
		lc[i] = i;
	for (int i = 'A'; i <= 'Z'; i++)
		lc[i] = i - 'A' + 'a';

	strhash freqs;

	const int WORDBUF = 256;
	byte wbuf[WORDBUF + 8];
	byte *wmax = wbuf + WORDBUF;
	byte *wcur = wbuf;

	const byte *rcur = fbegin;
	const byte *rmax = fbegin + fsz;
	while (rcur < rmax) {
		byte c = lc[*rcur++];
		if (!c) {
			if (wcur != wbuf) {
				*(uint *)wcur = 0;
				freqs[(char *)wbuf]++;
				wcur = wbuf;
			}
		} else {
			if (wcur < wmax)
				*wcur++ = c;
		}
	}

	// any last words?
	if (wcur != wbuf) {
		*(uint *)wcur = 0;
		freqs[(char *)wbuf]++;
	}

	freqs.dump(fp2);
	fclose(fp2);

	close(fd);
	return 0;
}
