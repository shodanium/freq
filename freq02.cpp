#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "flat_hash_map.hpp"

#if defined(_WIN32) || defined(WIN32)

#include <io.h>

extern "C" {
#include "windows-mmap.h"
};

#define lseek64	_lseeki64

#pragma warning( disable : 4996 )

#else

#include <unistd.h>
#include <sys/mman.h>


#endif

/*
  Linux only due to mmap
  to build
  clang++ -std=c++11 -ggdb -O3 -march=haswell -flto -fwhole-program-vtables -DNDEBUG -mllvm -inline-threshold=5000 -fomit-frame-pointer freq.cpp -o freq
 */

static constexpr uint64_t offset_basis = 14695981039346656037LU;
static constexpr uint64_t prime = 1099511628211;

inline uint64_t update_hash(uint64_t h, char ch) {
    return (uint64_t(h) * prime) ^ ch;
}

int usage(char* process_name) {
    std::cout << "Usage: " << process_name << " <filename>" << std::endl;
    return 1;
}

inline char letterize(uint8_t c) {
    if(c >= 'a' and c <= 'z') {
        return c;
    }
    if(c >= 'A' and c <= 'Z') {
        return c - ('A' - 'a');
    }
    return 0;
}

struct HashKey {
    uint64_t operator()(const uint64_t k) const { return k; }
};

using DictValue = std::pair<uint32_t,uint32_t>;
using Dict = ska::flat_hash_map<uint64_t, DictValue, HashKey>;//works with std::unordered_map as well, and still faster
//using Dict = std::unordered_map<uint64_t, DictValue, HashKey>;

const std::vector<std::string> *strings = nullptr;

struct IndicesIterator {
    bool operator() (const DictValue& a, const DictValue& b) {
        if(a.second == b.second) {
            return (*strings)[a.first] < (*strings)[b.first];
        }

        return a.second > b.second;
    }
};

int main(int argc, char** argv) {
    if(argc != 2) {
        exit(usage(argv[0]));
    }

    int fd = open(argv[1], O_RDONLY);
    if(fd == -1) {
        std::cerr << "Can't open file" << std::endl;
        exit(1);
    }
    const size_t fsz = lseek64(fd, 0, SEEK_END);

    const uint8_t* begin = reinterpret_cast<const uint8_t*>(mmap(NULL, fsz, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0));

    char letters[256];
    for(size_t i = 0; i < 256; ++i) {
        letters[i] = letterize(i);
    }

    if(begin == nullptr) {
        std::cerr << "Unable to mmap" << std::endl;
        exit(1);
    }

    Dict dict(5000000);
    std::vector<std::string> all_strings;
    all_strings.resize(500000);

    char buf[256];size_t clen = 0;


    size_t cnt = 0;
    uint32_t count = 0;
    uint64_t key_hash = offset_basis;
    for(auto end = begin+fsz; begin != end; ++begin) {
        const auto ch = letters[*begin];

        if(ch) {
            key_hash = update_hash(key_hash, ch);
            buf[clen++] = ch;
            continue;
        }

        // not letter and no word yet, skip
        if(key_hash == offset_basis) {
            continue;
        }

        // end of word
        auto it = dict.find(key_hash);
        if(it == dict.end()) {
            dict.insert(it, {key_hash, DictValue(count, 1)});
            buf[clen] = 0;
            all_strings[count] = buf;
            ++count;
        } else {
            ++(it->second.second);
        }
        ++cnt;
        key_hash = offset_basis;
        buf[clen = 0] = 0;
    }
    std::cout << cnt << std::endl;
    std::cout << count << std::endl;

    // last word
    if(key_hash != offset_basis) {
        auto it = dict.find(key_hash);
        if(it == dict.end()) {
            dict.insert(it, {key_hash, DictValue(count, 1)});
            buf[clen] = 0;
            all_strings[count] = buf;
            ++count;
        } else {
            ++(it->second.second);
        }
    }
    close(fd);

    std::vector<DictValue> freqs;
    freqs.resize(count);
    uint32_t fi = 0;
    for (auto &d:dict)
      freqs[fi++] = d.second;

    strings = &all_strings;
    std::sort(freqs.begin(), freqs.end(), IndicesIterator());//we can instead sort indices, ofc

    FILE* out = fopen("out.txt", "w");
    for(auto i: freqs) {
        fprintf(out, "%d %s\n", i.second, all_strings[i.first].c_str());
    }
    fclose(out);
    return 0;
}