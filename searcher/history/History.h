/* History.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_HISTORY__
#define __SUNFISH_HISTORY__

#include "core/base/Piece.h"
#include "core/base/Position.h"
#include "core/move/Move.h"
#include "logger/Logger.h"
#include <cstring>

namespace sunfish {

	class History {
	private:
		static const int Board = Position::N;
		static const int Hand = Piece::KindNum;
		static const int From = Board + Hand;
		static const int To = Board;

		uint64_t (*hist)[To];

		int from(const Move& move) const {
			if (move.isHand()) {
				return Board + move.piece().kindOnly() - Piece::KindBegin;
			} else {
				return move.from();
			}
		}

	public:

		static const unsigned Scale = 0x100;

		History() {
			hist = new uint64_t[From][To];
			assert(hist != NULL);
		}

		virtual ~History() {
			delete[] hist;
		}

		void init() {
			memset((void*)hist, 0, sizeof(uint64_t) * From * To);
		}

		void reduce() {
			for (int from = 0; from < From; from++) {
				for (int to = 0; to < To; to++) {
					hist[from][to] = (hist[from][to] >> 3) & ~0xe0000000ull;
				}
			}
		}

		void add(const Move& move, int appear, int good) {
			appear = std::max(appear, 0);
			good = std::max(good, 0);
			assert(good <= appear);
			unsigned f = from(move);
			uint64_t h = hist[f][move.to()];
			uint64_t d = ((uint64_t)appear << 32) + good;
			if (h >= 0x0100000000000000ull - d) {
				h = (h >> 1) & ~0x80000000ull;
			}
			hist[f][move.to()] = h + d;
		}

		unsigned get(const Move& move) const {
			int f = from(move);
			assert(f < From);
			assert(move.to() < To);
			uint64_t h = hist[f][move.to()];
			unsigned a = (unsigned)(h >> 32);
			unsigned g = (unsigned)h;
			unsigned r = g * Scale / (a + 1);
			assert(g <= a);
			assert(r < Scale);
			return r;
		}

	};

}

#endif // __SUNFISH_HISTORY__
