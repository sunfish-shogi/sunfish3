/* History.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_HISTORY__
#define SUNFISH_HISTORY__

#include "core/base/Piece.h"
#include "core/base/Square.h"
#include "core/move/Move.h"
#include "logger/Logger.h"
#include <cstring>

namespace sunfish {

class History {
private:
  static CONSTEXPR int Board = Square::N;
  static CONSTEXPR int Hand = Piece::KindNum;
  static CONSTEXPR int From = Board + Hand;
  static CONSTEXPR int To = Board;
  static CONSTEXPR int Size = From * To;

  uint64_t hist_[Size];

  static int from(const Move& move) {
    if (move.isHand()) {
      int result = Board + (uint8_t)move.piece() - Piece::KindBegin;
      assert(result >= Board);
      assert(result < From);
      return result;
    } else {
      int result = move.from();
      assert(result >= 0);
      assert(result < Board);
      return result;
    }
  }

  static int to(const Move& move) {
    int result = move.to();
    assert(result >= 0);
    assert(result < To);
    return result;
  }

public:
  static const uint32_t Scale = 0x2000;
  static const uint64_t Max = 0x0008000000000000ull;

  void init() {
    memset((void*)hist_, 0, sizeof(uint64_t) * From * To);
  }

  void reduce() {
    for (int i = 0; i < Size; i++) {
      hist_[i] = (hist_[i] >> 8) & ~0xff000000ull;
    }
  }

  void add(int key, int appear, int good) {
    assert(appear >= 0);
    assert(good >= 0);
    assert(good <= appear);
    uint64_t h = hist_[key];
    uint64_t d = ((uint64_t)appear << 32) + good;
    if (h >= Max - d) {
      h = (h >> 1) & ~0x80000000ull;
    }
    hist_[key] = h + d;
  }

  uint64_t getData(int key) const {
    return hist_[key];
  }

  static uint32_t getRatio(uint64_t data) {
    uint32_t a = getAppearCount(data);
    uint32_t g = getGoodCount(data);
    assert(g <= a);
    uint32_t r = (g + 1) * Scale / (a + 2);
    assert(r < Scale);
    return r;
  }

  static uint32_t getAppearCount(uint64_t data) {
    return (uint32_t)(data >> 32);
  }

  static uint32_t getGoodCount(uint64_t data) {
    return (uint32_t)data;
  }

  static int getKey(const Move& move) {
    int f = from(move);
    int t = to(move);
    return f * To + t;
  }

};

} // namespace sunfish

#endif // SUNFISH_HISTORY__
