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

		/**
		 * 指定した局面に対して探索を実行します。
		 * @return {負けたいか中断された場合にfalseを返します。}
		 */
		virtual bool search(const Board& board, Move& move) override final;

	};

}

#endif //__SUNFISH_RANDOMSEARCHER__
