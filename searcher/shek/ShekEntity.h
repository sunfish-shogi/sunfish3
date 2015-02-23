/* ShekEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SHEKENTITY__
#define __SUNFISH_SHEKENTITY__

#include "core/def.h"
#include "HandSet.h"
#include <cstdint>

namespace sunfish {

	class ShekEntity {
	private:

		HandSet _handSet;
		uint64_t _hash;
		int32_t _count;
		bool _blackTurn;

	public:

		void init(uint64_t invalidKey) {
			_hash = invalidKey;
		}

		ShekStat check(const HandSet& handSet, bool blackTurn) const {
			// 持ち駒をチェックする
			ShekStat stat = handSet.compareTo(_handSet, blackTurn);

			if (_blackTurn != blackTurn) {
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
			_hash = hash;
			_handSet = handSet;
			_blackTurn = blackTurn;
			_count = 0;
		}

		void retain() {
			_count++;
		}

		void release(uint64_t invalidKey) {
			_count--;
			if (_count == 0) {
				_hash = invalidKey;
			}
			assert(_count >= 0);
		}

		uint64_t getHash() const {
			return _hash;
		}

	};

	class ShekEntities {
	private:

		static CONSTEXPR uint32_t Size = 4;

		uint32_t _invalidKey;
		ShekEntity _entities[Size];

	public:

		void init(uint32_t key) {
			_invalidKey = key+1; // _entities のどのハッシュ値とも一致しない値
			for (uint32_t i = 0; i < Size; i++) {
				_entities[i].init(_invalidKey);
			}
		}

		ShekStat check(uint64_t hash, const HandSet& handSet, bool blackTurn) const {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].getHash() == hash) {
					return _entities[i].check(handSet, blackTurn);
				}
			}
			return ShekStat::None;
		}

		void set(uint64_t hash, const HandSet& handSet, bool blackTurn) {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].getHash() == hash) {
					_entities[i].retain();
					return;
				}
			}
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].getHash() == _invalidKey) {
					_entities[i].set(hash, handSet, blackTurn);
					_entities[i].retain();
					return;
				}
			}
		}

		void unset(uint64_t hash) {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].getHash() == hash) {
					_entities[i].release(_invalidKey);
					return;
				}
			}
			assert(false);
		}

		bool isCleared() const {
			for (uint32_t i = 0; i < Size; i++) {
				if (_entities[i].getHash() != _invalidKey) {
					return false;
				}
			}
			return true;
		}

	};

}

#endif // __SUNFISH_SHEKENTITY__
