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

		template <bool black, bool shallow, Direction dir, bool isFirst, bool shortOnly>
		void generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn);

		template <bool black, bool shallow, Direction dir, bool isFirst = false>
		void generateAttackersR(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn) {
			HSideType sideTypeH = to.sideTypeH();
			VSideType sideTypeV = to.sideTypeV();
			if ((sideTypeH == HSideType::Top && (dir == Direction::Up || dir == Direction::LeftUp || dir == Direction::RightUp)) ||
					(sideTypeH == HSideType::Bottom && (dir == Direction::Down || dir == Direction::LeftDown || dir == Direction::RightDown)) ||
					(sideTypeV == VSideType::Left && (dir == Direction::Left || dir == Direction::LeftUp || dir == Direction::LeftDown)) ||
					(sideTypeV == VSideType::Right && (dir == Direction::Right || dir == Direction::RightUp || dir == Direction::RightDown))) {
				return;
			} else if ((sideTypeH == HSideType::Top2 && (dir == Direction::Up || dir == Direction::LeftUp || dir == Direction::RightUp)) ||
								 (sideTypeH == HSideType::Bottom2 && (dir == Direction::Down || dir == Direction::LeftDown || dir == Direction::RightDown)) ||
								 (sideTypeV == VSideType::Left2 && (dir == Direction::Left || dir == Direction::LeftUp || dir == Direction::LeftDown)) ||
								 (sideTypeV == VSideType::Right2 && (dir == Direction::Right || dir == Direction::RightUp || dir == Direction::RightDown))) {
				generateAttackers<black, shallow, dir, false, true>(eval, board, to, occ, dependOn); // short only
			} else {
				generateAttackers<black, shallow, dir, false, false>(eval, board, to, occ, dependOn);
			}
		}

		template <bool black>
		void generateKnightAttacker(const Evaluator& eval, const Board& board, const Position& from);

		template <bool black, bool shallow, Direction exceptDir, HSideType sideTypeH, VSideType sideTypeV>
		void generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, const Position& exceptPos);

		template <bool shallow, HSideType sideTypeH, VSideType sideTypeV>
		void generateAttackers(const Evaluator& eval, const Board& board, const Move& move) {
			if (move.isHand()) {
  			auto to = move.to();
  			auto occ = board.getBOccupy() | board.getWOccupy();
				generateAttackers<true, shallow, Direction::None, sideTypeH, sideTypeV>(eval, board, to, occ, Position::Invalid);
				generateAttackers<false, shallow, Direction::None, sideTypeH, sideTypeV>(eval, board, to, occ, Position::Invalid);
			} else {
  			auto to = move.to();
  			auto from = move.from();
  			auto exceptMask = ~Bitboard::mask(from);
  			auto occ = (board.getBOccupy() | board.getWOccupy()) & exceptMask;
  			Direction exceptDir = to.dir(from);
  			switch (exceptDir) {
#define ___SUNFISH_SEE_CASE_DIR___(dirname) \
  				case Direction::dirname: \
  					generateAttackers<true, shallow, Direction::dirname, sideTypeH, sideTypeV>(eval, board, to, occ, from); \
  					generateAttackers<false, shallow, Direction::dirname, sideTypeH, sideTypeV>(eval, board, to, occ, from); \
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
		}

		template <bool shallow, HSideType sideTypeH>
		void generateAttackers(const Evaluator& eval, const Board& board, const Move& move) {
			switch (move.to().sideTypeV()) {
#define ___SUNFISH_AT_CASE_ST___(st) case VSideType::st: generateAttackers<shallow, sideTypeH, VSideType::st>(eval, board, move); break;
					___SUNFISH_AT_CASE_ST___(None)
					___SUNFISH_AT_CASE_ST___(Left)
					___SUNFISH_AT_CASE_ST___(Right)
					___SUNFISH_AT_CASE_ST___(Left2)
					___SUNFISH_AT_CASE_ST___(Right2)
#undef ___SUNFISH_AT_CASE_ST___
			}
		}

		template <bool black>
		Value search(Value value, Value alpha, Value beta);

	public:

		template <bool shallow = false>
		Value search(const Evaluator& eval, const Board& board, const Move& move, Value alpha, Value beta);

		template <bool shallow = false>
		void generateAttackers(const Evaluator& eval, const Board& board, const Move& move) {
			switch (move.to().sideTypeH()) {
#define ___SUNFISH_AT_CASE_ST___(st) case HSideType::st: generateAttackers<shallow, HSideType::st>(eval, board, move); break;
					___SUNFISH_AT_CASE_ST___(None)
					___SUNFISH_AT_CASE_ST___(Top)
					___SUNFISH_AT_CASE_ST___(Bottom)
					___SUNFISH_AT_CASE_ST___(Top2)
					___SUNFISH_AT_CASE_ST___(Bottom2)
#undef ___SUNFISH_AT_CASE_ST___
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
