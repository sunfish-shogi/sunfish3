/* ShekEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SHEKENTITY__
#define __SUNFISH_SHEKENTITY__

#include "core/def.h"
#include "HandSet.h"
#include <cstdint>

#define SHEK_INDEX_WIDTH 16

#define SHEK_HASH_WIDTH  56
#define SHEK_COUNT_WIDTH 5
#define SHEK_TURN_WIDTH  1

#define SHEK_ENC_HASH(hash) ((hash) >> (64 - SHEK_HASH_WIDTH))

static_assert((SHEK_INDEX_WIDTH > 64 - SHEK_HASH_WIDTH), "invalid hash length");

namespace sunfish {

	class ShekEntity {
	private:

		HandSet _handSet;
		struct {
			uint64_t hash : SHEK_HASH_WIDTH;
			uint32_t count : SHEK_COUNT_WIDTH;
			bool blackTurn : SHEK_TURN_WIDTH;
		} _;

	public:

		void init(uint64_t invalidKey) {
			_.hash = SHEK_ENC_HASH(invalidKey);
		}

		ShekStat check(const HandSet& handSet, bool blackTurn) const {
			// 持ち駒をチェックする
			ShekStat stat = handSet.compareTo(_handSet, blackTurn);

			if (_.blackTurn != blackTurn) {
				if (stat == ShekStat::Equal) {
					// 手番が逆で持ち駒が等しい => 優越
					stat = ShekStat::Superior;
				} else if (stat == ShekStat::Inferior) {
					// 手番が逆で持ち駒が劣っている => 該当なし
					stat = ShekStat::None;
				}
			}

			return stat;
		}

		void set(uint64_t hash, const HandSet& handSet, bool blackTurn) {
			_.hash = SHEK_ENC_HASH(hash);
			_handSet = handSet;
			_.blackTurn = blackTurn;
			_.count = 0;
		}

		void retain() {
			assert(_.count < ((0x01 << SHEK_COUNT_WIDTH) - 1));
			_.count++;
		}

		void release(uint64_t invalidKey) {
			assert(_.count != 0);
			_.count--;
			if (_.count == 0) {
				_.hash = SHEK_ENC_HASH(invalidKey);
			}
		}

		bool checkHash(uint64_t hash) const {
			return _.hash == SHEK_ENC_HASH(hash);
		}

	};

	class ShekEntities {
	private:

		static CONSTEXPR uint32_t Size = 4;

		uint32_t _invalidKey;
		ShekEntity _entities[Size];

	public:

		void init(uint32_t key) {
			_invalidKey = ~key;
			for (uint32_t i = 0; i < Size; i++) {
				_entities[i].init(_invalidKey);
			}
		}

		ShekStat check(uint64_t hash, const HandSet& handSet, bool blackTurn) const {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].checkHash(hash)) {
					return _entities[i].check(handSet, blackTurn);
				}
			}
			return ShekStat::None;
		}

		void set(uint64_t hash, const HandSet& handSet, bool blackTurn) {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].checkHash(hash)) {
					_entities[i].retain();
					return;
				}
			}
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].checkHash(_invalidKey)) {
					_entities[i].set(hash, handSet, blackTurn);
					_entities[i].retain();
					return;
				}
			}
		}

		void unset(uint64_t hash) {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].checkHash(hash)) {
					_entities[i].release(_invalidKey);
					return;
				}
			}
			assert(false);
		}

		bool isCleared() const {
			for (uint32_t i = 0; i < Size; i++) {
				if (!_entities[i].checkHash(_invalidKey)) {
					return false;
				}
			}
			return true;
		}

	};

}

#endif // __SUNFISH_SHEKENTITY__
