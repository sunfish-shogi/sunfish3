/* Position.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_POSITION__
#define __SUNFISH_POSITION__

#include "../def.h"
#include <string>
#include <cassert>

namespace sunfish {

	enum {
		P91 = 0, P92, P93, P94, P95, P96, P97, P98, P99,
		P81, P82, P83, P84, P85, P86, P87, P88, P89,
		P71, P72, P73, P74, P75, P76, P77, P78, P79,
		P61, P62, P63, P64, P65, P66, P67, P68, P69,
		P51, P52, P53, P54, P55, P56, P57, P58, P59,
		P41, P42, P43, P44, P45, P46, P47, P48, P49,
		P31, P32, P33, P34, P35, P36, P37, P38, P39,
		P21, P22, P23, P24, P25, P26, P27, P28, P29,
		P11, P12, P13, P14, P15, P16, P17, P18, P19,
	};

	enum class Direction : int {
		None,
		Up, Down, Left, Right,
		LeftUp, LeftDown,
		RightUp, RightDown,
		LeftUpKnight, LeftDownKnight,
		RightUpKnight, RightDownKnight,
	};

	enum class DirectionEx : int {
		None,
		Up, Down, Left, Right,
		LeftUp, LeftDown,
		RightUp, RightDown,
		LeftUpKnight, LeftDownKnight,
		RightUpKnight, RightDownKnight,
		LongUp, LongDown, LongLeft, LongRight,
		LongLeftUp, LongLeftDown,
		LongRightUp, LongRightDown,
	};

	enum class HSideType : int {
		None,
		Top,
		Bottom,
		Top2,
		Bottom2,
	};

	enum class VSideType : int {
		None,
		Left,
		Right,
		Left2,
		Right2,
	};

	inline Direction getReversedDir(Direction dir) {
		switch (dir) {
			case Direction::Up:              return Direction::Down;
			case Direction::Down:            return Direction::Up;
			case Direction::Left:            return Direction::Right;
			case Direction::Right:           return Direction::Left;
			case Direction::LeftUp:          return Direction::RightDown;
			case Direction::LeftDown:        return Direction::RightUp;
			case Direction::RightUp:         return Direction::LeftDown;
			case Direction::RightDown:       return Direction::LeftUp;
			case Direction::LeftUpKnight:    return Direction::RightDownKnight;
			case Direction::LeftDownKnight:  return Direction::RightUpKnight;
			case Direction::RightUpKnight:   return Direction::LeftDownKnight;
			case Direction::RightDownKnight: return Direction::LeftUpKnight;
			default: return Direction::None;
		}
	}

	class Position {
	private:

		static const int DistanceTable[17][17];
		static const Direction DirectionTable[17][17];
		static const DirectionEx DirectionTableEx[17][17];
		static const HSideType HSideTypeTable[81];
		static const VSideType VSideTypeTable[81];

		int _index;

	public:

		static CONSTEXPR int Invalid = -1;
		static CONSTEXPR int N = 81;
		static CONSTEXPR int RankN = 9;
		static CONSTEXPR int FileN = 9;
		static CONSTEXPR int Begin = 0;
		static CONSTEXPR int End = N + Begin;
                
		Position() : _index(Invalid) {
		}
		Position(int index) : _index(index) {
		}
		explicit Position(int file, int rank) {
			set(file, rank);
		}

		operator int() const {
			return _index;
		}
		int get() const {
			return _index;
		}

		Position& set(int index) {
			_index = index;
			return *this;
		}
		Position& set(int file, int rank) {
			_index = (9 - file) * RankN + rank - 1;
			return *this;
		}

		int getFile() const {
			return 9 - (_index / RankN);
		}
		int getRank() const {
			return _index % RankN + 1;
		}

		bool isValid() const {
			return _index != Invalid;
		}
		bool isInvalid() const {
			return !isValid();
		}
		bool isStrictValid() const {
			return _index >= 0 && _index < End;
		}
		static bool isValidFile(int file) {
			return file >= 1 && file <= 9;
		}
		static bool isValidRank(int rank) {
			return rank >= 1 && rank <= 9;
		}
		template<bool black>
		bool isPromotable() const {
			if (black) {
				return getRank() - 1 <= 2;
			} else {
				return getRank() - 1 >= 6;
			}
		}
		template<bool black>
		bool isPawnMovable() const {
			if (black) {
				return getRank() - 1 != 0;
			} else {
				return getRank() - 1 != 8;
			}
		}
		template<bool black>
		bool isPawnSignficant() const {
			if (black) {
				return getRank() - 1 >= 3;
			} else {
				return getRank() - 1 <= 5;
			}
		}
		template<bool black>
		bool isLanceMovable() const {
			return isPawnMovable<black>();
		}
		template<bool black>
		bool isLanceSignficant() const {
			if (black) {
				return getRank() - 1 >= 2;
			} else {
				return getRank() - 1 <= 6;
			}
		}
		template<bool black>
		bool isKnightMovable() const {
			if (black) {
				return getRank() - 1 >= 2;
			} else {
				return getRank() - 1 <= 6;
			}
		}
		Position reverse() const {
			return N - 1 - _index;
		}
		Position up(int distance = 1) const {
			assert(getRank() != 1);
			return _index - distance;
		}
		Position down(int distance = 1) const {
			assert(getRank() != 9);
			return _index + distance;
		}
		Position left(int distance = 1) const {
			assert(getFile() != 9);
			return _index - distance * RankN;
		}
		Position right(int distance = 1) const {
			assert(getFile() != 1);
			return _index + distance * RankN;
		}
		Position leftUp(int distance = 1) const {
			assert(getRank() != 1);
			assert(getFile() != 9);
			return (*this).left(distance).up(distance);
		}
		Position leftDown(int distance = 1) const {
			assert(getRank() != 9);
			assert(getFile() != 9);
			return (*this).left(distance).down(distance);
		}
		Position rightUp(int distance = 1) const {
			assert(getRank() != 1);
			assert(getFile() != 1);
			return (*this).right(distance).up(distance);
		}
		Position rightDown(int distance = 1) const {
			assert(getRank() != 9);
			assert(getFile() != 1);
			return (*this).right(distance).down(distance);
		}
		Position leftUpKnight() const {
			assert(getRank() >= 3);
			assert(getFile() != 9);
			return (*this).left().up(2);
		}
		Position leftDownKnight() const {
			assert(getRank() <= 7);
			assert(getFile() != 9);
			return (*this).left().down(2);
		}
		Position rightUpKnight() const {
			assert(getRank() >= 3);
			assert(getFile() != 1);
			return (*this).right().up(2);
		}
		Position rightDownKnight() const {
			assert(getRank() <= 7);
			assert(getFile() != 1);
			return (*this).right().down(2);
		}
		Position safetyUp(int distance = 1) const {
			if (isInvalid()) { return Position(Invalid); }
			int file = getFile();
			int rank = getRank() - distance;
			return rank >= 1 ? Position(file, rank) : Position(Invalid);
		}
		Position safetyDown(int distance = 1) const {
			if (isInvalid()) { return Position(Invalid); }
			int file = getFile();
			int rank = getRank() + distance;
			return rank <= 9 ? Position(file, rank) : Position(Invalid);
		}
		Position safetyLeft(int distance = 1) const {
			if (isInvalid()) { return Position(Invalid); }
			int file = getFile() + distance;
			int rank = getRank();
			return file <= 9 ? Position(file, rank) : Position(Invalid);
		}
		Position safetyRight(int distance = 1) const {
			if (isInvalid()) { return Position(Invalid); }
			int file = getFile() - distance;
			int rank = getRank();
			return file >= 1 ? Position(file, rank) : Position(Invalid);
		}
		Position safetyLeftUp(int distance = 1) const {
			return (*this).safetyLeft(distance).safetyUp(distance);
		}
		Position safetyLeftDown(int distance = 1) const {
			return (*this).safetyLeft(distance).safetyDown(distance);
		}
		Position safetyRightUp(int distance = 1) const {
			return (*this).safetyRight(distance).safetyUp(distance);
		}
		Position safetyRightDown(int distance = 1) const {
			return (*this).safetyRight(distance).safetyDown(distance);
		}
		Position safetyLeftUpKnight() const {
			return (*this).safetyLeft().safetyUp(2);
		}
		Position safetyLeftDownKnight() const {
			return (*this).safetyLeft().safetyDown(2);
		}
		Position safetyRightUpKnight() const {
			return (*this).safetyRight().safetyUp(2);
		}
		Position safetyRightDownKnight() const {
			return (*this).safetyRight().safetyDown(2);
		}
		Position next() const {
			return _index + 1;
		}
		Position nextRightDown() const {
			int file = _index / RankN;
			if (file == 8) {
				int rank = _index % RankN;
				if (rank == RankN - 1) {
					return End;
				}
				return _index - (FileN - 1) * RankN + 1;
			}
			return _index + RankN;
		}
		int distance(const Position& to) const {
			int rank = to._index % RankN - _index % RankN + 8;
			int file = to._index / RankN - _index / RankN + 8;
			return DistanceTable[rank][file];
		}
		Direction dir(const Position& to) const {
			int rank = to._index % RankN - _index % RankN + 8;
			int file = to._index / RankN - _index / RankN + 8;
			return DirectionTable[rank][file];
		}
		DirectionEx dirEx(const Position& to) const {
			int rank = to._index % RankN - _index % RankN + 8;
			int file = to._index / RankN - _index / RankN + 8;
			return DirectionTableEx[rank][file];
		}
		HSideType sideTypeH() const {
			assert(_index != Invalid);
			return HSideTypeTable[_index];
		}
		VSideType sideTypeV() const {
			assert(_index != Invalid);
			return VSideTypeTable[_index];
		}

		std::string toString() const;
		static Position parse(const char* str);

	};

}

#define POSITION_EACH(pos)				for (sunfish::Position (pos) = sunfish::Position::Begin; (pos) != sunfish::Position::End; (pos) = (pos).next())

// scanning right-down
#define POSITION_EACH_RD(pos)			for (sunfish::Position (pos) = P91; (pos) != sunfish::Position::End; (pos) = (pos).nextRightDown())

#endif //__SUNFISH_POSITION__
