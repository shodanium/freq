#include "ext/flat_hash_map.hpp"
#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(WIN32)

#include <io.h>

extern "C" {
#include "ext/windows-mmap.h"
};

#define lseek64 _lseeki64

#pragma warning(disable : 4996)

#else

#include <sys/mman.h>
#include <unistd.h>

#endif

/*
  Linux only due to mmap
  to build
  clang++ -std=c++11 -ggdb -O3 -march=haswell -flto -fwhole-program-vtables -DNDEBUG -mllvm -inline-threshold=5000 -fomit-frame-pointer freq.cpp -o freq
 */

static constexpr uint64_t offset_basis = 14695981039346656037LU;
static constexpr uint64_t prime = 1099511628211;

inline uint64_t update_hash(uint64_t h, char ch) { return (uint64_t(h) ^ ch) * prime; }

int usage(char *process_name) {
	std::cout << "Usage: " << process_name << " <filename>" << std::endl;
	return 1;
}

inline char letterize(uint8_t c) {
	if (c >= 'a' and c <= 'z') {
		return c;
	}
	if (c >= 'A' and c <= 'Z') {
		return c - ('A' - 'a');
	}
	return 0;
}

struct HashKey {
	uint64_t operator()(const uint64_t k) const { return k; }
};

using DictValue = std::pair<uint32_t, uint32_t>;
using Dict = ska::flat_hash_map<uint64_t, DictValue,
								HashKey>; //works with std::unordered_map as well, and still faster
//using Dict = std::unordered_map<uint64_t, DictValue, HashKey>;

const uint8_t *all_data = nullptr, *end_data = nullptr;
const char *all_letters = nullptr;

static void lower_to(const uint8_t *data, const uint8_t *end_data, const char *letters, char *buf)
{
  for(;data != end_data;++data, ++buf){
    *buf = letters[*data];
    if (!*buf)
      return;
  }
}

static bool compare_insensitive(const uint8_t *a, const uint8_t *b)
{
  for(const uint8_t *ab = a<b ? b : a; ab != end_data;++a,++b){
    char chA = all_letters[*a], chB = all_letters[*b];
    if (chA == chB)
      continue;
    return chA < chB;
  }
  return a>b;
}

struct IndicesIterator {
	bool operator()(const DictValue &a, const DictValue &b) {
		if (a.second == b.second) {
			return compare_insensitive(all_data+a.first, all_data+b.first);
		}

		return a.second > b.second;
	}
};

int main(int argc, char **argv) {
	if (argc != 3) {
		exit(usage(argv[0]));
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		std::cerr << "Can't open file" << std::endl;
		exit(1);
	}
	const size_t fsz = lseek64(fd, 0, SEEK_END);

	const uint8_t *begin = reinterpret_cast<const uint8_t *>(
		mmap(NULL, fsz, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0));
	const uint8_t *cbegin = begin;
	char letters[256];
	for (size_t i = 0; i < 256; ++i) {
		letters[i] = letterize(i);
	}

	if (begin == nullptr) {
		std::cerr << "Unable to mmap" << std::endl;
		exit(1);
	}

	Dict dict(500000);

	size_t clen = 0;

	uint64_t key_hash = offset_basis;
	for (auto end = begin + fsz; begin != end; ++begin) {
		const auto ch = letters[*begin];

		if (ch) {
			key_hash = update_hash(key_hash, ch);
			clen++;
			continue;
		}

		// not letter and no word yet, skip
		if (key_hash == offset_basis) {
			continue;
		}

		// end of word
		auto it = dict.find(key_hash);
		if (it == dict.end()) {
			dict.insert(it, {key_hash, DictValue(uint32_t(begin-cbegin-clen), 1)});
		} else {
			++(it->second.second);
		}
		key_hash = offset_basis;
		clen=0;
	}

	// last word
	if (key_hash != offset_basis) {
		auto it = dict.find(key_hash);
		if (it == dict.end()) {
			dict.insert(it, {key_hash, DictValue(uint32_t(begin-cbegin-clen), 1)});
		} else {
			++(it->second.second);
		}
	}

	std::vector<DictValue> freqs;
	freqs.resize(dict.size());
	uint32_t fi = 0;
	for (auto &d : dict)
		freqs[fi++] = d.second;

	all_data = cbegin;
	end_data = begin;
	all_letters = letters;
	std::sort(freqs.begin(), freqs.end(), IndicesIterator()); //we can instead sort indices, ofc

	FILE *out = fopen(argv[2], "w");
	if (!out) {
		std::cerr << "Can't write file" << argv[2] << std::endl;
		exit(1);
	}
    char buf[256];
	for (auto i : freqs) {
	    lower_to(cbegin + i.first, end_data, letters, buf);
		fprintf(out, "%d %s\n", i.second, buf);
	}
	fclose(out);
	close(fd);
	return 0;
}