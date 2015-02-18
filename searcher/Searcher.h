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
#include "core/record/Record.h"
#include "core/util/Timer.h"
#include <atomic>

namespace sunfish {

	struct AspSearchStatus {
		Value base;
		int upper;
		int lower;
		Value alpha;
	};

	class Searcher : public ISearcher {
	public:

		static CONSTEXPR int Depth1Ply = 16;

		/** 探索設定 */
		struct Config {
			int maxDepth;
			int treeSize;
			int wokerSize;
			bool limitEnable;
			int limitSeconds;
		};

		static const int DefaultMaxDepth = 7;

		/** 探索情報 */
		struct Info {
			uint64_t failHigh;
			uint64_t failHighFirst;
			uint64_t hashProbed;
			uint64_t hashExact;
			uint64_t hashLower;
			uint64_t hashUpper;
			uint64_t shekProbed;
			uint64_t shekSuperior;
			uint64_t shekInferior;
			uint64_t shekEqual;
			uint64_t nullMovePruning;
			uint64_t nullMovePruningTried;
			uint64_t futilityPruning;
			uint64_t extendedFutilityPruning;
			uint64_t expanded;
			uint64_t checkExtension;
			uint64_t onerepExtension;
			uint64_t recapExtension;
			uint64_t node;
			double time;
			double nps;
			Move move;
			Value eval;
		};

	private:

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

		/** record */
		std::vector<Move> _record;

		/** 中断フラグ */
		std::atomic<bool> _forceInterrupt;

		/** 実行中フラグ */
		std::atomic<bool> _isRunning;

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
		void reallocateTrees() {
			if (_trees != nullptr) {
				delete[] _trees;
			}
			_trees = new Tree[_config.treeSize];
		}

		/**
		 * 前処理
		 */
		void before(const Board& initialBoard);

		/**
		 * 後処理
		 */
		void after(const Board& initialBoard);

		/**
		 * 探索中断判定
		 */
		bool isInterrupted();

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
		void sortSee(Tree& tree, bool plusOnly = false, bool exceptSmallCapture = false);

		/**
		 * sort moves by history
		 */
		void sortHistory(Tree& tree);

		/**
		 * update history
		 */
		void updateHistory(Tree& tree, int depth, const Move& move);

		/**
		 * get LMR depth
		 */
		int getReductionDepth(const Move& move, bool isNullWindow);

		/**
		 * get next move
		 */
		bool nextMove(Tree& tree, Move& move);

		/**
		 * get next move
		 */
		bool nextMoveQuies(Tree& tree, Move& move, int ply);

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
		 * @return {負けたか中断された場合にfalseを返します。}
		 */
		bool search(int depth, Move& best, bool gen = true, Value* prevval = nullptr);

		void showPv(int depth, const Pv& pv, const Value& value);

		/**
		 * iterative deepening search from root node
		 * @return {負けたか深さ1で中断された場合にfalseを返します。}
		 */
		bool idsearch(Move& best);

	public:

		/**
		 * コンストラクタ
		 */
		Searcher()
		: _trees(nullptr)
		, _forceInterrupt(false)
		, _isRunning(false)
		{
			initConfig();
			_history.init();
		}

		/**
		 * 初期化
		 */
		void init() {
			reallocateTrees();
		}

		/**
		 * 設定を反映します。
		 */
		void setConfig(const Config& config) {
			auto org = _config;
			_config = config;
			if (_config.treeSize != org.treeSize) {
				reallocateTrees();
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
		 * SHEK と千日手検出のための過去の棋譜をクリアします。
		 */
		void clearRecord();

		/**
		 * SHEK と千日手検出のために過去の棋譜をセットします。
		 */
		void setRecord(const Record& record);

		/**
		 * 探索を強制的に打ち切ります。
		 */
		void forceInterrupt();

		/**
		 * 探索中かチェックします。
		 */
		bool isRunning() const {
			return _isRunning;
		}

		/**
		 * 指定した局面に対して探索を実行します。
		 * @return {負けたいか中断された場合にfalseを返します。}
		 */
		virtual bool search(const Board& initialBoard, Move& best) override final;

		/**
		 * 指定した局面に対して反復深化探索を実行します。
		 * @return {負けたか深さ1で中断された場合にfalseを返します。}
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
