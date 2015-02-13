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

		/**
		 * 指定した局面に対して探索を実行します。
		 * @return {負けたいか中断された場合にfalseを返します。}
		 */
		virtual bool search(const Board& board, Move& move) = 0;

	};

}

#endif //__SUNFISH_ISEARCHER__
