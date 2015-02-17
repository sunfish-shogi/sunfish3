/* MoveGenerator.cpp
 *
 * Kubo Ryosuke
 */

#include "./MoveGenerator.h"
#include "./MoveTable.h"
#include "logger/Logger.h"

namespace sunfish {

	/**
	 * 盤上の駒を動かす手を生成
	 * @param black 先手番
	 * @param exceptNonEffectiveProm 打ち歩詰め回避の不成の手を生成しない。
	 * @param exceptNonPromAll 全ての駒について成る手以外を生成しない。
	 * @param exceptKingMasking 玉の移動の手の場合に mask を使用しない。
	 */
	template <bool black, bool exceptNonEffectiveProm, bool exceptNonPromAll, bool exceptKingMasking>
	void MoveGenerator::_generateOnBoard(const Board& board, Moves& moves, const Bitboard& mask) {
		auto occ = board.getBOccupy() | board.getWOccupy();

		// pawn
		Bitboard bb = black ? board.getBPawn() : board.getWPawn();
		if (black) {
			bb.cheepRightShift(1);
		} else {
			bb.cheepLeftShift(1);
		}
		bb &= mask;
		BB_EACH_OPE(to, bb,
			if (to.isPromotable<black>()) {
				// 成る手を生成
				moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false));
				if (!exceptNonEffectiveProm && !exceptNonPromAll) {
					// 不成りを生成
					moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
				}
			} else if (!exceptNonPromAll) {
				moves.add(Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false));
			}
		);

		// lance
		bb = black ? board.getBLance() : board.getWLance();
		BB_EACH_OPE(from, bb,
			Bitboard bb2 = black ? MoveTables::BLance.get(from, occ) : MoveTables::WLance.get(from, occ);
			bb2 &= mask;
			BB_EACH_OPE(to, bb2, {
				if (to.isPromotable<black>()) {
					// 成る手を生成
					moves.add(Move(Piece::Lance, from, to, true, false));
  				if (!exceptNonPromAll && (!exceptNonEffectiveProm || to.isLanceSignficant<black>())) {
						// 意味のない不成でなければ不成を生成
  					moves.add(Move(Piece::Lance, from, to, false, false));
  				}
				} else if (!exceptNonPromAll) {
					moves.add(Move(Piece::Lance, from, to, false, false));
				}
			});
		);

		// knight
		bb = black ? board.getBKnight() : board.getWKnight();
		BB_EACH_OPE(from, bb,
			Bitboard bb2 = black ? MoveTables::BKnight.get(from) : MoveTables::WKnight.get(from);
			bb2 &= mask;
			BB_EACH_OPE(to, bb2, {
				if (to.isPromotable<black>()) {
					// 成る手を生成
					moves.add(Move(Piece::Knight, from, to, true, false));
					if (!exceptNonPromAll && to.isKnightMovable<black>()) {
						// 不成を生成
						moves.add(Move(Piece::Knight, from, to, false, false));
					}
				} else if (!exceptNonPromAll) {
					moves.add(Move(Piece::Knight, from, to, false, false));
				}
			});
		);

		// silver
		bb = black ? board.getBSilver() : board.getWSilver();
		BB_EACH_OPE(from, bb, {
			Bitboard bb2 = black ? MoveTables::BSilver.get(from) : MoveTables::WSilver.get(from);
			bb2 &= mask;
			BB_EACH_OPE(to, bb2, {
				moves.add(Move(Piece::Silver, from, to, false, false));
				if (to.isPromotable<black>() || from.isPromotable<black>()) {
					moves.add(Move(Piece::Silver, from, to, true, false));
				}
			});
		});

		// gold
		if (!exceptNonPromAll) {
			bb = black ? board.getBGold() : board.getWGold();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::Gold, from, to, false, false));
				});
			});
		}

		// bishop
		bb = black ? board.getBBishop() : board.getWBishop();
		BB_EACH_OPE(from, bb,
			Bitboard bb2 = MoveTables::Bishop.get(from, occ);
			bb2 &= mask;
			BB_EACH_OPE(to, bb2, {
				if (to.isPromotable<black>() || from.isPromotable<black>()) {
					moves.add(Move(Piece::Bishop, from, to, true, false));
  				if (!exceptNonEffectiveProm && !exceptNonPromAll) {
  					// 不成りを生成
						moves.add(Move(Piece::Bishop, from, to, false, false));
  				}
				} else if (!exceptNonPromAll) {
					moves.add(Move(Piece::Bishop, from, to, false, false));
				}
			});
		);

		// rook
		bb = black ? board.getBRook() : board.getWRook();
		BB_EACH_OPE(from, bb,
			Bitboard bb2 = MoveTables::Rook.get(from, occ);
			bb2 &= mask;
			BB_EACH_OPE(to, bb2, {
				if (to.isPromotable<black>() || from.isPromotable<black>()) {
					moves.add(Move(Piece::Rook, from, to, true, false));
  				if (!exceptNonEffectiveProm && !exceptNonPromAll) {
  					// 不成りを生成
						moves.add(Move(Piece::Rook, from, to, false, false));
  				}
				} else if (!exceptNonPromAll) {
					moves.add(Move(Piece::Rook, from, to, false, false));
				}
			});
		);

		// king
		if (!exceptNonPromAll) {
			Position from = black ? board.getBKingPosition() : board.getWKingPosition();
			Bitboard bb2 = MoveTables::King.get(from);
			if (exceptKingMasking) {
				bb2 &= black ? ~board.getBOccupy() : ~board.getWOccupy();
			} else {
				bb2 &= mask;
			}
			BB_EACH_OPE(to, bb2, {
				moves.add(Move(Piece::King, from, to, false, false));
			});
		}

		// tokin
		if (!exceptNonPromAll) {
			bb = black ? board.getBTokin() : board.getWTokin();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::Tokin, from, to, false, false));
				});
			});
		}

		// promoted lance
		if (!exceptNonPromAll) {
		bb = black ? board.getBProLance() : board.getWProLance();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::ProLance, from, to, false, false));
				});
			});
		}

		// promoted knight
		if (!exceptNonPromAll) {
		bb = black ? board.getBProKnight() : board.getWProKnight();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::ProKnight, from, to, false, false));
				});
			});
		}

		// promoted silver
		if (!exceptNonPromAll) {
		bb = black ? board.getBProSilver() : board.getWProSilver();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::ProSilver, from, to, false, false));
				});
			});
		}

		// horse
		if (!exceptNonPromAll) {
		bb = black ? board.getBHorse() : board.getWHorse();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = MoveTables::Horse.get(from, occ);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::Horse, from, to, false, false));
				});
			});
		}

		// dragon
		if (!exceptNonPromAll) {
		bb = black ? board.getBDragon() : board.getWDragon();
			BB_EACH_OPE(from, bb, {
				Bitboard bb2 = MoveTables::Dragon.get(from, occ);
				bb2 &= mask;
				BB_EACH_OPE(to, bb2, {
					moves.add(Move(Piece::Dragon, from, to, false, false));
				});
			});
		}
	}
	template void MoveGenerator::_generateOnBoard<true, true, true, false>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateOnBoard<true, true, false, false>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateOnBoard<true, true, false, true>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateOnBoard<false, true, true, false>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateOnBoard<false, true, false, false>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateOnBoard<false, true, false, true>(const Board& board, Moves& moves, const Bitboard& mask);

	/**
	 * 持ち駒を打つ手を生成
	 */
	template <bool black>
	void MoveGenerator::_generateDrop(const Board& board, Moves& moves, const Bitboard& mask) {
		// pawn
		int pawnCount = black ? board.getBlackHand(Piece::Pawn) : board.getWhiteHand(Piece::Pawn);
		if (pawnCount) {
			Bitboard bb = mask & (black ? Bitboard::BPawnMovable : Bitboard::WPawnMovable);
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
			Bitboard bb = mask & (black ? Bitboard::BLanceMovable : Bitboard::WLanceMovable);
			BB_EACH_OPE(to, bb,
				moves.add(Move(Piece::Lance, to, false));
			);
		}

		// knight
		int knightCount = black ? board.getBlackHand(Piece::Knight) : board.getWhiteHand(Piece::Knight);
		if (knightCount) {
			Bitboard bb = mask & (black ? Bitboard::BKnightMovable : Bitboard::WKnightMovable);
			BB_EACH_OPE(to, bb,
				moves.add(Move(Piece::Knight, to, false));
			);
		}

#define GEN_DROP(silver, gold, bishop, rook)					do { \
	Bitboard bb = mask; \
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
	template void MoveGenerator::_generateDrop<true>(const Board& board, Moves& moves, const Bitboard& mask);
	template void MoveGenerator::_generateDrop<false>(const Board& board, Moves& moves, const Bitboard& mask);

	/**
	 * 王手を防ぐ手を生成します。
	 * 王手がかかっている場合のみに使用します。
	 * 打ち歩詰めの手を含む可能性があります。
	 */
	template <bool black>
	void MoveGenerator::_generateEvasion(const Board& board, Moves& moves) {
		Bitboard occ = board.getBOccupy() | board.getWOccupy();
		const auto& king = black ? board.getBKingPosition() : board.getWKingPosition();

		bool shortAttack = false;
		int longAttack = 0;
		Bitboard shortAttacker;
		Bitboard longMask;
		Bitboard tempAttacker;
		Bitboard longAttacker;

		if (black) {

			// 先手玉

			// 1マス移動
			if ((shortAttacker = MoveTables::BPawn.get(king) & board.getWPawn()) ||
					(shortAttacker = MoveTables::BKnight.get(king) & board.getWKnight()) ||
					(shortAttacker = MoveTables::BSilver.get(king) & board.getWSilver()) ||
					(shortAttacker = MoveTables::BGold.get(king) & (board.getWGold() | board.getWTokin() | board.getWProLance() | board.getWProKnight() | board.getWProSilver())) ||
					(shortAttacker = MoveTables::Bishop1.get(king) & board.getWBishop()) ||
					(shortAttacker = MoveTables::Rook1.get(king) & board.getWRook()) ||
					(shortAttacker = MoveTables::King.get(king) & (board.getWHorse() | board.getWDragon()))) {
				shortAttack = true;
			}

			// 香車の利き
			if ((longAttacker = (longMask = MoveTables::BLance.get(king, occ)) & board.getWLance())) {
				longAttack++;
			}

			// 角の利き
			if ((tempAttacker = (MoveTables::Bishop2.get(king, occ) & (board.getWBishop() | board.getWHorse())))) {
				longMask = MoveTables::Bishop.get(king, occ);
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
			if ((tempAttacker = (MoveTables::Rook2.get(king, occ) & (board.getWRook() | board.getWDragon())))) {
				longMask = MoveTables::Rook.get(king, occ);
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
			if ((shortAttacker = MoveTables::WPawn.get(king) & board.getBPawn()) ||
					(shortAttacker = MoveTables::WKnight.get(king) & board.getBKnight()) ||
					(shortAttacker = MoveTables::WSilver.get(king) & board.getBSilver()) ||
					(shortAttacker = MoveTables::WGold.get(king) & (board.getBGold() | board.getBTokin() | board.getBProLance() | board.getBProKnight() | board.getBProSilver())) ||
					(shortAttacker = MoveTables::Bishop1.get(king) & board.getBBishop()) ||
					(shortAttacker = MoveTables::Rook1.get(king) & board.getBRook()) ||
					(shortAttacker = MoveTables::King.get(king) & (board.getBHorse() | board.getBDragon()))) {
				shortAttack = true;
			}

			// 香車の利き
			if ((tempAttacker = (longMask = MoveTables::WLance.get(king, occ)) & board.getBLance())) {
				longAttacker = tempAttacker;
				longAttack++;
			}

			// 角の利き
			if ((tempAttacker = (MoveTables::Bishop2.get(king, occ) & (board.getBBishop() | board.getBHorse())))) {
				longMask = MoveTables::Bishop.get(king, occ);
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
			if ((tempAttacker = (MoveTables::Rook2.get(king, occ) & (board.getBRook() | board.getBDragon())))) {
				longMask = MoveTables::Rook.get(king, occ);
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
			_generateKing<black>(board, moves);
		} else if (shortAttack) {
			// 近接王手
			_generateEvasionShort<black>(board, moves, shortAttacker);
		} else {
			// 跳び駒の利き

			// 1. 移動合と玉の移動
			_generateOnBoard<black, false, false, true>(board, moves, longMask);

			// 2. 持ち駒
			Bitboard dropMask = longMask & ~longAttacker;
			if (dropMask) {
				_generateDrop<black>(board, moves, dropMask);
			}
		}

	}
	template void MoveGenerator::_generateEvasion<true>(const Board& board, Moves& moves);
	template void MoveGenerator::_generateEvasion<false>(const Board& board, Moves& moves);

	template <bool black>
	void MoveGenerator::_generateEvasionShort(const Board& board, Moves& moves, const Bitboard& attacker) {
		Bitboard occ = board.getBOccupy() | board.getWOccupy();
		Position to = attacker.getFirst();

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
		bb &= black ? MoveTables::WLance.get(to, occ) : MoveTables::BLance.get(to, occ);
		BB_EACH_OPE(from, bb,
			if (to.isLanceSignficant<black>()) {
				moves.add(Move(Piece::Lance, from, to, false, false));
			}
			if (to.isPromotable<black>()) {
				moves.add(Move(Piece::Lance, from, to, true, false));
			}
		);

		// knight
		bb = black ? board.getBKnight() : board.getWKnight();
		bb &= black ? MoveTables::WKnight.get(to) : MoveTables::BKnight.get(to);
		BB_EACH_OPE(from, bb,
			if (to.isKnightMovable<black>()) {
				moves.add(Move(Piece::Knight, from, to, false, false));
			}
			if (to.isPromotable<black>()) {
				moves.add(Move(Piece::Knight, from, to, true, false));
			}
		);

		// silver
		bb = black ? board.getBSilver() : board.getWSilver();
		bb &= black ? MoveTables::WSilver.get(to) : MoveTables::BSilver.get(to);
		BB_EACH_OPE(from, bb, {
			moves.add(Move(Piece::Silver, from, to, false, false));
			if (to.isPromotable<black>() || from.isPromotable<black>()) {
				moves.add(Move(Piece::Silver, from, to, true, false));
			}
		});

		// gold
		bb = black ? board.getBGold() : board.getWGold();
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::Gold, from, to, false, false));
		);

		// bishop
		bb = black ? board.getBBishop() : board.getWBishop();
		bb &= MoveTables::Bishop.get(to, occ);
		BB_EACH_OPE(from, bb,
			if (to.isPromotable<black>() || from.isPromotable<black>()) {
				moves.add(Move(Piece::Bishop, from, to, true, false));
			} else {
				moves.add(Move(Piece::Bishop, from, to, false, false));
			}
		);

		// rook
		bb = black ? board.getBRook() : board.getWRook();
		bb &= MoveTables::Rook.get(to, occ);
		BB_EACH_OPE(from, bb,
			if (to.isPromotable<black>() || from.isPromotable<black>()) {
				moves.add(Move(Piece::Rook, from, to, true, false));
			} else {
				moves.add(Move(Piece::Rook, from, to, false, false));
			}
		);

		// king
		{
			Position from = black ? board.getBKingPosition() : board.getWKingPosition();
			bb = MoveTables::King.get(from);
			bb &= black ? ~board.getBOccupy() : ~board.getWOccupy();
			BB_EACH_OPE(to, bb, {
				moves.add(Move(Piece::King, from, to, false, false));
			});
		}

		// tokin
		bb = black ? board.getBTokin() : board.getWTokin();
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::Tokin, from, to, false, false));
		);

		// promoted lance
		bb = black ? board.getBProLance() : board.getWProLance();
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::ProLance, from, to, false, false));
		);

		// promoted knight
		bb = black ? board.getBProKnight() : board.getWProKnight();
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::ProKnight, from, to, false, false));
		);

		// promoted silver
		bb = black ? board.getBProSilver() : board.getWProSilver();
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::ProSilver, from, to, false, false));
		);

		// horse
		bb = black ? board.getBHorse() : board.getWHorse();
		bb &= MoveTables::Horse.get(to, occ);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::Horse, from, to, false, false));
		);

		// dragon
		bb = black ? board.getBDragon() : board.getWDragon();
		bb &= MoveTables::Dragon.get(to, occ);
		BB_EACH_OPE(from, bb,
			moves.add(Move(Piece::Dragon, from, to, false, false));
		);

	}

	/**
	 * 玉の移動する手を生成
	 */
	template <bool black>
	void MoveGenerator::_generateKing(const Board& board, Moves& moves) {
		const auto& from = black ? board.getBKingPosition() : board.getWKingPosition();
		Bitboard mask = black ? ~board.getBOccupy() : ~board.getWOccupy();

		Bitboard bb = MoveTables::King.get(from) & mask;
		BB_EACH_OPE(to, bb,
			moves.add(Move(Piece::King, from, to, false, false));
		);
	}
	template void MoveGenerator::_generateKing<true>(const Board& board, Moves& moves);
	template void MoveGenerator::_generateKing<false>(const Board& board, Moves& moves);
}
