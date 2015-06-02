/* Book.cpp
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH3__BOOK__
#define SUNFISH3__BOOK__

#include "core/move/Moves.h"
#include "core/util/Random.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace sunfish {

struct BookResult {
  Move move;
  uint32_t count;
  uint32_t total;
};

struct BookMove {
  Move move;
  uint32_t count;
};

using BookMoves = std::vector<BookMove>;

class BookElement {
private:
  uint32_t count_;
  BookMoves moves_;

public:
  BookElement() : count_(0) {}
  BookElement(const BookElement&) = default;
  BookElement(BookElement&) = default;

  bool add(const Move& move);
  uint32_t getCount() const {
    return count_;
  }
  const BookMoves getMoves() const {
    return moves_;
  }
  BookResult selectRandom(Random& random) const;
  template <class Filter>
  void filter(Filter filterFunc) {
    for (auto ite = moves_.begin(); ite != moves_.end(); ) {
      if (!filterFunc(*ite)) {
        count_ -= ite->count;
        ite = moves_.erase(ite);
      } else {
        ++ite;
      }
    }
  }

  void read(std::istream& is);
  void write(std::ostream& os) const;
};

class Book {
private:

  std::unordered_map<uint64_t, BookElement> map_;
  Random random;

public:

  Book() = default;
  Book(const Book&) = delete;
  Book(Book&&) = delete;
  ~Book() = default;

  bool add(uint64_t hash, const Move& move);
  void clear() { map_.clear(); }
  const BookElement* find(uint64_t hash) const;
  BookResult selectRandom(uint64_t hash);
  template <class Filter>
  void filter(Filter filterFunc) {
    for (auto ite = map_.begin(); ite != map_.end();) {
      ite->second.filter(filterFunc);
      if (ite->second.getCount() == 0) {
        ite = map_.erase(ite);
      } else {
        ++ite;
      }
    }
  }

  bool readFile();
  bool readFile(const char* filename);
  bool readFile(const std::string& filename) {
    return readFile(filename.c_str());
  }
  bool writeFile() const;
  bool writeFile(const char* filename) const;
  bool writeFile(const std::string& filename) const {
    return writeFile(filename.c_str());
  }

};

} // namespace sunfish

#endif /* defined(SUNFISH3__BOOK__) */
