/* Mate.cpp
 *
 * Kubo Ryosuke
 */

#include "Mate.h"
#include "core/move/MoveTable.h"

#include <iostream>

namespace sunfish {

	template<bool black>
	bool Mate::_isProtected(const Board& board, const Position& to, const Bitboard& occ, const Bitboard& occNoAttacker, const Position& king) {
		// pawn
		Bitboard bb = (black ? board.getBPawn() : board.getWPawn()) & occNoAttacker;
		if (bb.check(black ? to.safetyDown() : to.safetyUp())) {
			return true;
		}

		// lance
		bb = (black ? board.getBLance() : board.getWLance()) & occNoAttacker;
		bb &= black ? MoveTables::WLance.get(to, occ) : MoveTables::BLance.get(to, occ);
		if (bb) { return true; }

		// knight
		bb = (black ? board.getBKnight() : board.getWKnight()) & occNoAttacker;
		bb &= black ? MoveTables::WKnight.get(to) : MoveTables::BKnight.get(to);
		if (bb) { return true; }

		// silver
		bb = (black ? board.getBSilver() : board.getWSilver()) & occNoAttacker;
		bb &= black ? MoveTables::WSilver.get(to) : MoveTables::BSilver.get(to);
		if (bb) { return true; }

		// gold
		bb = (black ? board.getBGold() : board.getWGold()) & occNoAttacker;
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		if (bb) { return true; }

		// bishop
		bb = (black ? board.getBBishop() : board.getWBishop()) & occNoAttacker;
		bb &= MoveTables::Bishop.get(to, occ);
		if (bb) { return true; }

		// rook
		bb = (black ? board.getBRook() : board.getWRook()) & occNoAttacker;
		bb &= MoveTables::Rook.get(to, occ);
		if (bb) { return true; }

		// tokin
		bb = (black ? board.getBTokin() : board.getWTokin()) & occNoAttacker;
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		if (bb) { return true; }

		// promoted lance
		bb = (black ? board.getBProLance() : board.getWProLance()) & occNoAttacker;
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		if (bb) { return true; }

		// promoted knight
		bb = (black ? board.getBProKnight() : board.getWProKnight()) & occNoAttacker;
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		if (bb) { return true; }

		// promoted silver
		bb = (black ? board.getBProSilver() : board.getWProSilver()) & occNoAttacker;
		bb &= black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		if (bb) { return true; }

		// horse
		bb = (black ? board.getBHorse() : board.getWHorse()) & occNoAttacker;
		bb &= MoveTables::Horse.get(to, occ);
		if (bb) { return true; }

		// dragon
		bb = (black ? board.getBDragon() : board.getWDragon()) & occNoAttacker;
		bb &= MoveTables::Dragon.get(to, occ);
		if (bb) { return true; }

		// king
		if (king.isValid()) {
			if (MoveTables::King.get(king).check(to) &&
					!_isProtected<!black>(board, to, occ, occNoAttacker, Position::Invalid)) {
				return true;
			}
		}

		return false;
	}
	template bool Mate::_isProtected<true>(const Board&, const Position&, const Bitboard&, const Bitboard&, const Position&);
	template bool Mate::_isProtected<false>(const Board&, const Position&, const Bitboard&, const Bitboard&, const Position&);

	template<bool black>
	bool Mate::_isProtected(const Board& board, Bitboard& bb, const Bitboard& occ, const Bitboard& occNoAttacker) {
		const auto& king = black ? board.getBKingPosition() : board.getWKingPosition();
		bool hasHand = black
			? !(board.getBlackHand(Piece::Pawn) == 0 &&
					board.getBlackHand(Piece::Lance) == 0 &&
					board.getBlackHand(Piece::Knight) == 0 &&
					board.getBlackHand(Piece::Silver) == 0 &&
					board.getBlackHand(Piece::Gold) == 0 &&
					board.getBlackHand(Piece::Bishop) == 0 &&
					board.getBlackHand(Piece::Rook) == 0)
			: !(board.getWhiteHand(Piece::Pawn) == 0 &&
					board.getWhiteHand(Piece::Lance) == 0 &&
					board.getWhiteHand(Piece::Knight) == 0 &&
					board.getWhiteHand(Piece::Silver) == 0 &&
					board.getWhiteHand(Piece::Gold) == 0 &&
					board.getWhiteHand(Piece::Bishop) == 0 &&
					board.getWhiteHand(Piece::Rook) == 0);

		if (hasHand) {
			BB_EACH_OPE(to, bb, {
					if (_isProtected<black>(board, to, occ, occNoAttacker, king)) { return true; }
			});
		} else {
			BB_EACH_OPE(to, bb, {
					if (_isProtected<black>(board, to, occ, occNoAttacker, Position::Invalid)) { return true; }
			});
		}

		return false;
	}
	template bool Mate::_isProtected<true>(const Board&, Bitboard&, const Bitboard&, const Bitboard&);
	template bool Mate::_isProtected<false>(const Board&, Bitboard&, const Bitboard&, const Bitboard&);

	template<bool black>
	bool Mate::_isMate(const Board& board, const Move& move) {
		const auto& king = black ? board.getWKingPosition() : board.getBKingPosition();

		Position to = move.to();

		Bitboard occ = board.getBOccupy() | board.getWOccupy();
		if (!move.isHand()) {
			occ &= ~Bitboard::mask(move.from());
		}
		occ |= Bitboard::mask(to);
		Bitboard occNoKing = occ & ~Bitboard::mask(king);
		Bitboard occNoAttacker = occ & ~Bitboard::mask(to);

		// 王手している駒を取れるか調べる
		if (_isProtected<!black>(board, to, occ, occNoAttacker, king)) {
			return false;
		}

		// 玉が移動可能な箇所
		Bitboard movable = MoveTables::King.get(king); 
		movable &= black ? ~board.getWOccupy() : ~board.getBOccupy();

		Piece piece = move.piece();
		if (move.promote()) {
			piece = piece.promote();
		}

		switch (piece) {
		case Piece::Pawn: // fall-through
		case Piece::Knight: {
			// do nothing
			break;
		}
		case Piece::Lance: {
			Bitboard route = black
				? (MoveTables::BLance.get(to, occ) & MoveTables::WLance.get(king, occ))
				: (MoveTables::WLance.get(to, occ) & MoveTables::BLance.get(king, occ));
			if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
			Bitboard mask = black ? MoveTables::BLance.get(to, occNoKing) : MoveTables::WLance.get(to, occNoKing);
			movable &= ~mask;
			break;
		}
		case Piece::Silver: {
			Bitboard mask = black ? MoveTables::BSilver.get(to) : MoveTables::WSilver.get(to);
			movable &= ~mask;
			break;
		}
		case Piece::Gold: // fall-through
		case Piece::Tokin: // fall-through
		case Piece::ProLance: // fall-through
		case Piece::ProKnight: // fall-through
		case Piece::ProSilver: {
			Bitboard mask = black ? MoveTables::BGold.get(to) : MoveTables::WGold.get(to);
			movable &= ~mask;
			break;
		}
		case Piece::Bishop: {
			Bitboard route = MoveTables::Bishop.get(to, occ) & MoveTables::Bishop.get(king, occ);
			if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
			movable &= ~MoveTables::Bishop.get(to, occNoKing);
			break;
		}
		case Piece::Rook: {
			Bitboard route = MoveTables::Rook.get(to, occ) & MoveTables::Rook.get(king, occ);
			if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
			movable &= ~MoveTables::Rook.get(to, occNoKing);
			break;
		}
		case Piece::Horse: {
			Bitboard route = MoveTables::Bishop.get(to, occ) & MoveTables::Bishop.get(king, occ);
			if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
			movable &= ~MoveTables::Horse.get(to, occNoKing);
			break;
		}
		case Piece::Dragon: {
			Bitboard route = MoveTables::Rook.get(to, occ) & MoveTables::Rook.get(king, occ);
			if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
			movable &= ~MoveTables::Dragon.get(to, occNoKing);
			break;
		}
		default:
			assert(false);
		}

		BB_EACH_OPE(pos, movable, {
				if (!_isProtected<black>(board, pos, occ, occNoAttacker, Position::Invalid)) {
					return false;
				}
		});

		return true;
	}
	template bool Mate::_isMate<true>(const Board&, const Move&);
	template bool Mate::_isMate<false>(const Board&, const Move&);

	template<bool black>
	bool Mate::_mate1Ply(const Board& board) {
		// TODO: 開き王手の生成
		const auto& occ = board.getBOccupy() | board.getWOccupy();
		Bitboard movable = ~(black ? board.getBOccupy() : board.getWOccupy());
		const auto& king = black ? board.getWKingPosition() : board.getBKingPosition();

		// 金が王手できる位置
		Bitboard bbtTokin = movable & (black ? MoveTables::WGold.get(king) : MoveTables::BGold.get(king));
		bbtTokin &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

		// pawn
		{
			// drop
			Position to = black ? king.safetyDown() : king.safetyUp();
			if (to.isValid()) {
				// board
				Bitboard bb = black ? board.getBPawn() : board.getWPawn();
				if (black) {
					bb.cheepRightShift(1);
				} else {
					bb.cheepLeftShift(1);
				}
				bb &= bbtTokin | Bitboard::mask(to);
				bb &= movable;
				BB_EACH_OPE(to, bb,
					if (to.isPromotable<black>()) {
						if (_isMate<black>(board, Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false))) { return true; }
					} else {
						if (_isMate<black>(board, Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false))) { return true; }
					}
				);
			}
		}

		// lance
		{
			Bitboard bbt = black ? MoveTables::WLance.get(king, occ) : MoveTables::BLance.get(king, occ);
			bbt &= movable;

			// drop
			int handCount = black ? board.getBlackHand(Piece::Lance) : board.getWhiteHand(Piece::Lance);
			if (handCount) {
				Bitboard bb = bbt & ~occ;
				BB_EACH_OPE(to, bb,
					if (_isMate<black>(board, Move(Piece::Lance, to, false))) { return true; }
				);
			}

			// board
			Bitboard bb = black ? board.getBLance() : board.getWLance();
			bbt &= ~bbtTokin;
			BB_EACH_OPE(from, bb,
				Bitboard bbe = black ? MoveTables::BLance.get(from, occ) : MoveTables::WLance.get(from, occ);
				Bitboard bb2 = bbt & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Lance, from, to, false, false))) { return true; }
				});
				bb2 = bbtTokin & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Lance, from, to, true, false))) { return true; }
				});
			);
		}

		// knight
		{
			// drop
			Position to1 = black ? king.safetyLeftDownKnight() : king.safetyLeftUpKnight();
			Position to2 = black ? king.safetyRightDownKnight() : king.safetyRightUpKnight();
			int handCount = black ? board.getBlackHand(Piece::Knight) : board.getWhiteHand(Piece::Knight);
			if (handCount) {
				if (to1.isValid() && !occ.check(to1)) {
					if (_isMate<black>(board, Move(Piece::Knight, to1, false))) { return true; }
				}
				if (to2.isValid() && !occ.check(to2)) {
					if (_isMate<black>(board, Move(Piece::Knight, to2, false))) { return true; }
				}
			}

			// board
			Bitboard bb = black ? board.getBKnight() : board.getWKnight();
			Bitboard bbt = Bitboard::mask(to1) | Bitboard::mask(to2);
			bbt &= movable;
			BB_EACH_OPE(from, bb,
				Bitboard bbe = black ? MoveTables::BKnight.get(from) : MoveTables::WKnight.get(from);
				Bitboard bb2 = bbt & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Knight, from, to, false, false))) { return true; }
				});
				bb2 = bbtTokin & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Knight, from, to, true, false))) { return true; }
				});
			);
		}

		// silver
		{
			Bitboard bbt = black ? MoveTables::WSilver.get(king) : MoveTables::BSilver.get(king);
			bbt &= movable;

			// drop
			int handCount = black ? board.getBlackHand(Piece::Silver) : board.getWhiteHand(Piece::Silver);
			if (handCount) {
				Bitboard bb = bbt & ~occ;
				BB_EACH_OPE(to, bb,
					if (_isMate<black>(board, Move(Piece::Silver, to, false))) { return true; }
				);
			}

			// board
			Bitboard bb = black ? board.getBSilver() : board.getWSilver();
			bbt &= ~bbtTokin;
			BB_EACH_OPE(from, bb,
				Bitboard bbe = black ? MoveTables::BSilver.get(from) : MoveTables::WSilver.get(from);
				Bitboard bb2 = bbt & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Silver, from, to, false, false))) { return true; }
				});
				bb2 = bbtTokin & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Silver, from, to, true, false))) { return true; }
				});
			);
		}

		Bitboard bbtGold = black ? MoveTables::WGold.get(king) : MoveTables::BGold.get(king);
		bbtGold &= movable;

		// gold
		{
			// drop
			int handCount = black ? board.getBlackHand(Piece::Gold) : board.getWhiteHand(Piece::Gold);
			if (handCount) {
				Bitboard bb = bbtGold & ~occ;
				BB_EACH_OPE(to, bb,
					if (_isMate<black>(board, Move(Piece::Gold, to, false))) { return true; }
				);
			}

			// board
			Bitboard bb = black ? board.getBGold() : board.getWGold();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbtGold & (black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from));
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Gold, from, to, false, false))) { return true; }
				});
			);
		}

		// tokin
		{
			Bitboard bb = black ? board.getBTokin() : board.getWTokin();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbtGold & (black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from));
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Tokin, from, to, false, false))) { return true; }
				});
			);
		}

		// promoted lance
		{
			Bitboard bb = black ? board.getBProLance() : board.getWProLance();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbtGold & (black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from));
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::ProLance, from, to, false, false))) { return true; }
				});
			);
		}

		// promoted knight
		{
			Bitboard bb = black ? board.getBProKnight() : board.getWProKnight();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbtGold & (black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from));
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::ProKnight, from, to, false, false))) { return true; }
				});
			);
		}

		// promoted silver
		{
			Bitboard bb = black ? board.getBProSilver() : board.getWProSilver();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbtGold & (black ? MoveTables::BGold.get(from) : MoveTables::WGold.get(from));
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::ProSilver, from, to, false, false))) { return true; }
				});
			);
		}

		// 馬、竜が王手できる位置
		Bitboard bbtKing = movable & MoveTables::King.get(king);
		bbtKing &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

		// bishop
		{
			Bitboard bbt = MoveTables::Bishop.get(king, occ);
			bbt &= movable;

			// drop
			int handCount = black ? board.getBlackHand(Piece::Bishop) : board.getWhiteHand(Piece::Bishop);
			if (handCount) {
				Bitboard bb = bbt & ~occ;
				BB_EACH_OPE(to, bb,
					if (_isMate<black>(board, Move(Piece::Bishop, to, false))) { return true; }
				);
			}

			// board
			Bitboard bb = black ? board.getBBishop() : board.getWBishop();
			bbt &= ~bbtKing;
			BB_EACH_OPE(from, bb,
				Bitboard bbe = MoveTables::Bishop.get(from, occ);
				Bitboard bb2 = bbt & bbe;
				BB_EACH_OPE(to, bb2, {
					if (to.isPromotable<black>()) {
						if (_isMate<black>(board, Move(Piece::Bishop, from, to, true, false))) { return true; }
					} else {
						if (_isMate<black>(board, Move(Piece::Bishop, from, to, false, false))) { return true; }
					}
				});
				bb2 = bbtKing & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Bishop, from, to, true, false))) { return true; }
				});
			);
		}

		// rook
		{
			Bitboard bbt = MoveTables::Rook.get(king, occ);
			bbt &= movable;

			// drop
			int handCount = black ? board.getBlackHand(Piece::Rook) : board.getWhiteHand(Piece::Rook);
			if (handCount) {
				Bitboard bb = bbt & ~occ;
				BB_EACH_OPE(to, bb,
					if (_isMate<black>(board, Move(Piece::Rook, to, false))) { return true; }
				);
			}

			// board
			Bitboard bb = black ? board.getBRook() : board.getWRook();
			bbt &= ~bbtKing;
			BB_EACH_OPE(from, bb,
				Bitboard bbe = MoveTables::Rook.get(from, occ);
				Bitboard bb2 = bbt & bbe;
				BB_EACH_OPE(to, bb2, {
					if (to.isPromotable<black>()) {
						if (_isMate<black>(board, Move(Piece::Rook, from, to, true, false))) { return true; }
					} else {
						if (_isMate<black>(board, Move(Piece::Rook, from, to, false, false))) { return true; }
					}
				});
				bb2 = bbtKing & bbe;
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Rook, from, to, true, false))) { return true; }
				});
			);
		}

		// horse
		{
			Bitboard bbt = MoveTables::Horse.get(king, occ);
			bbt &= movable;
			Bitboard bb = black ? board.getBHorse() : board.getWHorse();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbt & MoveTables::Horse.get(from, occ);
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Horse, from, to, false, false))) { return true; }
				});
			);
		}

		// dragon
		{
			Bitboard bbt = MoveTables::Dragon.get(king, occ);
			bbt &= movable;
			Bitboard bb = black ? board.getBDragon() : board.getWDragon();
			BB_EACH_OPE(from, bb,
				Bitboard bb2 = bbt & MoveTables::Dragon.get(from, occ);
				BB_EACH_OPE(to, bb2, {
					if (_isMate<black>(board, Move(Piece::Dragon, from, to, false, false))) { return true; }
				});
			);
		}

		return false;
	}
	template bool Mate::_mate1Ply<true>(const Board&);
	template bool Mate::_mate1Ply<false>(const Board&);

}
