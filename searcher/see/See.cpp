/* See.cpp
 * 
 * Kubo Ryosuke
 */

#include "See.h"
#include "core/move/MoveGenerator.h"
#include "core/move/MoveTable.h"
#include "core/util/Data.h"
#include <algorithm>
#include <iostream>

namespace sunfish {

	template <bool shallow, Direction dir, bool isFirst>
	void See::generateAttacker(const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn, bool shortOnly) {

		// 斜め
		if (dir == Direction::LeftUp || dir == Direction::LeftDown ||
				dir == Direction::RightUp || dir == Direction::RightDown) {
			// 距離1
			if (isFirst) {
				auto from = (dir == Direction::LeftUp ? to.leftUp() :
										 dir == Direction::LeftDown ? to.leftDown() :
										 dir == Direction::RightUp ? to.rightUp() :
										 to.rightDown());
				auto piece = board.getBoardPiece(from);
				if (!piece.isEmpty()) {
  				if (dir == Direction::LeftUp ? MovableTable[piece].rightDown :
  						 dir == Direction::LeftDown ? MovableTable[piece].rightUp :
  						 dir == Direction::RightUp ? MovableTable[piece].leftDown :
  						 MovableTable[piece].leftUp) {
						if (piece.isBlack()) {
							_b[_bnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_b[_bnum-1]); }
						} else {
							assert(piece.isWhite());
							_w[_wnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_w[_wnum-1]); }
						}
  				}
					return;
				}
			}

			// 長い距離
			if (!shortOnly) {
  			auto bb = (dir == Direction::LeftUp ? MoveTables::leftUp(to, occ) :
  								 dir == Direction::LeftDown ? MoveTables::leftDown(to, occ) :
  								 dir == Direction::RightUp ? MoveTables::rightUp(to, occ) :
  								 MoveTables::rightDown(to, occ));
				bb &= occ;
				auto from = bb.pickFirst();
				if (from != Position::Invalid) {
					auto piece = board.getBoardPiece(from);
  				if (dir == Direction::LeftUp ? LongMovableTable[piece].rightDown :
  						 dir == Direction::LeftDown ? LongMovableTable[piece].rightUp :
  						 dir == Direction::RightUp ? LongMovableTable[piece].leftDown :
  						 LongMovableTable[piece].leftUp) {
						if (piece.isBlack()) {
							_b[_bnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_b[_bnum-1]); }
						} else {
							assert(piece.isWhite());
							_w[_wnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_w[_wnum-1]); }
						}
  				}
				}
			}
		}

		// 縦と横
		else if (dir == Direction::Up || dir == Direction::Down ||
						 dir == Direction::Left || dir == Direction::Right) {
			// 距離1
			if (isFirst) {
				auto from = (dir == Direction::Up ? to.up() :
										 dir == Direction::Down ? to.down() :
										 dir == Direction::Left ? to.left() :
										 to.right());
				auto piece = board.getBoardPiece(from);
				if (!piece.isEmpty()) {
  				if (dir == Direction::Up ? MovableTable[piece].down :
  						 dir == Direction::Down ? MovableTable[piece].up :
  						 dir == Direction::Left ? MovableTable[piece].right :
  						 MovableTable[piece].left) {
						if (piece.isBlack()) {
							_b[_bnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_b[_bnum-1]); }
						} else {
							assert(piece.isWhite());
							_w[_wnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_w[_wnum-1]); }
						}
  				}
					return;
				}
			}

			// 長い距離
			if (!shortOnly) {
  			auto bb = (dir == Direction::Up ? MoveTables::blance(to, occ) :
  								 dir == Direction::Down ? MoveTables::wlance(to, occ) :
  								 dir == Direction::Left ? MoveTables::left(to, occ) :
  								 MoveTables::right(to, occ));
				bb &= occ;
				auto from = bb.pickFirst();
				if (from != Position::Invalid) {
					auto piece = board.getBoardPiece(from);
  				if (dir == Direction::Up ? LongMovableTable[piece].down :
  						 dir == Direction::Down ? LongMovableTable[piece].up :
  						 dir == Direction::Left ? LongMovableTable[piece].right :
  						 LongMovableTable[piece].left) {
						if (piece.isBlack()) {
							_b[_bnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_b[_bnum-1]); }
						} else {
							assert(piece.isWhite());
							_w[_wnum++] = { material::pieceExchange(piece), false, dependOn };
							if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &_w[_wnum-1]); }
						}
  				}
				}
			}
		}

	}

	template <bool black>
	void See::generateKnightAttacker(const Board& board, const Position& from) {
		auto& num = black ? _bnum : _wnum;
		auto list = black ? _b : _w;

		auto piece = board.getBoardPiece(from);
		if ((black && piece == Piece::BKnight) || (!black && piece == Piece::WKnight)) {
			list[num++] = { material::pieceExchange(piece), false, nullptr };
			return;
		}
	}

	template <bool shallow>
	void See::generateAttackers(const Board& board, const Move& move) {
		Position to = move.to();
		HSideType sideTypeH = to.sideTypeH();
		VSideType sideTypeV = to.sideTypeV();
		Bitboard occ = board.getBOccupy() | board.getWOccupy();
		Position exceptPos;
		Direction exceptDir;

		if (move.isHand()) {
			exceptPos = Position::Invalid;
			exceptDir = Direction::None;
		} else {
			auto from = move.from();
			exceptPos = from;
			exceptDir = to.dir(from);
			occ &= ~Bitboard::mask(from);
		}

		_bnum = 0;
		_wnum = 0;

#define GEN(dirname, except, shortOnly) \
		if (!(except)) { \
  		if (exceptDir != Direction::dirname) { \
    		generateAttacker<shallow, Direction::dirname, true>(board, to, occ, nullptr, (shortOnly)); \
  		} else { \
    		generateAttackerR<shallow, Direction::dirname>(board, exceptPos, occ, nullptr); \
  		} \
		}

		GEN(Up,
				sideTypeH == HSideType::Top,
				sideTypeH == HSideType::Top2);
		GEN(Down,
				sideTypeH == HSideType::Bottom,
				sideTypeH == HSideType::Bottom2);
		GEN(Left,
				sideTypeV == VSideType::Left,
				sideTypeV == VSideType::Left2);
		GEN(Right,
				sideTypeV == VSideType::Right,
				sideTypeV == VSideType::Right2);
		GEN(LeftUp,
				sideTypeH == HSideType::Top || sideTypeV == VSideType::Left,
				sideTypeH == HSideType::Top2 || sideTypeV == VSideType::Left2);
		GEN(RightUp,
				sideTypeH == HSideType::Top || sideTypeV == VSideType::Right,
				sideTypeH == HSideType::Top2 || sideTypeV == VSideType::Right2);
		GEN(LeftDown,
				sideTypeH == HSideType::Bottom || sideTypeV == VSideType::Left,
				sideTypeH == HSideType::Bottom2 || sideTypeV == VSideType::Left2);
		GEN(RightDown,
				sideTypeH == HSideType::Bottom || sideTypeV == VSideType::Right,
				sideTypeH == HSideType::Bottom2 || sideTypeV == VSideType::Right2);
#undef GEN

		// 桂馬
		if (sideTypeH != HSideType::Bottom && sideTypeH != HSideType::Bottom2) {
			if (exceptDir != Direction::LeftDownKnight && sideTypeV != VSideType::Left) {
				generateKnightAttacker<true>(board, to.left().down(2));
			}
			if (exceptDir != Direction::RightDownKnight && sideTypeV != VSideType::Right) {
				generateKnightAttacker<true>(board, to.right().down(2));
			}
		}
		if (sideTypeH != HSideType::Top && sideTypeH != HSideType::Top2) {
			if (exceptDir != Direction::LeftUpKnight && sideTypeV != VSideType::Left) {
  			generateKnightAttacker<false>(board, to.left().up(2));
  		}
  		if (exceptDir != Direction::RightUpKnight && sideTypeV != VSideType::Right) {
  			generateKnightAttacker<false>(board, to.right().up(2));
  		}
		}

		assert(_bnum < (int)(sizeof(_b) / sizeof(_b[0])) - 1);
		assert(_wnum < (int)(sizeof(_w) / sizeof(_w[0])) - 1);
		Attacker dummyAttacker;
		dummyAttacker.value = Value::Inf;

		_bref[_bnum] = &dummyAttacker;
		_bref[_bnum-1] = &_b[_bnum-1];
		for (int i = _bnum - 1; i >= 0; i--) {
			AttackerRef tmp = _bref[i] = &_b[i];
			Value value = tmp->value;
			int j = i + 1;
			for (; _bref[j]->value < value; j++) {
				_bref[j-1] = _bref[j];
			}
			_bref[j-1] = tmp;
		}

		_wref[_wnum] = &dummyAttacker;
		_wref[_wnum-1] = &_w[_wnum-1];
		for (int i = _wnum - 1; i >= 0; i--) {
			AttackerRef tmp = _wref[i] = &_w[i];
			Value value = tmp->value;
			int j = i + 1;
			for (; _wref[j]->value < value; j++) {
				_wref[j-1] = _wref[j];
			}
			_wref[j-1] = tmp;
		}

	}

	Value See::search(bool black, Value value, Value alpha, Value beta) {

		auto ref = black ? _bref : _wref;
		int num = black ? _bnum : _wnum;

		for (int i = 0; i < num; i++) {
			auto att = ref[i];
			if (!att->used && (att->dependOn == nullptr || att->dependOn->used)) {
				if (value - att->value >= beta) { return beta; }
				att->used = true;
				auto result = Value::max(0, value - search(!black, att->value, -beta+value, -alpha+value));
				att->used = false;
				return result;
			}
		}

		return 0;

	}

	template <bool shallow>
	Value See::search(const Board& board, const Move& move, Value alpha, Value beta) {

		assert(beta <= Value::PieceInf);
		assert(alpha >= -Value::PieceInf);

		// 取った駒の価値
		Value captured = material::pieceExchange(board.getBoardPiece(move.to()));

		// 動かした駒の価値
		Piece piece = move.piece();
		if (move.promote()) {
			piece = piece.unpromote();
		}
		Value attacker = material::pieceExchange(piece);

		if (captured <= alpha) {
			return alpha;
		}

		if (captured - attacker >= beta) {
			return beta;
		}

		// 移動可能な駒を列挙する。
		generateAttackers<shallow>(board, move);

		return captured - search(!board.isBlack(), attacker, -beta+captured, -alpha+captured);

	}
	template Value See::search<true>(const Board&, const Move&, Value, Value);
	template Value See::search<false>(const Board&, const Move&, Value, Value);

}
