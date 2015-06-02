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
    bool used;
    Attacker* dependOn;
  };

  using AttackerRef = Attacker*;

private:

  // 8(近接) + 4(香) + 2(角/馬) + 2(飛/竜)
  Attacker b_[16];
  Attacker w_[16];
  AttackerRef bref_[32];
  AttackerRef wref_[32];
  int bnum_;
  int wnum_;

  template <bool shallow, Direction dir, bool isFirst>
  void generateAttacker(const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn, bool shortOnly);

  template <bool shallow, Direction dir>
  void generateAttackerR(const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn) {
    HSideType sideTypeH = to.sideTypeH();
    VSideType sideTypeV = to.sideTypeV();
    if ((sideTypeH == HSideType::Top && (dir == Direction::Up || dir == Direction::LeftUp || dir == Direction::RightUp)) ||
        (sideTypeH == HSideType::Bottom && (dir == Direction::Down || dir == Direction::LeftDown || dir == Direction::RightDown)) ||
        (sideTypeV == VSideType::Left && (dir == Direction::Left || dir == Direction::LeftUp || dir == Direction::LeftDown)) ||
        (sideTypeV == VSideType::Right && (dir == Direction::Right || dir == Direction::RightUp || dir == Direction::RightDown))) {
      return;
    } else if ((sideTypeH == HSideType::Top2 && (dir == Direction::Up || dir == Direction::LeftUp || dir == Direction::RightUp)) ||
               (sideTypeH == HSideType::Bottom2 && (dir == Direction::Down || dir == Direction::LeftDown || dir == Direction::RightDown)) ||
               (sideTypeV == VSideType::Left2 && (dir == Direction::Left || dir == Direction::LeftUp || dir == Direction::LeftDown)) ||
               (sideTypeV == VSideType::Right2 && (dir == Direction::Right || dir == Direction::RightUp || dir == Direction::RightDown))) {
      generateAttacker<shallow, dir, false>(board, to, occ, dependOn, true); // short only
    } else {
      generateAttacker<shallow, dir, false>(board, to, occ, dependOn, false);
    }
  }

  template <bool black>
  void generateKnightAttacker(const Board& board, const Position& from);

  Value search(bool black, Value value, Value alpha, Value beta);

public:

  template <bool shallow = false>
  Value search(const Board& board, const Move& move, Value alpha, Value beta);

  template <bool shallow = false>
  void generateAttackers(const Board& board, const Move& move);

  const AttackerRef* getBlackList() const {
    return bref_;
  }

  int getBlackNum() const {
    return bnum_;
  }

  const AttackerRef* getWhiteList() const {
    return wref_;
  }

  int getWhiteNum() const {
    return wnum_;
  }

};

} // namespace sunfish

#endif // SUNFISH_SEE__
