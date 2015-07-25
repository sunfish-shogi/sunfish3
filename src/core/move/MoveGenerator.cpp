/* MoveGenerator.cpp
 *
 * Kubo Ryosuke
 */

#include "./MoveGenerator.h"
#include "./MoveTable.h"
#include "../util/Data.h"
#include "logger/Logger.h"

namespace sunfish {

/**
 * 盤上の駒を動かす手を生成
 * @param black 先手番
 * @param genType
 */
template <bool black, MoveGenerator::GenType genType>
void MoveGenerator::generateOnBoard_(const Board& board, Moves& moves, const Bitboard* costumToMask) {
  const bool exceptNonEffectiveNonProm = true;
  const bool exceptProm = (genType == GenType::NoCapture);
  const bool tactical = (genType == GenType::Capture);
  const auto movable = ~(black ? board.getBOccupy() : board.getWOccupy());
  const auto occ = board.getBOccupy() | board.getWOccupy();
  const auto promotable = (black ? Bitboard::BPromotable : Bitboard::WPromotable) & movable;
  const auto promotable2 = (black ? Bitboard::BPromotable2 : Bitboard::WPromotable2) & movable;
  const Bitboard& toMask = (genType == GenType::Capture ? (black ? board.getWOccupy() : board.getBOccupy()) :
                            genType == GenType::NoCapture ? (~(board.getBOccupy() | board.getWOccupy())) :
                            *costumToMask);

  // pawn
  Bitboard bb = black ? board.getBPawn() : board.getWPawn();
  if (black) {
    bb.cheapRightShift(1);
  } else {
    bb.cheapLeftShift(1);
  }
  if (tactical) {
    bb &= toMask | promotable;
  } else if (exceptProm) {
    bb &= toMask & ~promotable;
  } else {
    bb &= toMask;
  }
  BB_EACH_OPE(to, bb,
    if (to.isPromotable<black>()) {
      // 成る手を生成
      if (!exceptProm) {
        moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false));
      }
      // 不成りを生成
      if (!exceptNonEffectiveNonProm) {
        moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
      }
    } else {
      moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
    }
  );

  // lance
  bb = black ? board.getBLance() : board.getWLance();
  if (exceptProm) {
    bb &= ~promotable;
  }
  BB_EACH_OPE(from, bb,
    Bitboard bb2 = black ? MoveTables::blance(from, occ) : MoveTables::wlance(from, occ);
    if (tactical) {
      bb2 &= toMask | promotable;
    } else if (exceptProm) {
      bb2 &= toMask & ~promotable2;
    } else {
      bb2 &= toMask;
    }
    BB_EACH_OPE(to, bb2, {
      if (to.isPromotable<black>()) {
        // 成る手を生成
        if (!exceptProm) {
          moves.add(Move(Piece::Lance, from, to, true, false));
        }
        // 意味のない不成でなければ不成を生成
        if (!exceptNonEffectiveNonProm ||
            ((!tactical || !board.getBoardPiece(to).isEmpty()) && to.isLanceSignficant<black>())) {
          moves.add(Move(Piece::Lance, from, to, false, false));
        }
      } else {
        moves.add(Move(Piece::Lance, from, to, false, false));
      }
    });
  );

  // knight
  bb = black ? board.getBKnight() : board.getWKnight();
  if (exceptProm) {
    bb &= ~promotable;
  }
  BB_EACH_OPE(from, bb,
    Bitboard bb2 = black ? MoveTables::bknight(from) : MoveTables::wknight(from);
    if (tactical) {
      bb2 &= toMask | promotable;
    } else if (exceptProm) {
      bb2 &= toMask & ~promotable2;
    } else {
      bb2 &= toMask;
    }
    BB_EACH_OPE(to, bb2, {
      if (to.isPromotable<black>()) {
        // 成る手を生成
        if (!exceptProm) {
          moves.add(Move(Piece::Knight, from, to, true, false));
        }
        // 不成を生成
        if ((!tactical || !board.getBoardPiece(to).isEmpty()) && to.isKnightMovable<black>()) {
          moves.add(Move(Piece::Knight, from, to, false, false));
        }
      } else {
        moves.add(Move(Piece::Knight, from, to, false, false));
      }
    });
  );

  // silver
  bb = black ? board.getBSilver() : board.getWSilver();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bsilver(from) : MoveTables::wsilver(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      if (to.isPromotable<black>() || from.isPromotable<black>()) {
        moves.add(Move(Piece::Silver, from, to, true, false));
      }
      moves.add(Move(Piece::Silver, from, to, false, false));
    });
  });

  // gold
  bb = black ? board.getBGold() : board.getWGold();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::Gold, from, to, false, false));
    });
  });

  // bishop
  bb = black ? board.getBBishop() : board.getWBishop();
  if (exceptProm) {
    bb &= ~promotable;
  }
  BB_EACH_OPE(from, bb,
    Bitboard bb2 = MoveTables::bishop(from, occ);
    if (tactical) {
      if (!from.isPromotable<black>()) {
        bb2 &= toMask | promotable;
      } else {
        bb2 &= movable;
      }
    } else if (exceptProm) {
      bb2 &= toMask & ~promotable;
    } else {
      bb2 &= toMask;
    }
    BB_EACH_OPE(to, bb2, {
      if (to.isPromotable<black>() || from.isPromotable<black>()) {
        if (!exceptProm) {
          moves.add(Move(Piece::Bishop, from, to, true, false));
        }
        // 不成りを生成
        if (!exceptNonEffectiveNonProm) {
          moves.add(Move(Piece::Bishop, from, to, false, false));
        }
      } else {
        moves.add(Move(Piece::Bishop, from, to, false, false));
      }
    });
  );

  // rook
  bb = black ? board.getBRook() : board.getWRook();
  if (exceptProm) {
    bb &= ~promotable;
  }
  BB_EACH_OPE(from, bb,
    Bitboard bb2 = MoveTables::rook(from, occ);
    if (tactical) {
      if (!from.isPromotable<black>()) {
        bb2 &= toMask | promotable;
      } else {
        bb2 &= movable;
      }
    } else if (exceptProm) {
      bb2 &= toMask & ~promotable;
    } else {
      bb2 &= toMask;
    }
    BB_EACH_OPE(to, bb2, {
      if (to.isPromotable<black>() || from.isPromotable<black>()) {
        if (!exceptProm) {
          moves.add(Move(Piece::Rook, from, to, true, false));
        }
        // 不成りを生成
        if (!exceptNonEffectiveNonProm) {
          moves.add(Move(Piece::Rook, from, to, false, false));
        }
      } else {
        moves.add(Move(Piece::Rook, from, to, false, false));
      }
    });
  );

  // king
  {
    Square from = black ? board.getBKingSquare() : board.getWKingSquare();
    Bitboard bb2 = MoveTables::king(from);
    if (genType == GenType::Evasion) {
      bb2 &= black ? ~board.getBOccupy() : ~board.getWOccupy();
    } else {
      bb2 &= toMask;
    }
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::King, from, to, false, false));
    });
  }

  // tokin
  bb = black ? board.getBTokin() : board.getWTokin();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::Tokin, from, to, false, false));
    });
  });

  // promoted lance
  bb = black ? board.getBProLance() : board.getWProLance();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::ProLance, from, to, false, false));
    });
  });

  // promoted knight
  bb = black ? board.getBProKnight() : board.getWProKnight();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::ProKnight, from, to, false, false));
    });
  });

  // promoted silver
  bb = black ? board.getBProSilver() : board.getWProSilver();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = black ? MoveTables::bgold(from) : MoveTables::wgold(from);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::ProSilver, from, to, false, false));
    });
  });

  // horse
  bb = black ? board.getBHorse() : board.getWHorse();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = MoveTables::horse(from, occ);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::Horse, from, to, false, false));
    });
  });

  // dragon
  bb = black ? board.getBDragon() : board.getWDragon();
  BB_EACH_OPE(from, bb, {
    Bitboard bb2 = MoveTables::dragon(from, occ);
    bb2 &= toMask;
    BB_EACH_OPE(to, bb2, {
      moves.add(Move(Piece::Dragon, from, to, false, false));
    });
  });
}
template void MoveGenerator::generateOnBoard_<true, MoveGenerator::GenType::Capture>(const Board&, Moves&, const Bitboard*);
template void MoveGenerator::generateOnBoard_<true, MoveGenerator::GenType::NoCapture>(const Board&, Moves&, const Bitboard*);
template void MoveGenerator::generateOnBoard_<true, MoveGenerator::GenType::Evasion>(const Board&, Moves&, const Bitboard*);
template void MoveGenerator::generateOnBoard_<false, MoveGenerator::GenType::Capture>(const Board&, Moves&, const Bitboard*);
template void MoveGenerator::generateOnBoard_<false, MoveGenerator::GenType::NoCapture>(const Board&, Moves&, const Bitboard*);
template void MoveGenerator::generateOnBoard_<false, MoveGenerator::GenType::Evasion>(const Board&, Moves&, const Bitboard*);

/**
 * 持ち駒を打つ手を生成
 */
template <bool black>
void MoveGenerator::generateDrop_(const Board& board, Moves& moves, const Bitboard& toMask) {
  // pawn
  int pawnCount = black ? board.getBlackHand(Piece::Pawn) : board.getWhiteHand(Piece::Pawn);
  if (pawnCount) {
    Bitboard bb = toMask & (black ? Bitboard::BPawnMovable : Bitboard::WPawnMovable);
    const Bitboard& bbPawn = black ? board.getBPawn() : board.getWPawn();
    if (bbPawn & Bitboard::file(1)) { bb &= Bitboard::notFile(1); }
    if (bbPawn & Bitboard::file(2)) { bb &= Bitboard::notFile(2); }
    if (bbPawn & Bitboard::file(3)) { bb &= Bitboard::notFile(3); }
    if (bbPawn & Bitboard::file(4)) { bb &= Bitboard::notFile(4); }
    if (bbPawn & Bitboard::file(5)) { bb &= Bitboard::notFile(5); }
    if (bbPawn & Bitboard::file(6)) { bb &= Bitboard::notFile(6); }
    if (bbPawn & Bitboard::file(7)) { bb &= Bitboard::notFile(7); }
    if (bbPawn & Bitboard::file(8)) { bb &= Bitboard::notFile(8); }
    if (bbPawn & Bitboard::file(9)) { bb &= Bitboard::notFile(9); }
    BB_EACH_OPE(to, bb,
      moves.add(Move(Piece::Pawn, to, false));
    );
  }

  // lance
  int lanceCount = black ? board.getBlackHand(Piece::Lance) : board.getWhiteHand(Piece::Lance);
  if (lanceCount) {
    Bitboard bb = toMask & (black ? Bitboard::BLanceMovable : Bitboard::WLanceMovable);
    BB_EACH_OPE(to, bb,
      moves.add(Move(Piece::Lance, to, false));
    );
  }

  // knight
  int knightCount = black ? board.getBlackHand(Piece::Knight) : board.getWhiteHand(Piece::Knight);
  if (knightCount) {
    Bitboard bb = toMask & (black ? Bitboard::BKnightMovable : Bitboard::WKnightMovable);
    BB_EACH_OPE(to, bb,
      moves.add(Move(Piece::Knight, to, false));
    );
  }

#define GEN_DROP(silver, gold, bishop, rook) do { \
Bitboard bb = toMask; \
BB_EACH_OPE(to, bb, \
  if (silver) { moves.add(Move(Piece::Silver, to, false)); } \
  if (gold) { moves.add(Move(Piece::Gold, to, false)); } \
  if (bishop) { moves.add(Move(Piece::Bishop, to, false)); } \
  if (rook) { moves.add(Move(Piece::Rook, to, false)); } \
); \
} while (false)
  unsigned index = 0x00;
  int silverCount = black ? board.getBlackHand(Piece::Silver) : board.getWhiteHand(Piece::Silver);
  int goldCount = black ? board.getBlackHand(Piece::Gold) : board.getWhiteHand(Piece::Gold);
  int bishopCount = black ? board.getBlackHand(Piece::Bishop) : board.getWhiteHand(Piece::Bishop);
  int rookCount = black ? board.getBlackHand(Piece::Rook) : board.getWhiteHand(Piece::Rook);
  index |= silverCount ? 0x08 : 0x00;
  index |= goldCount   ? 0x04 : 0x00;
  index |= bishopCount ? 0x02 : 0x00;
  index |= rookCount   ? 0x01 : 0x00;
  switch (index) {
  case 0x01: GEN_DROP(false, false, false, true ); break;
  case 0x02: GEN_DROP(false, false, true , false); break;
  case 0x03: GEN_DROP(false, false, true , true ); break;
  case 0x04: GEN_DROP(false, true , false, false); break;
  case 0x05: GEN_DROP(false, true , false, true ); break;
  case 0x06: GEN_DROP(false, true , true , false); break;
  case 0x07: GEN_DROP(false, true , true , true ); break;
  case 0x08: GEN_DROP(true , false, false, false); break;
  case 0x09: GEN_DROP(true , false, false, true ); break;
  case 0x0a: GEN_DROP(true , false, true , false); break;
  case 0x0b: GEN_DROP(true , false, true , true ); break;
  case 0x0c: GEN_DROP(true , true , false, false); break;
  case 0x0d: GEN_DROP(true , true , false, true ); break;
  case 0x0e: GEN_DROP(true , true , true , false); break;
  case 0x0f: GEN_DROP(true , true , true , true ); break;
  default: break;
  }
#undef GEN_DROP
}
template void MoveGenerator::generateDrop_<true>(const Board&, Moves&, const Bitboard&);
template void MoveGenerator::generateDrop_<false>(const Board&, Moves&, const Bitboard&);

/**
 * 王手を防ぐ手を生成します。
 * 王手がかかっている場合のみに使用します。
 * 打ち歩詰めの手を含む可能性があります。
 */
template <bool black>
void MoveGenerator::generateEvasion_(const Board& board, Moves& moves) {
  const auto& king = black ? board.getBKingSquare() : board.getWKingSquare();

  bool shortAttack = false;
  int longAttack = 0;
  Bitboard shortAttacker;
  Bitboard longMask;
  Bitboard longAttacker;

  Bitboard occ = board.getBOccupy() | board.getWOccupy();
  Bitboard tempAttacker;

  if (black) {

    // 先手玉

    // 1マス移動
    if ((shortAttacker = MoveTables::bpawn(king) & board.getWPawn()) ||
        (shortAttacker = MoveTables::bknight(king) & board.getWKnight()) ||
        (shortAttacker = MoveTables::bsilver(king) & board.getWSilver()) ||
        (shortAttacker = MoveTables::bgold(king) & (board.getWGold() | board.getWTokin() | board.getWProLance() | board.getWProKnight() | board.getWProSilver())) ||
        (shortAttacker = MoveTables::bishop1(king) & board.getWBishop()) ||
        (shortAttacker = MoveTables::rook1(king) & board.getWRook()) ||
        (shortAttacker = MoveTables::king(king) & (board.getWHorse() | board.getWDragon()))) {
      shortAttack = true;
    }

    // 香車の利き
    if ((longAttacker = (longMask = MoveTables::blance(king, occ)) & board.getWLance())) {
      longAttack++;
    }

    // 角の利き
    if ((tempAttacker = (MoveTables::bishop2(king, occ) & (board.getWBishop() | board.getWHorse())))) {
      longMask = MoveTables::bishop(king, occ);
      longAttacker = tempAttacker;
      if (dirMask.rightUp(king) & longAttacker) {
        longMask &= dirMask.rightUp(king);
      } else if (dirMask.leftDown(king) & longAttacker) {
        longMask &= dirMask.leftDown(king);
      } else if (dirMask.rightDown(king) & longAttacker) {
        longMask &= dirMask.rightDown(king);
      } else if (dirMask.leftUp(king) & longAttacker) {
        longMask &= dirMask.leftUp(king);
      }
      longAttack++;
    }

    // 飛車の利き
    if ((tempAttacker = (MoveTables::rook2(king, occ) & (board.getWRook() | board.getWDragon())))) {
      longMask = MoveTables::rook(king, occ);
      longAttacker = tempAttacker;
      if (dirMask.left(king) & longAttacker) {
        longMask &= dirMask.left(king);
      } else if (dirMask.right(king) & longAttacker) {
        longMask &= dirMask.right(king);
      } else if (dirMask.up(king) & longAttacker) {
        longMask &= dirMask.up(king);
      } else if (dirMask.down(king) & longAttacker) {
        longMask &= dirMask.down(king);
      }
      longAttack++;
    }

  } else {

    // 後手玉

    // 1マス移動
    if ((shortAttacker = MoveTables::wpawn(king) & board.getBPawn()) ||
        (shortAttacker = MoveTables::wknight(king) & board.getBKnight()) ||
        (shortAttacker = MoveTables::wsilver(king) & board.getBSilver()) ||
        (shortAttacker = MoveTables::wgold(king) & (board.getBGold() | board.getBTokin() | board.getBProLance() | board.getBProKnight() | board.getBProSilver())) ||
        (shortAttacker = MoveTables::bishop1(king) & board.getBBishop()) ||
        (shortAttacker = MoveTables::rook1(king) & board.getBRook()) ||
        (shortAttacker = MoveTables::king(king) & (board.getBHorse() | board.getBDragon()))) {
      shortAttack = true;
    }

    // 香車の利き
    if ((tempAttacker = (longMask = MoveTables::wlance(king, occ)) & board.getBLance())) {
      longAttacker = tempAttacker;
      longAttack++;
    }

    // 角の利き
    if ((tempAttacker = (MoveTables::bishop2(king, occ) & (board.getBBishop() | board.getBHorse())))) {
      longMask = MoveTables::bishop(king, occ);
      longAttacker = tempAttacker;
      if (dirMask.rightUp(king) & longAttacker) {
        longMask &= dirMask.rightUp(king);
      } else if (dirMask.leftDown(king) & longAttacker) {
        longMask &= dirMask.leftDown(king);
      } else if (dirMask.rightDown(king) & longAttacker) {
        longMask &= dirMask.rightDown(king);
      } else if (dirMask.leftUp(king) & longAttacker) {
        longMask &= dirMask.leftUp(king);
      }
      longAttack++;
    }

    // 飛車の利き
    if ((tempAttacker = (MoveTables::rook2(king, occ) & (board.getBRook() | board.getBDragon())))) {
      longMask = MoveTables::rook(king, occ);
      longAttacker = tempAttacker;
      if (dirMask.left(king) & longAttacker) {
        longMask &= dirMask.left(king);
      } else if (dirMask.right(king) & longAttacker) {
        longMask &= dirMask.right(king);
      } else if (dirMask.up(king) & longAttacker) {
        longMask &= dirMask.up(king);
      } else if (dirMask.down(king) & longAttacker) {
        longMask &= dirMask.down(king);
      }
      longAttack++;
    }
  }

  assert(shortAttack || longAttack);
  assert(longAttack <= 2);
  assert(longAttack <= 1 || !shortAttack);

  if ((longAttack == 2 || (shortAttack && longAttack))) {
    // 両王手
    generateKing_<black>(board, moves);
  } else if (shortAttack) {
    // 近接王手
    generateEvasionShort_<black>(board, moves, shortAttacker);
  } else {
    // 跳び駒の利き

    // 1. 移動合と玉の移動
    generateOnBoard_<black, GenType::Evasion>(board, moves, &longMask);

    // 2. 持ち駒
    Bitboard dropMask = longMask & ~longAttacker;
    if (dropMask) {
      generateDrop_<black>(board, moves, dropMask);
    }
  }

}
template void MoveGenerator::generateEvasion_<true>(const Board& board, Moves& moves);
template void MoveGenerator::generateEvasion_<false>(const Board& board, Moves& moves);

template <bool black>
void MoveGenerator::generateEvasionShort_(const Board& board, Moves& moves, const Bitboard& attacker) {
  Bitboard occ = board.getBOccupy() | board.getWOccupy();
  Square to = attacker.getFirst();

  // pawn
  Bitboard bb = black ? board.getBPawn() : board.getWPawn();
  if (bb & (black ? attacker.down() : attacker.up())) {
    if (to.isPromotable<black>()) {
      moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false));
    } else {
      moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
    }
  }

  // lance
  bb = black ? board.getBLance() : board.getWLance();
  bb &= black ? MoveTables::wlance(to, occ) : MoveTables::blance(to, occ);
  BB_EACH_OPE(from, bb,
    if (to.isPromotable<black>()) {
      moves.add(Move(Piece::Lance, from, to, true, false));
      if (to.isLanceSignficant<black>()) {
        moves.add(Move(Piece::Lance, from, to, false, false));
      }
    } else {
      moves.add(Move(Piece::Lance, from, to, false, false));
    }
  );

  // knight
  bb = black ? board.getBKnight() : board.getWKnight();
  bb &= black ? MoveTables::wknight(to) : MoveTables::bknight(to);
  BB_EACH_OPE(from, bb,
    if (to.isPromotable<black>()) {
      moves.add(Move(Piece::Knight, from, to, true, false));
    }
    if (to.isKnightMovable<black>()) {
      moves.add(Move(Piece::Knight, from, to, false, false));
    }
  );

  // silver
  bb = black ? board.getBSilver() : board.getWSilver();
  bb &= black ? MoveTables::wsilver(to) : MoveTables::bsilver(to);
  BB_EACH_OPE(from, bb, {
    moves.add(Move(Piece::Silver, from, to, false, false));
    if (to.isPromotable<black>() || from.isPromotable<black>()) {
      moves.add(Move(Piece::Silver, from, to, true, false));
    }
  });

  // gold
  bb = black ? board.getBGold() : board.getWGold();
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::Gold, from, to, false, false));
  );

  // bishop
  bb = black ? board.getBBishop() : board.getWBishop();
  bb &= MoveTables::bishop(to, occ);
  BB_EACH_OPE(from, bb,
    if (to.isPromotable<black>() || from.isPromotable<black>()) {
      moves.add(Move(Piece::Bishop, from, to, true, false));
    } else {
      moves.add(Move(Piece::Bishop, from, to, false, false));
    }
  );

  // rook
  bb = black ? board.getBRook() : board.getWRook();
  bb &= MoveTables::rook(to, occ);
  BB_EACH_OPE(from, bb,
    if (to.isPromotable<black>() || from.isPromotable<black>()) {
      moves.add(Move(Piece::Rook, from, to, true, false));
    } else {
      moves.add(Move(Piece::Rook, from, to, false, false));
    }
  );

  // king
  {
    Square from = black ? board.getBKingSquare() : board.getWKingSquare();
    bb = MoveTables::king(from);
    bb &= black ? ~board.getBOccupy() : ~board.getWOccupy();
    BB_EACH_OPE(to, bb, {
      moves.add(Move(Piece::King, from, to, false, false));
    });
  }

  // tokin
  bb = black ? board.getBTokin() : board.getWTokin();
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::Tokin, from, to, false, false));
  );

  // promoted lance
  bb = black ? board.getBProLance() : board.getWProLance();
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::ProLance, from, to, false, false));
  );

  // promoted knight
  bb = black ? board.getBProKnight() : board.getWProKnight();
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::ProKnight, from, to, false, false));
  );

  // promoted silver
  bb = black ? board.getBProSilver() : board.getWProSilver();
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::ProSilver, from, to, false, false));
  );

  // horse
  bb = black ? board.getBHorse() : board.getWHorse();
  bb &= MoveTables::horse(to, occ);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::Horse, from, to, false, false));
  );

  // dragon
  bb = black ? board.getBDragon() : board.getWDragon();
  bb &= MoveTables::dragon(to, occ);
  BB_EACH_OPE(from, bb,
    moves.add(Move(Piece::Dragon, from, to, false, false));
  );

}

/**
 * 玉の移動する手を生成
 */
template <bool black>
void MoveGenerator::generateKing_(const Board& board, Moves& moves) {
  const auto& from = black ? board.getBKingSquare() : board.getWKingSquare();
  Bitboard toMask = black ? ~board.getBOccupy() : ~board.getWOccupy();

  Bitboard bb = MoveTables::king(from) & toMask;
  BB_EACH_OPE(to, bb,
    moves.add(Move(Piece::King, from, to, false, false));
  );
}
template void MoveGenerator::generateKing_<true>(const Board& board, Moves& moves);
template void MoveGenerator::generateKing_<false>(const Board& board, Moves& moves);

/**
 * 王手を生成
 */
template <bool black, bool light>
void MoveGenerator::generateCheck_(const Board& board, Moves& moves) {
  // TODO: 開き王手の生成
  const auto& occ = board.getBOccupy() | board.getWOccupy();
  Bitboard movable = ~(black ? board.getBOccupy() : board.getWOccupy());
  const auto& king = black ? board.getWKingSquare() : board.getBKingSquare();

  // 金が王手できる位置
  Bitboard bbtGold = black ? MoveTables::wgold(king) : MoveTables::bgold(king);
  bbtGold &= movable;

  // gold
  {
    // drop
    int handCount = black ? board.getBlackHand(Piece::Gold) : board.getWhiteHand(Piece::Gold);
    if (handCount) {
      Bitboard bb = bbtGold & ~occ;
      BB_EACH_OPE(to, bb,
        moves.add(Move(Piece::Gold, to, false));
      );
    }

    // board
    Bitboard bb = (black ? board.getBGold() : board.getWGold())
      | (black ? board.getBTokin() : board.getWTokin())
      | (black ? board.getBProLance() : board.getWProLance())
      | (black ? board.getBProKnight() : board.getWProKnight())
      | (black ? board.getBProSilver() : board.getWProSilver());
    bb &= black ? AttackableTables::bgold(king) : AttackableTables::wgold(king);
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbtGold & (black ? MoveTables::bgold(from) : MoveTables::wgold(from));
      Piece piece = board.getBoardPiece(from).kindOnly();
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(piece, from, to, false, false));
      });
    );
  }

  bbtGold &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

  // silver
  {
    Bitboard bbt = black ? MoveTables::wsilver(king) : MoveTables::bsilver(king);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Silver) : board.getWhiteHand(Piece::Silver);
    if (handCount) {
      Bitboard bb = bbt & ~occ;
      BB_EACH_OPE(to, bb,
        moves.add(Move(Piece::Silver, to, false));
      );
    }

    // board
    Bitboard bb = black ? board.getBSilver() : board.getWSilver();
    bb &= black ? AttackableTables::bsilver(king) : AttackableTables::wsilver(king);
    bbt &= ~bbtGold;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::bsilver(from) : MoveTables::wsilver(from);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Silver, from, to, false, false));
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Silver, from, to, true, false));
      });
    );
  }

  // knight
  {
    // drop
    Square to1 = black ? king.safetyLeftDownKnight() : king.safetyLeftUpKnight();
    Square to2 = black ? king.safetyRightDownKnight() : king.safetyRightUpKnight();
    int handCount = black ? board.getBlackHand(Piece::Knight) : board.getWhiteHand(Piece::Knight);
    if (handCount) {
      if (to1.isValid() && !occ.check(to1)) {
        moves.add(Move(Piece::Knight, to1, false));
      }
      if (to2.isValid() && !occ.check(to2)) {
        moves.add(Move(Piece::Knight, to2, false));
      }
    }

    // board
    Bitboard bb = black ? board.getBKnight() : board.getWKnight();
    bb &= black ? AttackableTables::bknight(king) : AttackableTables::wknight(king);
    Bitboard bbt = Bitboard::mask(to1) | Bitboard::mask(to2);
    bbt &= movable;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::bknight(from) : MoveTables::wknight(from);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Knight, from, to, false, false));
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Knight, from, to, true, false));
      });
    );
  }

  // pawn
  {
    // drop
    Square to = black ? king.safetyDown() : king.safetyUp();
    if (to.isValid()) {
      if (!occ.check(to)) {
        int handCount = black ? board.getBlackHand(Piece::Pawn) : board.getWhiteHand(Piece::Pawn);
        if (handCount) {
          const Bitboard& bbPawn = black ? board.getBPawn() : board.getWPawn();
          // 2歩チェック
          if (!(bbPawn & Bitboard::file(king.getFile()))) {
            moves.add(Move(Piece::Pawn, to, false));
          }
        }
      }

      // board
      Bitboard bb = black ? board.getBPawn() : board.getWPawn();
      bb &= black ? AttackableTables::bpawn(king) : AttackableTables::wpawn(king);
      if (black) {
        bb.cheapRightShift(1);
      } else {
        bb.cheapLeftShift(1);
      }
      bb &= bbtGold | Bitboard::mask(to);
      bb &= movable;
      BB_EACH_OPE(to, bb,
        if (to.isPromotable<black>()) {
          moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false));
        } else {
          moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
        }
      );
    }
  }

  // lance
  {
    Bitboard bbt = black ? MoveTables::wlance(king, occ) : MoveTables::blance(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Lance) : board.getWhiteHand(Piece::Lance);
    if (handCount) {
      if (!light) {
        Bitboard bb = bbt & ~occ;
        BB_EACH_OPE(to, bb,
          moves.add(Move(Piece::Lance, to, false));
        );
      } else {
        Square to = black ? king.safetyDown() : king.safetyUp();
        if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_lance_drop_end; }
        moves.add(Move(Piece::Lance, to, false));

        to = black ? to.safetyDown() : to.safetyUp();
        if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_lance_drop_end; }
        moves.add(Move(Piece::Lance, to, false));

        to = black ? to.safetyDown() : to.safetyUp();
        if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_lance_drop_end; }
        moves.add(Move(Piece::Lance, to, false));
      }
    }
gencheck_lance_drop_end:

    // board
    Bitboard bb = black ? board.getBLance() : board.getWLance();
    bb &= black ? AttackableTables::blance(king) : AttackableTables::wlance(king);
    bbt &= ~bbtGold;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::blance(from, occ) : MoveTables::wlance(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Lance, from, to, false, false));
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Lance, from, to, true, false));
      });
    );
  }

  // 馬、竜が王手できる位置
  Bitboard bbtKing = movable & MoveTables::king(king);
  bbtKing &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

  // bishop
  {
    Bitboard bbt = MoveTables::bishop(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Bishop) : board.getWhiteHand(Piece::Bishop);
    if (handCount) {
      if (!light) {
        Bitboard bb = bbt & ~occ;
        BB_EACH_OPE(to, bb,
          moves.add(Move(Piece::Bishop, to, false));
        );
      } else {
#define GEN_CHECK_DROP_BISHOP(dir) { \
Square to = black ? king.safety ## dir() : king.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_bishop_drop_end ## dir; } \
moves.add(Move(Piece::Bishop, to, false)); \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_bishop_drop_end ## dir; } \
moves.add(Move(Piece::Bishop, to, false)); \
gencheck_bishop_drop_end ## dir: ; \
}
        GEN_CHECK_DROP_BISHOP(LeftUp);
        GEN_CHECK_DROP_BISHOP(LeftDown);
        GEN_CHECK_DROP_BISHOP(RightUp);
        GEN_CHECK_DROP_BISHOP(RightDown);
      }
    }

    // board
    Bitboard bb = black ? board.getBBishop() : board.getWBishop();
    bb &= black ? AttackableTables::bbishop(king) : AttackableTables::wbishop(king);
    bbt &= ~bbtKing;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = MoveTables::bishop(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (to.isPromotable<black>()) {
          moves.add(Move(Piece::Bishop, from, to, true, false));
        } else {
          moves.add(Move(Piece::Bishop, from, to, false, false));
        }
      });
      bb2 = bbtKing & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Bishop, from, to, true, false));
      });
    );
  }

  // rook
  {
    Bitboard bbt = MoveTables::rook(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Rook) : board.getWhiteHand(Piece::Rook);
    if (handCount) {
      if (!light) {
        Bitboard bb = bbt & ~occ;
        BB_EACH_OPE(to, bb,
          moves.add(Move(Piece::Rook, to, false));
        );
      } else {
#define GEN_CHECK_DROP_ROOK(dir) { \
Square to = black ? king.safety ## dir() : king.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_rook_drop_end ## dir; } \
moves.add(Move(Piece::Rook, to, false)); \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_rook_drop_end ## dir; } \
moves.add(Move(Piece::Rook, to, false)); \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto gencheck_rook_drop_end ## dir; } \
moves.add(Move(Piece::Rook, to, false)); \
gencheck_rook_drop_end ## dir: ; \
}
        GEN_CHECK_DROP_ROOK(Left);
        GEN_CHECK_DROP_ROOK(Right);
        GEN_CHECK_DROP_ROOK(Up);
        GEN_CHECK_DROP_ROOK(Down);
      }
    }

    // board
    Bitboard bb = black ? board.getBRook() : board.getWRook();
    bbt &= ~bbtKing;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = MoveTables::rook(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (to.isPromotable<black>()) {
          moves.add(Move(Piece::Rook, from, to, true, false));
        } else {
          moves.add(Move(Piece::Rook, from, to, false, false));
        }
      });
      bb2 = bbtKing & bbe;
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Rook, from, to, true, false));
      });
    );
  }

  // horse
  {
    Bitboard bbt = MoveTables::horse(king, occ);
    bbt &= movable;
    Bitboard bb = black ? board.getBHorse() : board.getWHorse();
    bb &= AttackableTables::horse(king);
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbt & MoveTables::horse(from, occ);
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Horse, from, to, false, false));
      });
    );
  }

  // dragon
  {
    Bitboard bbt = MoveTables::dragon(king, occ);
    bbt &= movable;
    Bitboard bb = black ? board.getBDragon() : board.getWDragon();
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbt & MoveTables::dragon(from, occ);
      BB_EACH_OPE(to, bb2, {
        moves.add(Move(Piece::Dragon, from, to, false, false));
      });
    );
  }

}
template void MoveGenerator::generateCheck_<true, true>(const Board& board, Moves& moves);
template void MoveGenerator::generateCheck_<false, true>(const Board& board, Moves& moves);
template void MoveGenerator::generateCheck_<true, false>(const Board& board, Moves& moves);
template void MoveGenerator::generateCheck_<false, false>(const Board& board, Moves& moves);

} // namespace sunfish
