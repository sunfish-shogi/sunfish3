/* Searcher.cpp
 *
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <iomanip>

#define ENABLE_SEE										1
#define ENABLE_HISTORY_HEURISTIC			1
#define ENABLE_LMR										1
#define ENABLE_HASH_MOVE							1
#define SHOW_ROOT_MOVES								0

namespace sunfish {

	namespace search_param {
		constexpr int FUT_MGN = 400;
		constexpr int EXT_CHECK = Searcher::Depth1Ply;
		constexpr int EXT_ONEREP = Searcher::Depth1Ply * 1 / 2;
		constexpr int EXT_RECAP = Searcher::Depth1Ply * 1 / 4;
	}

	/**
	 * 前処理
	 */
	void Searcher::before() {

		// 探索情報収集準備
		_info.node = 0;
		_info.time = 0.0;
		_info.nps = 0;
		_info.eval = Value::Zero;
		_info.failHigh = 0;
		_info.failHighFirst = 0;
		_info.hashProbed = 0;
		_info.hashExact = 0;
		_info.hashLower = 0;
		_info.hashUpper = 0;
		_info.nullMovePruning = 0;
		_info.nullMovePruningTried = 0;
		_info.futilityPruning = 0;
		_info.extendedFutilityPruning = 0;
		_info.expanded = 0;
		_info.checkExtension = 0;
		_info.onerepExtension = 0;
		_info.recapExtension = 0;
		_timer.set();

		// transposition table
		_tt.evolve(); // 世代更新

		// hisotory heuristic
		_history.reduce();

	}

	/**
	 * 後処理
	 */
	void Searcher::after() {

		auto& tree = _trees[0];

		// 探索情報収集
		_info.time = _timer.get();
		_info.nps = _info.node / _info.time;
		_info.move = tree.getPv().get(0);

	}

	/**
	 * 探索中断判定
	 */
	inline bool Searcher::isInterrupted() {
		if (_timer.get() >= _config.limitSeconds) {
			return true;
		}
		return false;
	}

	/**
	 * sort moves by see
	 */
	void Searcher::sortSee(Tree& tree, bool plusOnly /* = false */) {
#if ENABLE_SEE
		const auto& board = tree.getBoard();

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ite++) {
			Value value = _see.search(_eval, board, *ite);
			tree.setSortValue(ite, value.int32());
		}

		tree.sortAfterCurrent();

		if (plusOnly) {
			for (auto ite = tree.getNext(); ite != tree.getEnd(); ite++) {
				if (tree.getSortValue(ite) <= 0) {
					tree.removeAfter(ite);
					break;
				}
			}
		}
#endif // ENABLE_SEE

	}

	/**
	 * 優先的に探索する手を追加
	 */
	void Searcher::addPriorMove(Tree& tree, const Move& move) {

		if (!move.isEmpty()) {
			tree.getPriorMoves().add(move);
		}

	}

	/**
	 * PriorMove に含まれる手かチェックする。
	 */
	bool Searcher::isPriorMove(Tree& tree, const Move& move) {

		auto& priorMoves = tree.getPriorMoves();

		if (priorMoves.find(move) != priorMoves.end()) {
			return true;
		}
		return false;

	}

	/**
	 * PriorMove に含まれる手を除去する。
	 */
	void Searcher::removePriorMove(Tree& tree) {

		auto& priorMoves = tree.getPriorMoves();

		for (auto ite = tree.getNext(); ite != tree.getEnd(); ite++) {
			if (priorMoves.find(*ite) != priorMoves.end()) {
				ite = tree.remove(ite) - 1; // Moves::Iterator の -1 は安全
			}
		}

	}

	/**
	 * sort moves by history
	 */
	void Searcher::sortHistory(Tree& tree) {
#if ENABLE_HISTORY_HEURISTIC
		for (auto ite = tree.getNext(); ite != tree.getEnd(); ite++) {
			auto key = History::getKey(*ite);
			auto data = _history.getData(key);
			auto ratio = History::getRatio(data);
			tree.setSortValue(ite, (int32_t)ratio);
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
		auto good = History::getGoodCount(data);
		auto appear = History::getAppearCount(data);

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
		auto& priorMoves = tree.getPriorMoves();
		auto& genPhase = tree.getGenPhase();
		auto& board = tree.getBoard();

		while (true) {

			if (tree.getNext() != tree.getEnd()) {
				move = *tree.getNext();
				tree.selectNextMove();
				return true;
			}

			switch (genPhase) {
			case GenPhase::Prior:
				for (auto ite = priorMoves.begin(); ite != priorMoves.end(); ite++) {
					if (board.isValidMoveStrict(*ite)) {
						moves.add(*ite); // TODO: create Tree's method
					}
				}
				genPhase = GenPhase::Capture;
				break;

			case GenPhase::Capture:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					removePriorMove(tree);
					sortHistory(tree);
					genPhase = GenPhase::End;
					break;
					
				} else {
					MoveGenerator::generateCap(board, moves);
					removePriorMove(tree);
					sortSee(tree);
					genPhase = GenPhase::NoCapture;
					break;

				}

			case GenPhase::NoCapture:
				MoveGenerator::generateNoCap(board, moves);
				MoveGenerator::generateDrop(board, moves);
				removePriorMove(tree);
				sortHistory(tree);
				genPhase = GenPhase::End;
				break;

			case GenPhase::CaptureOnly:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortHistory(tree);
					genPhase = GenPhase::End;
					break;

				} else {
					MoveGenerator::generateCap(board, moves);
					sortSee(tree, true);
					genPhase = GenPhase::End;
					break;

				}

			case GenPhase::TacticalOnly:
				if (tree.isChecking()) {
					MoveGenerator::generateEvasion(board, moves);
					sortHistory(tree);
					genPhase = GenPhase::End;
					break;

				} else {
					MoveGenerator::generateTactical(board, moves);
					sortSee(tree, true);
					genPhase = GenPhase::End;
					break;

				}

			case GenPhase::End:
				return false;
			}
		}

	}

	/**
	 * reject current move
	 */
	void Searcher::rejectMove(Tree& tree) {

		tree.remove(tree.getNext()-1);

	}

	/**
	 * get estimated value
	 */
	Value Searcher::estimate(Tree& tree, const Move& move) {

		const auto& board = tree.getBoard();
		Position to = move.to();
		bool promote = move.promote();
		Piece captured = board.getBoardPiece(to);
		Value est = 0;

		if (promote) {
			est += _eval.piecePromote(move.piece());
		}

		if (!captured.isEmpty()) {
			est += _eval.pieceExchange(captured);
		}

		return est;

	}

	/**
	 * quiesence search
	 */
	Value Searcher::qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta) {

		_info.node++;

		// stand-pat
		Value sign(black ? 1 : -1);
		Value value = tree.getValue() * sign;

		// スタックサイズの限界
		if (tree.isStackFull()) {
			return value;
		}

		// beta-cut
		if (value >= beta) {
			return value;
		}

		// 合法手生成
		auto& moves = tree.getMoves();
		moves.clear();
		if (qply < 7) {
			tree.initGenPhase(GenPhase::TacticalOnly);
		} else {
			tree.initGenPhase(GenPhase::CaptureOnly);
		}

		Move move;
		while (nextMove(tree, move)) {
			// alpha value
			Value newAlpha = Value::max(alpha, value);

			// make move
			if (!tree.makeMove(move, _eval)) {
				continue;
			}

			// reccursive call
			Value currval;
			currval = -qsearch(tree, !black, qply + 1, -beta, -newAlpha);

			// unmake move
			tree.unmakeMove(move);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値更新
			if (currval > value) {
				value = currval;
				tree.updatePv(move);

				// beta-cut
				if (currval >= beta) {
					break;
				}
			}
		}

		return value;

	}

	/**
	 * nega-max search
	 */
	template <bool pvNode>
	Value Searcher::searchr(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat) {

		const auto& board = tree.getBoard();

		// distance pruning
		Value maxv = Value::Inf - tree.getPly();
		if (alpha >= maxv) {
			return maxv;
		}

		// スタックサイズの限界
		if (tree.isStackFull()) {
			return tree.getValue() * (black ? 1 : -1);
		}

		// 静止探索の結果を返す。
		if (!tree.isChecking() && depth <= 0) {
			return qsearch(tree, black, 0, alpha, beta);
		}

		_info.node++;

		uint64_t hash = board.getHash();

		// transposition table
		TTE tte;
		bool hashOk = false;
		Move hash1 = Move::empty();
		Move hash2 = Move::empty();
		_info.hashProbed++;
		if (_tt.get(hash, tte)) {
			Value ttv = tte.getValue(tree.getPly());
			switch (tte.getValueType()) {
			case TTE::Exact: // 確定
				if (!pvNode && stat.isHashCut() && tte.isSuperior(depth)) {
					_info.hashExact++;
					return ttv;
				}
				hash1 = tte.getMoves().getMove1();
				hash2 = tte.getMoves().getMove2();
				hashOk = true;
				break;
			case TTE::Lower: // 下界値
				if (ttv >= beta) {
					if (!pvNode && stat.isHashCut() && tte.isSuperior(depth)) {
						_info.hashLower++;
						return ttv;
					}
					hash1 = tte.getMoves().getMove1();
					hash2 = tte.getMoves().getMove2();
					hashOk = true;
				}
				break;
			case TTE::Upper: // 上界値
				if (!pvNode && stat.isHashCut() && ttv <= alpha && tte.isSuperior(depth)) {
					_info.hashUpper++;
					return ttv;
				}
				break;
			}
		}

		Value standPat = tree.getValue() * (black ? 1 : -1);

		if (!tree.isChecking()) {

			// null move pruning
			if (!pvNode && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
				auto newStat = NodeStat().unsetNullMove();
#if 0
				// same to bonanza
				int newDepth = (depth < Depth1Ply * 26 / 4 ? depth - Depth1Ply * 12 / 4 :
												(depth <= Depth1Ply * 30 / 4 ? Depth1Ply * 14 / 4 : depth - Depth1Ply * 16 / 4));
#else
				int newDepth = depth - Depth1Ply * 7 / 2;
#endif

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
					tree.updatePv();
					_info.nullMovePruning++;
					return beta;
				}
			}

		}

		// recursive iterative-deepening search
		if (!hashOk && depth >= Depth1Ply * 3) {
			auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
			searchr<pvNode>(tree, black, depth - Depth1Ply, alpha, beta, newStat);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// ハッシュ表から前回の最善手を取得
			if (_tt.get(hash, tte)) {
				hash1 = tte.getMoves().getMove1();
				hash2 = tte.getMoves().getMove2();
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
		addPriorMove(tree, hash1);
		addPriorMove(tree, hash2);
#endif // ENABLE_HASH_MOVE
		while (nextMove(tree, move)) {

			_info.expanded++;

			count++;

			// depth
			int newDepth = depth - Depth1Ply;
                          
			// stat
			NodeStat newStat = NodeStat::Default;

			// alpha value
			Value newAlpha = Value::max(alpha, value);

			bool isCheckCurr = board.isCheck(move);
			bool isCheckPrev = tree.isChecking();
			bool isCheck = isCheckCurr || isCheckPrev;

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
			if (newDepth >= Depth1Ply && count != 1 && !isCheck &&
					(!move.promote() || move.piece() != Piece::Silver) &&
					isPriorMove(tree, move)) {

				reduced = getReductionDepth(move, beta == newAlpha + 1);
				newDepth -= reduced;

			}
#endif // ENABLE_LMR

			// futility pruning
			if (!isCheck && standPat + estimate(tree, move) + search_param::FUT_MGN <= newAlpha) {
				value = newAlpha;
				_info.futilityPruning++;
				continue;
			}

			// make move
			if (!tree.makeMove(move, _eval)) {
				continue;
			}

			Value newStandPat = tree.getValue() * (black ? 1 : -1);

			// extended futility pruning
			if (!isCheck && newStandPat + search_param::FUT_MGN <= newAlpha) {
				tree.unmakeMove(move);
				value = newAlpha;
				_info.extendedFutilityPruning++;
				continue;
			}

			// reccursive call
			Value currval;
			if (count == 1) {
				currval = -searchr<pvNode>(tree, !black, newDepth, -beta, -newAlpha, newStat);

			} else {
				// nega-scout
				currval = -searchr<false>(tree, !black, newDepth, -newAlpha-1, -newAlpha, newStat);

				if (!isInterrupted() && currval > newAlpha &&  currval < beta) {
					newDepth += reduced;
					currval = -searchr<pvNode>(tree, !black, newDepth, -beta, -newAlpha, newStat);
				}

			}

			// unmake move
			tree.unmakeMove(move);

			// 中断判定
			if (isInterrupted()) {
				return Value::Zero;
			}

			// 値更新
			if (currval > value) {
				value = currval;
				best = move;
				tree.updatePv(move);

				// beta-cut
				if (currval >= beta) {
					_info.failHigh++;
					if (count == 1) {
						_info.failHighFirst++;
					}
					break;
				}
			}

		}

		if (!best.isEmpty()) {
			if (value > alpha) {
				updateHistory(tree, depth, best);
			}

			// TODO: GHI対策
			_tt.entry(hash, alpha, beta, value, depth, tree.getPly(), stat, best);
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

			// 最大 window 幅のため値が確定
			if (astat.upper == wcnt-1 && astat.lower == wcnt-1) {
				return value;
			}

			// alpha 値を広げる
			while (value <= alphas[astat.lower]) {
				astat.lower++;
				assert(astat.lower < wcnt);
			}

			// beta 値を広げる
			while (value >= betas[astat.upper]) {
				astat.upper++;
				assert(astat.upper < wcnt);
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
			int newDepth = depth;

			bool isCheck = tree.isChecking() || board.isCheck(move);

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (newDepth >= Depth1Ply && count != 1 && !isCheck &&
					(!move.promote() || move.piece() != Piece::Silver) &&
					isPriorMove(tree, move)) {

				reduced = getReductionDepth(move, false);
				newDepth -= reduced;

			}
#endif // ENABLE_LMR

			// make move
			if (!tree.makeMove(move, _eval)) {
				rejectMove(tree);
				continue;
			}

			Value currval;

			if (value == -Value::Inf) {

				// aspiration search
				astat.alpha = value;
				currval = asp(tree, !black, newDepth - Depth1Ply, astat);

			} else {

				// nega-scout
				currval = -searchr<true>(tree, !black, newDepth - Depth1Ply, -value-1, -value);
				if (!isInterrupted() && currval >= value + 1) {
					// full window search
					newDepth += reduced;
					currval = -searchr<true>(tree, !black, newDepth - Depth1Ply, -Value::Inf, -value);
				}

			}

			// unmake move
			tree.unmakeMove(move);

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
				tree.updatePv(move);
			}
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

	void Searcher::showPv(int depth, const Pv& pv, const Value& value) {
		auto realDepth = depth / Depth1Ply;
		double seconds = _timer.get();
		uint64_t node = _info.node;

		std::ostringstream oss;
		oss << std::setw(2) << realDepth;
		oss << ": " << std::setw(10) << node;
		oss << ": " << pv.toString();
		oss << ": " << value.int32();
		oss << " (" << seconds << "sec)";

		Loggers::message << oss.str();

	}

	/**
	 * iterative deepening search from root node
	 * @return {負けたか深さ1で中断された場合にfalseを返します。}
	 */
	bool Searcher::idsearch(Move& best) {

		auto& tree = _trees[0];
		const auto& board = tree.getBoard();
		bool black = board.isBlack();
		bool result = false;
		bool gen = true;

		// 前処理
		before();

		// 最大深さ
		int maxDepth = _config.maxDepth * Depth1Ply;

		Value value = -Value::Inf;

		for (int depth = Depth1Ply; depth <= maxDepth; depth += Depth1Ply) {
			bool ok = search(depth, best, gen, &value);

			gen = false;

#if !defined(NDEBUG) && SHOW_ROOT_MOVES
			auto& tree = _trees[0];
			std::ostringstream oss;
			for (auto ite = tree.getBegin(); ite != tree.getEnd(); ite++) {
				oss << ' ' << (*ite).toString() << '[' << tree.getSortValue(ite) << ']';
			}
			Loggers::debug << oss.str();
#endif

			showPv(depth, tree.getPv(), black ? value : -value);

			if (!ok) {
				break;
			}

			result = true;
		}

		// 後処理
		after();

		return result;

	}

	/**
	 * 指定した局面に対して探索を実行します。
	 * @return {負けたか中断された場合にfalseを返します。}
	 */
	bool Searcher::search(const Board& initialBoard, Move& best) {

		// 前処理
		before();

		// 最大深さ
		int depth = _config.maxDepth * Depth1Ply;

		// ツリーの初期化
		auto& tree = _trees[0];
		tree.init(initialBoard, _eval);

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
		before();

		// ツリーの初期化
		auto& tree = _trees[0];
		tree.init(initialBoard, _eval);

		bool result = idsearch(best);

		// 後処理
		after();

		return result;

	}

}
