/* Searcher.cpp
 *
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "tree/Worker.h"
#include "tree/NodeStat.h"
#include "see/See.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <iomanip>

#define ENABLE_HIST_REUSE							1
#define ENABLE_LMR										1
#define ENABLE_SMOOTH_FUT							1
#define ENABLE_RAZORING								1
#define ENABLE_MOVE_COUNT_PRUNING			0 // should be 0
#define ENABLE_HASH_MOVE							1
#define ENABLE_KILLER_MOVE						1
#define ENABLE_SHEK										1
#define ENABLE_MATE_1PLY							1
#define ENABLE_STORE_PV								1
#define SHALLOW_SEE                   0 // should be 0

// debugging flags
#define DEBUG_ROOT_MOVES							0
#define DEBUG_TREE										0
#define DEBUG_NODE										0

#define ITERATE_INFO_THRESHOLD        3 // must be greater than or equal to 2

namespace sunfish {

	namespace search_param {
#if ENABLE_SMOOTH_FUT
		CONSTEXPR int FUT_DEPTH = Searcher::Depth1Ply * 9;
#else
		CONSTEXPR int FUT_DEPTH = Searcher::Depth1Ply * 3;
#endif
		CONSTEXPR int EXT_CHECK = Searcher::Depth1Ply;
		CONSTEXPR int EXT_ONEREP = Searcher::Depth1Ply * 1 / 2;
		CONSTEXPR int EXT_RECAP = Searcher::Depth1Ply * 1 / 4;
		CONSTEXPR int EXT_RECAP2 = Searcher::Depth1Ply * 1 / 2;
		CONSTEXPR int REC_THRESHOLD = Searcher::Depth1Ply * 3;
		CONSTEXPR int RAZOR_DEPTH = Searcher::Depth1Ply * 4;
		CONSTEXPR int QUIES_RELIEVE_PLY = 7;
	}

	namespace search_func {
		inline int recDepth(int depth) {
			return (depth < Searcher::Depth1Ply * 9 / 2 ?
							Searcher::Depth1Ply * 3 / 2 :
							depth - Searcher::Depth1Ply * 3);
		}
		inline int nullDepth(int depth) {
			return (depth < Searcher::Depth1Ply * 26 / 4 ? depth - Searcher::Depth1Ply * 12 / 4 :
							(depth <= Searcher::Depth1Ply * 30 / 4 ? Searcher::Depth1Ply * 14 / 4 : depth - Searcher::Depth1Ply * 16 / 4));
		}
		inline int futilityMargin(int depth, int count) {
#if ENABLE_SMOOTH_FUT
			return (depth < Searcher::Depth1Ply * 3 ? 800 :
							128 + 270 / Searcher::Depth1Ply * std::max(depth, 0)) - 8 * count;
#else
			return 800 - 8 * count;
#endif
		}
		inline int razorMargin(int depth) {
			return 512 + 32 / Searcher::Depth1Ply * std::max(depth, 0);
		}
		inline int futilityMoveCounts(bool improving, int depth) {
			int d = (depth * depth) / (Searcher::Depth1Ply * Searcher::Depth1Ply);
			if (improving) {
				d = d * 19 / 10;
			} else {
				d = d * 13 / 10;
			}
			return 18 + d;
		}
	}

	/**
	 * コンストラクタ
	 */
	Searcher::Searcher()
	: _trees(nullptr)
	, _workers(nullptr)
	, _forceInterrupt(false)
	, _isRunning(false) {
		initConfig();
		_history.init();
	}

	/**
	 * デストラクタ
	 */
	Searcher::~Searcher() {
		releaseTrees();
		releaseWorkers();
	}

	/**
	 * tree の確保
	 */
	void Searcher::allocateTrees() {
		if (_trees == nullptr) {
			_trees = new Tree[_config.treeSize];
		}
	}

	/**
	 * worker の確保
	 */
	void Searcher::allocateWorkers() {
		if (_workers == nullptr) {
			_workers = new Worker[_config.workerSize];
		}
	}

	/**
	 * tree の再確保
	 */
	void Searcher::reallocateTrees() {
		if (_trees != nullptr) {
			delete[] _trees;
		}
		_trees = new Tree[_config.treeSize];
	}

	/**
	 * worker の再確保
	 */
	void Searcher::reallocateWorkers() {
		if (_workers != nullptr) {
			delete[] _workers;
		}
		_workers = new Worker[_config.workerSize];
	}

	/**
	 * tree の解放
	 */
	void Searcher::releaseTrees() {
		if (_trees != nullptr) {
			delete[] _trees;
			_trees = nullptr;
		}
	}

	/**
	 * worker の解放
	 */
	void Searcher::releaseWorkers() {
		if (_workers != nullptr) {
			delete[] _workers;
			_workers = nullptr;
		}
	}

	/**
	 * worker の取得
	 */
	Worker& Searcher::getWorker(Tree& tree) {
		return _workers[tree.getTlp().workerId];
	}

	void Searcher::mergeInfo() {
		memset(&_info, 0, sizeof(SearchInfoBase));
		for (int id = 0; id < _config.workerSize; id++) {
			auto& worker = _workers[id];
			_info.failHigh                   += worker.info.failHigh;
			_info.failHighFirst              += worker.info.failHighFirst;
			_info.failHighIsHash             += worker.info.failHighIsHash;
			_info.failHighIsKiller1          += worker.info.failHighIsKiller1;
			_info.failHighIsKiller2          += worker.info.failHighIsKiller2;
			_info.hashProbed                 += worker.info.hashProbed;
			_info.hashHit                    += worker.info.hashHit;
			_info.hashExact                  += worker.info.hashExact;
			_info.hashLower                  += worker.info.hashLower;
			_info.hashUpper                  += worker.info.hashUpper;
			_info.hashStore                  += worker.info.hashStore;
			_info.hashNew                    += worker.info.hashNew;
			_info.hashUpdate                 += worker.info.hashUpdate;
			_info.hashCollision              += worker.info.hashCollision;
			_info.hashReject                 += worker.info.hashReject;
			_info.mateProbed                 += worker.info.mateProbed;
			_info.mateHit                    += worker.info.mateHit;
			_info.expand                     += worker.info.expand;
			_info.expandHashMove             += worker.info.expandHashMove;
			_info.shekProbed                 += worker.info.shekProbed;
			_info.shekSuperior               += worker.info.shekSuperior;
			_info.shekInferior               += worker.info.shekInferior;
			_info.shekEqual                  += worker.info.shekEqual;
			_info.nullMovePruning            += worker.info.nullMovePruning;
			_info.nullMovePruningTried       += worker.info.nullMovePruningTried;
			_info.futilityPruning            += worker.info.futilityPruning;
			_info.extendedFutilityPruning    += worker.info.extendedFutilityPruning;
			_info.moveCountPruning           += worker.info.moveCountPruning;
			_info.razoring                   += worker.info.razoring;
			_info.razoringTried              += worker.info.razoringTried;
			_info.expanded                   += worker.info.expanded;
			_info.checkExtension             += worker.info.checkExtension;
			_info.onerepExtension            += worker.info.onerepExtension;
			_info.recapExtension             += worker.info.recapExtension;
			_info.split                      += worker.info.split;
			_info.node                       += worker.info.node;
			_info.qnode                      += worker.info.qnode;
		}
	}

	/**
	 * 前処理
	 */
	void Searcher::before(const Board& initialBoard) {

		if (_isRunning.load()) {
			Loggers::error << __FILE_LINE__ << ": Searcher is already running!!!";
		}

		int mainTreeId = 0;
		int mainWorkerId = 0;

		allocateTrees();
		allocateWorkers();

		// tree の初期化
		for (int id = 0; id < _config.treeSize; id++) {
			auto& tree = _trees[id];
			tree.init(id, initialBoard, _eval, _record);
		}
		_idleTreeCount.store(_config.treeSize - 1);

		// worker の初期化
		for (int id = 0; id < _config.workerSize; id++) {
			auto& worker = _workers[id];
			worker.init(id, this);
			if (id != mainWorkerId) {
				worker.startOnNewThread();
			}
		}
		_idleWorkerCount.store(_config.workerSize - 1);

		// 最初の tree を確保
		auto& tree0 = _trees[mainTreeId];
		auto& worker0 = _workers[mainWorkerId];
		tree0.use(mainWorkerId);
		worker0.startOnCurrentThread(mainTreeId);

		// timer 初期化
		_timer.set();

		// transposition table
		_tt.evolve(); // 世代更新

		// hisotory heuristic
#if ENABLE_HIST_REUSE
		_history.reduce();
#else
		_history.init();
#endif

		// gains
		_gains.clear();

		_forceInterrupt.store(false);
		_isRunning.store(true);

		_timeManager.init();
	}

	/**
	 * 後処理
	 */
	void Searcher::after() {

		if (!_isRunning.load()) {
			Loggers::error << __FILE_LINE__ << ": Searcher is not running???";
		}

		// worker の停止
		for (int id = 1; id < _config.workerSize; id++) {
			auto& worker = _workers[id];
			worker.stop();
		}

		// tree の解放
		for (int id = 0; id < _config.treeSize; id++) {
			auto& tree = _trees[id];
			tree.release(_record);
		}

		// 探索情報収集
		auto& tree0 = _trees[0];
		_info.time = _timer.get();
		_info.nps = (_info.node + _info.qnode) / _info.time;
		_info.move = tree0.getPv().get(0).move;
		mergeInfo();

		_isRunning.store(false);
		_forceInterrupt.store(false);

	}

	/**
	 * SHEK と千日手検出のための過去の棋譜をクリアします。
	 */
	void Searcher::clearRecord() {
		_record.clear();
	}

	/**
	 * SHEK と千日手検出のために過去の棋譜をセットします。
	 */
	void Searcher::setRecord(const Record& record) {
		for (unsigned i = 0; i < record.getCount(); i++) {
			_record.push_back(record.getMoveAt(i));
		}
	}

	/**
	 * 探索中断判定
	 */
	inline bool Searcher::isInterrupted(Tree& tree) {
		if (tree.getTlp().shutdown.load()) {
		}
		if (_forceInterrupt.load()) {
			return true;
		}
		if (_config.limitEnable && _timer.get() >= _config.limitSeconds) {
			return true;
		}
		return false;
	}

	/**
	 * 探索を強制的に打ち切ります。
	 */
	void Searcher::forceInterrupt() {
		_forceInterrupt.store(true);
	}

	/**
	 * get see value
	 */
	template <bool shallow>
	Value Searcher::searchSee(const Board& board, const Move& move, Value alpha, Value beta) {
		Value value;
		uint64_t hash;

		if (!shallow) {
			hash = board.getHash() ^ (uint64_t)Move::serialize(move);
			if (_seeCache.get(hash, value, alpha, beta)) {
				return value;
			}
		}

		See see;
		value = see.search<shallow>(board, move, alpha, beta);

		if (!shallow) {
			_seeCache.set(hash, value, alpha, beta);
		}

		return value;
	}

	/**
	 * sort moves by see
	 */
	void Searcher::sortSee(Tree& tree, int offset, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies) {
		const auto& board = tree.getBoard();
		auto& node = tree.getCurrentNode();
		auto& worker = getWorker(tree);
#if !ENABLE_KILLER_MOVE
		assert(node.killer1.isEmpty());
		assert(node.killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE
		assert(offset == 0 || offset == 1);
		assert(tree.getNextMove() + offset <= tree.getEnd());
		if (enableKiller) {
			node.capture1 = Move::empty();
			node.capture2 = Move::empty();
			node.cvalue1 = -Value::Inf;
			node.cvalue2 = -Value::Inf;
		}

		for (auto ite = tree.getNextMove() + offset; ite != tree.getEnd(); ) {
			const Move& move = *ite;
			Value value;

			if (exceptSmallCapture) {
				auto captured = tree.getBoard().getBoardPiece(move.to()).kindOnly();
				if ((captured == Piece::Pawn && !move.promote()) ||
						(captured.isEmpty() && move.piece() != Piece::Pawn)) {
					tree.setSortValue(ite, -Value::Inf);
					ite++;
					continue;
				}
			}

			if (isQuies) {
				// futility pruning
				if (standPat + tree.estimate(move, _eval) + _gains.get(move) <= alpha) {
					worker.info.futilityPruning++;
					ite = tree.getMoves().remove(ite);
					continue;
				}
			}

#if SHALLOW_SEE
			value = searchSee<true>(board, move, -1, Value::PieceInf);
#else
			value = searchSee<false>(board, move, -1, Value::PieceInf);
#endif
			if (estimate) {
				value += tree.estimate<true>(move, _eval);
			}

			if (enableKiller) {
				if (value > node.cvalue1) {
					node.capture2 = node.capture1;
					node.cvalue2 = node.cvalue1;
					node.capture1 = move;
					node.cvalue1 = value;
				} else if (value > node.cvalue2) {
					node.capture2 = move;
					node.cvalue2 = value;
				}
			}

			if (!isQuies) {
				if ((node.expStat & HashDone) && move == node.hash) {
					ite = tree.getMoves().remove(ite);
					continue;
				}
			}

			if (enableKiller) {
				if (move == node.killer1) {
					node.expStat |= Killer1Added;
					auto captured = board.getBoardPiece(move.to());
					Value kvalue = tree.getKiller1Value() + material::pieceExchange(captured);
					value = Value::max(value, kvalue);
				} else if (move == node.killer2) {
					node.expStat |= Killer2Added;
					auto captured = board.getBoardPiece(move.to());
					Value kvalue = tree.getKiller2Value() + material::pieceExchange(captured);
					value = Value::max(value, kvalue);
				}
			}

			tree.setSortValue(ite, value.int32());

			ite++;
		}

		if (enableKiller) {
			if (!(node.expStat & Killer1Added) && node.killer1 != node.hash
					&& tree.getKiller1Value() >= Value::Zero
					&& board.isValidMoveStrict(node.killer1)) {
				node.expStat |= Killer1Added;
				auto ite = tree.addMove(node.killer1);
				Value kvalue = tree.getKiller1Value();
				tree.setSortValue(ite, kvalue.int32());
			}
			if (!(node.expStat & Killer2Added) && node.killer2 != node.hash
					&& tree.getKiller2Value() >= Value::Zero
					&& board.isValidMoveStrict(node.killer2)) {
				node.expStat |= Killer2Added;
				auto ite = tree.addMove(node.killer2);
				Value kvalue = tree.getKiller2Value();
				tree.setSortValue(ite, kvalue.int32());
			}
		}

		tree.sortAfterCurrent(offset);

		if (isQuies) {
			for (auto ite = tree.getNextMove() + offset; ite != tree.getEnd(); ite++) {
				if (tree.getSortValue(ite) < 0) {
					tree.removeAfter(ite);
					break;
				}
			}
		}

	}

	/**
	 * except prior moves
	 */
	void Searcher::exceptPriorMoves(Tree& tree) {
		auto& node = tree.getCurrentNode();
#if !ENABLE_KILLER_MOVE
		assert(node.killer1.isEmpty());
		assert(node.killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE

		for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
			const Move& move = *ite;

			if ((node.expStat & HashDone) && move == node.hash) {
				ite = tree.getMoves().remove(ite);
				continue;
			}
			if (((node.expStat & Killer1Done) && move == node.killer1) ||
					((node.expStat & Killer2Done) && move == node.killer2)) {
				ite = tree.getMoves().remove(ite);
				continue;
			}

			ite++;
		}
	}

	/**
	 * pick best move by history
	 */
	bool Searcher::pickOneHistory(Tree& tree) {
		Moves::iterator best = tree.getEnd();
		uint32_t bestValue = 0;

		for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
			const Move& move = *ite;

			auto key = History::getKey(move);
			auto data = _history.getData(key);
			auto value = History::getRatio(data);
			if (value > bestValue) {
				best = ite;
				bestValue = value;
			}

			ite++;
		}

		if (best != tree.getEnd()) {
			Move temp = *tree.getNextMove();
			*tree.getNextMove() = *best;
			*best = temp;
			return true;
		}

		return false;
	}

	/**
	 * sort moves by history
	 */
	void Searcher::sortHistory(Tree& tree) {
		for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
			const Move& move = *ite;

			auto key = History::getKey(move);
			auto data = _history.getData(key);
			auto ratio = History::getRatio(data);
			tree.setSortValue(ite, (int32_t)ratio);

			ite++;
		}

		tree.sortAfterCurrent();

	}

	/**
	 * update history
	 */
	void Searcher::updateHistory(Tree& tree, int depth, const Move& move) {

		CONSTEXPR int HistPerDepth = 8;
		int value = std::max(depth * HistPerDepth / Depth1Ply, 1);
		const auto& moves = tree.getCurrentNode().histMoves;
		for (auto ite = moves.begin(); ite != moves.end(); ite++) {
			assert(ite != tree.getEnd());
			auto key = History::getKey(*ite);
			if (ite->equals(move)) {
				_history.add(key, value, value);
			} else {
				_history.add(key, value, 0);
			}

		}

	}

	/**
	 * get LMR depth
	 */
	int Searcher::getReductionDepth(bool improving, int depth, int count, const Move& move, bool isNullWindow) {
		auto key = History::getKey(move);
		auto data = _history.getData(key);
		auto good = History::getGoodCount(data) + 1;
		auto appear = History::getAppearCount(data) + 2;

		assert(good < appear);

		int reduced = 0;

		if (!isNullWindow) {
			if (good * 20 < appear) {
				reduced += Depth1Ply * 3 / 2;
			} else if (good * 7 < appear) {
				reduced += Depth1Ply * 2 / 2;
			} else if (good * 3 < appear) {
				reduced += Depth1Ply * 1 / 2;
			}
		} else {
			if (good * 10 < appear) {
				reduced += Depth1Ply * 4 / 2;
			} else if (good * 6 < appear) {
				reduced += Depth1Ply * 3 / 2;
			} else if (good * 4 < appear) {
				reduced +=  Depth1Ply * 2 / 2;
			} else if (good * 2 < appear) {
				reduced += Depth1Ply * 1 / 2;
			}
		}

		if (!improving && depth < Depth1Ply * 9) {
			if (count >= 24) {
				reduced += Depth1Ply * 4 / 2;
			} else if (count >= 18) {
				reduced += Depth1Ply * 3 / 2;
			} else if (count >= 12) {
				reduced += Depth1Ply * 2 / 2;
			} else if (count >= 6) {
				reduced += Depth1Ply * 1 / 2;
			}
		}

		return reduced;
	}

	/**
	 * get next move
	 */
	bool Searcher::nextMove(Tree& tree) {

		auto& moves = tree.getMoves();
		auto& node = tree.getCurrentNode();
		auto& worker = getWorker(tree);
		const auto& board = tree.getBoard();

		while (true) {

			if (!tree.isThroughPhase() && tree.getNextMove() != tree.getEnd()) {
				tree.selectNextMove();
				node.count++;

				const Move& move = *tree.getCurrentMove();
				if (move == node.hash) {
					node.expStat |= HashDone;
				} else if (move == node.killer1) {
					node.expStat |= Killer1Done;
				} else if (move == node.killer2) {
					node.expStat |= Killer2Done;
				} else if (move == node.capture1) {
					node.expStat |= Capture1Done;
				} else if (move == node.capture2) {
					node.expStat |= Capture2Done;
				}

				return true;
			}

			switch (node.genPhase) {
			case GenPhase::Hash:
				{
					Move hashMove = node.hash;
					if (!hashMove.isEmpty() && board.isValidMoveStrict(hashMove)) {
						tree.addMove(hashMove);
						tree.setThroughPhase(true);
						worker.info.expandHashMove++;
					}
					node.genPhase = GenPhase::Capture;
				}
				break;

			case GenPhase::Capture:
				tree.setThroughPhase(false);
				if (tree.isChecking()) {
					int offset = moves.end() - tree.getNextMove();
					MoveGenerator::generateEvasion(board, moves);
					sortSee(tree, offset, Value::Zero, Value::Zero, false, true, false, false);
					node.genPhase = GenPhase::End;
					break;
					
				} else {
					int offset = moves.end() - tree.getNextMove();
					MoveGenerator::generateCap(board, moves);
					sortSee(tree, offset, Value::Zero, Value::Zero, true, false, false, false);
					node.genPhase = GenPhase::History1;
					break;

				}

			case GenPhase::History1:
				node.count = 0;
				MoveGenerator::generateNoCap(board, moves);
				MoveGenerator::generateDrop(board, moves);
				exceptPriorMoves(tree);
				node.genPhase = GenPhase::History2;
				tree.setThroughPhase(true);
				if (pickOneHistory(tree)) {
					tree.selectNextMove();
					return true;
				}
				break;

			case GenPhase::History2:
				node.genPhase = GenPhase::Misc;
				tree.setThroughPhase(true);
				if (pickOneHistory(tree)) {
					tree.selectNextMove();
					return true;
				}
				break;

			case GenPhase::Misc:
				sortHistory(tree);
				node.genPhase = GenPhase::End;
				tree.setThroughPhase(false);
				break;

			case GenPhase::CaptureOnly:
				assert(false);

			case GenPhase::End:
				return false;
			}
		}

	}

	/**
	 * get next move
	 */
	bool Searcher::nextMoveQuies(Tree& tree, int qply, Value standPat, Value alpha) {

		auto& moves = tree.getMoves();
		auto& node = tree.getCurrentNode();
		const auto& board = tree.getBoard();

		while (true) {

			if (tree.getNextMove() != tree.getEnd()) {
				tree.selectNextMove();
				return true;
			}

			switch (node.genPhase) {
			case GenPhase::Hash: // fall through
			case GenPhase::Capture: // fall through
			case GenPhase::History1:
			case GenPhase::History2:
			case GenPhase::Misc:
				assert(false);
				break;

			case GenPhase::CaptureOnly:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortHistory(tree);
					node.genPhase = GenPhase::End;
					break;

				} else {
					MoveGenerator::generateCap(board, moves);
					if (qply >= search_param::QUIES_RELIEVE_PLY) {
  					sortSee(tree, 0, standPat, alpha, false, false, true, true);
					} else {
  					sortSee(tree, 0, standPat, alpha, false, false, false, true);
					}
					node.genPhase = GenPhase::End;
					break;

				}

			case GenPhase::End:
				return false;
			}
		}

	}

	/**
	 * store PV-nodes to TT
	 */
	void Searcher::storePv(Tree& tree, const Pv& pv, int ply) {
		if (ply >= pv.size()) {
			return;
		}

		int depth = pv.get(ply).depth;
		if (depth <= 0) {
			return;
		}

		const auto& move = pv.get(ply).move;
		if (move.isEmpty()) {
			return;
		}

		if (tree.makeMoveFast(move)) {
			storePv(tree, pv, ply + 1);
			tree.unmakeMoveFast();
		}

		auto hash = tree.getBoard().getHash();
		_tt.entryPv(hash, depth, Move::serialize16(move));
	}

	/**
	 * quiesence search
	 */
	Value Searcher::qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta) {

#if DEBUG_NODE
		bool debug = false;
#if 0
		if (tree.__debug__matchPath("-4233GI +5968OU -8586FU")) {
			std::cout << "#-- debug quies node begin --#" << std::endl;
			std::cout << tree.__debug__getPath() << std::endl;
			debug = true;
		}
#endif
#endif

#if DEBUG_TREE
		{
			for (int i = 0; i < tree.getPly(); i++) {
				std::cout << ' ';
			}
			std::cout << tree.__debug__getFrontMove().toString() << std::endl;
		}
#endif

		auto& worker = getWorker(tree);
		worker.info.qnode++;

		// stand-pat
		Value standPat = tree.getValue() * (black ? 1 : -1);

		// beta-cut
		if (standPat >= beta) {
			return standPat;
		}

		// スタックサイズの限界
		if (tree.isStackFull()) {
			return standPat;
		}

#if ENABLE_MATE_1PLY
		{
			// search mate in 1 ply
			bool mate;
			worker.info.mateProbed++;
			if (_mt.get(tree.getBoard().getHash(), mate)) {
				worker.info.mateHit++;
			} else {
				mate = Mate::mate1Ply(tree.getBoard());
				_mt.set(tree.getBoard().getHash(), mate);
			}
			if (mate) {
				return Value::Inf - tree.getPly() - 1;
			}
		}
#endif

		alpha = Value::max(alpha, standPat);

		// 合法手生成
		auto& moves = tree.getMoves();
		moves.clear();
		tree.initGenPhase(GenPhase::CaptureOnly);

		while (nextMoveQuies(tree, qply, standPat, alpha)) {
			// make move
			if (!tree.makeMove(_eval)) {
				continue;
			}

			// reccursive call
			Value currval;
			currval = -qsearch(tree, !black, qply + 1, -beta, -alpha);

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted(tree)) {
				return Value::Zero;
			}

			// 値更新
			if (currval > alpha) {
				alpha = currval;
				tree.updatePv(0);

				// beta-cut
				if (currval >= beta) {
					break;
				}
			}
		}

		return alpha;

	}

	void Searcher::updateKiller(Tree& tree, const Move& move) {
		auto& node = tree.getCurrentNode();
		const auto& board = tree.getBoard();

		Piece captured = board.getBoardPiece(move.to());
		Value capVal = material::pieceExchange(captured);

		if (move == node.capture1) {
			if ((node.expStat & Killer1Done) && move != node.killer1) {
				node.kvalue1 = node.cvalue1 - capVal - 1;
			}
			if ((node.expStat & Killer2Done) && move != node.killer2) {
				node.kvalue2 = node.cvalue1 - capVal - 1;
			}

		} else if (move == node.killer1) {
			if ((node.expStat & Capture1Done) &&
					node.kvalue2 + capVal <= node.cvalue1) {
				node.kvalue2 = node.cvalue1 - capVal + 1;
			}
			Piece captured2 = board.getBoardPiece(node.killer2.to());
			Value capVal2 = material::pieceExchange(captured2);
			if ((node.expStat & Killer2Done) &&
					node.kvalue1 + capVal <= node.kvalue2 + capVal2) {
				node.kvalue1 = node.kvalue2 + capVal2 - capVal + 1;
			}

		} else if (move == node.killer2) {
			if ((node.expStat & Capture1Done) &&
					node.kvalue2 + capVal <= node.cvalue1) {
				node.kvalue2 = node.cvalue1 - capVal + 1;
			}
			Piece captured1 = board.getBoardPiece(node.killer1.to());
			Value capVal1 = material::pieceExchange(captured1);
			if ((node.expStat & Killer1Done) &&
					node.kvalue2 + capVal <= node.kvalue1 + capVal1) {
				node.kvalue2 = node.kvalue1 + capVal1 - capVal + 1;
			}
			Move::swap(node.killer1, node.killer2);
			Value::swap(node.kvalue1, node.kvalue2);

		} else {
			if (node.expStat & Killer1Done) {
				Value val = searchSee<false>(board, move, -1, Value::PieceInf) - capVal + 1;
				node.kvalue1 = Value::min(node.kvalue1, val);
			}
			node.killer2 = node.killer1;
			node.kvalue2 = node.kvalue1;
			node.killer1 = move;
			node.kvalue1 = node.cvalue1 - capVal + 1;

		}

		if (tree.getBoard().getBoardPiece(move.to()).isEmpty() &&
				(!move.promote() || move.piece() == Piece::Silver)) {
			node.nocap2 = node.nocap1;
			node.nocap1 = move;
		}
	}

	/**
	 * nega-max search
	 */
	Value Searcher::search(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat) {

#if DEBUG_TREE
		{
			for (int i = 0; i < tree.getPly(); i++) {
				std::cout << ' ';
			}
			std::cout << tree.__debug__getFrontMove().toString() << std::endl;
		}
#endif

#if DEBUG_NODE
		bool debug = false;
#if 0
		if (tree.__debug__matchPath("-0095KA")) {
			std::cout << "#-- debug node begin --#" << std::endl;
			std::cout << tree.__debug__getPath() << std::endl;
			std::cout << "alpha=" << alpha.int32() << " beta=" << beta.int32() << " depth=" << depth << std::endl;
			debug = true;
		}
#endif
#endif

#if ENABLE_SHEK

		auto& worker = getWorker(tree);

		// SHEK
		ShekStat shekStat = tree.checkShek();
		worker.info.shekProbed++;
		switch (shekStat) {
			case ShekStat::Superior:
				// 既出の局面に対して優位な局面
				worker.info.shekSuperior++;
				return Value::Inf - tree.getPly();

			case ShekStat::Inferior:
				// 既出の局面に対して劣る局面
				worker.info.shekInferior++;
				return -Value::Inf + tree.getPly();

			case ShekStat::Equal:
				worker.info.shekEqual++;
				switch(tree.getCheckRepStatus()) {
				case RepStatus::Win:
					return Value::Inf - tree.getPly();
				case RepStatus::Lose:
					return -Value::Inf + tree.getPly();
				case RepStatus::None:
					assert(false);
				default:
					return Value::Zero;
				}

			default:
				break;
		}
#endif

		// スタックサイズの限界
		if (tree.isStackFull()) {
			return tree.getValue() * (black ? 1 : -1);
		}

		// 静止探索の結果を返す。
		if (!tree.isChecking() && depth < Depth1Ply) {
			return qsearch(tree, black, 0, alpha, beta);
		}

		Value oldAlpha = alpha;

		// distance pruning
		{
			Value value = -Value::Inf + tree.getPly();
			if (value > alpha) {
				if (value >= beta) {
					return value;
				}
				alpha = value;
			} else {
				value = Value::Inf - tree.getPly() - 1;
				if (value <= alpha) {
					return value;
				}
			}
		}

		worker.info.node++;

		uint64_t hash = tree.getBoard().getHash();
		bool isNullWindow = (beta == alpha + 1);

		// transposition table
		Move hashMove = Move::empty();
		{
			TTE tte;
			worker.info.hashProbed++;
			if (_tt.get(hash, tte)) {
				if (depth < search_param::REC_THRESHOLD ||
						tte.getDepth() >= search_func::recDepth(depth)) {
					auto ttv = tte.getValue(tree.getPly());
					auto valueType = tte.getValueType();

					// 前回の結果で枝刈り
					if (stat.isHashCut() && isNullWindow) {
						// 現在のノードに対して優位な条件の場合
						if (tte.getDepth() >= depth ||
								((valueType == TTE::Lower || valueType == TTE::Exact) && ttv >= Value::Mate) ||
								((valueType == TTE::Upper || valueType == TTE::Exact) && ttv <= -Value::Mate)) {
							if (valueType == TTE::Exact) {
								// 確定値
								worker.info.hashExact++;
								return ttv;
							} else if (valueType == TTE::Lower && ttv >= beta) {
								// 下界値
								worker.info.hashLower++;
								return ttv;
							} else if (valueType == TTE::Upper && ttv <= alpha) {
								// 上界値
								worker.info.hashUpper++;
								return ttv;
							}
						}
						// 十分なマージンを加味して beta 値を超える場合
						if ((valueType == TTE::Lower || valueType == TTE::Exact) &&
								!tree.isChecking() && !tree.isCheckingOnFrontier()) {
							if (depth < search_param::FUT_DEPTH && ttv >= beta + search_func::futilityMargin(depth, 0)) {
								return beta;
							}
						}
					}

					if (valueType == TTE::Upper || valueType == TTE::Exact) {
						// alpha 値を割るなら recursion 不要
						if (ttv <= alpha && tte.getDepth() >= search_func::recDepth(depth)) {
							stat.unsetRecursion();
						}
						// beta を超えないなら null move pruning を省略
						if (ttv < beta && tte.getDepth() >= search_func::nullDepth(depth)) {
							stat.unsetNullMove();
						}
					}

					// 前回の最善手を取得
					hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());

					if (tte.isMateThreat()) {
						stat.setMateThreat();
					}
				}
				worker.info.hashHit++;
			}
		}

		Value standPat = tree.getValue() * (black ? 1 : -1);
		bool improving = !tree.hasPrefrontierNode() ||
			standPat >= tree.getPrefrontValue() * (black ? 1 : -1);

		bool isFirst = true;
		Move best = Move::empty();

		if (!tree.isChecking()) {

#if ENABLE_MATE_1PLY
			if (stat.isMate()) {
				// search mate in 1 ply
				bool mate;
				worker.info.mateProbed++;
				if (_mt.get(tree.getBoard().getHash(), mate)) {
					worker.info.mateHit++;
				} else {
					mate = Mate::mate1Ply(tree.getBoard());
					_mt.set(tree.getBoard().getHash(), mate);
				}
				if (mate) {
					alpha = Value::Inf - tree.getPly() - 1;
					goto hash_store;
				}
			}
#endif

			if (!stat.isMateThreat()) {
#if ENABLE_RAZORING
				// razoring
				if (depth < search_param::RAZOR_DEPTH && hashMove.isEmpty() &&
						alpha > -Value::Mate && beta < Value::Mate && tree.getPly() >= 2) {
					Value razorAlpha = alpha - search_func::razorMargin(depth);
					if (standPat <= razorAlpha) {
						worker.info.razoringTried++;
						Value qval = qsearch(tree, black, 0, razorAlpha, razorAlpha+1);
						if (qval <= razorAlpha) {
							worker.info.razoring++;
							return qval;
						}
					}
				}
#endif

				// null move pruning
				if (isNullWindow && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
					auto newStat = NodeStat().unsetNullMove();
					int newDepth = search_func::nullDepth(depth);

					worker.info.nullMovePruningTried++;

					// make move
					tree.makeNullMove();

					Value currval = -search(tree, !black, newDepth, -beta, -beta+1, newStat);

					// unmake move
					tree.unmakeNullMove();

					// 中断判定
					if (isInterrupted(tree)) {
						return Value::Zero;
					}

					// beta-cut
					if (currval >= beta) {
						tree.updatePvNull(depth);
						worker.info.nullMovePruning++;
						alpha = beta;
						if (newDepth < Depth1Ply) {
							goto hash_store;
						}
						goto search_end;
					}

					// mate threat
					if (currval <= -Value::Mate) {
						stat.setMateThreat();
					}
				}
			}
		}

		// recursive iterative-deepening search
		if (hashMove.isEmpty() && stat.isRecursion() && depth >= search_param::REC_THRESHOLD) {
			auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
			search(tree, black, search_func::recDepth(depth), alpha, beta, newStat);

			// 中断判定
			if (isInterrupted(tree)) {
				return Value::Zero;
			}

			// ハッシュ表から前回の最善手を取得
			TTE tte;
			if (_tt.get(hash, tte)) {
				hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());
			}
		}

		tree.initGenPhase();
		worker.info.expand++;
#if ENABLE_HASH_MOVE
		tree.setHash(hashMove);
#else
		tree.setHash(Move::empty());
#endif
		while (nextMove(tree)) {
			Move move = *tree.getCurrentMove();

			worker.info.expanded++;

			// depth
			int newDepth = depth - Depth1Ply;
                          
			// stat
			NodeStat newStat = NodeStat::Default;

			isNullWindow = (beta == alpha + 1);

			const auto& board = tree.getBoard();
			bool isCheckCurr = board.isCheck(move);
			bool isCheckPrev = tree.isChecking();
			bool isCheck = isCheckCurr || isCheckPrev;
			Piece captured = board.getBoardPiece(move.to());
			int count = tree.getCurrentNode().count;

			if (!isCheckCurr && captured.isEmpty() &&
					(!move.promote() || move.piece() == Piece::Silver)) {
				tree.getCurrentNode().histMoves.add(move);
			}

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				worker.info.checkExtension++;

			} else if (isCheckPrev && isFirst && tree.getGenPhase() == GenPhase::End && tree.getNextMove() == tree.getEnd()) {
				// one-reply
				newDepth += search_param::EXT_ONEREP;
				worker.info.onerepExtension++;

			} else if (!isCheckPrev && stat.isRecapture() && tree.isRecapture(move) &&
								 (move == tree.getCapture1() ||
									(move == tree.getCapture2() && tree.getCapture1Value() < tree.getCapture2Value() + 180))
								 ) {
				// recapture
				Move fmove = tree.getFrontMove();
				if (!move.promote() && fmove.piece() == fmove.captured()) {
					newDepth += search_param::EXT_RECAP2;
				} else {
					newDepth += search_param::EXT_RECAP;
				}
				newStat.unsetRecapture();
				worker.info.recapExtension++;

			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (!isFirst && !isCheckPrev && newDepth >= Depth1Ply && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move)) {
				reduced = getReductionDepth(improving, newDepth, count, move, isNullWindow);
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

#if ENABLE_MOVE_COUNT_PRUNING
			// move count based pruning
			if (!isCheckPrev && newDepth < Depth1Ply * 8 &&
					alpha > -Value::Mate && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move) &&
					count >= search_func::futilityMoveCounts(improving, depth)) {
				isFirst = false;
				worker.info.moveCountPruning++;
				continue;
			}
#endif

			// futility pruning
			if (!isCheck && newDepth < search_param::FUT_DEPTH && alpha > -Value::Mate) {
				Value futAlpha = alpha;
				if (newDepth >= Depth1Ply) { futAlpha -= search_func::futilityMargin(newDepth, count); }
				if (standPat + tree.estimate(move, _eval) + _gains.get(move) <= futAlpha) {
					isFirst = false;
					worker.info.futilityPruning++;
					continue;
				}
			}

			// prune moves with negative SEE
			if (!isCheck && newDepth < Depth1Ply * 2 && isNullWindow &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move)) {
				if (searchSee<true>(board, move, -1, 0) < Value::Zero) {
					isFirst = false;
					continue;
				}
			}

			// make move
			if (!tree.makeMove(_eval)) {
				continue;
			}

			Value newStandPat = tree.getValue() * (black ? 1 : -1);

			// extended futility pruning
			if (!isCheck && alpha > -Value::Mate) {
				if ((newDepth < Depth1Ply && newStandPat <= alpha) ||
						(newDepth < search_param::FUT_DEPTH && newStandPat + search_func::futilityMargin(newDepth, count) <= alpha)) {
					tree.unmakeMove();
					isFirst = false;
					worker.info.extendedFutilityPruning++;
					continue;
				}
			}

			// reccursive call
			Value currval;
			if (isFirst) {
				currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);

			} else {
				// nega-scout
				currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);

				if (!isInterrupted(tree) && currval > alpha && reduced > 0) {
					newDepth += reduced;
					currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);
				}

				if (!isInterrupted(tree) && currval > alpha && currval < beta && !isNullWindow) {
					currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);
				}

			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted(tree)) {
				return Value::Zero;
			}

			// update gain
			_gains.update(move, newStandPat - standPat - tree.estimate(move, _eval));

			// 値更新
			if (currval > alpha) {
				alpha = currval;
				best = move;
				if (!isNullWindow) {
					tree.updatePv(depth);
				}

				// beta-cut
				if (currval >= beta) {
					worker.info.failHigh++;
					if (isFirst) {
						worker.info.failHighFirst++;
					}
					if (move == tree.getHash()) {
						worker.info.failHighIsHash++;
					} else if (move == tree.getKiller1()) {
						worker.info.failHighIsKiller1++;
					} else if (move == tree.getKiller2()) {
						worker.info.failHighIsKiller2++;
					}
					break;
				}
			}

			// split
			if ((depth >= Depth1Ply * 4 || (depth >= Depth1Ply * 3 && _rootDepth < Depth1Ply * 12)) &&
					(!isCheckPrev || tree.getEnd() - tree.getCurrentMove() >= 4) &&
					_idleWorkerCount.load() >= 1 && _idleTreeCount.load() >= 2) {
				if (split(tree, black, depth, alpha, beta, best, standPat, stat, improving)) {
					worker.info.split++;
					if (isInterrupted(tree)) {
						return Value::Zero;
					}
					if (tree.getTlp().alpha > alpha) {
						alpha = tree.getTlp().alpha;
						best = tree.getTlp().best;
					}
					break;
				}
			}

			isFirst = false;

		}

		if (!best.isEmpty() && !tree.isChecking()) {
#if ENABLE_KILLER_MOVE
			updateKiller(tree, best);
#endif // ENABLE_KILLER_MOVE
			if (tree.getBoard().getBoardPiece(best.to()).isEmpty() &&
					(!best.promote() || best.piece() == Piece::Silver)) {
				updateHistory(tree, depth, best);
			}
		}

hash_store:
		// TODO: GHI対策
		{
			TTStatus status = _tt.entry(hash, oldAlpha, beta, alpha, depth, tree.getPly(), Move::serialize16(best), stat);
			switch (status) {
				case TTStatus::New: worker.info.hashNew++; break;
				case TTStatus::Update: worker.info.hashUpdate++; break;
				case TTStatus::Collide: worker.info.hashCollision++; break;
				case TTStatus::Reject: worker.info.hashReject++; break;
				default: break;
			}
			worker.info.hashStore++;
		}

search_end:
		return alpha;

	}

	void Searcher::releaseTree(int tid) {
		auto& tree = _trees[tid];
		auto& parent = _trees[tree.getTlp().parentTreeId];
		tree.unuse();
		_idleTreeCount.fetch_add(1);
		parent.getTlp().childCount.fetch_sub(1);
	}

	/**
	 * split
	 */
	bool Searcher::split(Tree& parent, bool black, int depth, Value alpha, Value beta, Move best, Value standPat, NodeStat stat, bool improving) {
		int currTreeId = Tree::InvalidId;

		{
			std::lock_guard<std::mutex> lock(_splitMutex);

			if (_idleTreeCount.load() <= 1 || _idleWorkerCount.load() == 0) {
				return false;
			}

			// カレントスレッドに割り当てる tree を決定
			for (int tid = 1; tid < _config.treeSize; tid++) {
				auto& tree = _trees[tid];
				if (!tree.getTlp().used) {
					currTreeId = tid;
					tree.use(parent, parent.getTlp().workerId);
					_idleTreeCount.fetch_sub(1);
					break;
				}
			}

			// 他の worker と tree を確保
			int childCount = 1;
			for (int wid = 0; wid < _config.workerSize; wid++) {
				auto& worker = _workers[wid];
				if (!worker.job.load()) {
					for (int tid = 1; tid < _config.treeSize; tid++) {
						auto& tree = _trees[tid];
						if (!tree.getTlp().used) {
							tree.use(parent, wid);
							_idleTreeCount.fetch_sub(1);

							worker.setJob(tid);
							_idleWorkerCount.fetch_sub(1);

							childCount++;

							if (_idleTreeCount.load() == 0 || _idleWorkerCount.load() == 0) {
								goto lab_assign_end;
							}

							break;
						}
					}
				}
			}
			lab_assign_end:
				;

			parent.getTlp().childCount.store(childCount);
			parent.getTlp().black     = black;
			parent.getTlp().depth     = depth;
			parent.getTlp().alpha     = alpha;
			parent.getTlp().beta      = beta;
			parent.getTlp().best      = best;
			parent.getTlp().standPat  = standPat;
			parent.getTlp().stat      = stat;
			parent.getTlp().improving = improving;
		}

		auto& tree = _trees[currTreeId];
		auto& worker = _workers[parent.getTlp().workerId];
		worker.swapTree(currTreeId);
		searchTlp(tree);
		worker.swapTree(parent.getTlp().treeId);

		{
			std::lock_guard<std::mutex> lock(_splitMutex);
			tree.unuse();
			_idleTreeCount.fetch_add(1);
			parent.getTlp().childCount.fetch_sub(1);
		}

		if (!tree.getTlp().shutdown.load()) {
			// suspend
			worker.waitForJob(&parent);
		} else {
			// wait for brothers
			while (true) {
				if (parent.getTlp().childCount.load() == 0) {
					break;
				}
				std::this_thread::yield();
			}
		}

		return true;
	}

	void Searcher::searchTlp(Tree& tree) {
		{
			std::lock_guard<std::mutex> lock(_splitMutex);
		}

		auto& parent = _trees[tree.getTlp().parentTreeId];
		auto& worker = getWorker(tree);

		bool black = parent.getTlp().black;
		int depth = parent.getTlp().depth;
		Value beta = parent.getTlp().beta;
		Value standPat = parent.getTlp().standPat;
		NodeStat stat = parent.getTlp().stat;
		bool improving = parent.getTlp().improving;

		tree.initGenPhase();

		while (true) {
			Move move;
			Value alpha;
			const auto& board = tree.getBoard();
			bool isCheckCurr;
			bool isCheckPrev = tree.isChecking();
			bool isCheck;
			bool isPriorMove;
			Piece captured;
			int count;

			{
				std::lock_guard<std::mutex> lock(parent.getMutex());

				if (!nextMove(parent)) {
					return;
				}

				move = *parent.getCurrentMove();
				count = parent.getCurrentNode().count;
				alpha = parent.getTlp().alpha;
				captured = board.getBoardPiece(move.to());
				isCheckCurr = board.isCheck(move);
				isCheck = isCheckCurr || isCheckPrev;
				isPriorMove = parent.isPriorMove(move);

				if (!isCheckCurr && captured.isEmpty() &&
						(!move.promote() || move.piece() == Piece::Silver)) {
					parent.getCurrentNode().histMoves.add(move);
				}
			}

			tree.addMove(move);
			tree.selectNextMove();

			// depth
			int newDepth = depth - Depth1Ply;
                          
			// stat
			NodeStat newStat = NodeStat::Default;

			bool isNullWindow = (beta == alpha + 1);

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				worker.info.checkExtension++;

			} else if (!isCheckPrev && stat.isRecapture() && parent.isRecapture(move) &&
								 (move == parent.getCapture1() ||
									(move == parent.getCapture2() && parent.getCapture1Value() < parent.getCapture2Value() + 180))
								 ) {
				// recapture
				Move fmove = parent.getFrontMove();
				if (!move.promote() && fmove.piece() == fmove.captured()) {
					newDepth += search_param::EXT_RECAP2;
				} else {
					newDepth += search_param::EXT_RECAP;
				}
				newStat.unsetRecapture();
				worker.info.recapExtension++;

			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (!isCheckPrev && newDepth >= Depth1Ply && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!isPriorMove) {
				reduced = getReductionDepth(improving, newDepth, count, move, isNullWindow);
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

#if ENABLE_MOVE_COUNT_PRUNING
			// move count based pruning
			if (!isCheckPrev && newDepth < Depth1Ply * 8 &&
					alpha > -Value::Mate && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move) &&
					count >= search_func::futilityMoveCounts(improving, depth)) {
				worker.info.moveCountPruning++;
				continue;
			}
#endif

			// futility pruning
			if (!isCheck && newDepth < search_param::FUT_DEPTH && alpha > -Value::Mate) {
				Value futAlpha = alpha;
				if (newDepth >= Depth1Ply) { futAlpha -= search_func::futilityMargin(newDepth, count); }
				if (standPat + tree.estimate(move, _eval) + _gains.get(move) <= futAlpha) {
					worker.info.futilityPruning++;
					continue;
				}
			}

			if (newDepth < Depth1Ply * 2 && isNullWindow && !isCheck &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!isPriorMove) {
				if (searchSee<true>(board, move, -1, 0) < Value::Zero) {
					continue;
				}
			}

			if (!tree.makeMove(_eval)) {
				continue;
			}

			Value newStandPat = tree.getValue() * (black ? 1 : -1);

			// extended futility pruning
			if (!isCheck && alpha > -Value::Mate) {
				if ((newDepth < Depth1Ply && newStandPat <= alpha) ||
						(newDepth < search_param::FUT_DEPTH && newStandPat + search_func::futilityMargin(newDepth, count) <= alpha)) {
					tree.unmakeMove();
					worker.info.extendedFutilityPruning++;
					continue;
				}
			}

			// nega-scout
			Value currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);

			if (!isInterrupted(tree) && currval > alpha && reduced > 0) {
				newDepth += reduced;
				currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);
			}

			if (!isInterrupted(tree) && currval > alpha && currval < beta && !isNullWindow) {
				currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);
			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted(tree)) {
				return;
			}

			// update gain
			_gains.update(move, newStandPat - standPat - tree.estimate(move, _eval));

			{
				std::lock_guard<std::mutex> lock(parent.getMutex());

				if (tree.getTlp().shutdown.load()) {
					return;
				}

				// 値更新
				if (currval > parent.getTlp().alpha) {
					parent.getTlp().alpha = currval;
					parent.getTlp().best = move;
					if (!isNullWindow) {
						parent.updatePv(depth, tree);
					}

					// beta-cut
					if (currval >= parent.getTlp().beta) {
						worker.info.failHigh++;
						if (move == parent.getHash()) {
							worker.info.failHighIsHash++;
						} else if (move == parent.getKiller1()) {
							worker.info.failHighIsKiller1++;
						} else if (move == parent.getKiller2()) {
							worker.info.failHighIsKiller2++;
						}
						shutdownSiblings(parent);
					}
				}
			}
		}
	}

	void Searcher::shutdownSiblings(Tree& parent) {
		std::lock_guard<std::mutex> lock(_splitMutex);

		for (int id = 0; id < _config.treeSize; id++) {
			int parentId = _trees[id].getTlp().parentTreeId;
			while (parentId != Tree::InvalidId) {
				if (parentId == parent.getTlp().treeId) {
					_trees[id].getTlp().shutdown.store(true);
					break;
				}
				parentId = _trees[parentId].getTlp().parentTreeId;
			}
		}
	}

	/**
	 * search on root node
	 */
	Value Searcher::searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best,
			bool forceFullWindow /* = false */) {
		const auto& board = tree.getBoard();
		bool black = board.isBlack();
		bool isFirst = true;
		Value oldAlpha = alpha;

		_rootDepth = depth;

		while (nextMove(tree)) {
			Move move = *tree.getCurrentMove();

			// depth
			int newDepth = depth - Depth1Ply;

			bool isCheckPrev = board.isChecking();
			bool isCheckCurr = board.isCheck(move);
			Piece captured = board.getBoardPiece(move.to());

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				_info.checkExtension++;
			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (!isFirst && newDepth >= Depth1Ply * 2 && !isCheckPrev &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver)) {
				reduced = Depth1Ply;
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

			// make move
			bool ok = tree.makeMove(_eval);
			assert(ok);

			Value currval;

			if (forceFullWindow) {
				currval = -search(tree, !black, newDepth, -beta, -oldAlpha);

			} else if (isFirst) {
				// full window search
				currval = -search(tree, !black, newDepth, -beta, -alpha);

			} else {
				// nega-scout
				currval = -search(tree, !black, newDepth, -alpha-1, -alpha);

				if (!isInterrupted(tree) && currval >= alpha + 1 && reduced != 0) {
					// full depth
					newDepth += reduced;
					currval = -search(tree, !black, newDepth, -alpha-1, -alpha);
				}

				if (!isInterrupted(tree) && currval >= alpha + 1) {
					// full window search
					currval = -search(tree, !black, newDepth, -beta, -alpha);
				}
			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted(tree)) {
				return alpha;
			}

			if (isFirst && currval <= alpha && currval > -Value::Mate) {
				return currval;
			}

			// ソート用に値をセット
			auto index = tree.getIndexByMove(move);
			if (forceFullWindow || currval > alpha) {
				_rootValues[index] = currval.int32();
			} else {
				_rootValues[index] = -Value::Inf;
			}

			_timeManager.addMove(move, currval);

			// 値更新
			if (currval > alpha) {
				// update alpha
				alpha = currval;
				best = move;
				tree.updatePv(depth);

				if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD || currval >= Value::Mate) {
					showPv(depth / Depth1Ply, tree.getPv(), black ? currval : -currval);
				}

				// beta-cut or update best move
				if (alpha >= beta) {
					return alpha;
				}
			}

			isFirst = false;
		}

		return alpha;
	}

	/**
	 * aspiration search
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::searchAsp(int depth, Move& best, Value* pval /* = nullptr */) {
		auto& tree0 = _trees[0];

		bool hasPrevVal = pval != nullptr && (*pval != -Value::Inf);
		Value baseVal = hasPrevVal ? *pval : 0;
		CONSTEXPR int wmax = 3;
		const Value alphas[wmax] = { baseVal-198, baseVal-793, -Value::Mate };
		const Value betas[wmax] = { baseVal+198, baseVal+793, Value::Mate };
		int lower = hasPrevVal ? 0 : wmax - 1;
		int upper = hasPrevVal ? 0 : wmax - 1;
		Value value = alphas[lower];

		while (true) {

			_timeManager.startDepth();

			const Value alpha = alphas[lower];
			const Value beta = betas[upper];

			value = searchRoot(tree0, depth, alpha, beta, best);

			// 中断判定
			if (isInterrupted(tree0)) {
				return false;
			}

			// 値が確定
			if (value > alpha && value < beta) {
				break;
			}

			bool retry = false;

			// alpha 値を広げる
			while (value <= alphas[lower] && lower != wmax - 1) {
				lower++;
				assert(lower < wmax);
				retry = true;
				tree0.selectFirstMove();
			}

			// beta 値を広げる
			while (value >= betas[upper] && upper != wmax - 1) {
				upper++;
				assert(upper < wmax);
				retry = true;
				tree0.selectPreviousMove();
			}

			if (!retry) { break; }
		}

		if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD ||
				value >= Value::Mate || value <= -Value::Mate) {
			showEndOfIterate(depth / Depth1Ply);
		}

		tree0.setSortValues(_rootValues);
		tree0.sortAll();

		_info.eval = value;
		if (pval != nullptr) {
			*pval = value;
		}

		if (value <= -Value::Mate) {
			return false;
		}

		int limit = _config.limitEnable ? _config.limitSeconds : 0;
		if (!_config.ponder && _timeManager.isEasy(limit, _timer.get())) {
			return false;
		}

		return true;

	}

	void Searcher::showPv(int depth, const Pv& pv, const Value& value) {
		mergeInfo();

		uint64_t node = _info.node + _info.qnode;

		std::ostringstream oss;
		if (_info.lastDepth == depth) {
			oss << "    ";
		} else {
			oss << std::setw(2) << depth << ": ";
			_info.lastDepth = depth;
		}
		oss << std::setw(10) << node << ": ";
		oss << pv.toString() << ": ";
		oss << value.int32();
		Loggers::message << oss.str();
	}

	void Searcher::showEndOfIterate(int depth) {
		mergeInfo();

		uint64_t node = _info.node + _info.qnode;
		double seconds = _timer.get();

		std::ostringstream oss;
		if (_info.lastDepth == depth) {
			oss << "    ";
		} else {
			oss << std::setw(2) << depth << ": ";
			_info.lastDepth = depth;
		}
		oss << std::setw(10) << node;
		oss << ": " << seconds << "sec";
		Loggers::message << oss.str();
	}

	void Searcher::generateMovesOnRoot() {
		auto& tree0 = _trees[0];

		auto& moves = tree0.getMoves();
		const auto& board = tree0.getBoard();

		// 合法手生成
		tree0.initGenPhase();
		MoveGenerator::generate(board, moves);
		tree0.resetGenPhase();

		// 非合法手除去
		for (auto ite = moves.begin(); ite != moves.end();) {
			if (!board.isValidMove(*ite)) {
				ite = moves.remove(ite);
			} else {
				++ite;
			}
		}
	}

	/**
	 * iterative deepening search from root node
	 * @return {負けたか深さ1で中断された場合にfalseを返します。}
	 */
	bool Searcher::idsearch(Move& best) {
		auto& tree0 = _trees[0];
		bool result = false;

		generateMovesOnRoot();

		Value value = searchRoot(tree0, Depth1Ply, -Value::Inf, Value::Inf, best, true);
		tree0.setSortValues(_rootValues);
		tree0.sortAll();

		for (int depth = 1; depth <= _config.maxDepth; depth++) {
			bool cont = searchAsp(depth * Depth1Ply + Depth1Ply / 2, best, &value);

#if DEBUG_ROOT_MOVES
			std::ostringstream oss;
			for (auto ite = tree0.getBegin(); ite != tree0.getEnd(); ite++) {
				oss << ' ' << (*ite).toString() << '[' << tree0.getSortValue(ite) << ']';
			}
			Loggers::debug << oss.str();
#endif

#if ENABLE_STORE_PV
			storePv(tree0, tree0.getPv(), 0);
#endif // ENABLE_STORE_PV

			if (value >= Value::Mate) {
				result = true;
				break;
			}

			if (value <= -Value::Mate) {
				result = false;
				break;
			}

			if (!cont) {
				break;
			}

			result = true;

			_timeManager.nextDepth();
		}

		return result;

	}

	/**
	 * 指定した局面に対して探索を実行します。
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::search(const Board& initialBoard, Move& best) {

		// 前処理
		before(initialBoard);
           
		// 最大深さ
		int depth = _config.maxDepth * Depth1Ply;

		generateMovesOnRoot();

		bool result = searchAsp(depth, best);

		// 後処理
		after();

		return result;

	}

	/**
	 * 指定した局面に対して反復深化探索を実行します。
	 * @return {負けたか深さ1で中断された場合にfalseを返します。}
	 */
	bool Searcher::idsearch(const Board& initialBoard, Move& best) {

		// 前処理
		before(initialBoard);

		bool result = idsearch(best);

		// 後処理
		after();

		return result;

	}

}
