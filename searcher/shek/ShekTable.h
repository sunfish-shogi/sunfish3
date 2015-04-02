/* ShekTable.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SHEKTABLE__
#define __SUNFISH_SHEKTABLE__

#include "ShekEntity.h"
#include "../table/HashTable.h"
#include "core/board/Board.h"

namespace sunfish {

	class ShekTable : public HashTable<ShekEntities> {
	private:

	public:

		ShekTable() : HashTable<ShekEntities>(SHEK_INDEX_WIDTH) {
		}
		ShekTable(const ShekTable&) = delete;
		ShekTable(ShekTable&&) = delete;

		void set(const Board& board) {
			const Hand& hand = board.isBlack() ? board.getBlackHand() : board.getBlackHand();
			uint64_t hash = board.getBoardHash();
			getEntity(hash).set(hash, HandSet(hand), board.isBlack());
		}

		void unset(const Board& board) {
			uint64_t hash = board.getBoardHash();
			getEntity(hash).unset(hash);
		}

		ShekStat check(const Board& board) const {
			const Hand& hand = board.isBlack() ? board.getBlackHand() : board.getBlackHand();
			uint64_t hash = board.getBoardHash();
			return getEntity(hash).check(hash, HandSet(hand), board.isBlack());
		}

		bool isAllCleared() const {
			for (uint32_t i = 0; i < getSize(); i++) {
				if (!getEntity(i).isCleared()) {
					return false;
				}
			}
			return true;
		}

	};

}

#endif // __SUNFISH_SHEKTABLE__
