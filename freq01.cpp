#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

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

constexpr uint32_t H = 2166136261;

int usage(char* process_name) {
    std::cout << "Usage: " << process_name << " <filename>" << std::endl;
    return 1;
}

inline uint32_t update_hash(uint32_t h, char ch) {
    return (h * H) ^ ch;
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

struct Key {
    std::string text;
    uint32_t hash = 0;

    void update(char c) {
        text.push_back(c);
        hash = update_hash(hash, c);
    }

    bool empty() const {
        return hash == 0;
    }

    void reset() {
        text.clear();
        hash = 0;
    }

    bool operator == (const Key& k) const {
        return k.text == text;
    }
};

struct HashKey {
    uint32_t operator()(const Key& k) const {
        return k.hash;
    }
};


using Dict = std::unordered_map<Key, uint32_t, HashKey>;

struct LessIterator {
    bool operator() (const Dict::iterator& a, const Dict::iterator& b) {
        if(a->second == b->second) {
            return a->first.text < b->first.text;
        }

        return a->second > b->second;
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
    std::vector<Dict::iterator> sorted_list;
    sorted_list.reserve(500000);

    //dict.set_empty_key(Key());

    size_t cnt = 0;
    Key key;
    for(size_t i = 0; i < fsz; ++i) {
        const auto ch = letters[begin[i]];

        if(ch) {
            key.update(ch);
            continue;
        }

        // not letter and no word yet, skip
        if(key.empty()) {
            continue;
        }

        // end of word
        auto it = dict.find(key);
        if(it == dict.end()) {
            // TODO: this could be broken if rehash happens
            // switch to keeping pointers to std::pair
            it = dict.insert(it, {key, 1});
            sorted_list.emplace_back(it);
        } else {
            ++(it->second);
        }
        ++cnt;
        key.reset();
    }

    // last word
    if(not key.empty()) {
        auto it = dict.find(key);
        if(it == dict.end()) {
            it = dict.insert(it, {key, 1});
            sorted_list.emplace_back(it);
        } else {
            ++(it->second);
        }
    }

    close(fd);

    std::sort(sorted_list.begin(), sorted_list.end(), LessIterator());
    std::cout << cnt << std::endl;

    FILE* out = fopen("out.txt", "w");
    for(const auto& e : sorted_list) {
        fprintf(out, "%d %s\n", e->second, e->first.text.c_str());
    }
    fclose(out);
    return 0;
}
