/* Hand.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_HAND__
#define SUNFISH_HAND__

#include "../base/Piece.h"
#include "../def.h"
#include <cstdint>
#include <cstring>
#include <cassert>

namespace sunfish {

class Hand {
private:

  uint8_t counts_[Piece::HandNum];

public:

  CONSTEXPR Hand() : counts_{} {
  }

  void init() {
    memset(counts_, 0, sizeof(counts_));
  }

  int inc(const Piece& piece) {
    return incUnsafe(piece.kindOnly().unpromote());
  }
  int incUnsafe(const Piece& piece) {
    assert(!piece.isPromoted());
    assert(!piece.isWhite());
    assert(counts_[piece.index()] < 18);
    assert(piece.index() == Piece::Pawn || counts_[piece.unpromote().index()] < 4);
    assert(piece.index() <= Piece::Gold || counts_[piece.unpromote().index()] < 2);
    return ++counts_[piece.index()];
  }

  int dec(const Piece& piece) {
    return decUnsafe(piece.kindOnly().unpromote());
  }
  int decUnsafe(const Piece& piece) {
    assert(!piece.isPromoted());
    assert(!piece.isWhite());
    assert(counts_[piece.index()] > 0);
    return --counts_[piece.index()];
  }

  CONSTEXPR int get(const Piece& piece) const {
    return counts_[piece.kindOnly().unpromote().index()];
  }
  CONSTEXPR int getUnsafe(const Piece& piece) const {
    return counts_[piece.index()];
  }
  void set(const Piece& piece, int count) {
    counts_[piece.kindOnly().unpromote().index()] = (int8_t)count;
  }
};

} // namespace sunfish

#endif //SUNFISH_HAND__
