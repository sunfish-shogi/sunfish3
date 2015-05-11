/* MoveGenerator.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVEGENERATOR__
#define __SUNFISH_MOVEGENERATOR__

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
  static void _generateOnBoard(const Board& board, Moves& moves, const Bitboard* costumToMask);
  template <bool black>
  static void _generateDrop(const Board& board, Moves& moves, const Bitboard& toMask);
  template <bool black>
  static void _generateEvasion(const Board& board, Moves& moves);
  template <bool black>
  static void _generateEvasionShort(const Board& board, Moves& moves, const Bitboard& attacker);
  template <bool black>
  static void _generateKing(const Board& board, Moves& moves);
  template <bool black, bool light>
  static void _generateCheck(const Board& board, Moves& moves);

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
      _generateOnBoard<true, GenType::Capture>(board, moves, nullptr);
    } else {
      _generateOnBoard<false, GenType::Capture>(board, moves, nullptr);
    }
  }

  /**
   * 駒を取らずかつ成らない移動手を生成します。
   * 王手がかかっていない場合のみに使用します。
   * 王手放置の手を含む可能性があります。
   */
  static void generateNoCap(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      _generateOnBoard<true, GenType::NoCapture>(board, moves, nullptr);
    } else {
      _generateOnBoard<false, GenType::NoCapture>(board, moves, nullptr);
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
      _generateDrop<true>(board, moves, nocc);
    } else {
      _generateDrop<false>(board, moves, nocc);
    }
  }

  /**
   * 王手を防ぐ手を生成します。
   * 王手がかかっている場合のみに使用します。
   * 打ち歩詰めや王手放置の手を含む可能性があります。
   */
  static void generateEvasion(const Board& board, Moves& moves) {
    if (board.isBlack()) {
      _generateEvasion<true>(board, moves);
    } else {
      _generateEvasion<false>(board, moves);
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
      _generateCheck<true, false>(board, moves);
    } else {
      _generateCheck<false, false>(board, moves);
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
      _generateCheck<true, true>(board, moves);
    } else {
      _generateCheck<false, true>(board, moves);
    }
  }

};

} // namespace sunfish

#endif //__SUNFISH_MOVEGENERATOR__
