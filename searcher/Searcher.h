/* Searcher.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEARCHER__
#define __SUNFISH_SEARCHER__

#include "ISearcher.h"
#include "see/See.h"
#include "tree/Tree.h"
#include "history/History.h"
#include "tt/TT.h"
#include "core/util/Timer.h"

namespace sunfish {

	struct AspSearchStatus {
		Value base;
		int upper;
		int lower;
		Value alpha;
	};

	class Searcher : public ISearcher {
	public:

		/** 探索設定 */
		struct Config {
			int maxDepth;
			int treeSize;
			int wokerSize;
			bool limitEnable;
			int limitSeconds;
		};

		static const int DefaultMaxDepth = 4;

		/** 探索情報 */
		struct Info {
			Move move;
			int node;
			double time;
			double nps;
			Value eval;
			uint64_t hashPruning;
			uint64_t nullMovePruning;
			uint64_t futilityPruning;
			uint64_t extendedFutilityPruning;
		};

	private:

		static const int Depth1Ply = 4;

		Config _config;
		Info _info;
		Timer _timer;

		/** tree */
		Tree* _trees;

		/** 評価関数 */
		Evaluator _eval;

		/** static exchange evaluation */
		See _see;

		/** history heuristic */
		History _history;

		/** transposition table */
		TT _tt;

		/**
		 * 設定の初期化
		 */
		void initConfig() {
			_config.maxDepth = DefaultMaxDepth;
			_config.treeSize = 1;
			_config.wokerSize = 1;
			_config.limitEnable = true;
			_config.limitSeconds = 10;
		}

		/**
		 * ツリーの再確保
		 */
		void reallocTrees() {
			if (_trees != nullptr) {
				delete[] _trees;
			}
			_trees = new Tree[_config.treeSize];
		}

		/**
		 * 前処理
		 */
		void before();

		/**
		 * 後処理
		 */
		void after();

		/**
		 * 優先的に探索する手を追加
		 */
		void addPriorMove(Tree& tree, const Move& move);

		/**
		 * PriorMove に含まれる手かチェックする。
		 */
		bool isPriorMove(Tree& tree, const Move& move);

		/**
		 * PriorMove に含まれる手を除去する。
		 */
		void removePriorMove(Tree& tree);

		/**
		 * sort moves by see
		 */
		void sortSee(Tree& tree, bool plusOnly = false);

		/**
		 * sort moves by history
		 */
		void sortHistory(Tree& tree);

		/**
		 * update history
		 */
		void updateHistory(Tree& tree, int depth, const Move& move);

		/**
		 * get next move
		 */
		bool nextMove(Tree& tree, Move& move);

		/**
		 * reject current move
		 */
		void rejectMove(Tree& tree);

		/**
		 * get estimated value
		 */
		Value estimate(Tree& tree, const Move& move);

		/**
		 * quiesence search
		 */
		Value qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta);

		/**
		 * nega-max search
		 */
		template <bool pvNode>
		Value searchr(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat = NodeStat::Default);

		/**
		 * aspiration search
		 */
		Value asp(Tree& tree, bool black, int depth, AspSearchStatus& astat);

		/**
		 * search from root node
		 */
		bool search(int depth, Move& best, bool gen = true, Value* prevval = nullptr);

		/**
		 * iterative deepening search from root node
		 */
		bool idsearch(Move& best);

	public:

		/**
		 * コンストラクタ
		 */
		Searcher() : _trees(nullptr) {
			initConfig();
			_history.init();
		}

		/**
		 * 初期化
		 */
		void init() {
			reallocTrees();
		}

		/**
		 * 設定を反映します。
		 */
		void setConfig(const Config& config) {
			auto org = _config;
			_config = config;
			if (_config.treeSize != org.treeSize) {
				reallocTrees();
			}
		}

		/**
		 * 設定を取得します。
		 */
		const Config& getConfig() const {
			return _config;
		}

		/**
		 * 探索情報を取得します。
		 */
		const Info& getInfo() const {
			return _info;
		}

		/**
		 * 指定した局面に対して探索を実行します。
		 */
		bool search(const Board& initialBoard, Move& best);

		/**
		 * 指定した局面に対して反復深化探索を実行します。
		 */
		bool idsearch(const Board& initialBoard, Move& best);

		/**
		 * Evaluator を取得します。
		 */
		const Evaluator& getEvaluator() const {
			return _eval;
		}

		/**
		 * Evaluator を取得します。
		 */
		Evaluator& getEvaluator() {
			return _eval;
		}

	};

}

#endif //__SUNFISH_SEARCHER__
