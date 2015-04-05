/* Searcher.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEARCHER__
#define __SUNFISH_SEARCHER__

#include "Mate.h"
#include "see/See.h"
#include "tree/Tree.h"
#include "tree/NodeStat.h"
#include "history/History.h"
#include "tt/TT.h"
#include "time/TimeManager.h"
#include "core/record/Record.h"
#include "core/util/Timer.h"
#include <atomic>

namespace sunfish {

	class Searcher {
	public:

		static CONSTEXPR int Depth1Ply = 8;

		/** 探索設定 */
		struct Config {
			int maxDepth;
			int treeSize;
			int wokerSize;
			bool limitEnable;
			int limitSeconds;
			bool ponder;
		};

		static const int DefaultMaxDepth = 7;

		/** 探索情報 */
		struct Info {
			uint64_t failHigh;
			uint64_t failHighFirst;
			uint64_t failHighIsHash;
			uint64_t failHighIsKiller1;
			uint64_t failHighIsKiller2;
			uint64_t hashProbed;
			uint64_t hashHit;
			uint64_t hashExact;
			uint64_t hashLower;
			uint64_t hashUpper;
			uint64_t hashStore;
			uint64_t hashNew;
			uint64_t hashUpdate;
			uint64_t hashCollision;
			uint64_t hashReject;
			uint64_t mateProbed;
			uint64_t mateHit;
			uint64_t expand;
			uint64_t expandHashMove;
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
			uint64_t qnode;
			double time;
			double nps;
			Move move;
			Value eval;
			int lastDepth;
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

		/** mate table */
		MateTable _mt;

		/** record */
		std::vector<Move> _record;

		/** values of child node of root node */
		int _rootValues[1024];

		/** 中断フラグ */
		std::atomic<bool> _forceInterrupt;

		/** 実行中フラグ */
		std::atomic<bool> _isRunning;

		/** 思考時間制御 */
		TimeManager _timeManager;

		/**
		 * 設定の初期化
		 */
		void initConfig() {
			_config.maxDepth = DefaultMaxDepth;
			_config.treeSize = 1;
			_config.wokerSize = 1;
			_config.limitEnable = true;
			_config.limitSeconds = 10;
			_config.ponder = false;
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
		void after();

		/**
		 * 探索中断判定
		 */
		bool isInterrupted();

		/**
		 * sort moves by see
		 */
		void sortSee(Tree& tree, int offset, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies);

		/**
		 * except prior moves
		 */
		void exceptPriorMoves(Tree& tree);

		/**
		 * pick best move by history
		 */
		bool pickOneHistory(Tree& tree);

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
		bool nextMove(Tree& tree);

		/**
		 * get next move
		 */
		bool nextMoveQuies(Tree& tree, int ply, Value standPat, Value alpha);

		/**
		 * store PV-nodes to TT
		 */
		void storePv(Tree& tree, const Pv& pv, int ply);

		/**
		 * quiesence search
		 */
		Value qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta);

		/**
		 * update killer move
		 */
		void updateKiller(Tree& tree, const Move& move);

		/**
		 * nega-max search
		 */
		Value search(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat = NodeStat::Default);

		/**
		 * search on root node
		 */
		Value searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best, bool forceFullWindow = false);

		/**
		 * aspiration search
		 * @return {負けたか中断された場合にfalseを返します。}
		 */
		bool searchAsp(int depth, Move& best, Value* pval = nullptr);

		void showPv(int depth, const Pv& pv, const Value& value);

		void showEndOfIterate(int depth);

		void generateMovesOnRoot();

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
		bool search(const Board& initialBoard, Move& best);

		/**
		 * 指定した局面に対して反復深化探索を実行します。
		 * @return {負けたか深さ1で中断された場合にfalseを返します。}
		 */
		bool idsearch(const Board& initialBoard, Move& best);

		/**
		 * Evaluator を取得します。
		 */
		Evaluator& getEvaluator() {
			return _eval;
		}

		/**
		 * TT をクリアします。
		 */
		void clearTT() {
			_tt.init();
		}

		/**
		 * historyをクリアします。
		 */
		void clearHistory() {
			_history.init();
		}

	};

}

#endif //__SUNFISH_SEARCHER__
