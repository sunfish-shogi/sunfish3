/* PV.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_PV__
#define __SUNFISH_PV__

#include "core/def.h"
#include "core/move/Move.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace sunfish {

  class PV {
  public:
    static CONSTEXPR int MaxDepth = 64;

    struct PVMove {
      Move move;
      int depth;
    };

  private:
    PVMove _moves[MaxDepth];
    int _num;

  public:
    PV() {
      init();
    }

    PV(const PV& pv) {
      copy(pv);
    }

    void copy(const PV& pv) {
      _num = pv._num;
      memcpy(_moves, pv._moves, sizeof(PVMove) * _num);
    }

    void init() {
      _num = 0;
    }

    int size() const {
      return _num;
    }

    int set(const Move& move, int depth, const PV& pv) {
      _moves[0].move = move;
      _moves[0].depth = depth;
      _num = std::min(pv._num + 1, int(MaxDepth));
      memcpy(&_moves[1], pv._moves, sizeof(PVMove) * (_num - 1));
      return _num;
    }

    int set(const Move& move, int depth) {
      _moves[0].move = move;
      _moves[0].depth = depth;
      _num = 1;
      return _num;
    }

    const PVMove* getTop() const {
      if (_num > 0) {
        return &_moves[0];
      } else {
        return nullptr;
      }
    }

    PVMove& get(int depth) {
      return _moves[depth];
    }

    const PVMove& get(int depth) const {
      return _moves[depth];
    }

    std::string toString(int beginIndex = 0) const {
      std::ostringstream oss;
      for (int i = beginIndex; i < _num; i++) {
        oss << _moves[i].move.toString() << ' ';
      }
      return oss.str();
    }

    std::string toStringCsa(bool black, int beginIndex = 0) const {
      std::ostringstream oss;
      for (int i = beginIndex; i < _num; i++) {
        oss << _moves[i].move.toStringCsa((i%2)^black) << ' ';
      }
      return oss.str();
    }

  };

}

#endif // __SUNFISH_PV__
