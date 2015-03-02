/* See.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEE__
#define __SUNFISH_SEE__

#include "core/board/Board.h"
#include "../eval/Evaluator.h"

namespace sunfish {

	class See {
	public:

		struct Attacker {
			Value value;
			Attacker* dependOn;
			bool used;
		};

		struct AttackerRef {
			Attacker* attacker;
			bool operator>(const AttackerRef& o) const {
				return attacker->value > o.attacker->value;
			}
			bool operator<(const AttackerRef& o) const {
				return attacker->value < o.attacker->value;
			}
			bool operator==(const AttackerRef& o) const {
				return attacker->value == o.attacker->value;
			}
		};

	private:

		// 8(近接) + 4(香) + 2(角/馬) + 2(飛/竜)
		Attacker _b[16];
		Attacker _w[16];
		AttackerRef _bref[16];
		AttackerRef _wref[16];
		int _bnum;
		int _wnum;

		template <bool black, Direction dir, bool isFirst>
		void generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn);

		template <bool black>
		void generateKnightAttacker(const Evaluator& eval, const Board& board, const Position& from);

		template <bool black, Direction exceptDir>
		void generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, const Position& exceptPos);

		template <bool black>
		Value search(Value value, Value alpha, Value beta);

	public:

		Value search(const Evaluator& eval, const Board& board, const Move& move, Value alpha, Value beta);

		void generateAttackers(const Evaluator& eval, const Board& board, const Move& move) {
			auto from = move.from();
			auto to = move.to();
			auto exceptMask = ~Bitboard::mask(from);
			auto occ = (board.getBOccupy() | board.getWOccupy()) & exceptMask;
			Direction exceptDir = to.dir(from);
			switch (exceptDir) {
#define ___SUNFISH_SEE_CASE_DIR___(dirname) \
				case Direction::dirname: \
					generateAttackers<true, Direction::dirname>(eval, board, to, occ, from); \
					generateAttackers<false, Direction::dirname>(eval, board, to, occ, from); \
					return;
				___SUNFISH_SEE_CASE_DIR___(Up)
				___SUNFISH_SEE_CASE_DIR___(Down)
				___SUNFISH_SEE_CASE_DIR___(Left)
				___SUNFISH_SEE_CASE_DIR___(Right)
				___SUNFISH_SEE_CASE_DIR___(LeftUp)
				___SUNFISH_SEE_CASE_DIR___(LeftDown)
				___SUNFISH_SEE_CASE_DIR___(RightUp)
				___SUNFISH_SEE_CASE_DIR___(RightDown)
				___SUNFISH_SEE_CASE_DIR___(LeftUpKnight)
				___SUNFISH_SEE_CASE_DIR___(LeftDownKnight)
				___SUNFISH_SEE_CASE_DIR___(RightUpKnight)
				___SUNFISH_SEE_CASE_DIR___(RightDownKnight)
				default: assert(false);
#undef ___SUNFISH_SEE_CASE_DIR___
			}
		}

		const AttackerRef* getBlackList() const {
			return _bref;
		}

		int getBlackNum() const {
			return _bnum;
		}

		const AttackerRef* getWhiteList() const {
			return _wref;
		}

		int getWhiteNum() const {
			return _wnum;
		}

	};

}

#endif // __SUNFISH_SEE__
