/* See.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SEE__
#define SUNFISH_SEE__

#include "core/board/Board.h"
#include "../eval/Material.h"

namespace sunfish {

class See {
public:

  struct Attacker {
    Value value;
  };

private:

  // 8(近接) + 4(香) + 2(角/馬) + 2(飛/竜)
  Attacker b_[16];
  Attacker w_[16];
  int bnum_;
  int wnum_;

  template <bool black>
  void generateKnightAttacker(const Board& board, const Square& from);

  Value search(Value value, Value alpha, Value beta, Attacker* b, Attacker* w);

public:

  Value search(const Board& board, const Move& move, Value alpha, Value beta);

  void generateAttackers(const Board& board, const Move& move);

  const Attacker* getBlackList() const {
    return b_;
  }

  int getBlackNum() const {
    return bnum_;
  }

  const Attacker* getWhiteList() const {
    return w_;
  }

  int getWhiteNum() const {
    return wnum_;
  }

};

} // namespace sunfish

#endif // SUNFISH_SEE__
