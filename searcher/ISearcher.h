/* ISearcher.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_ISEARCHER__
#define __SUNFISH_ISEARCHER__

#include "core/board/Board.h"

namespace sunfish {

	class ISearcher {
	private:

	public:

		virtual bool search(const Board& board, Move& move) = 0;

	};

}

#endif //__SUNFISH_ISEARCHER__
