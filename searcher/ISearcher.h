/* ISearcher.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_ISEARCHER__
#define __SUNFISH_ISEARCHER__

#include "core/board/Board.h"

namespace sunfish {

	class ISearcher {
	protected:
		ISearcher() = default;
		ISearcher(const ISearcher&) = delete;
		ISearcher(ISearcher&&) = delete;
		~ISearcher() = default;

	public:

		/**
		 * 指定した局面に対して探索を実行します。
		 * @return {負けたいか中断された場合にfalseを返します。}
		 */
		virtual bool search(const Board& board, Move& move) = 0;

	};

}

#endif //__SUNFISH_ISEARCHER__
