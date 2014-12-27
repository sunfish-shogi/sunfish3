/* RandomSearcher.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_RANDOMSEARCHER__
#define __SUNFISH_RANDOMSEARCHER__

#include "ISearcher.h"
#include "core/util/Random.h"

namespace sunfish {

	class RandomSearcher : public ISearcher {
	private:

		Random random;

	public:

		virtual bool search(const Board& board, Move& move);

	};

}

#endif //__SUNFISH_RANDOMSEARCHER__
