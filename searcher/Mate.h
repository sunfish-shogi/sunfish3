/* Mate.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MATE__
#define __SUNFISH_MATE__

#include "table/HashTable.h"
#include "core/def.h"
#include "core/board/Board.h"

namespace sunfish {

	class MateEntity {
	private:
		static CONSTEXPR uint64_t KeyMask = 0x7fffffffffffffffllu;

		struct {
			uint64_t key : 63;
			bool mate : 1;
		} _;

	public:

		MateEntity() {
			_.key = 0x00llu;
			_.mate = false;
		}

		void init(unsigned) {
			_.key = 0x00llu;
			_.mate = false;
		}

		bool is(uint64_t key) const {
			return _.key == (key & KeyMask);
		}

		bool isMate() const {
			return _.mate;
		}

		void set(uint64_t key, bool mate) {
			_.key = key & KeyMask;
			_.mate = mate;
		}
	};

	class MateTable : public HashTable<MateEntity> {
	public:
		MateTable() : HashTable<MateEntity>() {
		}
		MateTable(const MateTable&) = delete;
		MateTable(MateTable&&) = delete;

		bool get(uint64_t hash, bool& mate) const {
			const auto& entity = getEntity(hash);
			if (entity.is(hash)) {
				mate = entity.isMate();
				return true;
			}
			return false;
		}

		void set(uint64_t hash, bool mate) {
			getEntity(hash).set(hash, mate);
		}
	};

	class Mate {
	private:

		Mate();

		template<bool black>
		static bool _isProtected(const Board& board, const Position& to, const Bitboard& occ, const Position& king);

		template<bool black>
		static bool _isProtected(const Board& board, Bitboard& bb, const Bitboard& occ);

		template<bool black>
		static bool _isMate(const Board& board, const Move& move);

		template<bool black>
		static bool _mate1Ply(const Board& board);

	public:

		/**
		 * 1手詰めを探します。
		 * 王手の局面では使用できません。
		 * TODO: 開き王手の生成
		 */
		static bool mate1Ply(const Board& board) {
			if (board.isBlack()) {
				return _mate1Ply<true>(board);
			} else {
				return _mate1Ply<false>(board);
			}
		}

	};

}

#endif // __SUNFISH_MATE__
