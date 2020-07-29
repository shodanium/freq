/*

  clang++ -std=c++ -O3 src/freq04.cpp -o freq

*/

#include <cstdint>
#include <algorithm>
#include <string>
#include <functional>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

#include <iostream>
#include <fcntl.h> // open

#if defined(_WIN32) || defined(WIN32)
// windows
#include "ext/windows-mmap.h"
#include <io.h>
#define lseek64 _lseeki64
#pragma warning(disable : 4996)
#else
// unix
#include <sys/mman.h> // mmap
#include <unistd.h> // lseek
#endif

#if __APPLE__
#define lseek64 lseek
#endif

#if __linux__
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
#define _MAP_POPULATE_AVAILABLE
#endif
#endif

#ifdef _MAP_POPULATE_AVAILABLE
#define MMAP_FLAGS (MAP_PRIVATE | MAP_POPULATE)
#else
#define MMAP_FLAGS MAP_PRIVATE
#endif

typedef unsigned char byte;
typedef unsigned long long ullong;

class Word {
  public:
  Word(const std::string &s, int i) : text(s), occurances(i) {}

  std::string text;
  int occurances;
};

int main(int argc, char **argv) {
  if (argc != 3) {
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    std::cerr << "Can't open file" << std::endl;
    exit(1);
  }

  FILE *out = fopen(argv[2], "wb+");
  if (!out) {
    std::cerr << "failed to open output file" << std::endl;
    exit(1);
  }

  const size_t fsz = lseek(fd, 0, SEEK_END);
  if (fsz == -1) {
    std::cerr << "Failed to calculate file size" << std::endl;
    exit(1);
  }


  const uint8_t *begin = reinterpret_cast<const uint8_t *>(
    mmap(NULL, fsz, PROT_READ, MMAP_FLAGS, fd, 0));

  if (begin == nullptr || begin == MAP_FAILED) {
    std::cerr << "Unable to mmap" << std::endl;
    exit(1);
  }

  std::cout << "file mmap'ed" << std::endl;

  char letters[256];
  memset(letters, 0, sizeof(letters));
  for (size_t c = 0; c < 256; ++c) {
    if (c >= 'a' and c <= 'z') {
      letters[c] = c;
    }
    if (c >= 'A' and c <= 'Z') {
      letters[c] = c - 'A' + 'a';
    }
  }

  std::cout << "letters: " <<  std::string(letters, 256) << std::endl;

  const char *start = nullptr;
  size_t len = 0;

  /*
   * Calculating the hash and comparing two std::string may be slow.
   * We can store the hash of each std::string in the hash map to make
   * the inserts and lookups faster by setting StoreHash to true.
   */
  tsl::hopscotch_map<std::string, int, std::hash<std::string>,
                     std::equal_to<std::string>,
                     std::allocator<std::pair<std::string, int>>,
                     30, true> map;

  auto end = begin + fsz;
  auto word = std::string(start, len);

  for (auto s = begin; s != end; ++s) {
    const auto ch = letters[*s];

    if (ch) {
      if (not start) {
        start = reinterpret_cast<const char*>(s);
      }
      ++len;
      continue;
    } else if (len == 0) {
      continue;
    } else {
      word.assign(start, len);

      for (size_t i = 0; i< len; ++i) {
        word[i] = letters[word[i]];
      }

      // const std::size_t precalculated_hash = std::hash<std::string>()(word);

      if (map.find(word) != map.end()) {
        map[word]++;
      } else {
        map[word] = 1;
      }

      // words.emplace_back(std::string(start, len));

      // auto &text = words.back();
      // for (size_t i = 0; i < len; ++i) {
      //   text[i] = letters[text[i]];
      // }
    }

    len = 0;
    start = nullptr;
  }

  std::vector<Word> words;
  words.reserve(map.size());

  for(const auto& key_value : map) {
    // TODO string copied here?..
    words.emplace_back(Word(key_value.first, key_value.second));
  }


  std::sort(words.begin(), words.end(), [](const Word& a, const Word& b) -> bool {
    return a.occurances > b.occurances;
  });

  for (const auto& word : words) {
    // std::cout << word.text << " " << word.occurances << std::endl;
    fprintf(out, "%d %s\n", word.occurances, word.text.c_str());
  }
}
