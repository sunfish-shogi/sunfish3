/* See.cpp
 * 
 * Kubo Ryosuke
 */

#include "See.h"
#include "core/move/MoveGenerator.h"
#include "core/move/MoveTable.h"
#include <algorithm>

namespace sunfish {

	template <bool black>
	void See::generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, const Bitboard& mask) {

		auto& num = black ? _bnum : _wnum;
		auto& list = black ? _b : _w;

		num = 0;

		// pawn
		auto bb = black ? MoveTables::WPawn.get(to) : MoveTables::BPawn.get(to);
		bb &= mask;
		if (bb & (black ? board.getBPawn() : board.getWPawn())) {
			list[num++] = eval.table().pawnEx;
		}

		// knight
		bb = black ? MoveTables::WKnight.get(to) : MoveTables::BKnight.get(to);
		bb &= mask;
		if (bb & (black ? board.getBKnight() : board.getWKnight())) {
			list[num++] = eval.table().knightEx;
		}

		// silver
		bb = black ? MoveTables::WSilver.get(to) : MoveTables::BSilver.get(to);
		bb &= mask;
		if (bb & (black ? board.getBSilver() : board.getWSilver())) {
			list[num++] = eval.table().silverEx;
		}

		// gold
		bb = black ? MoveTables::WGold.get(to) : MoveTables::BGold.get(to);
		bb &= mask;
		if (bb & (black ? board.getBGold() : board.getWGold())) {
			list[num++] = eval.table().goldEx;
		}
		if (bb & (black ? board.getBTokin() : board.getWTokin())) {
			list[num++] = eval.table().tokinEx;
		}
		if (bb & (black ? board.getBProLance() : board.getWProLance())) {
			list[num++] = eval.table().pro_lanceEx;
		}
		if (bb & (black ? board.getBProKnight() : board.getWProKnight())) {
			list[num++] = eval.table().pro_knightEx;
		}
		if (bb & (black ? board.getBProSilver() : board.getWProSilver())) {
			list[num++] = eval.table().pro_silverEx;
		}

		// horse, dragon
		bb = MoveTables::King.get(to);
		bb &= mask;
		if (bb & (black ? board.getBHorse() : board.getWHorse())) {
			list[num++] = eval.table().horseEx;
		}
		if (bb & (black ? board.getBDragon() : board.getWDragon())) {
			list[num++] = eval.table().dragonEx;
		}

		// lance
		bb = black ? MoveTables::WLance.get(to, occ) : MoveTables::BLance.get(to, occ);
		bb &= mask;
		if (bb & (black ? board.getBLance() : board.getWLance())) {
			list[num++] = eval.table().lanceEx;
		}

		// bishop, horse
		bb = MoveTables::Bishop2.get(to, occ);
		bb &= mask;
		if (bb & (black ? board.getBBishop() : board.getWBishop())) {
			list[num++] = eval.table().bishopEx;
		}
		if (bb & (black ? board.getBHorse() : board.getWHorse())) {
			list[num++] = eval.table().horseEx;
		}

		// rook, dragon
		bb = MoveTables::Rook2.get(to, occ);
		bb &= mask;
		if (bb & (black ? board.getBRook() : board.getWRook())) {
			list[num++] = eval.table().rookEx;
		}
		if (bb & (black ? board.getBDragon() : board.getWDragon())) {
			list[num++] = eval.table().dragonEx;
		}

		assert(num < (int)(sizeof(_b) / sizeof(_b[0])));

		std::sort(list, list + num);

	}

	template <bool black>
	Value See::search(int b, int w, Value value) {

		if (black) {
			if (b < _bnum) {
				return Value::max(0, value - search<false>(b+1, w, _b[b]));
			}
		} else {
			if (w < _wnum) {
				return Value::max(0, value - search<true>(b, w+1, _w[w]));
			}
		}

		return 0;

	}

	Value See::search(const Evaluator& eval, const Board& board, const Move& move) {

		// 移動可能な駒を列挙する。
		generateAttackers(eval, board, move);

		// 取った駒の価値
		Value captured = eval.pieceExchange(board.getBoardPiece(move.to()));

		// 動かした駒の価値
		Piece piece = move.piece();
		if (move.promote()) {
			piece = piece.unpromote();
		}
		Value attacker = eval.pieceExchange(piece);

		if (board.isBlack()) {
			return captured - search<false>(0, 0, attacker);
		} else {
			return captured - search<true>(0, 0, attacker);
		}

	}

}
