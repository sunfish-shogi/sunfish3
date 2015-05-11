/* Hand.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_HAND__
#define __SUNFISH_HAND__

#include "../base/Piece.h"
#include <cstring>
#include <cassert>

namespace sunfish {

class Hand {
private:

  int _counts[Piece::HandNum];

public:

  Hand() {
    init();
  }

  void init() {
    memset(_counts, 0, sizeof(_counts));
  }

  int inc(const Piece& piece) {
    return incUnsafe(piece.kindOnly().unpromote());
  }
  int incUnsafe(const Piece& piece) {
    assert(!piece.isPromoted());
    assert(!piece.isWhite());
    assert(_counts[piece] < 18);
    assert(piece == Piece::Pawn || _counts[piece.unpromote()] < 4);
    assert(piece <= Piece::Gold || _counts[piece.unpromote()] < 2);
    return ++_counts[piece];
  }

  int dec(const Piece& piece) {
    return decUnsafe(piece.kindOnly().unpromote());
  }
  int decUnsafe(const Piece& piece) {
    assert(!piece.isPromoted());
    assert(!piece.isWhite());
    assert(_counts[piece] > 0);
    return --_counts[piece];
  }

  int get(const Piece& piece) const {
    return _counts[piece.kindOnly().unpromote()];
  }
  int getUnsafe(const Piece& piece) const {
    return _counts[piece];
  }
  void set(const Piece& piece, int count) {
    _counts[piece.kindOnly().unpromote()] = count;
  }
};

} // namespace sunfish

#endif //__SUNFISH_HAND__
