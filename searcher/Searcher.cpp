/* Searcher.cpp
 *
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <iomanip>

#define ENABLE_LMR										1
#define ENABLE_HASH_MOVE							1
#define ENABLE_KILLER_MOVE						1
#define ENABLE_PRECEDE_KILLER					0 // should be 0
#define ENABLE_SHEK_PRESET						1
#define ENABLE_SHEK										1
#define ENABLE_MATE_1PLY							1
#define ENABLE_STORE_PV								1
#define SHALLOW_SEE                   0 // should be 0

// debugging flags
#define DEBUG_ROOT_MOVES							0
#define DEBUG_TREE										0
#define DEBUG_NODE										0

#define ITERATE_INFO_THRESHOLD        3

namespace sunfish {

	namespace search_param {
		CONSTEXPR int EFUT_MGN1 = 800;
		CONSTEXPR int EFUT_MGN2 = 800;
		CONSTEXPR int EXT_CHECK = Searcher::Depth1Ply;
		CONSTEXPR int EXT_ONEREP = Searcher::Depth1Ply * 1 / 2;
		CONSTEXPR int EXT_RECAP = Searcher::Depth1Ply * 1 / 4;
		CONSTEXPR int REC_THRESHOLD = Searcher::Depth1Ply * 3;
	}

	namespace search_func {
		inline int recDepth(int depth) {
			return (depth < Searcher::Depth1Ply * 9 / 2 ?
							Searcher::Depth1Ply * 3 / 2 :
							depth - Searcher::Depth1Ply * 3);
		}
		inline int nullDepth(int depth) {
#if 1
			// same to bonanza
			return (depth < Searcher::Depth1Ply * 26 / 4 ? depth - Searcher::Depth1Ply * 12 / 4 :
							(depth <= Searcher::Depth1Ply * 30 / 4 ? Searcher::Depth1Ply * 14 / 4 : depth - Searcher::Depth1Ply * 16 / 4));
#else
			return depth - Searcher::Depth1Ply * 7 / 2;
#endif
		}
	}

	/**
	 * 前処理
	 */
	void Searcher::before(const Board& initialBoard) {

		// ツリーの初期化
		auto& tree = _trees[0];
		tree.init(initialBoard, _eval);

		// 探索情報収集準備
		memset(&_info, 0, sizeof(_info));
		_timer.set();

		// transposition table
		_tt.evolve(); // 世代更新

		// hisotory heuristic
		_history.reduce();

#if ENABLE_SHEK_PRESET
		{
			// SHEK
			auto& tree = _trees[0];
			auto& shekTable = tree.getShekTable();
			Board board = tree.getBoard();
			for (int i = (int)_record.size()-1; i >= 0; i--) {
				bool ok = board.unmakeMove(_record[i]);
				assert(ok);
				shekTable.set(board);
			}
		}
#endif

		_forceInterrupt = false;
		_isRunning = true;

	}

	/**
	 * 後処理
	 */
	void Searcher::after() {

		auto& tree = _trees[0];

		// 探索情報収集
		_info.time = _timer.get();
		_info.nps = (_info.node + _info.qnode) / _info.time;
		_info.move = tree.getPv().get(0).move;

#if ENABLE_SHEK_PRESET
		{
			// SHEK
			auto& tree = _trees[0];
			auto& shekTable = tree.getShekTable();
			Board board = tree.getBoard();
			for (int i = (int)_record.size()-1; i >= 0; i--) {
				bool ok = board.unmakeMove(_record[i]);
				assert(ok);
				shekTable.unset(board);
			}
		}
#endif

#ifndef NDEBUG
		{
			// SHEK のテーブルが元に戻っているかチェックする。
			for (int ti = 0; ti < _config.treeSize; ti++) {
				if (!_trees[ti].getShekTable().isAllCleared()) {
					std::cout << "SHEK table has some pending record. (" << ti << ")" << std::endl;
				}
			}
		}
#endif

		_isRunning = false;
		_forceInterrupt = false;

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
	inline bool Searcher::isInterrupted() {
		if (_forceInterrupt) {
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
		_forceInterrupt = true;
	}

	/**
	 * sort moves by see
	 */
	void Searcher::sortSee(Tree& tree, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies) {
		const auto& board = tree.getBoard();
		auto& node = tree.getCurrentNode();
#if !ENABLE_KILLER_MOVE
		assert(node.killer1.isEmpty());
		assert(node.killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ) {
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

			if (!isQuies) {
				if ((node.expStat & HashDone) && move == node.hash) {
					ite = tree.getMoves().remove(ite);
					continue;
				}
			}

			if (isQuies) {
				// futility pruning
				if (standPat + tree.estimate(move, _eval) <= alpha) {
					_info.futilityPruning++;
					ite = tree.getMoves().remove(ite);
					continue;
				}
			}

#if SHALLOW_SEE
			value = _see.search<true>(board, move, -1, Value::PieceInf);
#else
			value = _see.search<false>(board, move, -1, Value::PieceInf);
#endif
			if (estimate) {
				value += tree.estimate<true>(move, _eval);
			}

			if (enableKiller) {
				if (move == node.killer1) {
					node.expStat |= Killer1Added;
					auto captured = board.getBoardPiece(move.to());
#if ENABLE_PRECEDE_KILLER
					value = Value::Inf;
#else
					Value kvalue = tree.getKiller1Value() + material::pieceExchange(captured);
					value = Value::max(value, kvalue);
#endif
				} else if (move == node.killer2) {
					node.expStat |= Killer2Added;
					auto captured = board.getBoardPiece(move.to());
#if ENABLE_PRECEDE_KILLER
					value = Value::Inf-1;
#else
					Value kvalue = tree.getKiller2Value() + material::pieceExchange(captured);
					value = Value::max(value, kvalue);
#endif
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
#if ENABLE_PRECEDE_KILLER
				tree.setSortValue(ite, Value::Inf);
#else
				Value kvalue = tree.getKiller1Value();
				tree.setSortValue(ite, kvalue.int32());
#endif
			}
			if (!(node.expStat & Killer2Added) && node.killer2 != node.hash
					&& tree.getKiller2Value() >= Value::Zero
					&& board.isValidMoveStrict(node.killer2)) {
				node.expStat |= Killer2Added;
				auto ite = tree.addMove(node.killer2);
#if ENABLE_PRECEDE_KILLER
				tree.setSortValue(ite, Value::Inf);
#else
				Value kvalue = tree.getKiller2Value();
				tree.setSortValue(ite, kvalue.int32());
#endif
			}
		}

		tree.sortAfterCurrent();

		if (isQuies) {
			for (auto ite = tree.getNext(); ite != tree.getEnd(); ite++) {
				if (tree.getSortValue(ite) < 0) {
					tree.removeAfter(ite);
					break;
				}
			}
		}

		if (enableKiller) {
			auto ite = tree.getNext();
			if (ite != tree.getEnd()) {
				tree.setCapture1(*ite, tree.getSortValue(ite));
				ite++;
				if (ite != tree.getEnd()) {
					tree.setCapture2(*ite, tree.getSortValue(ite));
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

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ) {
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

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ) {
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
			Move temp = *tree.getNext();
			*tree.getNext() = *best;
			*best = temp;
			return true;
		}

		return false;
	}

	/**
	 * sort moves by history
	 */
	void Searcher::sortHistory(Tree& tree) {
		for (auto ite = tree.getNext(); ite != tree.getEnd(); ) {
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

		int value = std::max(depth * 4 / Depth1Ply, 1);
		for (auto ite = tree.getBegin(); ite != tree.getNext(); ite++) {
			assert(ite != tree.getEnd());
			auto key = History::getKey(*ite);
			if (ite->equals(move)) {
				_history.add(key, value, value);
				return;
			} else {
				_history.add(key, value, 0);
			}

		}
		assert(false); // unreachable

	}

	/**
	 * get LMR depth
	 */
	int Searcher::getReductionDepth(const Move& move, bool isNullWindow) {
		auto key = History::getKey(move);
		auto data = _history.getData(key);
		auto good = History::getGoodCount(data) + 1;
		auto appear = History::getAppearCount(data) + 2;

		assert(good < appear);

		if (!isNullWindow) {
			if (good * 20 < appear) {
				return Depth1Ply * 3 / 2;
			} else if (good * 7 < appear) {
				return Depth1Ply * 2 / 2;
			} else if (good * 3 < appear) {
				return Depth1Ply * 1 / 2;
			}
		} else {
			if (good * 10 < appear) {
				return Depth1Ply * 4 / 2;
			} else if (good * 6 < appear) {
				return Depth1Ply * 3 / 2;
			} else if (good * 4 < appear) {
				return Depth1Ply * 2 / 2;
			} else if (good * 2 < appear) {
				return Depth1Ply * 1 / 2;
			}
		}

		return 0;
	}

	/**
	 * get next move
	 */
	bool Searcher::nextMove(Tree& tree, Move& move) {

		auto& moves = tree.getMoves();
		auto& node = tree.getCurrentNode();
		const auto& board = tree.getBoard();

		while (true) {

			if (!tree.isThroughPhase() &&
					tree.getNext() != tree.getEnd()) {
				move = *tree.getNext();
				tree.selectNextMove();
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
					}
					node.genPhase = GenPhase::Capture;
				}
				break;

			case GenPhase::Capture:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortSee(tree, Value::Zero, Value::Zero, false, true, false, false);
					node.genPhase = GenPhase::End;
					break;
					
				} else {
					MoveGenerator::generateCap(board, moves);
					sortSee(tree, Value::Zero, Value::Zero, true, false, false, false);
					node.genPhase = GenPhase::History1;
					break;

				}

			case GenPhase::History1:
				MoveGenerator::generateNoCap(board, moves);
				MoveGenerator::generateDrop(board, moves);
				exceptPriorMoves(tree);
				node.genPhase = GenPhase::History2;
				tree.setThroughPhase(true);
				if (pickOneHistory(tree)) {
					move = *tree.getNext();
					tree.selectNextMove();
					return true;
				}
				break;

			case GenPhase::History2:
				node.genPhase = GenPhase::Misc;
				tree.setThroughPhase(true);
				if (pickOneHistory(tree)) {
					move = *tree.getNext();
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
	bool Searcher::nextMoveQuies(Tree& tree, Move& move, int qply, Value standPat, Value alpha) {

		auto& moves = tree.getMoves();
		auto& node = tree.getCurrentNode();
		const auto& board = tree.getBoard();

		while (true) {

			if (tree.getNext() != tree.getEnd()) {
				move = *tree.getNext();
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
					if (qply >= 7) {
  					sortSee(tree, standPat, alpha, false, false, true, true);
					} else {
  					sortSee(tree, standPat, alpha, false, false, false, true);
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

#if DEBUG_TREE
		{
			for (int i = 0; i < tree.getPly(); i++) {
				std::cout << ' ';
			}
			std::cout << tree.__debug__getFrontMove().toString() << std::endl;
		}
#endif

		_info.qnode++;

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
			_info.mateProbed++;
			if (_mt.get(tree.getBoard().getHash(), mate)) {
				_info.mateHit++;
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

		Move move;
		while (nextMoveQuies(tree, move, qply, standPat, alpha)) {
			// make move
			if (!tree.makeMove(move, _eval)) {
				continue;
			}

			// reccursive call
			Value currval;
			currval = -qsearch(tree, !black, qply + 1, -beta, -alpha);

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値更新
			if (currval > alpha) {
				alpha = currval;
				tree.updatePv(move, 0);

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
				Value val = _see.search<false>(board, move, -1, Value::PieceInf) - capVal + 1;
				node.kvalue1 = Value::min(node.kvalue1, val);
			}
			node.killer2 = node.killer1;
			node.kvalue2 = node.kvalue1;
			node.killer1 = move;
			node.kvalue1 = node.cvalue1 - capVal + 1;

		}
	}

	/**
	 * nega-max search
	 */
	template <bool pvNode>
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
#if 1
		if (tree.__debug__matchPath("-5354FU")) {
			std::cout << " ** debug begin **" << std::endl;
			std::cout << tree.__debug__getPath() << std::endl;
			std::cout << "alpha=" << alpha.int32() << " beta=" << beta.int32() << " depth=" << depth << std::endl;
			debug = true;
		}
#endif
#endif

#if ENABLE_SHEK
		// SHEK
		ShekStat shekStat = tree.checkShek();
		_info.shekProbed++;
		switch (shekStat) {
			case ShekStat::Superior:
				// 既出の局面に対して優位な局面
				_info.shekSuperior++;
				return Value::Inf - tree.getPly();

			case ShekStat::Inferior:
				// 既出の局面に対して劣る局面
				_info.shekInferior++;
				return -Value::Inf + tree.getPly();

			case ShekStat::Equal:
				// TODO: 連続王手千日手の検出
				_info.shekEqual++;
				return Value::Zero;

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

		// distance pruning
		Value oldAlpha = alpha;
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

		_info.node++;

		uint64_t hash = tree.getBoard().getHash();
		bool isNullWindow = (beta == alpha + 1);

		// transposition table
		Move hashMove = Move::empty();
		{
			TTE tte;
			_info.hashProbed++;
			if (_tt.get(hash, tte)) {
				if (depth < search_param::REC_THRESHOLD ||
						tte.getDepth() >= search_func::recDepth(depth)) {
					auto ttv = tte.getValue(tree.getPly());
					auto valueType = tte.getValueType();

					// 前回の結果で枝刈り
					if (!pvNode && stat.isHashCut() && isNullWindow) {
						// 現在のノードに対して優位な条件の場合
						if (tte.getDepth() >= depth ||
								(ttv >= Value::Mate && (valueType == TTE::Lower || valueType == TTE::Exact)) ||
								(ttv <= -Value::Mate && (valueType == TTE::Upper || valueType == TTE::Exact))) {
							if (valueType == TTE::Exact) {
								// 確定値
								_info.hashExact++;
								return ttv;
							} else if (valueType == TTE::Lower && ttv >= beta) {
								// 下界値
								_info.hashLower++;
								return ttv;
							} else if (valueType == TTE::Upper && ttv <= alpha) {
								// 上界値
								_info.hashUpper++;
								return ttv;
							}
						}
						// 十分なマージンを加味して beta 値を超える場合
						if ((valueType == TTE::Lower || valueType == TTE::Exact) &&
								tree.isChecking() && tree.isCheckingOnFrontier()) {
							if ((depth <= Depth1Ply * 2 && ttv >= beta + search_param::EFUT_MGN1) ||
									(depth <= Depth1Ply * 3 && ttv >= beta + search_param::EFUT_MGN2)) {
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
					if (depth < search_param::REC_THRESHOLD ||
							tte.getDepth() >= search_func::recDepth(depth)) {
						hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());
					}

					if (tte.isMateThreat()) {
						stat.setMateThreat();
					}
				}
				_info.hashHit++;
			}
		}

		Value standPat = tree.getValue() * (black ? 1 : -1);

		int count = 0;
		Move move;
		Move best = Move::empty();

		if (!tree.isChecking()) {

#if ENABLE_MATE_1PLY
			if (stat.isMate()) {
				// search mate in 1 ply
				bool mate;
				_info.mateProbed++;
				if (_mt.get(tree.getBoard().getHash(), mate)) {
					_info.mateHit++;
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

			// null move pruning
			if (!pvNode && isNullWindow && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
				auto newStat = NodeStat().unsetNullMove();
				int newDepth = search_func::nullDepth(depth);

				_info.nullMovePruningTried++;

				// make move
				tree.makeNullMove();

				Value currval = -search<false>(tree, !black, newDepth, -beta, -beta+1, newStat);

				// unmake move
				tree.unmakeNullMove();

				// 中断判定
				if (isInterrupted()) {
					return Value::Zero;
				}

				// beta-cut
				if (currval >= beta) {
					tree.updatePv(Move::empty(), depth);
					_info.nullMovePruning++;
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

		// recursive iterative-deepening search
		if (!hashMove.isEmpty() && stat.isRecursion() && depth >= search_param::REC_THRESHOLD) {
			auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
			search<pvNode>(tree, black, search_func::recDepth(depth), alpha, beta, newStat);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// ハッシュ表から前回の最善手を取得
			TTE tte;
			if (_tt.get(hash, tte)) {
				hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());
			}
		}

		tree.initGenPhase();
		_info.expand++;
#if ENABLE_HASH_MOVE
		tree.setHash(hashMove);
		if (!hashMove.isEmpty()) {
			_info.expandHashMove++;
		}
#else
		tree.setHash(Move::empty());
#endif
		while (nextMove(tree, move)) {

			_info.expanded++;

			// depth
			int newDepth = depth - Depth1Ply;
                          
			// stat
			NodeStat newStat = NodeStat::Default;

			isNullWindow = (beta == alpha + 1);

			const auto& board = tree.getBoard();
			bool isCheckCurr = board.isCheck(move);
			bool isCheckPrev = tree.isChecking();
			bool isCheck = isCheckCurr || isCheckPrev;
			bool isCap = board.getBoardPiece(move.to());

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				_info.checkExtension++;

			} else if (isCheckPrev && count == 0 && tree.getGenPhase() == GenPhase::End && tree.getNext() == tree.getEnd()) {
				// one-reply
				newDepth += search_param::EXT_ONEREP;
				_info.onerepExtension++;

			} else if (!isCheckPrev && stat.isRecapture() && tree.isRecapture() &&
								 (move == tree.getCapture1() ||
									(move == tree.getCapture2() && tree.getCapture1Value() < tree.getCapture2Value() + 180))
								 ) {
				// recapture
				newDepth += search_param::EXT_RECAP;
				newStat.unsetRecapture();
				_info.recapExtension++;

			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (count != 0 && newDepth >= Depth1Ply && !isCheck && !stat.isMateThreat() &&
					isCap && (!move.promote() || move.piece() != Piece::Silver) &&
					!tree.isPriorMove(move)) {
				reduced = getReductionDepth(move, isNullWindow);
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

			// futility pruning
			if (!isCheck && newDepth < Depth1Ply * 3 && alpha > -Value::Mate) {
				Value futAlpha = alpha;
				if (newDepth >= Depth1Ply * 2) { futAlpha -= search_param::EFUT_MGN2; }
				else if (newDepth >= Depth1Ply) { futAlpha -= search_param::EFUT_MGN1; }
				if (standPat + tree.estimate(move, _eval) <= futAlpha) {
					count++;
					_info.futilityPruning++;
					continue;
				}
			}

			if (newDepth < Depth1Ply * 2 && isNullWindow && !isCheck &&
					!isCap && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move) &&
					_see.search<true>(board, move, -1, 0) < Value::Zero) {
				count++;
				continue;
			}

			// make move
			if (!tree.makeMove(move, _eval)) {
				continue;
			}

			Value newStandPat = tree.getValue() * (black ? 1 : -1);

			// extended futility pruning
			if (!isCheck && alpha > -Value::Mate) {
				if ((newDepth < Depth1Ply && newStandPat <= alpha) ||
						(newDepth < Depth1Ply * 2 && newStandPat + search_param::EFUT_MGN1 <= alpha) ||
						(newDepth < Depth1Ply * 3 && newStandPat + search_param::EFUT_MGN2 <= alpha)) {
					tree.unmakeMove();
					count++;
					_info.extendedFutilityPruning++;
					continue;
				}
			}

			// reccursive call
			Value currval;
			if (count == 0) {
				currval = -search<pvNode>(tree, !black, newDepth, -beta, -alpha, newStat);

			} else {
				// nega-scout
				currval = -search<false>(tree, !black, newDepth, -alpha-1, -alpha, newStat);

				if (!isInterrupted() && currval > alpha && reduced > 0) {
					newDepth += reduced;
					currval = -search<pvNode>(tree, !black, newDepth, -alpha-1, -alpha, newStat);
				}

				if (!isInterrupted() && currval > alpha && currval < beta && !isNullWindow) {
					currval = -search<pvNode>(tree, !black, newDepth, -beta, -alpha, newStat);
				}

			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値更新
			if (currval > alpha) {
				alpha = currval;
				best = move;
				if (!isNullWindow) {
					tree.updatePv(move, depth);
				}

				// beta-cut
				if (currval >= beta) {
					_info.failHigh++;
					if (count == 0) {
						_info.failHighFirst++;
					}
					if (move == tree.getHash()) {
						_info.failHighIsHash++;
					} else if (move == tree.getKiller1()) {
						_info.failHighIsKiller1++;
					} else if (move == tree.getKiller2()) {
						_info.failHighIsKiller2++;
					}
					break;
				}
			}
			count++;

		}

		if (!best.isEmpty() && !tree.isChecking()) {
			if (!isNullWindow || alpha >= beta) {
#if ENABLE_KILLER_MOVE
				updateKiller(tree, move);
#endif // ENABLE_KILLER_MOVE
			}
			if (tree.getBoard().getBoardPiece(best.to()).isEmpty() &&
					(!best.promote() || best.piece() != Piece::Silver)) {
				updateHistory(tree, depth, best);
			}
		}

hash_store:
		// TODO: GHI対策
		{
			TTStatus status = _tt.entry(hash, oldAlpha, beta, alpha, depth, tree.getPly(), Move::serialize16(best), stat);
			switch (status) {
				case TTStatus::New: _info.hashNew++; break;
				case TTStatus::Update: _info.hashUpdate++; break;
				case TTStatus::Collide: _info.hashCollision++; break;
				case TTStatus::Reject: _info.hashReject++; break;
				default: break;
			}
			_info.hashStore++;
		}

search_end:
		return alpha;

	}
	template Value Searcher::search<true>(Tree&, bool, int, Value, Value, NodeStat);
	template Value Searcher::search<false>(Tree&, bool, int, Value, Value, NodeStat);

	/**
	 * search on root node
	 */
	Value Searcher::searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best) {
		const auto& board = tree.getBoard();
		bool black = board.isBlack();
		int count = 0;
		Move move;

		while (nextMove(tree, move)) {
			// depth
			int newDepth = depth - Depth1Ply;

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
			if (count != 0 && newDepth >= Depth1Ply * 2 && !isCheckCurr &&
					captured.isEmpty() && (!move.promote() || move.piece() != Piece::Silver)) {
				reduced = getReductionDepth(move, false);
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

			// make move
			bool ok = tree.makeMove(move, _eval);
			assert(ok);

			Value currval;

			if (count == 0) {
				// full window search
				currval = -search<true>(tree, !black, newDepth, -beta, -alpha);
			} else {
				// nega-scout
				currval = -search<true>(tree, !black, newDepth, -alpha-1, -alpha);
				if (!isInterrupted() && currval >= alpha + 1 && reduced != 0) {
					// full depth
					newDepth += reduced;
					currval = -search<true>(tree, !black, newDepth, -alpha-1, -alpha);
				}
				if (!isInterrupted() && currval >= alpha + 1) {
					// full window search
					currval = -search<true>(tree, !black, newDepth, -beta, -alpha);
				}
			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted()) {
				return false;
			}

			// ソート用に値をセット
			auto index = tree.getIndexByMove(move);
			_rootValues[index] = (currval != alpha) ? (currval.int32()) : (currval.int32() - 1);

			// 値更新
			if (currval > alpha) {
				// update alpha
				alpha = currval;
				best = move;
				tree.updatePv(move, depth);
				if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD ||
						currval >= Value::Mate || currval <= -Value::Mate) {
					showPv(depth / Depth1Ply, tree.getPv(), black ? alpha : -alpha);
				}

				// beta-cut or update best move
				if (alpha >= beta) {
					return alpha;
				}
			}

			count++;
		}

		return alpha;
	}

	/**
	 * aspiration search
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::searchAsp(int depth, Move& best, bool gen /* = true */, Value* pval /* = nullptr */) {

		// tree
		auto& tree = _trees[0];

		auto& moves = tree.getMoves();
		const auto& board = tree.getBoard();

		// 合法手生成
		if (gen) {
			tree.initGenPhase();
			MoveGenerator::generate(board, moves);
		}
		tree.resetGenPhase();

		// 非合法手除去
		for (auto ite = moves.begin(); ite != moves.end();) {
			if (!board.isValidMove(*ite)) {
				ite = moves.remove(ite);
			} else {
				++ite;
			}
		}

		bool hasPrevVal = pval != nullptr && (*pval != -Value::Inf);
		Value baseVal = hasPrevVal ? *pval : 0;
		CONSTEXPR int wmax = 3;
		const Value alphas[wmax] = { baseVal-320, baseVal-1280, -Value::Mate };
		const Value betas[wmax] = { baseVal+320, baseVal+1280, Value::Mate };
		int lower = hasPrevVal ? 0 : wmax - 1;
		int upper = hasPrevVal ? 0 : wmax - 1;
		Value value = alphas[lower];

		while (true) {

			const Value alpha = alphas[lower];
			const Value beta = betas[upper];

			Value currval = searchRoot(tree, depth, alpha, beta, best);

			// 中断判定
			if (isInterrupted()) {
				return false;
			}

			if (currval >= value) {
				value = currval;
			}

			// 値が確定
			if (currval > alpha && currval < beta) {
				break;
			}

			bool retry = false;

			// alpha 値を広げる
			while (currval <= alphas[lower] && lower != wmax - 1) {
				lower++;
				assert(lower < wmax);
				retry = true;
				tree.selectFirstMove();
			}

			// beta 値を広げる
			while (currval >= betas[upper] && upper != wmax - 1) {
				upper++;
				assert(upper < wmax);
				retry = true;
				tree.selectPreviousMove();
			}

			if (!retry) { break; }
		}

		if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD ||
				value >= Value::Mate || value <= -Value::Mate) {
			showEndOfIterate();
		}

		tree.setSortValues(_rootValues);
		tree.sortAll();

		_info.eval = value;
		if (pval != nullptr) {
			*pval = value;
		}

		if (value <= -Value::Mate) {
			return false;
		}

		return true;

	}

	void Searcher::showPv(int depth, const Pv& pv, const Value& value) {
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

	void Searcher::showEndOfIterate() {
		uint64_t node = _info.node + _info.qnode;
		double seconds = _timer.get();

		std::ostringstream oss;
		oss << "    " << std::setw(10) << node;
		oss << ": " << seconds << "sec";

		Loggers::message << oss.str();

	}

	/**
	 * iterative deepening search from root node
	 * @return {負けたか深さ1で中断された場合にfalseを返します。}
	 */
	bool Searcher::idsearch(Move& best) {

		auto& tree = _trees[0];
		bool result = false;
		bool gen = true;

		Value value = -Value::Inf;

		for (int depth = 1; depth <= _config.maxDepth; depth++) {
			bool ok = searchAsp(depth * Depth1Ply + Depth1Ply / 2, best, gen, &value);

			gen = false;

#if DEBUG_ROOT_MOVES
			auto& tree = _trees[0];
			std::ostringstream oss;
			for (auto ite = tree.getBegin(); ite != tree.getEnd(); ite++) {
				oss << ' ' << (*ite).toString() << '[' << tree.getSortValue(ite) << ']';
			}
			Loggers::debug << oss.str();
#endif

#if ENABLE_STORE_PV
			storePv(tree, tree.getPv(), 0);
#endif // ENABLE_STORE_PV

			if (value >= Value::Mate) {
				result = true;
				break;
			}

			if (value <= -Value::Mate) {
				result = false;
				break;
			}

			if (!ok) {
				break;
			}

			result = true;
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
