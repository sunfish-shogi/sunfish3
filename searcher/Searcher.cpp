/* Searcher.cpp
 *
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <iomanip>

#define ENABLE_HISTORY_HEURISTIC			1
#define ENABLE_LMR										1
#define ENABLE_HASH_MOVE							1
#define ENABLE_KILLER_MOVE						0 // should be 0
#define ENABLE_PRECEDE_KILLER					1
#define ENABLE_SHEK_PRESET						1
#define ENABLE_SHEK										1
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

// ハッシュ表の手や killer move が本当に探索済みかチェックする
#define DEBUG_CHECK_PRIOR_MOVE do {\
	bool __debugHashCheck__ = false; \
	if (tree.getBoard().isValidMoveStrict(move)) { /* 不正な手はいずれにせよmakeMoveで弾かれる */ \
		for (auto __ite__ = tree.getBegin(); __ite__ != ite; ++__ite__) { \
			if (*__ite__ == move) { \
				__debugHashCheck__ = true; \
				break; \
			} \
		} \
		if (!__debugHashCheck__) { \
			std::cout << tree.getBoard().toStringCsa() << std::endl; \
			std::cout << move.toString() << std::endl; \
			assert(false); \
		} \
	} \
} while(false)

	/**
	 * sort moves by see
	 */
	void Searcher::sortSee(Tree& tree, Value standPat, Value alpha, bool exceptSmallCapture, bool isQuies) {
		const auto& board = tree.getBoard();
		Move hash1 = tree.getHash1();
		Move hash2 = tree.getHash2();
		Move killer1 = tree.getKiller1();
		Move killer2 = tree.getKiller2();
#if !ENABLE_KILLER_MOVE
		assert(killer1.isEmpty());
		assert(killer2.isEmpty());
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
				if (move == hash1 || move == hash2) {
#ifndef NDEBUG
					DEBUG_CHECK_PRIOR_MOVE;
#endif
					ite = tree.getMoves().remove(ite);
					continue;
				}
			}

			// futility pruning
			if (standPat + tree.estimate(move, _eval) <= alpha) {
				_info.futilityPruning++;
				ite = tree.getMoves().remove(ite);
				continue;
			}

#if SHALLOW_SEE
			value = _see.search<true>(_eval, board, move, -1, Evaluator::PieceInf);
#else
			value = _see.search(_eval, board, move, -1, Value::PieceInf);
#endif

			if (!isQuies) {
				if (move == killer1) {
#if !ENABLE_KILLER_MOVE
					assert(false);
#endif // ENABLE_KILLER_MOVE
					killer1 = Move::empty();
					auto captured = board.getBoardPiece(move.to());
#if ENABLE_PRECEDE_KILLER
					value = Value::Inf;
#else
					Value kvalue = tree.getKiller1Value() + _eval.pieceExchange(captured);
					value = Value::max(value, kvalue);
#endif
				} else if (move == killer2) {
#if !ENABLE_KILLER_MOVE
					assert(false);
#endif // ENABLE_KILLER_MOVE
					killer2 = Move::empty();
					auto captured = board.getBoardPiece(move.to());
#if ENABLE_PRECEDE_KILLER
					value = Value::Inf-1;
#else
					Value kvalue = tree.getKiller2Value() + _eval.pieceExchange(captured);
					value = Value::max(value, kvalue);
#endif
				}
			}

			tree.setSortValue(ite, value.int32());

			ite++;
		}

		if (!isQuies) {
			if (!killer1.isEmpty() && killer1 != hash1 && killer1 != hash2
					&& tree.getKiller1Value() >= Value::Zero
					&& board.isValidMoveStrict(killer1)) {
#if !ENABLE_KILLER_MOVE
				assert(false);
#endif // ENABLE_KILLER_MOVE
				auto ite = tree.addMove(killer1);
				auto captured = board.getBoardPiece(killer1.to());
#if ENABLE_PRECEDE_KILLER
				tree.setSortValue(ite, Value::Inf);
#else
				Value kvalue = tree.getKiller1Value();
				tree.setSortValue(ite, kvalue.int32());
#endif
			}
			if (!killer2.isEmpty() && killer2 != hash1 && killer2 != hash2
					&& tree.getKiller2Value() >= Value::Zero
					&& board.isValidMoveStrict(killer2)) {
#if !ENABLE_KILLER_MOVE
				assert(false);
#endif // ENABLE_KILLER_MOVE
				auto ite = tree.addMove(killer2);
				auto captured = board.getBoardPiece(killer2.to());
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

	}

	/**
	 * sort moves by history
	 */
	void Searcher::sortHistory(Tree& tree, bool exceptHash, bool exceptKiller) {
#if ENABLE_HISTORY_HEURISTIC
		Move hash1 = tree.getHash1();
		Move hash2 = tree.getHash2();
		Move killer1 = tree.getKiller1();
		Move killer2 = tree.getKiller2();
#if !ENABLE_KILLER_MOVE
		assert(killer1.isEmpty());
		assert(killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ) {
			const Move& move = *ite;

			if (exceptHash && (move == hash1 || move == hash2)) {
#ifndef NDEBUG
				DEBUG_CHECK_PRIOR_MOVE;
#endif
				ite = tree.getMoves().remove(ite);
				continue;
			}
			if (exceptKiller && (move == killer1 || move == killer2)) {
#ifndef NDEBUG
				DEBUG_CHECK_PRIOR_MOVE;
#endif
				ite = tree.getMoves().remove(ite);
				continue;
			}

			auto key = History::getKey(move);
			auto data = _history.getData(key);
			auto ratio = History::getRatio(data);
			tree.setSortValue(ite, (int32_t)ratio);

			ite++;
		}

		tree.sortAfterCurrent();
#endif // ENABLE_HISTORY_HEURISTIC

	}

	/**
	 * update history
	 */
	void Searcher::updateHistory(Tree& tree, int depth, const Move& move) {

		int value = std::max(depth / (Depth1Ply/4), 1);
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
		auto& genPhase = tree.getGenPhase();
		const auto& board = tree.getBoard();

		while (true) {

			if (tree.getNext() != tree.getEnd()) {
				move = *tree.getNext();
				tree.selectNextMove();
				return true;
			}

			switch (genPhase) {
			case GenPhase::Hash:
				{
					Move hash1 = tree.getHash1();
					Move hash2 = tree.getHash2();
					if (!hash1.isEmpty() && board.isValidMoveStrict(hash1)) {
						tree.addMove(hash1);
					}
					if (!hash1.isEmpty() && board.isValidMoveStrict(hash2)) {
						tree.addMove(hash2);
					}
					genPhase = GenPhase::Capture;
				}
				break;
			case GenPhase::Capture:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortHistory(tree, true, false);
					genPhase = GenPhase::End;
					break;
					
				} else {
					MoveGenerator::generateCap(board, moves);
					sortSee(tree, Value::Zero, Value::Zero, false, false);
					genPhase = GenPhase::NoCapture;
					break;

				}

			case GenPhase::NoCapture:
				MoveGenerator::generateNoCap(board, moves);
				MoveGenerator::generateDrop(board, moves);
				sortHistory(tree, true, true);
				genPhase = GenPhase::End;
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
		auto& genPhase = tree.getGenPhase();
		const auto& board = tree.getBoard();

		while (true) {

			if (tree.getNext() != tree.getEnd()) {
				move = *tree.getNext();
				tree.selectNextMove();
				return true;
			}

			switch (genPhase) {
			case GenPhase::Hash: // fall through
			case GenPhase::Capture: // fall through
			case GenPhase::NoCapture:
				assert(false);
				break;

			case GenPhase::CaptureOnly:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortHistory(tree, false, false);
					genPhase = GenPhase::End;
					break;

				} else {
					MoveGenerator::generateCap(board, moves);
					if (qply >= 7) {
  					sortSee(tree, standPat, alpha, true, true);
					} else {
  					sortSee(tree, standPat, alpha, false, true);
					}
					genPhase = GenPhase::End;
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

		// スタックサイズの限界
		if (tree.isStackFull()) {
			return standPat;
		}

		// beta-cut
		if (standPat >= beta) {
			return standPat;
		}

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

	/**
	 * nega-max search
	 */
	template <bool pvNode>
	Value Searcher::searchr(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat) {

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
		if (tree.__debug__matchPath("+0052KA -6465FU +5243UM")) {
			std::cout << " ** debug begin **" << std::endl;
			std::cout << tree.__debug__getPath() << std::endl;
			std::cout << "alpha=" << alpha.int32() << " beta=" << beta.int32() << " depth=" << depth << std::endl;
			debug = true;
		}
#endif

		// distance pruning
		Value maxv = Value::Inf - tree.getPly();
		if (alpha >= maxv) {
			return maxv;
		}

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

		_info.node++;

		uint64_t hash = tree.getBoard().getHash();
		bool isNullWindow = (beta == alpha + 1);

		// transposition table
		bool hashOk = false;
		Move hash1 = Move::empty();
		Move hash2 = Move::empty();
		{
			TTE tte;
			_info.hashProbed++;
			if (_tt.get(hash, tte) &&
					(depth < search_param::REC_THRESHOLD ||
					 tte.getDepth() >= search_func::recDepth(depth))) {
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
					hash1 = Move::deserialize16(tte.getMove1(), tree.getBoard());
					hash2 = Move::deserialize16(tte.getMove2(), tree.getBoard());
					if (!hash1.isEmpty()) {
  					hashOk = true;
					}
				}

				if (tte.isMateThreat()) {
					stat.setMateThreat();
				}

				_info.hashHit++;
			}
		}

		Value standPat = tree.getValue() * (black ? 1 : -1);

		if (!tree.isChecking()) {

			// null move pruning
			if (!pvNode && isNullWindow && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
				auto newStat = NodeStat().unsetNullMove();
				int newDepth = search_func::nullDepth(depth);

				_info.nullMovePruningTried++;

				// make move
				tree.makeNullMove();

				Value currval = -searchr<false>(tree, !black, newDepth, -beta, -beta+1, newStat);

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
					return beta;
				}

				// mate threat
				if (currval <= -Value::Mate) {
					stat.setMateThreat();
				}
			}

		}

		// recursive iterative-deepening search
		if (!hashOk && stat.isRecursion() && depth >= search_param::REC_THRESHOLD) {
			auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
			searchr<pvNode>(tree, black, search_func::recDepth(depth), alpha, beta, newStat);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// ハッシュ表から前回の最善手を取得
			TTE tte;
			if (_tt.get(hash, tte)) {
				hash1 = Move::deserialize16(tte.getMove1(), tree.getBoard());
				hash2 = Move::deserialize16(tte.getMove2(), tree.getBoard());
				if (!hash1.isEmpty()) {
					hashOk = true;
				}
			}
		}

		// fail-soft
		Value value = -Value::Inf + tree.getPly();

		int count = 0;
		Move move;
		Move best;
		best.setEmpty();
		tree.initGenPhase();
#if ENABLE_HASH_MOVE
		tree.setHash1(hash1);
		tree.setHash2(hash2);
#else
		tree.setHash1(Move::empty());
		tree.setHash2(Move::empty());
#endif
		while (nextMove(tree, move)) {

			_info.expanded++;

			count++;

			// depth
			int newDepth = depth - Depth1Ply;
                          
			// stat
			NodeStat newStat = NodeStat::Default;

			// alpha value
			Value newAlpha = Value::max(alpha, value);

			const auto& board = tree.getBoard();
			bool isCheckCurr = board.isCheck(move);
			bool isCheckPrev = tree.isChecking();
			bool isCheck = isCheckCurr || isCheckPrev;
			Piece captured = board.getBoardPiece(move.to());

			// extensions
			if (isCheckCurr) {
				// check
				newDepth += search_param::EXT_CHECK;
				_info.checkExtension++;

			} else if (isCheckPrev && count == 1 && tree.getGenPhase() == GenPhase::End && tree.getNext() == tree.getEnd()) {
				// one-reply
				newDepth += search_param::EXT_ONEREP;
				_info.onerepExtension++;

			} else if (!isCheckPrev && stat.isRecapture() && tree.isRecapture()
								 // TODO: 前回の最善のcaptureを除外
								 ) {
				// recapture
				newDepth += search_param::EXT_RECAP;
				newStat.unsetRecapture();
				_info.recapExtension++;

			}

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (newDepth >= Depth1Ply && count != 1 && !isCheck && !stat.isMateThreat() &&
					captured.isEmpty() && (!move.promote() || move.piece() != Piece::Silver) &&
					!tree.isPriorMove(move)) {
				reduced = getReductionDepth(move, isNullWindow);
				newDepth -= reduced;
			}
#endif // ENABLE_LMR

			// futility pruning
			if (!isCheck && newDepth < Depth1Ply * 3 && newAlpha > -Value::Mate) {
				Value futAlpha = newAlpha;
				if (newDepth >= Depth1Ply * 2) { futAlpha -= search_param::EFUT_MGN2; }
				else if (newDepth >= Depth1Ply) { futAlpha -= search_param::EFUT_MGN1; }
				if (standPat + tree.estimate(move, _eval) <= futAlpha) {
					value = newAlpha;
					_info.futilityPruning++;
					continue;
				}
			}

			if (newDepth < Depth1Ply * 2 && isNullWindow && !isCheck &&
					captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
					!tree.isPriorMove(move) &&
					_see.search<true>(_eval, board, move, -1, 0) < Value::Zero) {
				value = newAlpha;
				continue;
			}

			// make move
			if (!tree.makeMove(move, _eval)) {
				continue;
			}

			Value newStandPat = tree.getValue() * (black ? 1 : -1);

			// extended futility pruning
			if (!isCheck && newAlpha > -Value::Mate) {
				if ((newDepth < Depth1Ply && newStandPat <= newAlpha) ||
						(newDepth < Depth1Ply * 2 && newStandPat + search_param::EFUT_MGN1 <= newAlpha) ||
						(newDepth < Depth1Ply * 3 && newStandPat + search_param::EFUT_MGN2 <= newAlpha)) {
					tree.unmakeMove();
					value = newAlpha;
					_info.extendedFutilityPruning++;
					continue;
				}
			}

			// reccursive call
			Value currval;
			if (count == 1) {
				currval = -searchr<pvNode>(tree, !black, newDepth, -beta, -newAlpha, newStat);

			} else {
				// nega-scout
				currval = -searchr<false>(tree, !black, newDepth, -newAlpha-1, -newAlpha, newStat);

#if 1
				if (!isInterrupted() && currval > newAlpha && currval < beta && reduced > 0) {
					newDepth += reduced;
					currval = -searchr<pvNode>(tree, !black, newDepth, -newAlpha-1, -newAlpha, newStat);
				}

				if (!isInterrupted() && currval > newAlpha && currval < beta && !isNullWindow) {
					currval = -searchr<pvNode>(tree, !black, newDepth, -beta, -newAlpha, newStat);
				}
#else
				if (!isInterrupted() && currval > newAlpha &&  currval < beta) {
					newDepth += reduced;
					currval = -searchr<pvNode>(tree, !black, newDepth, -beta, -newAlpha, newStat);
				}
#endif

			}

			// unmake move
			tree.unmakeMove();

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値更新
			if (currval > value) {
				value = currval;
				best = move;
				if (!isNullWindow) {
					tree.updatePv(move, depth);
				}

				// beta-cut
				if (currval >= beta) {
					_info.failHigh++;
					if (count == 1) {
						_info.failHighFirst++;
					}
#if ENABLE_KILLER_MOVE
					Value kvalue = currval - standPat - _eval.pieceExchange(captured);
					tree.addKiller(move, kvalue);
#endif // ENABLE_KILLER_MOVE
					break;
				}
			}

		}

		if (!best.isEmpty()) {
			if (!isNullWindow && value > alpha) {
				updateHistory(tree, depth, best);
			}

			// TODO: GHI対策
			TTStatus status = _tt.entry(hash, alpha, beta, value, depth, tree.getPly(), Move::serialize16(best), stat);
			switch (status) {
				case TTStatus::New: _info.hashNew++; break;
				case TTStatus::Update: _info.hashUpdate++; break;
				case TTStatus::Collide: _info.hashCollision++; break;
				case TTStatus::Reject: _info.hashReject++; break;
				default: break;
			}
			_info.hashStore++;
		}

		return value;

	}
	template Value Searcher::searchr<true>(Tree&, bool, int, Value, Value, NodeStat);
	template Value Searcher::searchr<false>(Tree&, bool, int, Value, Value, NodeStat);

	/**
	 * aspiration search
	 */
	Value Searcher::asp(Tree& tree, bool black, int depth, AspSearchStatus& astat) {

		const int wcnt = 3;
		const Value alphas[wcnt] = { astat.base-320, astat.base-1280, -Value::Inf };
		const Value betas[wcnt] = { astat.base+320, astat.base+1280, Value::Inf };

		if (astat.base == -Value::Inf) {
			astat.lower = wcnt - 1;
			astat.upper = wcnt - 1;
		}

		while (true) {

			Value alpha = std::max(astat.alpha, alphas[astat.lower]);
			Value beta = std::max(astat.alpha + 1, betas[astat.upper]);

			Value value = -searchr<true>(tree, black, depth, -beta, -alpha);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値が確定 (alpha < value < beta)
			if (value > alpha && value < beta) {
				return value;
			}

			// alpha-cut
			if (value <= astat.alpha) {
				return value;
			}

			bool retry = false;

			// alpha 値を広げる
			while (value <= alphas[astat.lower] && astat.lower != wcnt - 1) {
				astat.lower++;
				assert(astat.lower < wcnt);
				retry = true;
			}

			// beta 値を広げる
			while (value >= betas[astat.upper] && astat.upper != wcnt - 1) {
				astat.upper++;
				assert(astat.upper < wcnt);
				retry = true;
			}

			if (!retry) {
				return value;
			}

		}

	}

	/**
	 * search from root node
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::search(int depth, Move& best, bool gen /* = true */, Value* prevval /* = nullptr */) {

		// tree
		auto& tree = _trees[0];

		// sort values
		int sortValues[1024];

		auto& moves = tree.getMoves();
		const auto& board = tree.getBoard();
		bool black = board.isBlack();

		Value value = -Value::Inf;

		AspSearchStatus astat;
		astat.base = prevval != nullptr ? *prevval : -Value::Inf;
		astat.upper = 0;
		astat.lower = 0;

		// 合法手生成
		if (gen) {
			tree.initGenPhase();
			MoveGenerator::generate(board, moves);
		}
		tree.resetGenPhase();

		int count = 0;
		Move move;
		while (nextMove(tree, move)) {

			count++;

			// depth
			int newDepth = depth - Depth1Ply;

			bool isCheck = tree.isChecking() || board.isCheck(move);

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (newDepth >= Depth1Ply && count != 1 && !isCheck &&
					(!move.promote() || move.piece() != Piece::Silver) &&
					!tree.isPriorMove(move)) {

				reduced = getReductionDepth(move, false);
				newDepth -= reduced;

			}
#endif // ENABLE_LMR

			// make move
			if (!tree.makeMove(move, _eval)) {
				tree.rejectPreviousMove();
				continue;
			}

			Value currval;

			if (value == -Value::Inf) {

				// aspiration search
				astat.alpha = value;
				currval = asp(tree, !black, newDepth, astat);

			} else {

				// nega-scout
				currval = -searchr<true>(tree, !black, newDepth, -value-1, -value);
				if (!isInterrupted() && currval >= value + 1) {
					// full window search
					newDepth += reduced;
					currval = -searchr<true>(tree, !black, newDepth, -Value::Inf, -value);
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
			sortValues[index] = (currval != value) ? (currval.int32()) : (currval.int32() - 1);

			// 値更新
			if (currval > value) {
				best = move;
				value = currval;
				tree.updatePv(move, depth);
				if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD) {
					showPv(depth / Depth1Ply, tree.getPv(), black ? value : -value, count == 1);
				}
			}
		}

		if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD) {
			showEndOfIterate();
		}

		tree.setSortValues(sortValues);
		tree.sortAll();

		_info.eval = value;
		if (prevval != nullptr) {
			*prevval = value;
		}

		if (value <= -Value::Mate) {
			return false;
		}

		return true;

	}

	void Searcher::showPv(int depth, const Pv& pv, const Value& value, bool isFirst) {
		uint64_t node = _info.node + _info.qnode;

		std::ostringstream oss;
		if (isFirst) {
			oss << std::setw(2) << depth << ": ";
		} else {
			oss << "    ";
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
			bool ok = search(depth * Depth1Ply + Depth1Ply / 2, best, gen, &value);

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

			if (!ok) {
				break;
			}

			if (value >= Value::Mate) {
				result = true;
				break;
			}

			if (value <= -Value::Mate) {
				result = false;
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

		bool result = search(depth, best);

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
