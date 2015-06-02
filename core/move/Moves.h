/* Moves.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MOVES__
#define SUNFISH_MOVES__

#include "../board/Board.h"
#include "Moves.h"
#include <algorithm>

namespace sunfish {

template <int maxSize>
class TempMoves {
private:

  Move moves_[maxSize];
  int size_;

public:

  typedef Move* iterator;
  typedef const Move* const_iterator;

  TempMoves() : size_(0) {
  }

  void clear() { size_ = 0; }
  int size() const { return size_; }

  void add(const Move& move) {
    moves_[size_++] = move;
  }

  void remove(int index) {
    moves_[index] = moves_[--size_];
  }
  iterator remove(iterator ite) {
    (*ite) = moves_[--size_];
    return ite;
  }
  void removeStable(int index) {
    removeStable(begin()+index);
  }
  iterator removeStable(iterator ite) {
    for (auto itmp = ite+1; itmp != end(); itmp++) {
      *(itmp-1) = *(itmp);
    }
    size_--;
    return ite;
  }
  void removeAfter(int index) {
    size_ = index;
  }
  void removeAfter(iterator ite) {
    size_ = (int)(ite - moves_);
  }

  // random accessor
  Move& get(int index) { return moves_[index]; }
  const Move& get(int index) const { return moves_[index]; }
  Move& operator[](int index) { return moves_[index]; }
  const Move& operator[](int index) const { return moves_[index]; }

  // iterator
  iterator begin() { return moves_; }
  const_iterator begin() const { return moves_; }
  iterator end() { return moves_ + size_; }
  const_iterator end() const { return moves_ + size_; }

  iterator find(const Move& move) {
    for (auto ite = begin(); ite != end(); ite++) {
      if (ite->equals(move)) {
        return ite;
      }
    }
    return end();
  }

};

typedef TempMoves<1024> Moves;

} // namespace sunfish

#endif //SUNFISH_MOVES__
