/* See.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEE__
#define __SUNFISH_SEE__

#include "core/board/Board.h"
#include "../eval/Material.h"

namespace sunfish {

	class See {
	public:

		struct Attacker {
			Value value;
			bool used;
			Attacker* dependOn;
		};

		using AttackerRef = Attacker*;

	private:

		// 8(近接) + 4(香) + 2(角/馬) + 2(飛/竜)
		Attacker _b[16];
		Attacker _w[16];
		AttackerRef _bref[32];
		AttackerRef _wref[32];
		int _bnum;
		int _wnum;

		template <bool shallow, Direction dir, bool isFirst>
		void generateAttacker(const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn, bool shortOnly);

		template <bool shallow, Direction dir>
		void generateAttackerR(const Board& board, const Position& to, const Bitboard& occ, Attacker* dependOn) {
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
				generateAttacker<shallow, dir, false>(board, to, occ, dependOn, true); // short only
			} else {
				generateAttacker<shallow, dir, false>(board, to, occ, dependOn, false);
			}
		}

		template <bool black>
		void generateKnightAttacker(const Board& board, const Position& from);

		Value search(bool black, Value value, Value alpha, Value beta);

	public:

		template <bool shallow = false>
		Value search(const Board& board, const Move& move, Value alpha, Value beta);

		template <bool shallow = false>
		void generateAttackers(const Board& board, const Move& move);

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
