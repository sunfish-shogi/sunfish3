/* PV.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_PV__
#define SUNFISH_PV__

#include "core/def.h"
#include "core/move/Move.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace sunfish {

  class PV {
  public:
    static CONSTEXPR_CONST int MaxDepth = 64;

    struct PVMove {
      Move move;
      int depth;
    };

  private:
    PVMove moves_[MaxDepth];
    int num_;

  public:
    PV() {
      init();
    }

    PV(const PV& pv) {
      copy(pv);
    }

    void copy(const PV& pv) {
      num_ = pv.num_;
      memcpy(moves_, pv.moves_, sizeof(PVMove) * num_);
    }

    void init() {
      num_ = 0;
    }

    int size() const {
      return num_;
    }

    int set(const Move& move, int depth, const PV& pv) {
      moves_[0].move = move;
      moves_[0].depth = depth;
      num_ = std::min(pv.num_ + 1, int(MaxDepth));
      memcpy(&moves_[1], pv.moves_, sizeof(PVMove) * (num_ - 1));
      return num_;
    }

    int set(const Move& move, int depth) {
      moves_[0].move = move;
      moves_[0].depth = depth;
      num_ = 1;
      return num_;
    }

    const PVMove* getTop() const {
      if (num_ > 0) {
        return &moves_[0];
      } else {
        return nullptr;
      }
    }

    PVMove& get(int depth) {
      return moves_[depth];
    }

    const PVMove& get(int depth) const {
      return moves_[depth];
    }

    std::string toString(int beginIndex = 0) const {
      std::ostringstream oss;
      for (int i = beginIndex; i < num_; i++) {
        oss << moves_[i].move.toString() << ' ';
      }
      return oss.str();
    }

    std::string toStringCsa(bool black, int startIndex = 0) const {
      std::ostringstream oss;
      for (int i = startIndex; i < num_; i++) {
        oss << moves_[i].move.toStringCsa((i%2)^black) << ' ';
      }
      return oss.str();
    }

  };

}

#endif // SUNFISH_PV__
