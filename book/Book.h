/* Book.cpp
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH3__BOOK__
#define __SUNFISH3__BOOK__

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
  uint32_t _count;
  BookMoves _moves;

public:
  BookElement() : _count(0) {}
  BookElement(const BookElement&) = default;
  BookElement(BookElement&) = default;

  bool add(const Move& move);
  uint32_t getCount() const {
    return _count;
  }
  const BookMoves getMoves() const {
    return _moves;
  }
  BookResult selectRandom(Random& random) const;
  template <class Filter>
  void filter(Filter filterFunc) {
    for (auto ite = _moves.begin(); ite != _moves.end(); ) {
      if (!filterFunc(*ite)) {
        _count -= ite->count;
        ite = _moves.erase(ite);
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

  std::unordered_map<uint64_t, BookElement> _map;
  Random random;

public:

  Book() = default;
  Book(const Book&) = delete;
  Book(Book&&) = delete;
  ~Book() = default;

  bool add(uint64_t hash, const Move& move);
  void clear() { _map.clear(); }
  const BookElement* find(uint64_t hash) const;
  BookResult selectRandom(uint64_t hash);
  template <class Filter>
  void filter(Filter filterFunc) {
    for (auto ite = _map.begin(); ite != _map.end();) {
      ite->second.filter(filterFunc);
      if (ite->second.getCount() == 0) {
        ite = _map.erase(ite);
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

#endif /* defined(__SUNFISH3__BOOK__) */
