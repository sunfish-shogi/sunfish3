/* MoveTable.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVETABLE__
#define __SUNFISH_MOVETABLE__

#include "../base/Piece.h"
#include "../board/Bitboard.h"

namespace sunfish {

	/**
	 * DirectionMaskTable
	 */
	template <bool full>
	class DirectionMaskTable {
	private:
		Bitboard _file[Position::N];
		Bitboard _rank[Position::N];
		Bitboard _leftUpX[Position::N];
		Bitboard _rightUpX[Position::N];

		Bitboard _right[Position::N];
		Bitboard _left[Position::N];
		Bitboard _up[Position::N];
		Bitboard _down[Position::N];
		Bitboard _leftUp[Position::N];
		Bitboard _rightDown[Position::N];
		Bitboard _rightUp[Position::N];
		Bitboard _leftDown[Position::N];

	public:
		DirectionMaskTable();
		DirectionMaskTable(const DirectionMaskTable&) = delete;
		DirectionMaskTable(DirectionMaskTable&) = delete;
		const Bitboard& file(const Position& pos) const {
			return _file[pos];
		}
		const Bitboard& rank(const Position& pos) const {
			return _rank[pos];
		}
		const Bitboard& leftUpX(const Position& pos) const {
			return _leftUpX[pos];
		}
		const Bitboard& rightUpX(const Position& pos) const {
			return _rightUpX[pos];
		}
		const Bitboard& left(const Position& pos) const {
			return _left[pos];
		}
		const Bitboard& right(const Position& pos) const {
			return _right[pos];
		}
		const Bitboard& up(const Position& pos) const {
			return _up[pos];
		}
		const Bitboard& down(const Position& pos) const {
			return _down[pos];
		}
		const Bitboard& leftUp(const Position& pos) const {
			return _leftUp[pos];
		}
		const Bitboard& rightDown(const Position& pos) const {
			return _rightDown[pos];
		}
		const Bitboard& rightUp(const Position& pos) const {
			return _rightUp[pos];
		}
		const Bitboard& leftDown(const Position& pos) const {
			return _leftDown[pos];
		}
	};

	/**
	 * MagicNumberTable
	 */
	class MagicNumberTable {
	private:
		Bitboard _rank[Position::N];
		Bitboard _leftMagic[Position::N];
		Bitboard _leftUp[Position::N];
		Bitboard _rightUp[Position::N];

	public:
		MagicNumberTable();
		MagicNumberTable(const MagicNumberTable&) = delete;
		MagicNumberTable(MagicNumberTable&&) = delete;
		const Bitboard& rank(const Position& pos) const {
			return _rank[pos];
		}
		const Bitboard& leftUp(const Position& pos) const {
			return _leftUp[pos];
		}
		const Bitboard& rightUp(const Position& pos) const {
			return _rightUp[pos];
		}
	};

	/**
	 * MovePatternTable
	 */
	class MovePatternTable {
	private:
		Bitboard _up[Position::N][0x80];
		Bitboard _down[Position::N][0x80];
		Bitboard _file[Position::N][0x80];
		Bitboard _rank[Position::N][0x80];
		Bitboard _leftUpX[Position::N][0x80];
		Bitboard _rightUpX[Position::N][0x80];
		Bitboard _leftUp[Position::N][0x80];
		Bitboard _leftDown[Position::N][0x80];
		Bitboard _rightUp[Position::N][0x80];
		Bitboard _rightDown[Position::N][0x80];
		Bitboard _left[Position::N][0x80];
		Bitboard _right[Position::N][0x80];

	public:
		MovePatternTable();
		MovePatternTable(const MovePatternTable&) = delete;
		MovePatternTable(MovePatternTable&&) = delete;
		const Bitboard& up(const Position& pos, unsigned pattern) const {
			return _up[pos][pattern];
		}
		const Bitboard& down(const Position& pos, unsigned pattern) const {
			return _down[pos][pattern];
		}
		const Bitboard& file(const Position& pos, unsigned pattern) const {
			return _file[pos][pattern];
		}
		const Bitboard& rank(const Position& pos, unsigned pattern) const {
			return _rank[pos][pattern];
		}
		const Bitboard& leftUpX(const Position& pos, unsigned pattern) const {
			return _leftUpX[pos][pattern];
		}
		const Bitboard& rightUpX(const Position& pos, unsigned pattern) const {
			return _rightUpX[pos][pattern];
		}
		const Bitboard& leftUp(const Position& pos, unsigned pattern) const {
			return _leftUp[pos][pattern];
		}
		const Bitboard& leftDown(const Position& pos, unsigned pattern) const {
			return _leftDown[pos][pattern];
		}
		const Bitboard& rightUp(const Position& pos, unsigned pattern) const {
			return _rightUp[pos][pattern];
		}
		const Bitboard& rightDown(const Position& pos, unsigned pattern) const {
			return _rightDown[pos][pattern];
		}
		const Bitboard& left(const Position& pos, unsigned pattern) const {
			return _left[pos][pattern];
		}
		const Bitboard& right(const Position& pos, unsigned pattern) const {
			return _right[pos][pattern];
		}
	};

	namespace _MoveTableType {
		enum Type {
			BPawn, BLance, BKnight, BSilver, BGold,
			WPawn, WLance, WKnight, WSilver, WGold,
			Bishop, Rook, Bishop2, Rook2, King, Horse, Dragon,
			Vertical, Horizontal, RightUpX, RightDownX,
			RightUp, RightDown, LeftUp, LeftDown,
			Left, Right
		};
	}
	typedef _MoveTableType::Type MoveTableType;

	/**
	 * OneStepMoveTable
	 * 跳び駒以外の移動
	 */
	template <MoveTableType type>
	class OneStepMoveTable {
	private:
		Bitboard _table[Position::N];

	public:
		OneStepMoveTable();
		OneStepMoveTable(const OneStepMoveTable&) = delete;
		OneStepMoveTable(OneStepMoveTable&&) = delete;
                     
		const Bitboard& get(const Position& pos) const {
			return _table[pos];
		}
	};

	/**
	 * LongMoveTable
	 * 跳び駒の移動
	 */
	template <MoveTableType type>
	class LongMoveTable {
	public:
		LongMoveTable() {}
		LongMoveTable(const LongMoveTable&) = delete;
		LongMoveTable(LongMoveTable&&) = delete;
		Bitboard get(const Position& pos, const Bitboard& bb) const;
	};

	/**
	 * 利き算出テーブル
	 */
	class MoveTables {
	private:
		MoveTables();
	public:
		/** 先手の歩の利き */
		static const OneStepMoveTable<MoveTableType::BPawn> BPawn;
		/** 先手の桂の利き */
		static const OneStepMoveTable<MoveTableType::BKnight> BKnight;
		/** 先手の銀の利き */
		static const OneStepMoveTable<MoveTableType::BSilver> BSilver;
		/** 先手の金の利き */
		static const OneStepMoveTable<MoveTableType::BGold> BGold;
		/** 後手の歩の利き */
		static const OneStepMoveTable<MoveTableType::WPawn> WPawn;
		/** 後手の桂の利き */
		static const OneStepMoveTable<MoveTableType::WKnight> WKnight;
		/** 後手の銀の利き */
		static const OneStepMoveTable<MoveTableType::WSilver> WSilver;
		/** 後手の金の利き */
		static const OneStepMoveTable<MoveTableType::WGold> WGold;
		/** 1マス先のみの角の利き */
		static const OneStepMoveTable<MoveTableType::Bishop> Bishop1;
		/** 1マス先のみの飛車の利き */
		static const OneStepMoveTable<MoveTableType::Rook> Rook1;
		/** 玉の利き */
		static const OneStepMoveTable<MoveTableType::King> King;

		/** 先手の香車の利き */
		static const LongMoveTable<MoveTableType::BLance> BLance;
		/** 後手の香車の利き */
		static const LongMoveTable<MoveTableType::WLance> WLance;
		/** 角の利き */
		static const LongMoveTable<MoveTableType::Bishop> Bishop;
		/** 飛車の利き */
		static const LongMoveTable<MoveTableType::Rook> Rook;
		/** 2マス先以上の角の利き */
		static const LongMoveTable<MoveTableType::Bishop2> Bishop2;
		/** 2マス先以上の飛車の利き */
		static const LongMoveTable<MoveTableType::Rook2> Rook2;
		/** 竜の利き */
		static const LongMoveTable<MoveTableType::Horse> Horse;
		/** 馬の利き */
		static const LongMoveTable<MoveTableType::Dragon> Dragon;

		/** 筋 */
		static const LongMoveTable<MoveTableType::Vertical> Vertical;
		/** 段 */
		static const LongMoveTable<MoveTableType::Horizontal> Horizontal;
		/** 双方向右上がり */
		static const LongMoveTable<MoveTableType::RightUpX> RightUpX;
		/** 双方向右下がり */
		static const LongMoveTable<MoveTableType::RightDownX> RightDownX;

		/** 右上がり */
		static const LongMoveTable<MoveTableType::RightUp> RightUp;
		/** 右下がり */
		static const LongMoveTable<MoveTableType::RightDown> RightDown;
		/** 左上がり */
		static const LongMoveTable<MoveTableType::LeftUp> LeftUp;
		/** 左下がり */
		static const LongMoveTable<MoveTableType::LeftDown> LeftDown;
		/** 右 */
		static const LongMoveTable<MoveTableType::Right> Right;
		/** 左 */
		static const LongMoveTable<MoveTableType::Left> Left;
	};

	extern const sunfish::DirectionMaskTable<true> dirMask;
	extern const sunfish::MagicNumberTable magic;
	extern const sunfish::MovePatternTable movePattern;

}

#endif //__SUNFISH_MOVETABLE__
