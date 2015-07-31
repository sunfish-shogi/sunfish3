/* Board.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BOARD__
#define SUNFISH_BOARD__

#include "Bitboard.h"
#include "Hand.h"
#include "Zobrist.h"
#include "../move/Move.h"
#include "../def.h"

namespace sunfish {

struct CompactBoard {
  enum {
    SquareMask = 0x007f,
    PieceMask  = 0x0f80,
    Black      = 0x4000,
    End        = 0x8000,
    PieceShift = 7,
    Hand       = 0x007f,
  };

  uint16_t buf[41];
};

class Board {
private:

  Bitboard bbBOccupy_;
  Bitboard bbBPawn_;
  Bitboard bbBLance_;
  Bitboard bbBKnight_;
  Bitboard bbBSilver_;
  Bitboard bbBGold_;
  Bitboard bbBBishop_;
  Bitboard bbBRook_;
  Bitboard bbBKing_;
  Bitboard bbBTokin_;
  Bitboard bbBProLance_;
  Bitboard bbBProKnight_;
  Bitboard bbBProSilver_;
  Bitboard bbBHorse_;
  Bitboard bbBDragon_;

  Bitboard bbWOccupy_;
  Bitboard bbWPawn_;
  Bitboard bbWLance_;
  Bitboard bbWKnight_;
  Bitboard bbWSilver_;
  Bitboard bbWGold_;
  Bitboard bbWBishop_;
  Bitboard bbWRook_;
  Bitboard bbWKing_;
  Bitboard bbWTokin_;
  Bitboard bbWProLance_;
  Bitboard bbWProKnight_;
  Bitboard bbWProSilver_;
  Bitboard bbWHorse_;
  Bitboard bbWDragon_;

  uint64_t boardHash_;
  uint64_t handHash_;

  Square sqBKing_;
  Square sqWKing_;

  Hand blackHand_;
  Hand whiteHand_;

  bool black_;

  Piece board_[Square::N];

  Bitboard& getBB(const Piece& piece) {
    return *(const_cast<Bitboard*>(&getBB_(piece)));
  }
  const Bitboard& getBB(const Piece& piece) const {
    return getBB_(piece);
  }
  const Bitboard& getBB_(const Piece& piece) const;
  template<bool black> bool isPin_(const Square& sq, const Bitboard& occ) const;
  template<bool black> bool isChecking_(const Square& king, const Bitboard& occ) const;
  template<bool black> bool isChecking_() const {
    Bitboard occ = bbBOccupy_ | bbWOccupy_;
    return isChecking_<black>(black ? sqBKing_ : sqWKing_, occ);
  }
  template<bool black, DirectionEx dir> bool isDirectCheck_(const Move& move) const;
  template<bool black, Direction dir> bool isDiscoveredCheck_(const Square& king, const Square& from) const;
  template<bool black> bool isCheck_(const Move& move) const;
  template<bool black> bool isPawnDropMate_() const;
  template<bool black> bool isValidMove_(const Piece& piece, const Square& to) const;
  template<bool black> bool isValidMove_(const Piece& piece, const Square& from, const Square& to) const;
  template<bool black> bool isValidMoveStrict_(const Move& move) const;
  template<bool black> bool makeMove_(Move& move);
  template<bool black> bool unmakeMove_(const Move& move);

public:

  enum class Handicap {
    Even,
    TwoPieces
  };

  Board();
  explicit Board(Handicap handicap);
  explicit Board(const CompactBoard& compactBoard);

  void init();
  void init(Handicap handicap);
  void init(const CompactBoard& compactBoard);
  void refreshHash();

  /** 冗長性の低い表現に変換します。 */
  CompactBoard getCompactBoard() const;

  /** 局面のハッシュ値を返します。 */
  uint64_t getHash() const {
    return getBoardHash() ^ getHandHash() ^ getTurnHash();
  }
  /** 盤上のみのハッシュ値を返します。 */
  uint64_t getBoardHash() const {
    return boardHash_;
  }
  /** 持ち駒のみのハッシュ値を返します。 */
  uint64_t getHandHash() const {
    return handHash_;
  }
  /** 手番のみのハッシュ値を返します。 */
  uint64_t getTurnHash() const {
    return black_ ? Zobrist::black() : 0ull;
  }
  /** 手番を除く局面のハッシュ値を返します。 */
  uint64_t getNoTurnHash() const {
    return getBoardHash() ^ getHandHash();
  }

  /** 盤面の駒を取得します。 */
  Piece getBoardPiece(const Square& sq) const {
    return board_[sq.index()];
  }
  /** 先手の持ち駒を取得します。 */
  const Hand& getBlackHand() const {
    return blackHand_;
  }
  /** 先手の持ち駒を取得します。 */
  Hand& getBlackHand() {
    return blackHand_;
  }
  /** 後手の持ち駒を取得します。 */
  const Hand& getWhiteHand() const {
    return blackHand_;
  }
  /** 後手の持ち駒を取得します。 */
  Hand& getWhiteHand() {
    return blackHand_;
  }
  /** 先手の持ち駒を取得します。 */
  int getBlackHand(const Piece& piece) const {
    return blackHand_.get(piece);
  }
  /** 先手の持ち駒を取得します。 */
  int getBlackHandUnsafe(const Piece& piece) const {
    return blackHand_.getUnsafe(piece);
  }
  /** 後手の持ち駒を取得します。 */
  int getWhiteHand(const Piece& piece) const {
    return whiteHand_.get(piece);
  }
  /** 後手の持ち駒を取得します。 */
  int getWhiteHandUnsafe(const Piece& piece) const {
    return whiteHand_.getUnsafe(piece);
  }
  /** 先手番の場合に true を返します。 */
  bool isBlack() const {
    return black_;
  }
  /** 後手番の場合に true を返します。 */
  bool isWhite() const {
    return !black_;
  }
  /** 先手の玉の位置を取得します。 */
  const Square& getBKingSquare() const {
    return sqBKing_;
  }
  /** 後手の玉の位置を取得します。 */
  const Square& getWKingSquare() const {
    return sqWKing_;
  }

  /** 盤面の駒をセットします。 */
  void setBoardPiece(const Square& sq, const Piece& piece);
  /** 先手の持ち駒をセットします。 */
  void setBlackHand(const Piece& piece, int count) {
    blackHand_.set(piece, count);
  }
  /** 後手の持ち駒をセットします。 */
  void setWhiteHand(const Piece& piece, int count) {
    whiteHand_.set(piece, count);
  }
  /** 先手の駒を増やします。 */
  void incBlackHand(const Piece& piece) {
    blackHand_.inc(piece);
  }
  /** 先手の駒を減らします。 */
  void decBlackHand(const Piece& piece) {
    blackHand_.dec(piece);
  }
  /** 後手の駒を増やします。 */
  void incWhiteHand(const Piece& piece) {
    whiteHand_.inc(piece);
  }
  /** 後手の駒を減らします。 */
  void decWhiteHand(const Piece& piece) {
    whiteHand_.dec(piece);
  }
  /** 先手番にします。 */
  void setBlack() {
    black_ = true;
  }
  /** 後手番にします。 */
  void setWhite() {
    black_ = false;
  }

  /** 王手がかかっているか判定します。 */
  bool isChecking() const {
    if (black_) {
      return isChecking_<true>();
    } else {
      return isChecking_<false>();
    }
  }

  /** 王手か判定します。 */
  bool isCheck(const Move& move) const {
    if (black_) {
      return isCheck_<true>(move);
    } else {
      return isCheck_<false>(move);
    }
  }

  /**
   * MoveGenerator によって生成された手が合法手であるかチェックします。
   * 厳密なチェックは行いません。
   * @param move
   */
  bool isValidMove(const Move& move) const {
    const Piece& piece = move.piece();
    const Square& to = move.to();
    if (move.isHand()) {
      return black_ ? isValidMove_<true>(piece, to) : isValidMove_<false>(piece, to);
    } else {
      const Square& from = move.from();
      return black_ ? isValidMove_<true>(piece, from, to) : isValidMove_<false>(piece, from, to);
    }
  }

  /**
   * 任意の手が合法手であるか厳密なチェックをします。
   * MoveGenerator で生成した手に対しては isValidMove を使用してください。
   * @param move
   */
  bool isValidMoveStrict(const Move& move) const {
    if (black_) {
      return isValidMoveStrict_<true>(move);
    } else {
      return isValidMoveStrict_<false>(move);
    }
  }

  /**
   * 指定した手で局面を進めます。
   * 厳密な合法手チェックを行いません。
   * @param move
   */
  bool makeMove(Move& move) {
    if (black_) {
      return makeMove_<true>(move);
    } else {
      return makeMove_<false>(move);
    }
  }

  /**
   * 指定した手で局面を進めます。
   * 厳密な合法手チェックをします。
   * 低速な処理のため探索中で使用すべきではありません。
   * @param move
   */
  bool makeMoveStrict(Move& move) {
    if (black_) {
      return isValidMoveStrict_<true>(move) && makeMove_<true>(move);
    } else {
      return isValidMoveStrict_<false>(move) && makeMove_<false>(move);
    }
  }

  /**
   * 局面を1手戻します。
   */
  bool unmakeMove(const Move& move) {
    if (black_) {
      return unmakeMove_<false>(move);
    } else {
      return unmakeMove_<true>(move);
    }
  }

  /**
   * 指定した手で局面を進めます。
   * move に unmakeMove のための情報を書き込みません。
   * @param move
   */
  bool makeMoveIrr(const Move& move) {
    Move mtemp = move;
    return makeMove(mtemp);
  }

  /**
   * パスをして相手に手番を渡します。
   * 王手放置のチェックは行いません。
   */
  void makeNullMove();

  /**
   * パスした手を元に戻します。
   */
  void unmakeNullMove();

  const Bitboard& getBOccupy() const { return bbBOccupy_; }
  const Bitboard& getBPawn() const { return bbBPawn_; }
  const Bitboard& getBLance() const { return bbBLance_; }
  const Bitboard& getBKnight() const { return bbBKnight_; }
  const Bitboard& getBSilver() const { return bbBSilver_; }
  const Bitboard& getBGold() const { return bbBGold_; }
  const Bitboard& getBBishop() const { return bbBBishop_; }
  const Bitboard& getBRook() const { return bbBRook_; }
  const Bitboard& getBKing() const { return bbBKing_; }
  const Bitboard& getBTokin() const { return bbBTokin_; }
  const Bitboard& getBProLance() const { return bbBProLance_; }
  const Bitboard& getBProKnight() const { return bbBProKnight_; }
  const Bitboard& getBProSilver() const { return bbBProSilver_; }
  const Bitboard& getBHorse() const { return bbBHorse_; }
  const Bitboard& getBDragon() const { return bbBDragon_; }
  const Bitboard& getWOccupy() const { return bbWOccupy_; }
  const Bitboard& getWPawn() const { return bbWPawn_; }
  const Bitboard& getWLance() const { return bbWLance_; }
  const Bitboard& getWKnight() const { return bbWKnight_; }
  const Bitboard& getWSilver() const { return bbWSilver_; }
  const Bitboard& getWGold() const { return bbWGold_; }
  const Bitboard& getWBishop() const { return bbWBishop_; }
  const Bitboard& getWRook() const { return bbWRook_; }
  const Bitboard& getWKing() const { return bbWKing_; }
  const Bitboard& getWTokin() const { return bbWTokin_; }
  const Bitboard& getWProLance() const { return bbWProLance_; }
  const Bitboard& getWProKnight() const { return bbWProKnight_; }
  const Bitboard& getWProSilver() const { return bbWProSilver_; }
  const Bitboard& getWHorse() const { return bbWHorse_; }
  const Bitboard& getWDragon() const { return bbWDragon_; }

  /**
   * データが壊れていないか検査します。
   */
  bool validate() const;

  /**
   * ダンプを出力します。
   */
  std::string dump() const;

  /**
   * 盤面を表す文字列を生成します。
   */
  std::string toString(bool showNumbers = true) const;

  /**
   * 盤面を表すCSA形式の文字列を生成します。
   */
  std::string toStringCsa() const;

};

} // namespace sunfish

#endif //SUNFISH_BOARD__
