/* MateHistory.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MATE_HISTORY__
#define SUNFISH_MATE_HISTORY__

#include "core/def.h"
#include "core/move/Move.h"
#include <cstring>

namespace sunfish {

class MateHistory {
private:

  uint64_t (*hist_)[Piece::Num][Square::N];

public:
  static const uint32_t Scale = 0x2000;
  static const uint32_t Margin = 100;
  static const uint64_t Max = 0x0008000000000000ull;

  MateHistory() {
    hist_ = new uint64_t[Square::N][Piece::Num][Square::N];
  }

  ~MateHistory() {
    delete[] hist_;
  }

  void clear() {
    memset(hist_, 0, sizeof(uint64_t) * Square::N * Piece::Num * Square::N);
  }

  void update(const Square& king, const Move& move, bool mate) {
    uint64_t v = mate ? 0x100000001llu : 0x100000000llu;
    Piece piece = move.piece();
    Square to = move.to();
    uint64_t h = hist_[king.index()][piece.index()][to.index()];
    h += v;
    if (h >= Max) {
      h = (h >> 1) & ~0x80000000ull;
    }
    hist_[king.index()][piece.index()][to.index()] = h;
  }

  uint64_t getData(const Square& king, const Move& move) const {
    Piece piece = move.piece();
    Square to = move.to();
    uint64_t data = hist_[king.index()][piece.index()][to.index()];
    return data;
  }

  static uint32_t getProbed(uint64_t data) {
    uint32_t probed = data >> 32;
    return probed;
  }

  static uint32_t getMated(uint64_t data) {
    uint32_t mated = (uint32_t)data;
    return mated;
  }

};

} // namespace sunfish

#endif // SUNFISH_MATE_HISTORY__
