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
		CONSTEXPR int EFUT_MGN1 = 800;
		CONSTEXPR int EFUT_MGN2 = 800;
		CONSTEXPR int EXT_CHECK = Searcher::Depth1Ply;
		CONSTEXPR int EXT_ONEREP = Searcher::Depth1Ply * 1 / 2;
		CONSTEXPR int EXT_RECAP = Searcher::Depth1Ply * 1 / 4;
		CONSTEXPR int EXT_RECAP2 = Searcher::Depth1Ply * 1 / 2;
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
		// TODO: 全treeに適用
		auto& tree = _trees[0];
		tree.init(initialBoard, _eval, _record);

		// 探索情報収集準備
		memset(&_info, 0, sizeof(_info));
		_timer.set();

		// transposition table
		_tt.evolve(); // 世代更新

		// hisotory heuristic
		_history.reduce();

		_forceInterrupt = false;
		_isRunning = true;

		_timeManager.init();
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

		// ツリーの解放
		// TODO: 全treeに適用
		tree.release(_record);

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
	void Searcher::sortSee(Tree& tree, int offset, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies) {
		const auto& board = tree.getBoard();
		auto& node = tree.getCurrentNode();
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
	bool Searcher::nextMove(Tree& tree) {

		auto& moves = tree.getMoves();
		auto& node = tree.getCurrentNode();
		const auto& board = tree.getBoard();

		while (true) {

			if (!tree.isThroughPhase() &&
					tree.getNextMove() != tree.getEnd()) {
				tree.selectNextMove();
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
						_info.expandHashMove++;
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
					if (qply >= 7) {
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
			if (isInterrupted()) {
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
				Value val = _see.search<false>(board, move, -1, Value::PieceInf) - capVal + 1;
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
				_info.shekEqual++;
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
					if (stat.isHashCut() && isNullWindow) {
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
					hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());

					if (tte.isMateThreat()) {
						stat.setMateThreat();
					}
				}
				_info.hashHit++;
			}
		}

		Value standPat = tree.getValue() * (black ? 1 : -1);

		int count = 0;
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
			if (isNullWindow && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
				auto newStat = NodeStat().unsetNullMove();
				int newDepth = search_func::nullDepth(depth);

				_info.nullMovePruningTried++;

				// make move
				tree.makeNullMove();

				Value currval = -search(tree, !black, newDepth, -beta, -beta+1, newStat);

				// unmake move
				tree.unmakeNullMove();

				// 中断判定
				if (isInterrupted()) {
					return Value::Zero;
				}

				// beta-cut
				if (currval >= beta) {
					tree.updatePvNull(depth);
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
		if (hashMove.isEmpty() && stat.isRecursion() && depth >= search_param::REC_THRESHOLD) {
			auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
			search(tree, black, search_func::recDepth(depth), alpha, beta, newStat);

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
#else
		tree.setHash(Move::empty());
#endif
		while (nextMove(tree)) {
			Move move = *tree.getCurrentMove();

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
			Piece captured = board.getBoardPiece(move.to());

			if (!isCheckCurr && captured.isEmpty() &&
					(!move.promote() || move.piece() == Piece::Silver)) {
				tree.getCurrentNode().histMoves.add(move);
			}

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				_info.checkExtension++;

			} else if (isCheckPrev && count == 0 && tree.getGenPhase() == GenPhase::End && tree.getNextMove() == tree.getEnd()) {
				// one-reply
				newDepth += search_param::EXT_ONEREP;
				_info.onerepExtension++;

			} else if (!isCheckPrev && stat.isRecapture() && tree.isRecapture() &&
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
				_info.recapExtension++;

			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (count != 0 && newDepth >= Depth1Ply && !isCheckPrev && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
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
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move) &&
					_see.search<true>(board, move, -1, 0) < Value::Zero) {
				count++;
				continue;
			}

			// make move
			if (!tree.makeMove(_eval)) {
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
				currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);

			} else {
				// nega-scout
				currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);

				if (!isInterrupted() && currval > alpha && reduced > 0) {
					newDepth += reduced;
					currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);
				}

				if (!isInterrupted() && currval > alpha && currval < beta && !isNullWindow) {
					currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);
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
					tree.updatePv(depth);
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

	/**
	 * search on root node
	 */
	Value Searcher::searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best,
			bool forceFullWindow /* = false */) {
		const auto& board = tree.getBoard();
		bool black = board.isBlack();
		int count = 0;
		Value oldAlpha = alpha;

		while (nextMove(tree)) {
			Move move = *tree.getCurrentMove();

			// depth
			int newDepth = depth - Depth1Ply;

			bool isCheckPrev = board.isChecking(); // TODO: tree.isChecking を使えるようにする
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
			if (count != 0 && newDepth >= Depth1Ply * 2 && !isCheckPrev &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver)) {
#if 0
				reduced = getReductionDepth(move, false);
#else
				reduced = Depth1Ply;
#endif
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

			// make move
			bool ok = tree.makeMove(_eval);
			assert(ok);

			Value currval;

			if (forceFullWindow) {
				currval = -search(tree, !black, newDepth, -beta, -oldAlpha);

			} else if (count == 0) {
				// full window search
				currval = -search(tree, !black, newDepth, -beta, -alpha);

			} else {
				// nega-scout
				currval = -search(tree, !black, newDepth, -alpha-1, -alpha);

				if (!isInterrupted() && currval >= alpha + 1 && reduced != 0) {
					// full depth
					newDepth += reduced;
					currval = -search(tree, !black, newDepth, -alpha-1, -alpha);
				}

				if (!isInterrupted() && currval >= alpha + 1) {
					// full window search
					currval = -search(tree, !black, newDepth, -beta, -alpha);
				}
			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted()) {
				return alpha;
			}

			if (count == 0 && currval <= alpha && currval > -Value::Mate) {
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

			count++;
		}

		return alpha;
	}

	/**
	 * aspiration search
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::searchAsp(int depth, Move& best, Value* pval /* = nullptr */) {

		// tree
		auto& tree = _trees[0];

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

			value = searchRoot(tree, depth, alpha, beta, best);

			// 中断判定
			if (isInterrupted()) {
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
				tree.selectFirstMove();
			}

			// beta 値を広げる
			while (value >= betas[upper] && upper != wmax - 1) {
				upper++;
				assert(upper < wmax);
				retry = true;
				tree.selectPreviousMove();
			}

			if (!retry) { break; }
		}

		if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD ||
				value >= Value::Mate || value <= -Value::Mate) {
			showEndOfIterate(depth / Depth1Ply);
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

		int limit = _config.limitEnable ? _config.limitSeconds : 0;
		if (!_config.ponder && _timeManager.isEasy(limit, _timer.get())) {
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

	void Searcher::showEndOfIterate(int depth) {
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
		// tree
		auto& tree = _trees[0];

		auto& moves = tree.getMoves();
		const auto& board = tree.getBoard();

		// 合法手生成
		tree.initGenPhase();
		MoveGenerator::generate(board, moves);
		tree.resetGenPhase();

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

		auto& tree = _trees[0];
		bool result = false;

		generateMovesOnRoot();

		Value value = searchRoot(tree, Depth1Ply, -Value::Inf, Value::Inf, best, true);
		tree.setSortValues(_rootValues);
		tree.sortAll();

		for (int depth = 1; depth <= _config.maxDepth; depth++) {
			bool cont = searchAsp(depth * Depth1Ply + Depth1Ply / 2, best, &value);

#if DEBUG_ROOT_MOVES
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
