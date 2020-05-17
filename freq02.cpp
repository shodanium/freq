#include <vector>
#include <array>
#include <iostream>
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
  clang++ -std=c++11 -ggdb -O3 -march=haswell -flto -fwhole-program-vtables -DNDEBUG -mllvm -inline-threshold=5000 -fomit-frame-pointer freq02.cpp -o freq
 */

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

struct Trie {
    std::array<Trie*, 'z' - 'a' + 1> trielets = {};
    uint32_t z = 0;

    Trie* step(char c) {
        auto idx = c - 'a';
        auto t = trielets[idx];
        if(t) {
            return t;
        }
        t = trielets[idx] = new_trie();
        return t;
    }

    static Trie* new_trie();
    static size_t total;
};

size_t Trie::total = 0;

Trie* Trie::new_trie() {
    constexpr size_t bsz = 2048;

    static Trie* block = new Trie[bsz];
    static size_t i = 0;

    if(i == bsz) {
        block = new Trie[bsz];
        i = 0;
    }

    auto item = block + i;
    ++i; ++total;
    return item;
}

struct Word {
    Word(Trie* t, std::string&& s)
        : trie(t)
        , text(s)
    {}

    Trie* trie = nullptr;
    std::string text;
};

struct LessIterator {
    bool operator() (const Word& a, const Word& b) {
        if(a.trie->z == b.trie->z) {
            return a.text < b.text;
        }

        return a.trie->z > b.trie->z;
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

    std::vector<Word> words;
    words.reserve(500000);

    size_t cnt = 0;
    Trie* root = Trie::new_trie();
    Trie* cur = root;
    const char* start = nullptr;
    size_t len = 0;

    auto end = begin + fsz;
    for(auto s = begin; s != end; ++s) {
        const auto ch = letters[*s];

        if(ch) {
            if(not start) {
                start = reinterpret_cast<const char*>(s);
            }
            cur =  cur->step(ch);
            ++len;
            continue;
        }

        // not letter and no word yet, skip
        if(cur == root) {
            continue;
        }

        // end of word
        if(cur->z == 0) {
            // first time to see the word
            words.emplace_back(cur, std::string(start, len));

            // need to lowercase again to avoid buffering on each word
            auto &text = words.back().text;
            for(size_t i = 0; i < len; ++i) {
                text[i] = letters[text[i]];
            }
        }
        cur->z++;

        // reset state machine
        cur = root;
        len = 0;
        start = nullptr;

        // count each word we encountered
        ++cnt;
    }

    // last word
    // TODO: do something about this dumb copy/paste after loop
    if(start) {
        if(cur->z == 0) {
            // first time to see the word
            words.emplace_back(cur, std::string(start, len));

            // need to lowercase again to avoid buffering on each word
            auto &text = words.back().text;
            for(size_t i = 0; i < len; ++i) {
                text[i] = letters[text[i]];
            }
        }
        cur->z++;
    }

    close(fd);
    std::cout << "Total words seen: " << cnt << std::endl;
    std::cout << "Total tries : " << Trie::total << ", size=" << sizeof(Trie) * Trie::total << std::endl;


    std::sort(words.begin(), words.end(), LessIterator());

    FILE* out = fopen("out.txt", "w");
    for(const auto& w : words) {
        fprintf(out, "%d %s\n", w.trie->z, w.text.c_str());
    }
    fclose(out);
    return 0;
}
