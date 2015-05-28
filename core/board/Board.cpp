/* Board.cpp
 *
 * Kubo Ryosuke
 */

#include "Board.h"
#include "../move/MoveTable.h"
#include "logger/Logger.h"
#include <sstream>

#include <iostream>

#define BB_EXE_OPE(p) { Bitboard Board::*__P__ = &Board::_bb ## p; { BB_OPE } }

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

Board::Board(const CheepBoard& cheepBoard) {
  init(cheepBoard);
}

const Bitboard& Board::_getBB(const Piece& piece) const {
  switch(piece) {
  case Piece::BPawn     : return _bbBPawn;
  case Piece::BLance    : return _bbBLance;
  case Piece::BKnight   : return _bbBKnight;
  case Piece::BSilver   : return _bbBSilver;
  case Piece::BGold     : return _bbBGold;
  case Piece::BBishop   : return _bbBBishop;
  case Piece::BRook     : return _bbBRook;
  case Piece::BKing     : return _bbBKing;
  case Piece::BTokin    : return _bbBTokin;
  case Piece::BProLance : return _bbBProLance;
  case Piece::BProKnight: return _bbBProKnight;
  case Piece::BProSilver: return _bbBProSilver;
  case Piece::BHorse    : return _bbBHorse;
  case Piece::BDragon   : return _bbBDragon;
  case Piece::WPawn     : return _bbWPawn;
  case Piece::WLance    : return _bbWLance;
  case Piece::WKnight   : return _bbWKnight;
  case Piece::WSilver   : return _bbWSilver;
  case Piece::WGold     : return _bbWGold;
  case Piece::WBishop   : return _bbWBishop;
  case Piece::WRook     : return _bbWRook;
  case Piece::WKing     : return _bbWKing;
  case Piece::WTokin    : return _bbWTokin;
  case Piece::WProLance : return _bbWProLance;
  case Piece::WProKnight: return _bbWProKnight;
  case Piece::WProSilver: return _bbWProSilver;
  case Piece::WHorse    : return _bbWHorse;
  case Piece::WDragon   : return _bbWDragon;
  default:
    assert(false);
  }
  return *(Bitboard*)nullptr; // unreachable
}

void Board::init() {
  _black = true;

  _blackHand.init();
  _whiteHand.init();

#define BB_OPE ((*this).*__P__).init();
  BB_OPE_EACH;
#undef BB_OPE

  _posBKing = Position::Invalid;
  _posWKing = Position::Invalid;

  POSITION_EACH(pos) {
    _board[pos] = Piece::Empty;
  }

  refreshHash();
}

void Board::init(Handicap handicap) {
  init();

  _board[P19] = Piece::BLance;
  _board[P29] = Piece::BKnight;
  _board[P39] = Piece::BSilver;
  _board[P49] = Piece::BGold;
  _board[P59] = Piece::BKing;
  _board[P69] = Piece::BGold;
  _board[P79] = Piece::BSilver;
  _board[P89] = Piece::BKnight;
  _board[P99] = Piece::BLance;

  _board[P28] = Piece::BRook;
  _board[P88] = Piece::BBishop;

  _board[P17] = Piece::BPawn;
  _board[P27] = Piece::BPawn;
  _board[P37] = Piece::BPawn;
  _board[P47] = Piece::BPawn;
  _board[P57] = Piece::BPawn;
  _board[P67] = Piece::BPawn;
  _board[P77] = Piece::BPawn;
  _board[P87] = Piece::BPawn;
  _board[P97] = Piece::BPawn;

  _board[P11] = Piece::WLance;
  _board[P21] = Piece::WKnight;
  _board[P31] = Piece::WSilver;
  _board[P41] = Piece::WGold;
  _board[P51] = Piece::WKing;
  _board[P61] = Piece::WGold;
  _board[P71] = Piece::WSilver;
  _board[P81] = Piece::WKnight;
  _board[P91] = Piece::WLance;

  _board[P22] = Piece::WBishop;
  _board[P82] = Piece::WRook;

  _board[P13] = Piece::WPawn;
  _board[P23] = Piece::WPawn;
  _board[P33] = Piece::WPawn;
  _board[P43] = Piece::WPawn;
  _board[P53] = Piece::WPawn;
  _board[P63] = Piece::WPawn;
  _board[P73] = Piece::WPawn;
  _board[P83] = Piece::WPawn;
  _board[P93] = Piece::WPawn;

  POSITION_EACH(pos) {
    auto piece = _board[pos];
    if (!piece.isEmpty()) {
      Bitboard& bb = getBB(piece);
      Bitboard& occ = piece.isBlack() ? _bbBOccupy : _bbWOccupy;
      bb.set(pos);
      occ.set(pos);
    }
  }

  _posBKing = P59;
  _posWKing = P51;

  if (handicap == Handicap::TwoPieces) {
    _black = false;
    _bbWBishop.unset(Position(2, 2));
    _bbWRook.unset(Position(8, 2));
  }

  refreshHash();
}

void Board::init(const CheepBoard& cheepBoard) {
  init();

  for (int index = 0; ; index++) {
    if (cheepBoard.buf[index] & CheepBoard::End) {
      _black = (cheepBoard.buf[index] & CheepBoard::Black) ? true : false;
      break;
    }

    uint16_t d = cheepBoard.buf[index];
    uint16_t c = (d & CheepBoard::PieceMask) >> CheepBoard::PieceShift;
    uint16_t s = d & CheepBoard::PositionMask;

    Piece piece = c;
    if (s == CheepBoard::Hand) {
      auto& hand = piece.isBlack() ? _blackHand : _whiteHand;
      hand.inc(piece.kindOnly());

    } else {
      Position pos = s;

      _board[pos] = piece;
      Bitboard& bb = getBB(piece);
      Bitboard& occ = piece.isBlack() ? _bbBOccupy : _bbWOccupy;
      bb.set(pos);
      occ.set(pos);
    }
  }
}

void Board::refreshHash() {

  _boardHash = 0ull;
  _handHash = 0ull;

  POSITION_EACH(pos) {
    auto& piece = _board[pos];
    if (piece.exists()) {
      _boardHash ^= Zobrist::board(pos, piece);
    }
  }

#define __HASH_HAND__(piece) { \
int num = _blackHand.getUnsafe(Piece::piece); \
for (int i = 0; i < num; i++) { \
  _handHash ^= Zobrist::handB ## piece (i); \
} \
num = _whiteHand.getUnsafe(Piece::piece); \
for (int i = 0; i < num; i++) { \
  _handHash ^= Zobrist::handW ## piece (i); \
} \
}

  __HASH_HAND__(Pawn);
  __HASH_HAND__(Lance);
  __HASH_HAND__(Knight);
  __HASH_HAND__(Silver);
  __HASH_HAND__(Gold);
  __HASH_HAND__(Bishop);
  __HASH_HAND__(Rook);

}

/**
 * 冗長性の低いデータに変換します。
 */
CheepBoard Board::getCheepBoard() const {
  CheepBoard cb;

  int index = 0;

  POSITION_EACH(pos) {
    Piece piece = _board[pos];
    if (!piece.isEmpty()) {
      uint16_t c = static_cast<uint16_t>(piece.operator uint8_t()) << CheepBoard::PieceShift;
      uint16_t s = static_cast<uint16_t>(pos.operator int32_t());
      cb.buf[index++] = c | s;
    }
  }

  HAND_EACH(piece) {
    int num = _blackHand.get(piece);
    uint16_t c = piece.black() << CheepBoard::PieceShift;
    for (int n = 0; n < num; n++) {
      cb.buf[index++] = c | CheepBoard::Hand;
    }
  }

  HAND_EACH(piece) {
    int num = _whiteHand.get(piece);
    uint16_t c = piece.white() << CheepBoard::PieceShift;
    for (int n = 0; n < num; n++) {
      cb.buf[index++] = c | CheepBoard::Hand;
    }
  }

  cb.buf[index] = CheepBoard::End;
  if (_black) {
    cb.buf[index] |= CheepBoard::Black;
  }

  return cb;
}

namespace _PinDir {
  enum Type {
    Up, Down, Hor, RightUp, RightDown, None
  };
};
typedef _PinDir::Type PinDir;

class PinDirTable {
private:
  PinDir _pinDir[Position::N][Position::N];
  PinDirTable() {
    POSITION_EACH(pos1) {
      POSITION_EACH(pos2) {
        int file1 = pos1.getFile();
        int rank1 = pos1.getRank();
        int file2 = pos2.getFile();
        int rank2 = pos2.getRank();
        if (file1 == file2) {
          _pinDir[pos1][pos2] = rank1 < rank2 ? PinDir::Up : PinDir::Down;
        } else if (rank1 == rank2) {
          _pinDir[pos1][pos2] = PinDir::Hor;
        } else if (rank1 - rank2 == file1 - file2) {
          _pinDir[pos1][pos2] = PinDir::RightUp;
        } else if (rank1 - rank2 == file2 - file1) {
          _pinDir[pos1][pos2] = PinDir::RightDown;
        } else {
          _pinDir[pos1][pos2] = PinDir::None;
        }
      }
    }
  }
  static const PinDirTable table;
public:
  static PinDir get(const Position& pos1, const Position& pos2) {
    return table._pinDir[pos1][pos2];
  }
};
const PinDirTable PinDirTable::table;

/**
 * ピンチェック
 */
template<bool black>
bool Board::_isPin(const Position& pos, const Bitboard& occ) const {
  if (black) {

    switch (PinDirTable::get(pos, _posBKing)) {
    case PinDir::Up: {
      // 上
      Bitboard bb = MoveTables::vertical(pos, occ);
      return bb & _bbBKing && bb & (_bbWLance | _bbWRook | _bbWDragon);
    }
    case PinDir::Down: {
      // 下
      Bitboard bb = MoveTables::vertical(pos, occ);
      return bb & _bbBKing && bb & (_bbWRook | _bbWDragon);
    }
    case PinDir::Hor: {
      // 横
      Bitboard bb = MoveTables::horizontal(pos, occ);
      return bb & _bbBKing && bb & (_bbWRook | _bbWDragon);
    }
    case PinDir::RightUp: {
      // 右上がり/左下がり
      Bitboard bb = MoveTables::rightUpX(pos, occ);
      return bb & _bbBKing && bb & (_bbWBishop | _bbWHorse);
    }
    case PinDir::RightDown: {
      // 右下がり/左上がり
      Bitboard bb = MoveTables::rightDownX(pos, occ);
      return bb & _bbBKing && bb & (_bbWBishop | _bbWHorse);
    }
    default:
      return false;
    }

  } else {

    switch (PinDirTable::get(pos, _posWKing)) {
    case PinDir::Up: {
      // 上
      Bitboard bb = MoveTables::vertical(pos, occ);
      return bb & _bbWKing && bb & (_bbBRook | _bbBDragon);
    }
    case PinDir::Down: {
      // 下
      Bitboard bb = MoveTables::vertical(pos, occ);
      return bb & _bbWKing && bb & (_bbBLance | _bbBRook | _bbBDragon);
    }
    case PinDir::Hor: {
      // 横
      Bitboard bb = MoveTables::horizontal(pos, occ);
      return bb & _bbWKing && bb & (_bbBRook | _bbBDragon);
    }
    case PinDir::RightUp: {
      // 右上がり/左下がり
      Bitboard bb = MoveTables::rightUpX(pos, occ);
      return bb & _bbWKing && bb & (_bbBBishop | _bbBHorse);
    }
    case PinDir::RightDown: {
      // 右下がり/左上がり
      Bitboard bb = MoveTables::rightDownX(pos, occ);
      return bb & _bbWKing && bb & (_bbBBishop | _bbBHorse);
    }
    default:
      return false;
    }

  }
}
template bool Board::_isPin<true>(const Position& pos, const Bitboard& occ) const;
template bool Board::_isPin<false>(const Position& pos, const Bitboard& occ) const;

/**
 * 盤面の駒をセットします。
 */
void Board::setBoardPiece(const Position& pos, const Piece& piece) {
  _board[pos] = piece;
  if (_posBKing == pos) {
    _posBKing = Position::Invalid;
  }
  if (_posWKing == pos) {
    _posWKing = Position::Invalid;
  }
#define BB_OPE ((*this).*__P__).unset(pos);
  BB_OPE_EACH;
#undef BB_OPE
  if (piece.exists()) {
    switch (piece) {
    case Piece::BPawn     : _bbBPawn.set(pos); break;
    case Piece::BLance    : _bbBLance.set(pos); break;
    case Piece::BKnight   : _bbBKnight.set(pos); break;
    case Piece::BSilver   : _bbBSilver.set(pos); break;
    case Piece::BGold     : _bbBGold.set(pos); break;
    case Piece::BBishop   : _bbBBishop.set(pos); break;
    case Piece::BRook     : _bbBRook.set(pos); break;
    case Piece::BKing     : _bbBKing.set(pos); _posBKing = pos; break;
    case Piece::BTokin    : _bbBTokin.set(pos); break;
    case Piece::BProLance : _bbBProLance.set(pos); break;
    case Piece::BProKnight: _bbBProKnight.set(pos); break;
    case Piece::BProSilver: _bbBProSilver.set(pos); break;
    case Piece::BHorse    : _bbBHorse.set(pos); break;
    case Piece::BDragon   : _bbBDragon.set(pos); break;
    case Piece::WPawn     : _bbWPawn.set(pos); break;
    case Piece::WLance    : _bbWLance.set(pos); break;
    case Piece::WKnight   : _bbWKnight.set(pos); break;
    case Piece::WSilver   : _bbWSilver.set(pos); break;
    case Piece::WGold     : _bbWGold.set(pos); break;
    case Piece::WBishop   : _bbWBishop.set(pos); break;
    case Piece::WRook     : _bbWRook.set(pos); break;
    case Piece::WKing     : _bbWKing.set(pos); _posWKing = pos; break;
    case Piece::WTokin    : _bbWTokin.set(pos); break;
    case Piece::WProLance : _bbWProLance.set(pos); break;
    case Piece::WProKnight: _bbWProKnight.set(pos); break;
    case Piece::WProSilver: _bbWProSilver.set(pos); break;
    case Piece::WHorse    : _bbWHorse.set(pos); break;
    case Piece::WDragon   : _bbWDragon.set(pos); break;
    }
    (piece.isBlack() ? _bbBOccupy : _bbWOccupy).set(pos);
  }
}

/**
 * 王手判定
 */
template<bool black>
bool Board::_isChecking(const Position& king, const Bitboard& occ) const {
  if (black) {
    // 1マス移動
    if (MoveTables::bpawn(king) & _bbWPawn ||
        MoveTables::bknight(king) & _bbWKnight ||
        MoveTables::bsilver(king) & _bbWSilver ||
        MoveTables::bgold(king) & (_bbWGold | _bbWTokin | _bbWProLance | _bbWProKnight | _bbWProSilver) ||
        MoveTables::king(king) & (_bbWKing | _bbWHorse | _bbWDragon)) {
      return true;
    }

    // 跳び駒の利き
    if (MoveTables::blance(king, occ) & _bbWLance ||
        MoveTables::bishop(king, occ) & (_bbWBishop | _bbWHorse) ||
        MoveTables::rook(king, occ) & (_bbWRook | _bbWDragon)) {
      return true;
    }
  } else {
    // 1マス移動
    if (MoveTables::wpawn(king) & _bbBPawn ||
        MoveTables::wknight(king) & _bbBKnight ||
        MoveTables::wsilver(king) & _bbBSilver ||
        MoveTables::wgold(king) & (_bbBGold | _bbBTokin | _bbBProLance | _bbBProKnight | _bbBProSilver) ||
        MoveTables::king(king) & (_bbBKing | _bbBHorse | _bbBDragon)) {
      return true;
    }

    // 跳び駒の利き
    if (MoveTables::wlance(king, occ) & _bbBLance ||
        MoveTables::bishop(king, occ) & (_bbBBishop | _bbBHorse) ||
        MoveTables::rook(king, occ) & (_bbBRook | _bbBDragon)) {
      return true;
    }
  }
  return false;
}
template bool Board::_isChecking<true>(const Position& king, const Bitboard& occ) const;
template bool Board::_isChecking<false>(const Position& king, const Bitboard& occ) const;

/**
 * 移動する駒による王手かどうかチェック
 */
template<bool black, DirectionEx dir>
bool Board::_isDirectCheck(const Move& move) const {

#define SHORT_ATTACK_CHECK ((black ? _posWKing : _posBKing) == \
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
#define LONG_ATTACK_CHECK(PieceType) (MoveTables::PieceType(move.to(), getBOccupy() | getWOccupy()).check(black ? _posWKing : _posBKing))
#define LONG_ATTACK_CHECK_LANCE ((black ? MoveTables::blance(move.to(), getBOccupy() | getWOccupy()) : MoveTables::wlance(move.to(), getBOccupy() | getWOccupy())).check(black ? _posWKing : _posBKing))

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
bool Board::_isDiscoveredCheck(const Position& king, const Position& from) const {
  auto occ = getBOccupy() | getWOccupy();
  occ.unset(from);

  if (black) {
    if (dir == Direction::Down) {
      if (MoveTables::wlance(king, occ) & (_bbBLance | _bbBRook | _bbBDragon)) {
        return true;
      }
    } else if (dir == Direction::Up || dir == Direction::Left || dir == Direction::Right) {
      if (MoveTables::rook(king, occ) & (_bbBRook | _bbBDragon)) {
        return true;
      }
    } else {
      assert(dir == Direction::LeftUp || dir == Direction::LeftDown || dir == Direction::RightUp || dir == Direction::RightDown);
      if (MoveTables::bishop(king, occ) & (_bbBBishop | _bbBHorse)) {
        return true;
      }
    }
  } else {
    if (dir == Direction::Up) {
      if (MoveTables::blance(king, occ) & (_bbWLance | _bbWRook | _bbWDragon)) {
        return true;
      }
    } else if (dir == Direction::Down || dir == Direction::Left || dir == Direction::Right) {
      if (MoveTables::rook(king, occ) & (_bbWRook | _bbWDragon)) {
        return true;
      }
    } else {
      assert(dir == Direction::LeftUp || dir == Direction::LeftDown || dir == Direction::RightUp || dir == Direction::RightDown);
      if (MoveTables::bishop(king, occ) & (_bbWBishop | _bbWHorse)) {
        return true;
      }
    }
  }

  return false;
}

template<bool black>
bool Board::_isCheck(const Move& move) const {

  // 1. 動かした駒による王手を調べる
  auto to = move.to();
  auto king = black ? _posWKing : _posBKing;

  if (move.piece() != Piece::King) {
    switch (to.dirEx(king)) {
#define AS_DIR(dirname) case DirectionEx::dirname: if (_isDirectCheck<black, DirectionEx::dirname>(move)) { return true; } break;
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
    case Direction::dirname: return _isDiscoveredCheck<black, Direction::dirname>(king, from);
    AS_DIR(Up) AS_DIR(Down) AS_DIR(Left) AS_DIR(Right)
    AS_DIR(LeftUp) AS_DIR(LeftDown) AS_DIR(RightUp) AS_DIR(RightDown)
#undef AS_DIR
    default: assert(false); return false;
  }

}
template bool Board::_isCheck<true>(const Move& move) const;
template bool Board::_isCheck<false>(const Move& move) const;

/**
 * 現在の局面で歩を打って王手をする手が打ち歩詰めであるかチェックします。
 */
template<bool black>
bool Board::_isPawnDropMate() const {
  auto occ = _bbBOccupy | _bbWOccupy;
  auto king = !black ? _posBKing : _posWKing;
  auto pawn = !black ? _posBKing.up() : _posWKing.down();

  // king
  auto bb = MoveTables::king(king);
  bb &= !black ? ~_bbBOccupy : ~_bbWOccupy;
  BB_EACH_OPE(to, bb, {
    auto occ2 = occ;
    occ2.set(pawn);
    if (!_isChecking<!black>(to, occ2)) {
      return false;
    }
  });

  // knight
  bb = !black ? _bbBKnight : _bbWKnight;
  bb &= !black ? MoveTables::wknight(pawn) : MoveTables::bknight(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Knight, from, pawn)) {
      return false;
    }
  );

  // silver
  bb = !black ? _bbBSilver : _bbWSilver;
  bb &= !black ? MoveTables::wsilver(pawn) : MoveTables::bsilver(pawn);
  BB_EACH_OPE(from, bb, {
    if (_isValidMove<!black>(Piece::Silver, from, pawn)) {
      return false;
    }
  });

  // gold
  bb = !black ? _bbBGold : _bbWGold;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Gold, from, pawn)) {
      return false;
    }
  );

  // bishop
  bb = !black ? _bbBBishop : _bbWBishop;
  bb &= MoveTables::bishop(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Bishop, from, pawn)) {
      return false;
    }
  );

  // rook
  bb = !black ? _bbBRook : _bbWRook;
  bb &= MoveTables::rook(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Rook, from, pawn)) {
      return false;
    }
  );

  // pawnkin
  bb = !black ? _bbBTokin : _bbWTokin;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Tokin, from, pawn)) {
      return false;
    }
  );

  // promoted lance
  bb = !black ? _bbBProLance : _bbWProLance;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::ProLance, from, pawn)) {
      return false;
    }
  );

  // promoted knight
  bb = !black ? _bbBProKnight : _bbWProKnight;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::ProKnight, from, pawn)) {
      return false;
    }
  );

  // promoted silver
  bb = !black ? _bbBProSilver : _bbWProSilver;
  bb &= !black ? MoveTables::wgold(pawn) : MoveTables::bgold(pawn);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::ProSilver, from, pawn)) {
      return false;
    }
  );

  // horse
  bb = !black ? _bbBHorse : _bbWHorse;
  bb &= MoveTables::horse(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Horse, from, pawn)) {
      return false;
    }
  );

  // dragon
  bb = !black ? _bbBDragon : _bbWDragon;
  bb &= MoveTables::dragon(pawn, occ);
  BB_EACH_OPE(from, bb,
    if (_isValidMove<!black>(Piece::Dragon, from, pawn)) {
      return false;
    }
  );

  return true;
}
template bool Board::_isPawnDropMate<true>() const;
template bool Board::_isPawnDropMate<false>() const;

/**
 * 持駒を打つ手が合法手かどうかチェックします。
 * TODO: rename
 */
template<bool black>
inline bool Board::_isValidMove(const Piece& piece, const Position& to) const {
  // 打ち歩詰め判定
  if (black && piece == Piece::Pawn && to.up() == _posWKing) {
    if (_isPawnDropMate<true>()) {
      return false;
    }
  } else if (!black && piece == Piece::Pawn && to.down() == _posBKing) {
    if (_isPawnDropMate<false>()) {
      return false;
    }
  }

  return true;
}
template bool Board::_isValidMove<true>(const Piece& piece, const Position& to) const;
template bool Board::_isValidMove<false>(const Piece& piece, const Position& to) const;

/**
 * 盤上の駒を移動させる手が合法手かどうかチェックします。
 * TODO: rename
 */
template<bool black>
inline bool Board::_isValidMove(const Piece& piece, const Position& from, const Position& to) const {
  if (piece == Piece::King) {

    // 王手放置判定
    auto occ = _bbBOccupy | _bbWOccupy;
    occ = (black ? _bbBKing : _bbWKing).andNot(occ); // 玉の元居た位置を除外
    if (_isChecking<black>(to, occ)) {
      return false;
    }

  } else { // not (piece == Piece::King)

    // pin判定
    auto king = black ? _posBKing : _posWKing;
    // 玉からの方向が変化している場合のみチェック
    if (PinDirTable::get(king, from) != PinDirTable::get(king, to)) {
      auto occ = _bbBOccupy | _bbWOccupy;
      occ.set(to); // 移動先のビットをセット
      if (_isPin<black>(from, occ)) {
        return false;
      }
    }

  }

  return true;
}
template bool Board::_isValidMove<true>(const Piece& piece, const Position& from, const Position& to) const;
template bool Board::_isValidMove<false>(const Piece& piece, const Position& from, const Position& to) const;

/**
 * 指し手が合法手であるか厳密なチェックをします。
 * 低速な処理のため探索中で使用すべきではありません。
 * @param move
 */
template<bool black>
bool Board::_isValidMoveStrict(const Move& move) const {
  auto occ = _bbBOccupy | _bbWOccupy;
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

    auto& hand = black ? _blackHand : _whiteHand;

    if (hand.get(piece) == 0) {
      return false;
    }

    if (occ.check(to)) {
      return false;
    }

    if (piece == Piece::Pawn) {
      for (int rank = 1; rank <= Position::RankN; rank++) {
        auto piece0 = _board[Position(move.to().getFile(), rank)];
        if (piece0 == (black ? Piece::BPawn : Piece::WPawn)) {
          return false;
        }
      }
    }

    if (!_isValidMove<black>(piece, to)) {
      return false;
    }

  } else {

    auto from = move.from();
    bool promote = move.promote();

    if (_board[from] != (black ? piece.black() : piece.white())) {
      return false;
    }

    if ((black ? _bbBOccupy : _bbWOccupy).check(to)) {
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
  temp._black = black; // 無理やり手番を変更する。
  if (temp.isChecking()) {
    return false;
  }

  return true;
}
template bool Board::_isValidMoveStrict<true>(const Move& move) const;
template bool Board::_isValidMoveStrict<false>(const Move& move) const;

/**
 * 指定した手で局面を進めます。
 * @param move
 */
template<bool black>
bool Board::_makeMove(Move& move) {
  bool promote = move.promote();
  const auto& piece = move.piece();
  const auto& to = move.to();

  if (move.isHand()) {

    if (!_isValidMove<black>(piece, to)) {
      return false;
    }

    Hand& hand = black ? _blackHand : _whiteHand;
    assert(!piece.isPromoted());
    int num = hand.decUnsafe(piece);
    _handHash ^= black ? Zobrist::handBlack(piece, num) : Zobrist::handWhite(piece, num);

  } else { // !move.isHand()

    const auto& from = move.from();

    if (!_isValidMove<black>(piece, from, to)) {
      return false;
    }

    if (black) {
      assert(_board[from] == piece.black());
      _bbBOccupy.unset(from);
      switch (piece) {
      case Piece::Pawn     : _bbBPawn.unset(from); break;
      case Piece::Lance    : _bbBLance.unset(from); break;
      case Piece::Knight   : _bbBKnight.unset(from); break;
      case Piece::Silver   : _bbBSilver.unset(from); break;
      case Piece::Gold     : _bbBGold.unset(from); break;
      case Piece::Bishop   : _bbBBishop.unset(from); break;
      case Piece::Rook     : _bbBRook.unset(from); break;
      case Piece::King     : _bbBKing.unset(from); break;
      case Piece::Tokin    : _bbBTokin.unset(from); break;
      case Piece::ProLance : _bbBProLance.unset(from); break;
      case Piece::ProKnight: _bbBProKnight.unset(from); break;
      case Piece::ProSilver: _bbBProSilver.unset(from); break;
      case Piece::Horse    : _bbBHorse.unset(from); break;
      case Piece::Dragon   : _bbBDragon.unset(from); break;
      default:
        assert(false);
      }
    } else { // white
      assert(_board[from] == piece.white());
      _bbWOccupy.unset(from);
      switch (piece) {
      case Piece::Pawn     : _bbWPawn.unset(from); break;
      case Piece::Lance    : _bbWLance.unset(from); break;
      case Piece::Knight   : _bbWKnight.unset(from); break;
      case Piece::Silver   : _bbWSilver.unset(from); break;
      case Piece::Gold     : _bbWGold.unset(from); break;
      case Piece::Bishop   : _bbWBishop.unset(from); break;
      case Piece::Rook     : _bbWRook.unset(from); break;
      case Piece::King     : _bbWKing.unset(from); break;
      case Piece::Tokin    : _bbWTokin.unset(from); break;
      case Piece::ProLance : _bbWProLance.unset(from); break;
      case Piece::ProKnight: _bbWProKnight.unset(from); break;
      case Piece::ProSilver: _bbWProSilver.unset(from); break;
      case Piece::Horse    : _bbWHorse.unset(from); break;
      case Piece::Dragon   : _bbWDragon.unset(from); break;
      default:
        assert(false);
      }
    }
    _board[from] = Piece::Empty;
    _boardHash ^= Zobrist::board(from, black ? piece : piece.white());

    // capturing
    const auto& captured = _board[to];
    if (captured.exists()) {
      if (black) {
        assert(captured.isWhite());
        _bbWOccupy.unset(to);
        switch (captured) {
        case Piece::WPawn     : _bbWPawn.unset(to); move.setCaptured(Piece::Pawn); break;
        case Piece::WLance    : _bbWLance.unset(to); move.setCaptured(Piece::Lance); break;
        case Piece::WKnight   : _bbWKnight.unset(to); move.setCaptured(Piece::Knight); break;
        case Piece::WSilver   : _bbWSilver.unset(to); move.setCaptured(Piece::Silver); break;
        case Piece::WGold     : _bbWGold.unset(to); move.setCaptured(Piece::Gold); break;
        case Piece::WBishop   : _bbWBishop.unset(to); move.setCaptured(Piece::Bishop); break;
        case Piece::WRook     : _bbWRook.unset(to); move.setCaptured(Piece::Rook); break;
        case Piece::WKing     : assert(false); break;
        case Piece::WTokin    : _bbWTokin.unset(to); move.setCaptured(Piece::Tokin); break;
        case Piece::WProLance : _bbWProLance.unset(to); move.setCaptured(Piece::ProLance); break;
        case Piece::WProKnight: _bbWProKnight.unset(to); move.setCaptured(Piece::ProKnight); break;
        case Piece::WProSilver: _bbWProSilver.unset(to); move.setCaptured(Piece::ProSilver); break;
        case Piece::WHorse    : _bbWHorse.unset(to); move.setCaptured(Piece::Horse); break;
        case Piece::WDragon   : _bbWDragon.unset(to); move.setCaptured(Piece::Dragon); break;
        default:
          assert(false);
        }
      } else { // white
        assert(captured.isBlack());
        _bbBOccupy.unset(to);
        switch (captured) {
        case Piece::BPawn     : _bbBPawn.unset(to); move.setCaptured(Piece::Pawn); break;
        case Piece::BLance    : _bbBLance.unset(to); move.setCaptured(Piece::Lance); break;
        case Piece::BKnight   : _bbBKnight.unset(to); move.setCaptured(Piece::Knight); break;
        case Piece::BSilver   : _bbBSilver.unset(to); move.setCaptured(Piece::Silver); break;
        case Piece::BGold     : _bbBGold.unset(to); move.setCaptured(Piece::Gold); break;
        case Piece::BBishop   : _bbBBishop.unset(to); move.setCaptured(Piece::Bishop); break;
        case Piece::BRook     : _bbBRook.unset(to); move.setCaptured(Piece::Rook); break;
        case Piece::BKing     : assert(false); break;
        case Piece::BTokin    : _bbBTokin.unset(to); move.setCaptured(Piece::Tokin); break;
        case Piece::BProLance : _bbBProLance.unset(to); move.setCaptured(Piece::ProLance); break;
        case Piece::BProKnight: _bbBProKnight.unset(to); move.setCaptured(Piece::ProKnight); break;
        case Piece::BProSilver: _bbBProSilver.unset(to); move.setCaptured(Piece::ProSilver); break;
        case Piece::BHorse    : _bbBHorse.unset(to); move.setCaptured(Piece::Horse); break;
        case Piece::BDragon   : _bbBDragon.unset(to); move.setCaptured(Piece::Dragon); break;
        default:
          assert(false);
        }
      }
      _boardHash ^= Zobrist::board(to, captured);

      // hand
      auto& hand = black ? _blackHand : _whiteHand;
      Piece captured_k = captured.kindOnly().unpromote();
      int num = hand.incUnsafe(captured_k) - 1;
      _handHash ^= black ? Zobrist::handBlack(captured_k, num) : Zobrist::handWhite(captured_k, num);
    }
  }

  // to
  if (black) {
    _bbBOccupy.set(to);
  } else {
    _bbWOccupy.set(to);
  }
  if (!promote) {
    if (black) {
      switch (piece) {
      case Piece::Pawn     : _bbBPawn.set(to); assert(to.getRank() >= 2); break;
      case Piece::Lance    : _bbBLance.set(to); assert(to.getRank() >= 2); break;
      case Piece::Knight   : _bbBKnight.set(to); assert(to.getRank() >= 3); break;
      case Piece::Silver   : _bbBSilver.set(to); break;
      case Piece::Gold     : _bbBGold.set(to); break;
      case Piece::Bishop   : _bbBBishop.set(to); break;
      case Piece::Rook     : _bbBRook.set(to); break;
      case Piece::King     : _bbBKing.set(to); _posBKing = to; break;
      case Piece::Tokin    : _bbBTokin.set(to); break;
      case Piece::ProLance : _bbBProLance.set(to); break;
      case Piece::ProKnight: _bbBProKnight.set(to); break;
      case Piece::ProSilver: _bbBProSilver.set(to); break;
      case Piece::Horse    : _bbBHorse.set(to); break;
      case Piece::Dragon   : _bbBDragon.set(to); break;
      default:
        assert(false);
      }
      _board[to] = piece;
      _boardHash ^= Zobrist::board(to, piece);
    } else {
      switch (piece) {
      case Piece::Pawn     : _bbWPawn.set(to); assert(to.getRank() <= 8); break;
      case Piece::Lance    : _bbWLance.set(to); assert(to.getRank() <= 8); break;
      case Piece::Knight   : _bbWKnight.set(to); assert(to.getRank() <= 7); break;
      case Piece::Silver   : _bbWSilver.set(to); break;
      case Piece::Gold     : _bbWGold.set(to); break;
      case Piece::Bishop   : _bbWBishop.set(to); break;
      case Piece::Rook     : _bbWRook.set(to); break;
      case Piece::King     : _bbWKing.set(to); _posWKing = to; break;
      case Piece::Tokin    : _bbWTokin.set(to); break;
      case Piece::ProLance : _bbWProLance.set(to); break;
      case Piece::ProKnight: _bbWProKnight.set(to); break;
      case Piece::ProSilver: _bbWProSilver.set(to); break;
      case Piece::Horse    : _bbWHorse.set(to); break;
      case Piece::Dragon   : _bbWDragon.set(to); break;
      default:
        assert(false);
      }
      Piece piece_w = piece.white();
      _board[to] = piece_w;
      _boardHash ^= Zobrist::board(to, piece_w);
    }
  } else { // promote
    Piece piece_p;
    if (black) { // black
      assert(!move.isHand());
      assert(to.getRank() <= 3 || move.from().getRank() <= 3);
      switch (piece) {
      case Piece::Pawn:
        _bbBTokin.set(to);
        piece_p = Piece::BTokin;
        break;
      case Piece::Lance:
        _bbBProLance.set(to);
        piece_p = Piece::BProLance;
        break;
      case Piece::Knight:
        _bbBProKnight.set(to);
        piece_p = Piece::BProKnight;
        break;
      case Piece::Silver:
        _bbBProSilver.set(to);
        piece_p = Piece::BProSilver;
        break;
      case Piece::Bishop:
        _bbBHorse.set(to);
        piece_p = Piece::BHorse;
        break;
      case Piece::Rook:
        _bbBDragon.set(to);
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
        _bbWTokin.set(to);
        piece_p = Piece::WTokin;
        break;
      case Piece::Lance:
        _bbWProLance.set(to);
        piece_p = Piece::WProLance;
        break;
      case Piece::Knight:
        _bbWProKnight.set(to);
        piece_p = Piece::WProKnight;
        break;
      case Piece::Silver:
        _bbWProSilver.set(to);
        piece_p = Piece::WProSilver;
        break;
      case Piece::Bishop:
        _bbWHorse.set(to);
        piece_p = Piece::WHorse;
        break;
      case Piece::Rook:
        _bbWDragon.set(to);
        piece_p = Piece::WDragon;
        break;
      default:
        assert(false);
      }
    }
    _board[to] = piece_p;
    _boardHash ^= Zobrist::board(to, piece_p);
  }

  // next turn
  _black = !black;

  return true;
}

template bool Board::_makeMove<true>(Move&);
template bool Board::_makeMove<false>(Move&);

/**
 * 局面を1手戻します。
 */
template<bool black>
bool Board::_unmakeMove(const Move& move) {
  bool promote = move.promote();
  auto piece = move.piece();
  auto to = move.to();

  if (move.isHand()) {

    auto& hand = black ? _blackHand : _whiteHand;
    assert(!piece.isPromoted());
    int num = hand.incUnsafe(piece) - 1;
    _handHash ^= black ? Zobrist::handBlack(piece, num) : Zobrist::handWhite(piece, num);

    _board[to] = Piece::Empty;

  } else { // !move.isHand()

    // from
    auto from = move.from();
    assert(_board[from] == Piece::Empty);
    if (black) {
      assert(_board[to] == (promote ? piece.black().promote() : piece.black()));
      _bbBOccupy.set(from);
      switch (piece) {
      case Piece::Pawn     : _bbBPawn.set(from); break;
      case Piece::Lance    : _bbBLance.set(from); break;
      case Piece::Knight   : _bbBKnight.set(from); break;
      case Piece::Silver   : _bbBSilver.set(from); break;
      case Piece::Gold     : _bbBGold.set(from); break;
      case Piece::Bishop   : _bbBBishop.set(from); break;
      case Piece::Rook     : _bbBRook.set(from); break;
      case Piece::King     : _bbBKing.set(from); _posBKing = from; break;
      case Piece::Tokin    : _bbBTokin.set(from); break;
      case Piece::ProLance : _bbBProLance.set(from); break;
      case Piece::ProKnight: _bbBProKnight.set(from); break;
      case Piece::ProSilver: _bbBProSilver.set(from); break;
      case Piece::Horse    : _bbBHorse.set(from); break;
      case Piece::Dragon   : _bbBDragon.set(from); break;
      default:
        assert(false);
      }
      _board[from] = piece;
      _boardHash ^= Zobrist::board(from, piece);
    } else {
      assert(_board[to] == (promote ? piece.white().promote() : piece.white()));
      _bbWOccupy.set(from);
      switch (piece) {
      case Piece::Pawn     : _bbWPawn.set(from); break;
      case Piece::Lance    : _bbWLance.set(from); break;
      case Piece::Knight   : _bbWKnight.set(from); break;
      case Piece::Silver   : _bbWSilver.set(from); break;
      case Piece::Gold     : _bbWGold.set(from); break;
      case Piece::Bishop   : _bbWBishop.set(from); break;
      case Piece::Rook     : _bbWRook.set(from); break;
      case Piece::King     : _bbWKing.set(from); _posWKing = from; break;
      case Piece::Tokin    : _bbWTokin.set(from); break;
      case Piece::ProLance : _bbWProLance.set(from); break;
      case Piece::ProKnight: _bbWProKnight.set(from); break;
      case Piece::ProSilver: _bbWProSilver.set(from); break;
      case Piece::Horse    : _bbWHorse.set(from); break;
      case Piece::Dragon   : _bbWDragon.set(from); break;
      default:
        assert(false);
      }
      Piece piece_w = piece.white();
      _board[from] = piece_w;
      _boardHash ^= Zobrist::board(from, piece_w);
    }

    // capturing
    if (move.isCapturing()) {
      auto captured = move.captured();
      if (black) {
        _bbWOccupy.set(to);
        switch (captured) {
        case Piece::Pawn     : _bbWPawn.set(to); assert(to.getRank() <= 8); break;
        case Piece::Lance    : _bbWLance.set(to); assert(to.getRank() <= 8); break;
        case Piece::Knight   : _bbWKnight.set(to); assert(to.getRank() <= 7); break;
        case Piece::Silver   : _bbWSilver.set(to); break;
        case Piece::Gold     : _bbWGold.set(to); break;
        case Piece::Bishop   : _bbWBishop.set(to); break;
        case Piece::Rook     : _bbWRook.set(to); break;
        case Piece::King     : assert(false); break;
        case Piece::Tokin    : _bbWTokin.set(to); break;
        case Piece::ProLance : _bbWProLance.set(to); break;
        case Piece::ProKnight: _bbWProKnight.set(to); break;
        case Piece::ProSilver: _bbWProSilver.set(to); break;
        case Piece::Horse    : _bbWHorse.set(to); break;
        case Piece::Dragon   : _bbWDragon.set(to); break;
        default:
          assert(false);
        }
        Piece captured_w = captured.white();
        _board[to] = captured_w;
        _boardHash ^= Zobrist::board(to, captured_w);
      } else {
        _bbBOccupy.set(to);
        switch (captured) {
        case Piece::Pawn     : _bbBPawn.set(to); assert(to.getRank() >= 2); break;
        case Piece::Lance    : _bbBLance.set(to); assert(to.getRank() >= 2); break;
        case Piece::Knight   : _bbBKnight.set(to); assert(to.getRank() >= 3); break;
        case Piece::Silver   : _bbBSilver.set(to); break;
        case Piece::Gold     : _bbBGold.set(to); break;
        case Piece::Bishop   : _bbBBishop.set(to); break;
        case Piece::Rook     : _bbBRook.set(to); break;
        case Piece::King     : assert(false); break;
        case Piece::Tokin    : _bbBTokin.set(to); break;
        case Piece::ProLance : _bbBProLance.set(to); break;
        case Piece::ProKnight: _bbBProKnight.set(to); break;
        case Piece::ProSilver: _bbBProSilver.set(to); break;
        case Piece::Horse    : _bbBHorse.set(to); break;
        case Piece::Dragon   : _bbBDragon.set(to); break;
        default:
          assert(false);
        }
        _board[to] = captured;
        _boardHash ^= Zobrist::board(to, captured);
      }

      // hand
      auto& hand = black ? _blackHand : _whiteHand;
      Piece captured_u = captured.unpromote();
      int num = hand.decUnsafe(captured_u);
      _handHash ^= black ? Zobrist::handBlack(captured_u, num) : Zobrist::handWhite(captured_u, num);
    } else {
      _board[to] = Piece::Empty;
    }

  }

  // to
  if (black) {
    _bbBOccupy.unset(to);
  } else {
    _bbWOccupy.unset(to);
  }
  if (!promote) {
    if (black) {
      switch (piece) {
      case Piece::Pawn     : _bbBPawn.unset(to); break;
      case Piece::Lance    : _bbBLance.unset(to); break;
      case Piece::Knight   : _bbBKnight.unset(to); break;
      case Piece::Silver   : _bbBSilver.unset(to); break;
      case Piece::Gold     : _bbBGold.unset(to); break;
      case Piece::Bishop   : _bbBBishop.unset(to); break;
      case Piece::Rook     : _bbBRook.unset(to); break;
      case Piece::King     : _bbBKing.unset(to); break;
      case Piece::Tokin    : _bbBTokin.unset(to); break;
      case Piece::ProLance : _bbBProLance.unset(to); break;
      case Piece::ProKnight: _bbBProKnight.unset(to); break;
      case Piece::ProSilver: _bbBProSilver.unset(to); break;
      case Piece::Horse    : _bbBHorse.unset(to); break;
      case Piece::Dragon   : _bbBDragon.unset(to); break;
      default:
        assert(false);
      }
    } else {
      switch (piece) {
      case Piece::Pawn     : _bbWPawn.unset(to); break;
      case Piece::Lance    : _bbWLance.unset(to); break;
      case Piece::Knight   : _bbWKnight.unset(to); break;
      case Piece::Silver   : _bbWSilver.unset(to); break;
      case Piece::Gold     : _bbWGold.unset(to); break;
      case Piece::Bishop   : _bbWBishop.unset(to); break;
      case Piece::Rook     : _bbWRook.unset(to); break;
      case Piece::King     : _bbWKing.unset(to); break;
      case Piece::Tokin    : _bbWTokin.unset(to); break;
      case Piece::ProLance : _bbWProLance.unset(to); break;
      case Piece::ProKnight: _bbWProKnight.unset(to); break;
      case Piece::ProSilver: _bbWProSilver.unset(to); break;
      case Piece::Horse    : _bbWHorse.unset(to); break;
      case Piece::Dragon   : _bbWDragon.unset(to); break;
      default:
        assert(false);
      }
    }
    _boardHash ^= Zobrist::board(to, black ? piece : piece.white());
  } else {
    if (black) {
      switch (piece) {
      case Piece::Pawn: _bbBTokin.unset(to); break;
      case Piece::Lance: _bbBProLance.unset(to); break;
      case Piece::Knight: _bbBProKnight.unset(to); break;
      case Piece::Silver: _bbBProSilver.unset(to); break;
      case Piece::Bishop: _bbBHorse.unset(to); break;
      case Piece::Rook: _bbBDragon.unset(to); break;
      default:
        assert(false);
      }
    } else {
      switch (piece) {
      case Piece::Pawn: _bbWTokin.unset(to); break;
      case Piece::Lance: _bbWProLance.unset(to); break;
      case Piece::Knight: _bbWProKnight.unset(to); break;
      case Piece::Silver: _bbWProSilver.unset(to); break;
      case Piece::Bishop: _bbWHorse.unset(to); break;
      case Piece::Rook: _bbWDragon.unset(to); break;
      default:
        assert(false);
      }
    }
    _boardHash ^= Zobrist::board(to, black ? piece.promote() : piece.promote().white());
  }

  // next turn
  _black = black;

  return true;
}

template bool Board::_unmakeMove<true>(const Move& move);
template bool Board::_unmakeMove<false>(const Move& move);

/**
 * パスをして相手に手番を渡します。
 */
void Board::makeNullMove() {
  _black = !_black;
}

/**
 * パスした手を元に戻します。
 */
void Board::unmakeNullMove() {
  _black = !_black;
}

/**
 * データが壊れていないか検査します。
 */
bool Board::validate() const {
  PIECE_EACH(piece) {
    const auto& bb = getBB(piece);
    POSITION_EACH(pos) {
      if (_board[pos] == piece) {
        if (!bb.check(pos)) {
          return false;
        }

        if (piece.isBlack()) {
          if (!_bbBOccupy.check(pos)) {
            return false;
          }
          if (_bbWOccupy.check(pos)) {
            return false;
          }
        } else if (piece.isWhite()) {
          if (_bbBOccupy.check(pos)) {
            return false;
          }
          if (!_bbWOccupy.check(pos)) {
            return false;
          }
        } else {
          if (_bbBOccupy.check(pos)) {
            return false;
          }
          if (_bbWOccupy.check(pos)) {
            return false;
          }
        }
      } else {
        if (bb.check(pos)) {
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

  POSITION_EACH_RD(pos) {
    // piece
    Piece piece = getBoardPiece(pos);
    if (piece.exists()) {
      oss << piece.toString();
    } else {
      oss << "  ";
    }

    // new line
    if (++count % 9 == 0) {
      if (showNumbers) {
        oss << pos.getRank();
      }
      oss << '\n';
    }
  }

  bool is_first = true;
  oss << "black: ";
  HAND_EACH(piece) {
    int count = _blackHand.get(piece);
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
    int count = _whiteHand.get(piece);
    if (count) {
      if (is_first) { is_first = false; } else { oss << ' '; }
      oss << piece.toString();
      if (count >= 2) {
        oss << count;
      }
    }
  }
  oss << '\n';

  oss << "next: " << (_black ? "black" : "white") << '\n';

  return oss.str();
}

/**
 * 盤面を表すCSA形式の文字列を生成します。
 */
std::string Board::toStringCsa() const {
  std::ostringstream oss;
  int count = 0;

  POSITION_EACH_RD(pos) {
    if (count % 9 == 0) {
      if (count != 0) { oss << '\n'; }
      oss << 'P' << (count / 9 + 1);
    }
    count++;

    // piece
    Piece piece = getBoardPiece(pos);
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
    existsBlackHand |= _blackHand.get(piece) != 0;
    existsWhiteHand |= _whiteHand.get(piece) != 0;
  }

  if (existsBlackHand) {
    oss << "P+";
    HAND_EACH(piece) {
      int count = _blackHand.get(piece);
      for (int i = 0; i < count; i++) {
        oss << "00" << piece.toStringCsa(true);
      }
    }
    oss << '\n';
  }

  if (existsWhiteHand) {
    oss << "P-";
    HAND_EACH(piece) {
      int count = _whiteHand.get(piece);
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
