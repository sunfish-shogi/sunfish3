/* Board.cpp
 *
 * Kubo Ryosuke
 */

#include "Board.h"
#include "../move/MoveTable.h"
#include "logger/Logger.h"
#include <sstream>

#include <iostream>

#define BB_EXE_OPE(p) { Bitboard Board::*P__ = &Board::bb ## p ## _; { BB_OPE } }

#define BB_OPE_EACH \
BB_EXE_OPE(BOccupy) \
BB_EXE_OPE(BPawn) BB_EXE_OPE(BLance) BB_EXE_OPE(BKnight) BB_EXE_OPE(BSilver) \
BB_EXE_OPE(BGold) BB_EXE_OPE(BBishop) BB_EXE_OPE(BRook) BB_EXE_OPE(BKing) \
BB_EXE_OPE(BTokin) BB_EXE_OPE(BProLance) BB_EXE_OPE(BProKnight) \
BB_EXE_OPE(BProSilver) BB_EXE_OPE(BHorse) BB_EXE_OPE(BDragon) \
BB_EXE_OPE(WOccupy) \
BB_EXE_OPE(WPawn) BB_EXE_OPE(WLance) BB_EXE_OPE(WKnight) BB_EXE_OPE(WSilver) \
BB_EXE_OPE(WGold) BB_EXE_OPE(WBishop) BB_EXE_OPE(WRook) BB_EXE_OPE(WKing) \
BB_EXE_OPE(WTokin) BB_EXE_OPE(WProLance) BB_EXE_OPE(WProKnight) \
BB_EXE_OPE(WProSilver) BB_EXE_OPE(WHorse) BB_EXE_OPE(WDragon)

namespace sunfish {

Board::Board() {
  init();
}

Board::Board(Handicap handicap) {
  init(handicap);
}

Board::Board(const CompactBoard& cheapBoard) {
  init(cheapBoard);
}

const Bitboard& Board::getBB_(const Piece& piece) const {
  switch(piece) {
  case Piece::BPawn     : return bbBPawn_;
  case Piece::BLance    : return bbBLance_;
  case Piece::BKnight   : return bbBKnight_;
  case Piece::BSilver   : return bbBSilver_;
  case Piece::BGold     : return bbBGold_;
  case Piece::BBishop   : return bbBBishop_;
  case Piece::BRook     : return bbBRook_;
  case Piece::BKing     : return bbBKing_;
  case Piece::BTokin    : return bbBTokin_;
  case Piece::BProLance : return bbBProLance_;
  case Piece::BProKnight: return bbBProKnight_;
  case Piece::BProSilver: return bbBProSilver_;
  case Piece::BHorse    : return bbBHorse_;
  case Piece::BDragon   : return bbBDragon_;
  case Piece::WPawn     : return bbWPawn_;
  case Piece::WLance    : return bbWLance_;
  case Piece::WKnight   : return bbWKnight_;
  case Piece::WSilver   : return bbWSilver_;
  case Piece::WGold     : return bbWGold_;
  case Piece::WBishop   : return bbWBishop_;
  case Piece::WRook     : return bbWRook_;
  case Piece::WKing     : return bbWKing_;
  case Piece::WTokin    : return bbWTokin_;
  case Piece::WProLance : return bbWProLance_;
  case Piece::WProKnight: return bbWProKnight_;
  case Piece::WProSilver: return bbWProSilver_;
  case Piece::WHorse    : return bbWHorse_;
  case Piece::WDragon   : return bbWDragon_;
  default:
    assert(false);
  }
  return *(Bitboard*)nullptr; // unreachable
}

void Board::init() {
  black_ = true;

  blackHand_.init();
  whiteHand_.init();

#define BB_OPE ((*this).*P__).init();
  BB_OPE_EACH;
#undef BB_OPE

  sqBKing_ = Square::Invalid;
  sqWKing_ = Square::Invalid;

  SQUARE_EACH(sq) {
    board_[sq.index()] = Piece::Empty;
  }

  refreshHash();
}

void Board::init(Handicap handicap) {
  init();

  board_[P19] = Piece::BLance;
  board_[P29] = Piece::BKnight;
  board_[P39] = Piece::BSilver;
  board_[P49] = Piece::BGold;
  board_[P59] = Piece::BKing;
  board_[P69] = Piece::BGold;
  board_[P79] = Piece::BSilver;
  board_[P89] = Piece::BKnight;
  board_[P99] = Piece::BLance;

  board_[P28] = Piece::BRook;
  board_[P88] = Piece::BBishop;

  board_[P17] = Piece::BPawn;
  board_[P27] = Piece::BPawn;
  board_[P37] = Piece::BPawn;
  board_[P47] = Piece::BPawn;
  board_[P57] = Piece::BPawn;
  board_[P67] = Piece::BPawn;
  board_[P77] = Piece::BPawn;
  board_[P87] = Piece::BPawn;
  board_[P97] = Piece::BPawn;

  board_[P11] = Piece::WLance;
  board_[P21] = Piece::WKnight;
  board_[P31] = Piece::WSilver;
  board_[P41] = Piece::WGold;
  board_[P51] = Piece::WKing;
  board_[P61] = Piece::WGold;
  board_[P71] = Piece::WSilver;
  board_[P81] = Piece::WKnight;
  board_[P91] = Piece::WLance;

  board_[P22] = Piece::WBishop;
  board_[P82] = Piece::WRook;

  board_[P13] = Piece::WPawn;
  board_[P23] = Piece::WPawn;
  board_[P33] = Piece::WPawn;
  board_[P43] = Piece::WPawn;
  board_[P53] = Piece::WPawn;
  board_[P63] = Piece::WPawn;
  board_[P73] = Piece::WPawn;
  board_[P83] = Piece::WPawn;
  board_[P93] = Piece::WPawn;

  SQUARE_EACH(sq) {
    auto piece = board_[sq.index()];
    if (!piece.isEmpty()) {
      Bitboard& bb = getBB(piece);
      Bitboard& occ = piece.isBlack() ? bbBOccupy_ : bbWOccupy_;
      bb.set(sq);
      occ.set(sq);
    }
  }

  sqBKing_ = P59;
  sqWKing_ = P51;

  if (handicap == Handicap::TwoPieces) {
    black_ = false;
    bbWBishop_.unset(Square(2, 2));
    bbWRook_.unset(Square(8, 2));
  }

  refreshHash();
}

void Board::init(const CompactBoard& cheapBoard) {
  init();

  for (int index = 0; ; index++) {
    if (cheapBoard.buf[index] & CompactBoard::End) {
      black_ = (cheapBoard.buf[index] & CompactBoard::Black) ? true : false;
      break;
    }

    uint16_t d = cheapBoard.buf[index];
    uint16_t c = (d & CompactBoard::PieceMask) >> CompactBoard::PieceShift;
    uint16_t s = d & CompactBoard::SquareMask;

    Piece piece = c;
    if (s == CompactBoard::Hand) {
      auto& hand = piece.isBlack() ? blackHand_ : whiteHand_;
      hand.inc(piece.kindOnly());

    } else {
      Square sq = s;

      board_[sq.index()] = piece;
      Bitboard& bb = getBB(piece);
      Bitboard& occ = piece.isBlack() ? bbBOccupy_ : bbWOccupy_;
      bb.set(sq);
      occ.set(sq);

      if (piece == Piece::BKing) {
        sqBKing_ = sq;
      } else if (piece == Piece::WKing) {
        sqWKing_ = sq;
      }
    }
  }
}

void Board::refreshHash() {

  boardHash_ = 0ull;
  handHash_ = 0ull;

  SQUARE_EACH(sq) {
    auto& piece = board_[sq.index()];
    if (piece.exists()) {
      boardHash_ ^= Zobrist::board(sq, piece);
    }
  }

#define HASH_HAND__(piece) { \
int num = blackHand_.getUnsafe(Piece::piece); \
for (int i = 0; i < num; i++) { \
  handHash_ ^= Zobrist::handB ## piece (i); \
} \
num = whiteHand_.getUnsafe(Piece::piece); \
for (int i = 0; i < num; i++) { \
  handHash_ ^= Zobrist::handW ## piece (i); \
} \
}

  HASH_HAND__(Pawn);
  HASH_HAND__(Lance);
  HASH_HAND__(Knight);
  HASH_HAND__(Silver);
  HASH_HAND__(Gold);
  HASH_HAND__(Bishop);
  HASH_HAND__(Rook);

}

/**
 * 冗長性の低いデータに変換します。
 */
CompactBoard Board::getCompactBoard() const {
  CompactBoard cb;

  int index = 0;

  SQUARE_EACH(sq) {
    Piece piece = board_[sq.index()];
    if (!piece.isEmpty()) {
      uint16_t c = static_cast<uint16_t>(piece.operator uint8_t()) << CompactBoard::PieceShift;
      uint16_t s = static_cast<uint16_t>(sq.operator int8_t());
      cb.buf[index++] = c | s;
    }
  }

  HAND_EACH(piece) {
    int num = blackHand_.get(piece);
    uint16_t c = piece.black() << CompactBoard::PieceShift;
    for (int n = 0; n < num; n++) {
      cb.buf[index++] = c | CompactBoard::Hand;
    }
  }

  HAND_EACH(piece) {
    int num = whiteHand_.get(piece);
    uint16_t c = piece.white() << CompactBoard::PieceShift;
    for (int n = 0; n < num; n++) {
      cb.buf[index++] = c | CompactBoard::Hand;
    }
  }

  cb.buf[index] = CompactBoard::End;
  if (black_) {
    cb.buf[index] |= CompactBoard::Black;
  }

  return cb;
}

enum class PinDir : int {
  Up, Down, Hor, RightUp, RightDown, None
};

class PinDirTable {
private:
  PinDir pinDir_[Square::N][Square::N];
  PinDirTable() {
    SQUARE_EACH(sq1) {
      SQUARE_EACH(sq2) {
        int file1 = sq1.getFile();
        int rank1 = sq1.getRank();
        int file2 = sq2.getFile();
        int rank2 = sq2.getRank();
        if (file1 == file2) {
          pinDir_[sq1.index()][sq2.index()] = rank1 < rank2 ? PinDir::Up : PinDir::Down;
        } else if (rank1 == rank2) {
          pinDir_[sq1.index()][sq2.index()] = PinDir::Hor;
        } else if (rank1 - rank2 == file1 - file2) {
          pinDir_[sq1.index()][sq2.index()] = PinDir::RightUp;
        } else if (rank1 - rank2 == file2 - file1) {
          pinDir_[sq1.index()][sq2.index()] = PinDir::RightDown;
        } else {
          pinDir_[sq1.index()][sq2.index()] = PinDir::None;
        }
      }
    }
  }
  static const PinDirTable table;
public:
  static PinDir get(const Square& sq1, const Square& sq2) {
    return table.pinDir_[sq1.index()][sq2.index()];
  }
};
const PinDirTable PinDirTable::table;

/**
 * ピンチェック
 */
template<bool black>
bool Board::isPin_(const Square& sq, const Bitboard& occ) const {
  if (black) {

    switch (PinDirTable::get(sq, sqBKing_)) {
    case PinDir::Up: {
      // 上
      Bitboard bb = MoveTables::vertical(sq, occ);
      return bb & bbBKing_ && bb & (bbWLance_ | bbWRook_ | bbWDragon_);
    }
    case PinDir::Down: {
      // 下
      Bitboard bb = MoveTables::vertical(sq, occ);
      return bb & bbBKing_ && bb & (bbWRook_ | bbWDragon_);
    }
    case PinDir::Hor: {
      // 横
      Bitboard bb = MoveTables::horizontal(sq, occ);
      return bb & bbBKing_ && bb & (bbWRook_ | bbWDragon_);
    }
    case PinDir::RightUp: {
      // 右上がり/左下がり
      Bitboard bb = MoveTables::rightUpX(sq, occ);
      return bb & bbBKing_ && bb & (bbWBishop_ | bbWHorse_);
    }
    case PinDir::RightDown: {
      // 右下がり/左上がり
      Bitboard bb = MoveTables::rightDownX(sq, occ);
      return bb & bbBKing_ && bb & (bbWBishop_ | bbWHorse_);
    }
    default:
      return false;
    }

  } else {

    switch (PinDirTable::get(sq, sqWKing_)) {
    case PinDir::Up: {
      // 上
      Bitboard bb = MoveTables::vertical(sq, occ);
      return bb & bbWKing_ && bb & (bbBRook_ | bbBDragon_);
    }
    case PinDir::Down: {
      // 下
      Bitboard bb = MoveTables::vertical(sq, occ);
      return bb & bbWKing_ && bb & (bbBLance_ | bbBRook_ | bbBDragon_);
    }
    case PinDir::Hor: {
      // 横
      Bitboard bb = MoveTables::horizontal(sq, occ);
      return bb & bbWKing_ && bb & (bbBRook_ | bbBDragon_);
    }
    case PinDir::RightUp: {
      // 右上がり/左下がり
      Bitboard bb = MoveTables::rightUpX(sq, occ);
      return bb & bbWKing_ && bb & (bbBBishop_ | bbBHorse_);
    }
    case PinDir::RightDown: {
      // 右下がり/左上がり
      Bitboard bb = MoveTables::rightDownX(sq, occ);
      return bb & bbWKing_ && bb & (bbBBishop_ | bbBHorse_);
    }
    default:
      return false;
    }

  }
}
template bool Board::isPin_<true>(const Square& sq, const Bitboard& occ) const;
template bool Board::isPin_<false>(const Square& sq, const Bitboard& occ) const;

/**
 * 盤面の駒をセットします。
 */
void Board::setBoardPiece(const Square& sq, const Piece& piece) {
  board_[sq.index()] = piece;
  if (sqBKing_ == sq) {
    sqBKing_ = Square::Invalid;
  }
  if (sqWKing_ == sq) {
    sqWKing_ = Square::Invalid;
  }
#define BB_OPE ((*this).*P__).unset(sq);
  BB_OPE_EACH;
#undef BB_OPE
  if (piece.exists()) {
    switch (piece) {
    case Piece::BPawn     : bbBPawn_.set(sq); break;
    case Piece::BLance    : bbBLance_.set(sq); break;
    case Piece::BKnight   : bbBKnight_.set(sq); break;
    case Piece::BSilver   : bbBSilver_.set(sq); break;
    case Piece::BGold     : bbBGold_.set(sq); break;
    case Piece::BBishop   : bbBBishop_.set(sq); break;
    case Piece::BRook     : bbBRook_.set(sq); break;
    case Piece::BKing     : bbBKing_.set(sq); sqBKing_ = sq; break;
    case Piece::BTokin    : bbBTokin_.set(sq); break;
    case Piece::BProLance : bbBProLance_.set(sq); break;
    case Piece::BProKnight: bbBProKnight_.set(sq); break;
    case Piece::BProSilver: bbBProSilver_.set(sq); break;
    case Piece::BHorse    : bbBHorse_.set(sq); break;
    case Piece::BDragon   : bbBDragon_.set(sq); break;
    case Piece::WPawn     : bbWPawn_.set(sq); break;
    case Piece::WLance    : bbWLance_.set(sq); break;
    case Piece::WKnight   : bbWKnight_.set(sq); break;
    case Piece::WSilver   : bbWSilver_.set(sq); break;
    case Piece::WGold     : bbWGold_.set(sq); break;
    case Piece::WBishop   : bbWBishop_.set(sq); break;
    case Piece::WRook     : bbWRook_.set(sq); break;
    case Piece::WKing     : bbWKing_.set(sq); sqWKing_ = sq; break;
    case Piece::WTokin    : bbWTokin_.set(sq); break;
    case Piece::WProLance : bbWProLance_.set(sq); break;
    case Piece::WProKnight: bbWProKnight_.set(sq); break;
    case Piece::WProSilver: bbWProSilver_.set(sq); break;
    case Piece::WHorse    : bbWHorse_.set(sq); break;
    case Piece::WDragon   : bbWDragon_.set(sq); break;
    }
    (piece.isBlack() ? bbBOccupy_ : bbWOccupy_).set(sq);
  }
}

/**
 * 王手判定
 */
template<bool black>
bool Board::isChecking_(const Square& king, const Bitboard& occ) const {
  if (black) {
    // 1マス移動
    if (MoveTables::bpawn(king) & bbWPawn_ ||
        MoveTables::bknight(king) & bbWKnight_ ||
        MoveTables::bsilver(king) & bbWSilver_ ||
        MoveTables::bgold(king) & (bbWGold_ | bbWTokin_ | bbWProLance_ | bbWProKnight_ | bbWProSilver_) ||
        MoveTables::king(king) & (bbWKing_ | bbWHorse_ | bbWDragon_)) {
      return true;
    }

    // 跳び駒の利き
    if (MoveTables::blance(king, occ) & bbWLance_ ||
        MoveTables::bishop(king, occ) & (bbWBishop_ | bbWHorse_) ||
        MoveTables::rook(king, occ) & (bbWRook_ | bbWDragon_)) {
      return true;
    }
  } else {
    // 1マス移動
    if (MoveTables::wpawn(king) & bbBPawn_ ||
        MoveTables::wknight(king) & bbBKnight_ ||
        MoveTables::wsilver(king) & bbBSilver_ ||
        MoveTables::wgold(king) & (bbBGold_ | bbBTokin_ | bbBProLance_ | bbBProKnight_ | bbBProSilver_) ||
        MoveTables::king(king) & (bbBKing_ | bbBHorse_ | bbBDragon_)) {
      return true;
    }

    // 跳び駒の利き
    if (MoveTables::wlance(king, occ) & bbBLance_ ||
        MoveTables::bishop(king, occ) & (bbBBishop_ | bbBHorse_) ||
        MoveTables::rook(king, occ) & (bbBRook_ | bbBDragon_)) {
      return true;
    }
  }
  return false;
}
template bool Board::isChecking_<true>(const Square& king, const Bitboard& occ) const;
template bool Board::isChecking_<false>(const Square& king, const Bitboard& occ) const;

/**
 * 移動する駒による王手かどうかチェック
 */
template<bool black, DirectionEx dir>
bool Board::isDirectCheck_(const Move& move) const {

#define SHORT_ATTACK_CHECK ((black ? sqWKing_ : sqBKing_) == \
  (dir == DirectionEx::Up ? move.to().up() : \
  dir == DirectionEx::Down ? move.to().down() : \
  dir == DirectionEx::Left ? move.to().left() : \
  dir == DirectionEx::Right ? move.to().right() : \
  dir == DirectionEx::LeftUp ? move.to().leftUp() : \
  dir == DirectionEx::LeftDown ? move.to().leftDown() : \
  dir == DirectionEx::RightUp ? move.to().rightUp() : \
  dir == DirectionEx::RightDown ? move.to().rightDown() : \
  dir == DirectionEx::LeftUpKnight ? move.to().leftUpKnight() : \
  dir == DirectionEx::LeftDownKnight ? move.to().leftDownKnight() : \
  dir == DirectionEx::RightUpKnight ? move.to().rightUpKnight() : move.to().rightDownKnight()))
#define LONG_ATTACK_CHECK(PieceType) (MoveTables::PieceType(move.to(), getBOccupy() | getWOccupy()).check(black ? sqWKing_ : sqBKing_))
#define LONG_ATTACK_CHECK_LANCE ((black ? MoveTables::blance(move.to(), getBOccupy() | getWOccupy()) : MoveTables::wlance(move.to(), getBOccupy() | getWOccupy())).check(black ? sqWKing_ : sqBKing_))

  auto piece = move.promote() ? move.piece().promote() : move.piece();

  switch(piece) {
  case Piece::Pawn: {
    if ((black && dir == DirectionEx::Up) || (!black && dir == DirectionEx::Down)) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Knight: {
    if ((black && dir == DirectionEx::LeftUpKnight) || (black && dir == DirectionEx::RightUpKnight) ||
        (!black && dir == DirectionEx::LeftDownKnight) || (!black && dir == DirectionEx::RightDownKnight)) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Silver: {
    if (dir == DirectionEx::LeftUp || dir == DirectionEx::LeftDown || dir == DirectionEx::RightUp || dir == DirectionEx::RightDown ||
        (black && dir == DirectionEx::Up) || (!black && dir == DirectionEx::Down)) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Gold:
  case Piece::Tokin:
  case Piece::ProLance:
  case Piece::ProKnight:
  case Piece::ProSilver: {
    if (dir == DirectionEx::Up || dir == DirectionEx::Down || dir == DirectionEx::Left || dir == DirectionEx::Right ||
        (black && dir == DirectionEx::LeftUp) || (black && dir == DirectionEx::RightUp) ||
        (!black && dir == DirectionEx::LeftDown) || (!black && dir == DirectionEx::RightDown)) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Lance: {
    if ((black && dir == DirectionEx::LongUp) || (!black && dir == DirectionEx::LongDown)) {
      return LONG_ATTACK_CHECK_LANCE;
    } else if ((black && dir == DirectionEx::Up) || (!black && dir == DirectionEx::Down)) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Bishop: {
    if (dir == DirectionEx::LongLeftUp || dir == DirectionEx::LongLeftDown || dir == DirectionEx::LongRightUp || dir == DirectionEx::LongRightDown) {
      return LONG_ATTACK_CHECK(bishop);
    } else if (dir == DirectionEx::LeftUp || dir == DirectionEx::LeftDown || dir == DirectionEx::RightUp || dir == DirectionEx::RightDown) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Rook: {
    if (dir == DirectionEx::LongUp || dir == DirectionEx::LongDown || dir == DirectionEx::LongLeft || dir == DirectionEx::LongRight) {
      return LONG_ATTACK_CHECK(rook);
    } else if (dir == DirectionEx::Up || dir == DirectionEx::Down || dir == DirectionEx::Left || dir == DirectionEx::Right) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Horse: {
    if (dir == DirectionEx::LongLeftUp || dir == DirectionEx::LongLeftDown || dir == DirectionEx::LongRightUp || dir == DirectionEx::LongRightDown) {
      return LONG_ATTACK_CHECK(bishop);
    } else if (dir == DirectionEx::Up || dir == DirectionEx::Down || dir == DirectionEx::Left || dir == DirectionEx::Right ||
               dir == DirectionEx::LeftUp || dir == DirectionEx::LeftDown || dir == DirectionEx::RightUp || dir == DirectionEx::RightDown) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  case Piece::Dragon: {
    if (dir == DirectionEx::LongUp || dir == DirectionEx::LongDown || dir == DirectionEx::LongLeft || dir == DirectionEx::LongRight) {
      return LONG_ATTACK_CHECK(rook);
    } else if (dir == DirectionEx::Up || dir == DirectionEx::Down || dir == DirectionEx::Left || dir == DirectionEx::Right ||
               dir == DirectionEx::LeftUp || dir == DirectionEx::LeftDown || dir == DirectionEx::RightUp || dir == DirectionEx::RightDown) {
      return SHORT_ATTACK_CHECK;
    }
    break;
  }
  }

#undef SHORT_ATTACK_CHECK
#undef LONG_ATTACK_CHECK
#undef LONG_ATTACK_CHECK_LANCE

  return false;

}

/**
 * 開き王手かどうかチェック
 */
template<bool black, Direction dir>
bool Board::isDiscoveredCheck_(const Square& king, const Square& from) const {
  auto occ = getBOccupy() | getWOccupy();
  occ.unset(from);

  if (black) {
    if (dir == Direction::Down) {
      if (MoveTables::wlance(king, occ) & (bbBLance_ | bbBRook_ | bbBDragon_)) {
        return true;
      }
    } else if (dir == Direction::Up || dir == Direction::Left || dir == Direction::Right) {
      if (MoveTables::rook(king, occ) & (bbBRook_ | bbBDragon_)) {
        return true;
      }
    } else {
      assert(dir == Direction::LeftUp || dir == Direction::LeftDown || dir == Direction::RightUp || dir == Direction::RightDown);
      if (MoveTables::bishop(king, occ) & (bbBBishop_ | bbBHorse_)) {
        return true;
      }
    }
  } else {
    if (dir == Direction::Up) {
      if (MoveTables::blance(king, occ) & (bbWLance_ | bbWRook_ | bbWDragon_)) {
        return true;
      }
    } else if (dir == Direction::Down || dir == Direction::Left || dir == Direction::Right) {
      if (MoveTables::rook(king, occ) & (bbWRook_ | bbWDragon_)) {
        return true;
      }
    } else {
      assert(dir == Direction::LeftUp || dir == Direction::LeftDown || dir == Direction::RightUp || dir == Direction::RightDown);
      if (MoveTables::bishop(king, occ) & (bbWBishop_ | bbWHorse_)) {
        return true;
      }
    }
  }

  return false;
}

template<bool black>
bool Board::isCheck_(const Move& move) const {

  // 1. 動かした駒による王手を調べる
  auto to = move.to();
  auto king = black ? sqWKing_ : sqBKing_;

  if (move.piece() != Piece::King) {
    switch (to.dirEx(king)) {
#define AS_DIR(dirname) case DirectionEx::dirname: if (isDirectCheck_<black, DirectionEx::dirname>(move)) { return true; } break;
      AS_DIR(Up) AS_DIR(Down) AS_DIR(Left) AS_DIR(Right)
      AS_DIR(LeftUp) AS_DIR(LeftDown) AS_DIR(RightUp) AS_DIR(RightDown)
      AS_DIR(LeftUpKnight) AS_DIR(LeftDownKnight) AS_DIR(RightUpKnight) AS_DIR(RightDownKnight)
      AS_DIR(LongUp) AS_DIR(LongDown) AS_DIR(LongLeft) AS_DIR(LongRight)
      AS_DIR(LongLeftUp) AS_DIR(LongLeftDown) AS_DIR(LongRightUp) AS_DIR(LongRightDown)
#undef AS_DIR
      default: break;
    }
  }

  // 2. 開き王手を調べる
  if (move.isHand()) {
    return false;
  }

  auto from = move.from();
  auto dirT = to.dir(king);
  auto dirF = king.dir(from);
  if (dirF == Direction::None ||
      dirF == Direction::LeftUpKnight ||
      dirF == Direction::LeftDownKnight ||
      dirF == Direction::RightUpKnight ||
      dirF == Direction::RightDownKnight ||
      dirF == dirT || dirF == getReversedDir(dirT)) {
    return false;
  }

  switch (dirF) {
#define AS_DIR(dirname) \
    case Direction::dirname: return isDiscoveredCheck_<black, Direction::dirname>(king, from);
    AS_DIR(Up) AS_DIR(Down) AS_DIR(Left) AS_DIR(Right)
    AS_DIR(LeftUp) AS_DIR(LeftDown) AS_DIR(RightUp) AS_DIR(RightDown)
#undef AS_DIR
    default: assert(false); return false;
  }

}
template bool Board::isCheck_<true>(const Move& move) const;
template bool Board::isCheck_<false>(const Move& move) const;

/**
 * 現在の局面で歩を打って王手をする手が打ち歩詰めであるかチェックします。
 */
template<bool black>
bool Board::isPawnDropMate_() const {
  auto occ = bbBOccupy_ | bbWOccupy_;
  auto king = !black ? sqBKing_ : sqWKing_;
  auto pawn = !black ? sqBKing_.up() : sqWKing_.down();

  // king
  auto bb = MoveTables::king(king);
  bb &= !black ? ~bbBOccupy_ : ~bbWOccupy_;
  BB_EACH_OPE(to, bb, {
    auto occ2 = occ;
    occ2.set(pawn);
    if (!isChecking_<!black>(to, occ2)) {
      return false;
    }
  });

  // knight
  bb = !black ? bbBKnight_ : bbWKnight_;
  bb &= !black ? MoveTables::wknight(pawn) : MoveTables::bknight(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Knight, from, pawn)) {
      return false;
    }
  );

  // silver
  bb = !black ? bbBSilver_ : bbWSilver_;
  bb &= !black ? MoveTables::wsilver(pawn) : MoveTables::bsilver(pawn);
  BB_EACH_OPE(from, bb, {
    if (isValidMove_<!black>(Piece::Silver, from, pawn)) {
      return false;
    }
  });

  // gold
  bb = !black ? bbBGold_ : bbWGold_;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Gold, from, pawn)) {
      return false;
    }
  );

  // bishop
  bb = !black ? bbBBishop_ : bbWBishop_;
  bb &= MoveTables::bishop(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Bishop, from, pawn)) {
      return false;
    }
  );

  // rook
  bb = !black ? bbBRook_ : bbWRook_;
  bb &= MoveTables::rook(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Rook, from, pawn)) {
      return false;
    }
  );

  // pawnkin
  bb = !black ? bbBTokin_ : bbWTokin_;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Tokin, from, pawn)) {
      return false;
    }
  );

  // promoted lance
  bb = !black ? bbBProLance_ : bbWProLance_;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::ProLance, from, pawn)) {
      return false;
    }
  );

  // promoted knight
  bb = !black ? bbBProKnight_ : bbWProKnight_;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::ProKnight, from, pawn)) {
      return false;
    }
  );

  // promoted silver
  bb = !black ? bbBProSilver_ : bbWProSilver_;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::ProSilver, from, pawn)) {
      return false;
    }
  );

  // horse
  bb = !black ? bbBHorse_ : bbWHorse_;
  bb &= MoveTables::horse(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Horse, from, pawn)) {
      return false;
    }
  );

  // dragon
  bb = !black ? bbBDragon_ : bbWDragon_;
  bb &= MoveTables::dragon(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (isValidMove_<!black>(Piece::Dragon, from, pawn)) {
      return false;
    }
  );

  return true;
}
template bool Board::isPawnDropMate_<true>() const;
template bool Board::isPawnDropMate_<false>() const;

/**
 * 持駒を打つ手が合法手かどうかチェックします。
 * TODO: rename
 */
template<bool black>
inline bool Board::isValidMove_(const Piece& piece, const Square& to) const {
  // 打ち歩詰め判定
  if (black && piece == Piece::Pawn && to.up() == sqWKing_) {
    if (isPawnDropMate_<true>()) {
      return false;
    }
  } else if (!black && piece == Piece::Pawn && to.down() == sqBKing_) {
    if (isPawnDropMate_<false>()) {
      return false;
    }
  }

  return true;
}
template bool Board::isValidMove_<true>(const Piece& piece, const Square& to) const;
template bool Board::isValidMove_<false>(const Piece& piece, const Square& to) const;

/**
 * 盤上の駒を移動させる手が合法手かどうかチェックします。
 * TODO: rename
 */
template<bool black>
inline bool Board::isValidMove_(const Piece& piece, const Square& from, const Square& to) const {
  if (piece == Piece::King) {

    // 王手放置判定
    auto occ = bbBOccupy_ | bbWOccupy_;
    occ = (black ? bbBKing_ : bbWKing_).andNot(occ); // 玉の元居た位置を除外
    if (isChecking_<black>(to, occ)) {
      return false;
    }

  } else { // not (piece == Piece::King)

    // pin判定
    auto king = black ? sqBKing_ : sqWKing_;
    // 玉からの方向が変化している場合のみチェック
    if (PinDirTable::get(king, from) != PinDirTable::get(king, to)) {
      auto occ = bbBOccupy_ | bbWOccupy_;
      occ.set(to); // 移動先のビットをセット
      if (isPin_<black>(from, occ)) {
        return false;
      }
    }

  }

  return true;
}
template bool Board::isValidMove_<true>(const Piece& piece, const Square& from, const Square& to) const;
template bool Board::isValidMove_<false>(const Piece& piece, const Square& from, const Square& to) const;

/**
 * 指し手が合法手であるか厳密なチェックをします。
 * 低速な処理のため探索中で使用すべきではありません。
 * @param move
 */
template<bool black>
bool Board::isValidMoveStrict_(const Move& move) const {
  auto occ = bbBOccupy_ | bbWOccupy_;
  auto piece = move.piece();
  auto to = move.to();

  if (move.isHand() || !move.promote()) {
    switch(piece) {
    case Piece::Pawn:
      if (!to.isPawnMovable<black>()) {
        return false;
      }
      break;
    case Piece::Lance:
      if (!to.isLanceMovable<black>()) {
        return false;
      }
      break;
    case Piece::Knight:
      if (!to.isKnightMovable<black>()) {
        return false;
      }
      break;
    }
  }

  if (move.isHand()) {

    auto& hand = black ? blackHand_ : whiteHand_;

    if (hand.get(piece) == 0) {
      return false;
    }

    if (occ.check(to)) {
      return false;
    }

    if (piece == Piece::Pawn) {
      for (int rank = 1; rank <= Square::RankN; rank++) {
        auto piece0 = board_[Square(move.to().getFile(), rank).index()];
        if (piece0 == (black ? Piece::BPawn : Piece::WPawn)) {
          return false;
        }
      }
    }

    if (!isValidMove_<black>(piece, to)) {
      return false;
    }

  } else {

    auto from = move.from();
    bool promote = move.promote();

    if (board_[from.index()] != (black ? piece.black() : piece.white())) {
      return false;
    }

    if ((black ? bbBOccupy_ : bbWOccupy_).check(to)) {
      return false;
    }

    if (promote && !from.isPromotable<black>() && !to.isPromotable<black>()) {
      return false;
    }

    if (promote && piece.isPromoted()) {
      return false;
    }

    if (promote && (piece == Piece::Gold || piece == Piece::King)) {
      return false;
    }

    Bitboard bb;
    switch(piece) {
    case Piece::Pawn:
      if (black ? to != from.up() : to != from.down()) {
        return false;
      }
      break;
    case Piece::Lance:
      bb = black ? MoveTables::blance(from, occ) : MoveTables::wlance(from, occ);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Knight:
      bb = black ? MoveTables::bknight(from) : MoveTables::wknight(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Silver:
      bb = black ? MoveTables::bsilver(from) : MoveTables::wsilver(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Gold:
      bb = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Bishop:
      bb = MoveTables::bishop(from, occ);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Rook:
      bb = MoveTables::rook(from, occ);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::King:
      bb = MoveTables::king(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Tokin:
      bb = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::ProLance:
      bb = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::ProKnight:
      bb = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::ProSilver:
      bb = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Horse:
      bb = MoveTables::horse(from, occ);
      if (!bb.check(to)) {
        return false;
      }
      break;
    case Piece::Dragon:
      bb = MoveTables::dragon(from, occ);
      if (!bb.check(to)) {
        return false;
      }
      break;
    default:
      return false;
    }

  }

  // 1手進めて次玉が王手になっていないか調べる
  // TODO: メモリ取らないで計算できるようになおす。
  Board temp;
  temp = *this;
  if (!temp.makeMoveIrr(move)) {
    return false;
  }
  temp.black_ = black; // 無理やり手番を変更する。
  if (temp.isChecking()) {
    return false;
  }

  return true;
}
template bool Board::isValidMoveStrict_<true>(const Move& move) const;
template bool Board::isValidMoveStrict_<false>(const Move& move) const;

/**
 * 指定した手で局面を進めます。
 * @param move
 */
template<bool black>
bool Board::makeMove_(Move& move) {
  bool promote = move.promote();
  const auto& piece = move.piece();
  const auto& to = move.to();

  if (move.isHand()) {

    if (!isValidMove_<black>(piece, to)) {
      return false;
    }

    Hand& hand = black ? blackHand_ : whiteHand_;
    assert(!piece.isPromoted());
    int num = hand.decUnsafe(piece);
    handHash_ ^= black ? Zobrist::handBlack(piece, num) : Zobrist::handWhite(piece, num);

  } else { // !move.isHand()

    const auto& from = move.from();

    if (!isValidMove_<black>(piece, from, to)) {
      return false;
    }

    if (black) {
      assert(board_[from.index()] == piece.black());
      bbBOccupy_.unset(from);
      switch (piece) {
      case Piece::Pawn     : bbBPawn_.unset(from); break;
      case Piece::Lance    : bbBLance_.unset(from); break;
      case Piece::Knight   : bbBKnight_.unset(from); break;
      case Piece::Silver   : bbBSilver_.unset(from); break;
      case Piece::Gold     : bbBGold_.unset(from); break;
      case Piece::Bishop   : bbBBishop_.unset(from); break;
      case Piece::Rook     : bbBRook_.unset(from); break;
      case Piece::King     : bbBKing_.unset(from); break;
      case Piece::Tokin    : bbBTokin_.unset(from); break;
      case Piece::ProLance : bbBProLance_.unset(from); break;
      case Piece::ProKnight: bbBProKnight_.unset(from); break;
      case Piece::ProSilver: bbBProSilver_.unset(from); break;
      case Piece::Horse    : bbBHorse_.unset(from); break;
      case Piece::Dragon   : bbBDragon_.unset(from); break;
      default:
        assert(false);
      }
    } else { // white
      assert(board_[from.index()] == piece.white());
      bbWOccupy_.unset(from);
      switch (piece) {
      case Piece::Pawn     : bbWPawn_.unset(from); break;
      case Piece::Lance    : bbWLance_.unset(from); break;
      case Piece::Knight   : bbWKnight_.unset(from); break;
      case Piece::Silver   : bbWSilver_.unset(from); break;
      case Piece::Gold     : bbWGold_.unset(from); break;
      case Piece::Bishop   : bbWBishop_.unset(from); break;
      case Piece::Rook     : bbWRook_.unset(from); break;
      case Piece::King     : bbWKing_.unset(from); break;
      case Piece::Tokin    : bbWTokin_.unset(from); break;
      case Piece::ProLance : bbWProLance_.unset(from); break;
      case Piece::ProKnight: bbWProKnight_.unset(from); break;
      case Piece::ProSilver: bbWProSilver_.unset(from); break;
      case Piece::Horse    : bbWHorse_.unset(from); break;
      case Piece::Dragon   : bbWDragon_.unset(from); break;
      default:
        assert(false);
      }
    }
    board_[from.index()] = Piece::Empty;
    boardHash_ ^= Zobrist::board(from, black ? piece : piece.white());

    // capturing
    const auto& captured = board_[to.index()];
    if (captured.exists()) {
      if (black) {
        assert(captured.isWhite());
        bbWOccupy_.unset(to);
        switch (captured) {
        case Piece::WPawn     : bbWPawn_.unset(to); move.setCaptured(Piece::Pawn); break;
        case Piece::WLance    : bbWLance_.unset(to); move.setCaptured(Piece::Lance); break;
        case Piece::WKnight   : bbWKnight_.unset(to); move.setCaptured(Piece::Knight); break;
        case Piece::WSilver   : bbWSilver_.unset(to); move.setCaptured(Piece::Silver); break;
        case Piece::WGold     : bbWGold_.unset(to); move.setCaptured(Piece::Gold); break;
        case Piece::WBishop   : bbWBishop_.unset(to); move.setCaptured(Piece::Bishop); break;
        case Piece::WRook     : bbWRook_.unset(to); move.setCaptured(Piece::Rook); break;
        case Piece::WKing     : assert(false); break;
        case Piece::WTokin    : bbWTokin_.unset(to); move.setCaptured(Piece::Tokin); break;
        case Piece::WProLance : bbWProLance_.unset(to); move.setCaptured(Piece::ProLance); break;
        case Piece::WProKnight: bbWProKnight_.unset(to); move.setCaptured(Piece::ProKnight); break;
        case Piece::WProSilver: bbWProSilver_.unset(to); move.setCaptured(Piece::ProSilver); break;
        case Piece::WHorse    : bbWHorse_.unset(to); move.setCaptured(Piece::Horse); break;
        case Piece::WDragon   : bbWDragon_.unset(to); move.setCaptured(Piece::Dragon); break;
        default:
          assert(false);
        }
      } else { // white
        assert(captured.isBlack());
        bbBOccupy_.unset(to);
        switch (captured) {
        case Piece::BPawn     : bbBPawn_.unset(to); move.setCaptured(Piece::Pawn); break;
        case Piece::BLance    : bbBLance_.unset(to); move.setCaptured(Piece::Lance); break;
        case Piece::BKnight   : bbBKnight_.unset(to); move.setCaptured(Piece::Knight); break;
        case Piece::BSilver   : bbBSilver_.unset(to); move.setCaptured(Piece::Silver); break;
        case Piece::BGold     : bbBGold_.unset(to); move.setCaptured(Piece::Gold); break;
        case Piece::BBishop   : bbBBishop_.unset(to); move.setCaptured(Piece::Bishop); break;
        case Piece::BRook     : bbBRook_.unset(to); move.setCaptured(Piece::Rook); break;
        case Piece::BKing     : assert(false); break;
        case Piece::BTokin    : bbBTokin_.unset(to); move.setCaptured(Piece::Tokin); break;
        case Piece::BProLance : bbBProLance_.unset(to); move.setCaptured(Piece::ProLance); break;
        case Piece::BProKnight: bbBProKnight_.unset(to); move.setCaptured(Piece::ProKnight); break;
        case Piece::BProSilver: bbBProSilver_.unset(to); move.setCaptured(Piece::ProSilver); break;
        case Piece::BHorse    : bbBHorse_.unset(to); move.setCaptured(Piece::Horse); break;
        case Piece::BDragon   : bbBDragon_.unset(to); move.setCaptured(Piece::Dragon); break;
        default:
          assert(false);
        }
      }
      boardHash_ ^= Zobrist::board(to, captured);

      // hand
      auto& hand = black ? blackHand_ : whiteHand_;
      Piece captured_k = captured.kindOnly().unpromote();
      int num = hand.incUnsafe(captured_k) - 1;
      handHash_ ^= black ? Zobrist::handBlack(captured_k, num) : Zobrist::handWhite(captured_k, num);
    }
  }

  // to
  if (black) {
    bbBOccupy_.set(to);
  } else {
    bbWOccupy_.set(to);
  }
  if (!promote) {
    if (black) {
      switch (piece) {
      case Piece::Pawn     : bbBPawn_.set(to); assert(to.getRank() >= 2); break;
      case Piece::Lance    : bbBLance_.set(to); assert(to.getRank() >= 2); break;
      case Piece::Knight   : bbBKnight_.set(to); assert(to.getRank() >= 3); break;
      case Piece::Silver   : bbBSilver_.set(to); break;
      case Piece::Gold     : bbBGold_.set(to); break;
      case Piece::Bishop   : bbBBishop_.set(to); break;
      case Piece::Rook     : bbBRook_.set(to); break;
      case Piece::King     : bbBKing_.set(to); sqBKing_ = to; break;
      case Piece::Tokin    : bbBTokin_.set(to); break;
      case Piece::ProLance : bbBProLance_.set(to); break;
      case Piece::ProKnight: bbBProKnight_.set(to); break;
      case Piece::ProSilver: bbBProSilver_.set(to); break;
      case Piece::Horse    : bbBHorse_.set(to); break;
      case Piece::Dragon   : bbBDragon_.set(to); break;
      default:
        assert(false);
      }
      board_[to.index()] = piece;
      boardHash_ ^= Zobrist::board(to, piece);
    } else {
      switch (piece) {
      case Piece::Pawn     : bbWPawn_.set(to); assert(to.getRank() <= 8); break;
      case Piece::Lance    : bbWLance_.set(to); assert(to.getRank() <= 8); break;
      case Piece::Knight   : bbWKnight_.set(to); assert(to.getRank() <= 7); break;
      case Piece::Silver   : bbWSilver_.set(to); break;
      case Piece::Gold     : bbWGold_.set(to); break;
      case Piece::Bishop   : bbWBishop_.set(to); break;
      case Piece::Rook     : bbWRook_.set(to); break;
      case Piece::King     : bbWKing_.set(to); sqWKing_ = to; break;
      case Piece::Tokin    : bbWTokin_.set(to); break;
      case Piece::ProLance : bbWProLance_.set(to); break;
      case Piece::ProKnight: bbWProKnight_.set(to); break;
      case Piece::ProSilver: bbWProSilver_.set(to); break;
      case Piece::Horse    : bbWHorse_.set(to); break;
      case Piece::Dragon   : bbWDragon_.set(to); break;
      default:
        assert(false);
      }
      Piece piece_w = piece.white();
      board_[to.index()] = piece_w;
      boardHash_ ^= Zobrist::board(to, piece_w);
    }
  } else { // promote
    Piece piece_p;
    if (black) { // black
      assert(!move.isHand());
      assert(to.getRank() <= 3 || move.from().getRank() <= 3);
      switch (piece) {
      case Piece::Pawn:
        bbBTokin_.set(to);
        piece_p = Piece::BTokin;
        break;
      case Piece::Lance:
        bbBProLance_.set(to);
        piece_p = Piece::BProLance;
        break;
      case Piece::Knight:
        bbBProKnight_.set(to);
        piece_p = Piece::BProKnight;
        break;
      case Piece::Silver:
        bbBProSilver_.set(to);
        piece_p = Piece::BProSilver;
        break;
      case Piece::Bishop:
        bbBHorse_.set(to);
        piece_p = Piece::BHorse;
        break;
      case Piece::Rook:
        bbBDragon_.set(to);
        piece_p = Piece::BDragon;
        break;
      default:
        assert(false);
      }
    } else { // white
      assert(!move.isHand());
      assert(to.getRank() >= 7 || move.from().getRank() >= 7);
      switch (piece) {
      case Piece::Pawn:
        bbWTokin_.set(to);
        piece_p = Piece::WTokin;
        break;
      case Piece::Lance:
        bbWProLance_.set(to);
        piece_p = Piece::WProLance;
        break;
      case Piece::Knight:
        bbWProKnight_.set(to);
        piece_p = Piece::WProKnight;
        break;
      case Piece::Silver:
        bbWProSilver_.set(to);
        piece_p = Piece::WProSilver;
        break;
      case Piece::Bishop:
        bbWHorse_.set(to);
        piece_p = Piece::WHorse;
        break;
      case Piece::Rook:
        bbWDragon_.set(to);
        piece_p = Piece::WDragon;
        break;
      default:
        assert(false);
      }
    }
    board_[to.index()] = piece_p;
    boardHash_ ^= Zobrist::board(to, piece_p);
  }

  // next turn
  black_ = !black;

  return true;
}

template bool Board::makeMove_<true>(Move&);
template bool Board::makeMove_<false>(Move&);

/**
 * 局面を1手戻します。
 */
template<bool black>
bool Board::unmakeMove_(const Move& move) {
  bool promote = move.promote();
  auto piece = move.piece();
  auto to = move.to();

  if (move.isHand()) {

    auto& hand = black ? blackHand_ : whiteHand_;
    assert(!piece.isPromoted());
    int num = hand.incUnsafe(piece) - 1;
    handHash_ ^= black ? Zobrist::handBlack(piece, num) : Zobrist::handWhite(piece, num);

    board_[to.index()] = Piece::Empty;

  } else { // !move.isHand()

    // from
    auto from = move.from();
    assert(board_[from.index()] == Piece::Empty);
    if (black) {
      assert(board_[to.index()] == (promote ? piece.black().promote() : piece.black()));
      bbBOccupy_.set(from);
      switch (piece) {
      case Piece::Pawn     : bbBPawn_.set(from); break;
      case Piece::Lance    : bbBLance_.set(from); break;
      case Piece::Knight   : bbBKnight_.set(from); break;
      case Piece::Silver   : bbBSilver_.set(from); break;
      case Piece::Gold     : bbBGold_.set(from); break;
      case Piece::Bishop   : bbBBishop_.set(from); break;
      case Piece::Rook     : bbBRook_.set(from); break;
      case Piece::King     : bbBKing_.set(from); sqBKing_ = from; break;
      case Piece::Tokin    : bbBTokin_.set(from); break;
      case Piece::ProLance : bbBProLance_.set(from); break;
      case Piece::ProKnight: bbBProKnight_.set(from); break;
      case Piece::ProSilver: bbBProSilver_.set(from); break;
      case Piece::Horse    : bbBHorse_.set(from); break;
      case Piece::Dragon   : bbBDragon_.set(from); break;
      default:
        assert(false);
      }
      board_[from.index()] = piece;
      boardHash_ ^= Zobrist::board(from, piece);
    } else {
      assert(board_[to.index()] == (promote ? piece.white().promote() : piece.white()));
      bbWOccupy_.set(from);
      switch (piece) {
      case Piece::Pawn     : bbWPawn_.set(from); break;
      case Piece::Lance    : bbWLance_.set(from); break;
      case Piece::Knight   : bbWKnight_.set(from); break;
      case Piece::Silver   : bbWSilver_.set(from); break;
      case Piece::Gold     : bbWGold_.set(from); break;
      case Piece::Bishop   : bbWBishop_.set(from); break;
      case Piece::Rook     : bbWRook_.set(from); break;
      case Piece::King     : bbWKing_.set(from); sqWKing_ = from; break;
      case Piece::Tokin    : bbWTokin_.set(from); break;
      case Piece::ProLance : bbWProLance_.set(from); break;
      case Piece::ProKnight: bbWProKnight_.set(from); break;
      case Piece::ProSilver: bbWProSilver_.set(from); break;
      case Piece::Horse    : bbWHorse_.set(from); break;
      case Piece::Dragon   : bbWDragon_.set(from); break;
      default:
        assert(false);
      }
      Piece piece_w = piece.white();
      board_[from.index()] = piece_w;
      boardHash_ ^= Zobrist::board(from, piece_w);
    }

    // capturing
    if (move.isCapturing()) {
      auto captured = move.captured();
      if (black) {
        bbWOccupy_.set(to);
        switch (captured) {
        case Piece::Pawn     : bbWPawn_.set(to); assert(to.getRank() <= 8); break;
        case Piece::Lance    : bbWLance_.set(to); assert(to.getRank() <= 8); break;
        case Piece::Knight   : bbWKnight_.set(to); assert(to.getRank() <= 7); break;
        case Piece::Silver   : bbWSilver_.set(to); break;
        case Piece::Gold     : bbWGold_.set(to); break;
        case Piece::Bishop   : bbWBishop_.set(to); break;
        case Piece::Rook     : bbWRook_.set(to); break;
        case Piece::King     : assert(false); break;
        case Piece::Tokin    : bbWTokin_.set(to); break;
        case Piece::ProLance : bbWProLance_.set(to); break;
        case Piece::ProKnight: bbWProKnight_.set(to); break;
        case Piece::ProSilver: bbWProSilver_.set(to); break;
        case Piece::Horse    : bbWHorse_.set(to); break;
        case Piece::Dragon   : bbWDragon_.set(to); break;
        default:
          assert(false);
        }
        Piece captured_w = captured.white();
        board_[to.index()] = captured_w;
        boardHash_ ^= Zobrist::board(to, captured_w);
      } else {
        bbBOccupy_.set(to);
        switch (captured) {
        case Piece::Pawn     : bbBPawn_.set(to); assert(to.getRank() >= 2); break;
        case Piece::Lance    : bbBLance_.set(to); assert(to.getRank() >= 2); break;
        case Piece::Knight   : bbBKnight_.set(to); assert(to.getRank() >= 3); break;
        case Piece::Silver   : bbBSilver_.set(to); break;
        case Piece::Gold     : bbBGold_.set(to); break;
        case Piece::Bishop   : bbBBishop_.set(to); break;
        case Piece::Rook     : bbBRook_.set(to); break;
        case Piece::King     : assert(false); break;
        case Piece::Tokin    : bbBTokin_.set(to); break;
        case Piece::ProLance : bbBProLance_.set(to); break;
        case Piece::ProKnight: bbBProKnight_.set(to); break;
        case Piece::ProSilver: bbBProSilver_.set(to); break;
        case Piece::Horse    : bbBHorse_.set(to); break;
        case Piece::Dragon   : bbBDragon_.set(to); break;
        default:
          assert(false);
        }
        board_[to.index()] = captured;
        boardHash_ ^= Zobrist::board(to, captured);
      }

      // hand
      auto& hand = black ? blackHand_ : whiteHand_;
      Piece captured_u = captured.unpromote();
      int num = hand.decUnsafe(captured_u);
      handHash_ ^= black ? Zobrist::handBlack(captured_u, num) : Zobrist::handWhite(captured_u, num);
    } else {
      board_[to.index()] = Piece::Empty;
    }

  }

  // to
  if (black) {
    bbBOccupy_.unset(to);
  } else {
    bbWOccupy_.unset(to);
  }
  if (!promote) {
    if (black) {
      switch (piece) {
      case Piece::Pawn     : bbBPawn_.unset(to); break;
      case Piece::Lance    : bbBLance_.unset(to); break;
      case Piece::Knight   : bbBKnight_.unset(to); break;
      case Piece::Silver   : bbBSilver_.unset(to); break;
      case Piece::Gold     : bbBGold_.unset(to); break;
      case Piece::Bishop   : bbBBishop_.unset(to); break;
      case Piece::Rook     : bbBRook_.unset(to); break;
      case Piece::King     : bbBKing_.unset(to); break;
      case Piece::Tokin    : bbBTokin_.unset(to); break;
      case Piece::ProLance : bbBProLance_.unset(to); break;
      case Piece::ProKnight: bbBProKnight_.unset(to); break;
      case Piece::ProSilver: bbBProSilver_.unset(to); break;
      case Piece::Horse    : bbBHorse_.unset(to); break;
      case Piece::Dragon   : bbBDragon_.unset(to); break;
      default:
        assert(false);
      }
    } else {
      switch (piece) {
      case Piece::Pawn     : bbWPawn_.unset(to); break;
      case Piece::Lance    : bbWLance_.unset(to); break;
      case Piece::Knight   : bbWKnight_.unset(to); break;
      case Piece::Silver   : bbWSilver_.unset(to); break;
      case Piece::Gold     : bbWGold_.unset(to); break;
      case Piece::Bishop   : bbWBishop_.unset(to); break;
      case Piece::Rook     : bbWRook_.unset(to); break;
      case Piece::King     : bbWKing_.unset(to); break;
      case Piece::Tokin    : bbWTokin_.unset(to); break;
      case Piece::ProLance : bbWProLance_.unset(to); break;
      case Piece::ProKnight: bbWProKnight_.unset(to); break;
      case Piece::ProSilver: bbWProSilver_.unset(to); break;
      case Piece::Horse    : bbWHorse_.unset(to); break;
      case Piece::Dragon   : bbWDragon_.unset(to); break;
      default:
        assert(false);
      }
    }
    boardHash_ ^= Zobrist::board(to, black ? piece : piece.white());
  } else {
    if (black) {
      switch (piece) {
      case Piece::Pawn: bbBTokin_.unset(to); break;
      case Piece::Lance: bbBProLance_.unset(to); break;
      case Piece::Knight: bbBProKnight_.unset(to); break;
      case Piece::Silver: bbBProSilver_.unset(to); break;
      case Piece::Bishop: bbBHorse_.unset(to); break;
      case Piece::Rook: bbBDragon_.unset(to); break;
      default:
        assert(false);
      }
    } else {
      switch (piece) {
      case Piece::Pawn: bbWTokin_.unset(to); break;
      case Piece::Lance: bbWProLance_.unset(to); break;
      case Piece::Knight: bbWProKnight_.unset(to); break;
      case Piece::Silver: bbWProSilver_.unset(to); break;
      case Piece::Bishop: bbWHorse_.unset(to); break;
      case Piece::Rook: bbWDragon_.unset(to); break;
      default:
        assert(false);
      }
    }
    boardHash_ ^= Zobrist::board(to, black ? piece.promote() : piece.promote().white());
  }

  // next turn
  black_ = black;

  return true;
}

template bool Board::unmakeMove_<true>(const Move& move);
template bool Board::unmakeMove_<false>(const Move& move);

/**
 * パスをして相手に手番を渡します。
 */
void Board::makeNullMove() {
  black_ = !black_;
}

/**
 * パスした手を元に戻します。
 */
void Board::unmakeNullMove() {
  black_ = !black_;
}

/**
 * データが壊れていないか検査します。
 */
bool Board::validate() const {
  PIECE_EACH(piece) {
    const auto& bb = getBB(piece);
    SQUARE_EACH(sq) {
      if (board_[sq.index()] == piece) {
        if (!bb.check(sq)) {
          return false;
        }

        if (piece.isBlack()) {
          if (!bbBOccupy_.check(sq)) {
            return false;
          }
          if (bbWOccupy_.check(sq)) {
            return false;
          }
        } else if (piece.isWhite()) {
          if (bbBOccupy_.check(sq)) {
            return false;
          }
          if (!bbWOccupy_.check(sq)) {
            return false;
          }
        } else {
          if (bbBOccupy_.check(sq)) {
            return false;
          }
          if (bbWOccupy_.check(sq)) {
            return false;
          }
        }
      } else {
        if (bb.check(sq)) {
          return false;
        }
      }
    }
  }
  return true;
}

/**
 * ダンプを出力します。
 */
std::string Board::dump() const {
  std::ostringstream oss;

  oss << toString(false);

  PIECE_EACH(piece) {
    const auto& bb = getBB(piece);
    oss << piece.toString() << '\t' << bb.toString() << '\n';
  }

  return oss.str();
}

/**
 * 盤面を表す文字列を生成します。
 */
std::string Board::toString(bool showNumbers) const {
  std::ostringstream oss;
  int count = 0;

  if (showNumbers) {
    oss << " 9 8 7 6 5 4 3 2 1\n";
  }

  SQUARE_EACH_RD(sq) {
    // piece
    Piece piece = getBoardPiece(sq);
    if (piece.exists()) {
      oss << piece.toString();
    } else {
      oss << "  ";
    }

    // new line
    if (++count % 9 == 0) {
      if (showNumbers) {
        oss << sq.getRank();
      }
      oss << '\n';
    }
  }

  bool is_first = true;
  oss << "black: ";
  HAND_EACH(piece) {
    int count = blackHand_.get(piece);
    if (count) {
      if (is_first) { is_first = false; } else { oss << ' '; }
      oss << piece.toString();
      if (count >= 2) {
        oss << count;
      }
    }
  }
  oss << '\n';

  is_first = true;
  oss << "white: ";
  HAND_EACH(piece) {
    int count = whiteHand_.get(piece);
    if (count) {
      if (is_first) { is_first = false; } else { oss << ' '; }
      oss << piece.toString();
      if (count >= 2) {
        oss << count;
      }
    }
  }
  oss << '\n';

  oss << "next: " << (black_ ? "black" : "white") << '\n';

  return oss.str();
}

/**
 * 盤面を表すCSA形式の文字列を生成します。
 */
std::string Board::toStringCsa() const {
  std::ostringstream oss;
  int count = 0;

  SQUARE_EACH_RD(sq) {
    if (count % 9 == 0) {
      if (count != 0) { oss << '\n'; }
      oss << 'P' << (count / 9 + 1);
    }
    count++;

    // piece
    Piece piece = getBoardPiece(sq);
    if (piece.exists()) {
      oss << piece.toStringCsa();
    } else {
      oss << " * ";
    }
  }
  oss << '\n';

  bool existsBlackHand = false;
  bool existsWhiteHand = false;
  HAND_EACH(piece) {
    existsBlackHand |= blackHand_.get(piece) != 0;
    existsWhiteHand |= whiteHand_.get(piece) != 0;
  }

  if (existsBlackHand) {
    oss << "P+";
    HAND_EACH(piece) {
      int count = blackHand_.get(piece);
      for (int i = 0; i < count; i++) {
        oss << "00" << piece.toStringCsa(true);
      }
    }
    oss << '\n';
  }

  if (existsWhiteHand) {
    oss << "P-";
    HAND_EACH(piece) {
      int count = whiteHand_.get(piece);
      for (int i = 0; i < count; i++) {
        oss << "00" << piece.toStringCsa(true);
      }
    }
    oss << '\n';
  }

  // 手番
  oss << (isBlack() ? '+' : '-') << '\n';

  return oss.str();
}

} // namespace sunfish
