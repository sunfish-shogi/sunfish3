/* MoveGenerator.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MOVEGENERATOR__
#define SUNFISH_MOVEGENERATOR__

#include "../board/Board.h"
#include "Moves.h"
#include <cassert>

namespace sunfish {

/**
 * MoveGenerator
 * 指し手生成
 */
class MoveGenerator {
public:

  enum class GenType : int {
    Capture,
    NoCapture,
    Evasion,
  };

private:

  template <bool black, GenType genType>
  static void generateOnBoard_(const Board& board, Moves& moves, const Bitboard* costumToMask);
  template <bool black>
  static void generateDrop_(const Board& board, Moves& moves, const Bitboard& toMask);
  template <bool black>
  static void generateEvasion_(const Board& board, Moves& moves);
  template <bool black>
  static void generateEvasionShort_(const Board& board, Moves& moves, const Bitboard& attacker);
  template <bool black>
  static void generateKing_(const Board& board, Moves& moves);
  template <bool black, bool light>
  static void generateCheck_(const Board& board, Moves& moves);

public:

  MoveGenerator() {}
  MoveGenerator(const MoveGenerator&) = delete;
  MoveGenerator(MoveGenerator&&) = delete;

  /**
   * 全ての合法手を生成します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   */
  static void generate(const Board& board, Moves& moves) {
    if (!board.isChecking()) {
      generateCap(board, moves);
      generateNoCap(board, moves);
      generateDrop(board, moves);
    } else {
      generateEvasion(board, moves);
    }
  }

  /**
   * 駒を取る手と成る手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 王手放置の手を含む可能性があります。
   */
  static void generateCap(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      generateOnBoard_<true, GenType::Capture>(board, moves, nullptr);
    } else {
      generateOnBoard_<false, GenType::Capture>(board, moves, nullptr);
    }
  }

  /**
   * 駒を取らずかつ成らない移動手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 王手放置の手を含む可能性があります。
   */
  static void generateNoCap(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      generateOnBoard_<true, GenType::NoCapture>(board, moves, nullptr);
    } else {
      generateOnBoard_<false, GenType::NoCapture>(board, moves, nullptr);
    }
  }

  /**
   * 持ち駒を打つ手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   */
  static void generateDrop(const Board& board, Moves& moves) {
    Bitboard nocc = ~(board.getBOccupy() | board.getWOccupy());
    if (board.isBlack()) {
      generateDrop_<true>(board, moves, nocc);
    } else {
      generateDrop_<false>(board, moves, nocc);
    }
  }

  /**
   * 王手を防ぐ手を生成します。
   * 王手がかかっている場合のみに使用します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   */
  static void generateEvasion(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      generateEvasion_<true>(board, moves);
    } else {
      generateEvasion_<false>(board, moves);
    }
  }

  /**
   * 王手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   * TODO: 開き王手の生成
   */
  static void generateCheck(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      generateCheck_<true, false>(board, moves);
    } else {
      generateCheck_<false, false>(board, moves);
    }
  }

  /**
   * 王手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   * 詰将棋に効果のない遠くからの王手を除外します。
   * TODO: 開き王手の生成
   */
  static void generateCheckLight(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      generateCheck_<true, true>(board, moves);
    } else {
      generateCheck_<false, true>(board, moves);
    }
  }

};

} // namespace sunfish

#endif //SUNFISH_MOVEGENERATOR__
