/* MateHistory.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MATE_HISTORY__
#define __SUNFISH_MATE_HISTORY__

#include "core/def.h"
#include "core/move/Move.h"
#include <cstring>

namespace sunfish {

	class MateHistory {
	private:

		uint64_t _hist[Position::N][Piece::Num][Position::N];

	public:
		static const uint32_t Scale = 0x2000;
		static const uint32_t Margin = 100;
		static const uint64_t Max = 0x0008000000000000ull;

		void clear() {
			memset(_hist, 0, sizeof(_hist));
		}

		void update(const Position& king, const Move& move, bool mate) {
			uint64_t v = mate ? 0x100000001llu : 0x100000000llu;
			Piece piece = move.piece();
			Position to = move.to();
			uint64_t h = _hist[king][piece][to];
			h += v;
			if (h >= Max) {
				h = (h >> 1) & ~0x80000000ull;
			}
			_hist[king][piece][to] = h;
		}

		uint64_t getData(const Position& king, const Move& move) const {
			Piece piece = move.piece();
			Position to = move.to();
			uint64_t data = _hist[king][piece][to];
			return data;
		}

		static uint32_t getProbed(uint64_t data) {
			uint32_t probed = data >> 32;
			return probed;
		}

		static uint32_t getMated(uint64_t data) {
			uint32_t mated = (uint32_t)data;
			return mated;
		}

	};

}

#endif // __SUNFISH_MATE_HISTORY__
