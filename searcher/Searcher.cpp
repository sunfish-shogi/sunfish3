/* Searcher.cpp
 * 
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"

#define ENABLE_LMR					1

#define FUT_MGN							400

namespace sunfish {

	/**
	 * 前処理
	 */
	void Searcher::before() {

		// 探索情報収集準備
		_info.node = 0;
		_info.time = 0.0;
		_info.nps = 0;
		_info.eval = Value::Zero;
		_info.hashPruning = 0;
		_info.nullMovePruning = 0;
		_info.futilityPruning = 0;
		_info.extendedFutilityPruning = 0;
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
	 * sort moves by see
	 */
	void Searcher::sortSee(Tree& tree, bool plusOnly /* = false */) {

		auto& moves = tree.getMoves();
		const auto& board = tree.getBoard();

		for (auto ite = tree.getIterator(); ite != moves.end(); ite++) {
			Value value = _see.search(_eval, board, *ite);
			ite->setExt(value.int32(), -10000, 10000);
		}

		moves.sortDesc(tree.getIterator(), moves.end());

		if (plusOnly) {
			for (auto ite = tree.getIterator(); ite != moves.end(); ite++) {
				if (ite->ext(-10000, 10000) <= 0) {
					moves.removeAfter(ite);
					break;
				}
			}
		}

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

		auto& moves = tree.getMoves();
		auto& priorMoves = tree.getPriorMoves();

		for (auto ite = tree.getIterator(); ite != moves.end(); ite++) {
			if (priorMoves.find(*ite) != priorMoves.end()) {
				ite = moves.remove(ite) - 1;
			}
		}

	}

	/**
	 * sort moves by history
	 */
	void Searcher::sortHistory(Tree& tree) {

		auto& moves = tree.getMoves();

		for (auto ite = tree.getIterator(); ite != moves.end(); ite++) {
			unsigned h = _history.get(*ite);
			ite->setExt(h, 0, History::Scale-1);
		}

		moves.sortDesc(tree.getIterator(), moves.end());

	}

	/**
	 * update history
	 */
	void Searcher::updateHistory(Tree& tree, int depth, const Move& move) {

		auto& moves = tree.getMoves();

		for (auto ite = moves.begin(); ite != moves.end(); ite++) {

			if (*ite == move) {
				_history.add(*ite, depth, depth);
				break;
			} else {
				_history.add(*ite, depth, 0);
			}

		}

	}

	/**
	 * get next move
	 */
	bool Searcher::nextMove(Tree& tree, Move& move) {

		auto& moves = tree.getMoves();
		auto& priorMoves = tree.getPriorMoves();
		auto& genPhase = tree.getGenPhase();
		auto& ite = tree.getIterator();
		auto& board = tree.getBoard();

		while (true) {

			if (ite != moves.end()) {
				move = *ite;
				ite++;
				return true;
			}

			switch (genPhase) {
			case GenPhase::Prior:
				for (auto ite = priorMoves.begin(); ite != priorMoves.end(); ite++) {
					if (board.isValidMoveStrict(*ite)) {
						moves.add(*ite);
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
				removePriorMove(tree);
				sortHistory(tree);
				genPhase = GenPhase::Drop;
				break;

			case GenPhase::Drop:
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

		auto& moves = tree.getMoves();
		auto& ite = tree.getIterator();

		ite = moves.remove(ite-1);

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
		if (_tt.get(hash, tte)) {
			Value ttv = tte.getValue(tree.getPly());
			switch (tte.getValueType()) {
			case TTE::Exact: // 確定
				if (!pvNode && stat.isHashCut() && tte.isSuperior(depth)) {
					_info.hashPruning++;
					return ttv;
				}
				hash1 = tte.getMoves().getMove1();
				hash2 = tte.getMoves().getMove2();
				hashOk = true;
				break;
			case TTE::Lower: // 下界値
				if (ttv >= beta) {
					if (!pvNode && stat.isHashCut() && tte.isSuperior(depth)) {
						_info.hashPruning++;
						return ttv;
					}
					hash1 = tte.getMoves().getMove1();
					hash2 = tte.getMoves().getMove2();
					hashOk = true;
				}
				break;
			case TTE::Upper: // 上界値
				if (!pvNode && stat.isHashCut() && ttv <= alpha && tte.isSuperior(depth)) {
					_info.hashPruning++;
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
				int newDepth = depth / 2;

				// make move
				tree.makeNullMove();

				Value currval = -searchr<false>(tree, !black, newDepth, -beta, -beta+1, newStat);

				// unmake move
				tree.unmakeNullMove();

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
		addPriorMove(tree, hash1);
		addPriorMove(tree, hash2);
		while (nextMove(tree, move)) {

			count++;

			// depth
			int newDepth = depth;

			// alpha value
			Value newAlpha = Value::max(alpha, value);

			bool isCheck = tree.isChecking() || board.isCheck(move);

			// late move reduction
			int reduced = 0;
#if ENABLE_LMR
			if (newDepth >= Depth1Ply && count != 1 &&
					!isCheck && isPriorMove(tree, move) &&
					(!move.promote() || move.piece() != Piece::Silver)) {

				reduced = Depth1Ply;
				newDepth -= reduced;

			}
#endif // ENABLE_LMR

			// futility pruning
			if (!isCheck && standPat + estimate(tree, move) + FUT_MGN <= newAlpha) {
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
			if (!isCheck && newStandPat + FUT_MGN <= newAlpha) {
				tree.unmakeMove(move);
				value = newAlpha;
				_info.extendedFutilityPruning++;
				continue;
			}

			// reccursive call
			Value currval;
			if (count == 1) {
				currval = -searchr<pvNode>(tree, !black, newDepth - Depth1Ply, -beta, -newAlpha);

			} else {
				// nega-scout
				currval = -searchr<false>(tree, !black, newDepth - Depth1Ply, -newAlpha-1, -newAlpha);

				if (currval > newAlpha &&  currval < beta) {
					newDepth += reduced;
					currval = -searchr<pvNode>(tree, !black, newDepth - Depth1Ply, -beta, -newAlpha);
				}

			}

			// unmake move
			tree.unmakeMove(move);

			// 値更新
			if (currval > value) {
				value = currval;
				best = move;
				tree.updatePv(move);

				// beta-cut
				if (currval >= beta) {
					updateHistory(tree, depth, move);
					break;
				}
			}

		}

		if (!best.isEmpty()) {
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
		const Value alphas[wcnt] = { astat.base-32, astat.base-256, -Value::Inf };
		const Value betas[wcnt] = { astat.base+32, astat.base+256, Value::Inf };

		while (true) {

			Value alpha = std::max(astat.alpha, alphas[astat.lower]);
			Value beta = std::max(astat.alpha + 1, betas[astat.upper]);

			Value value = -searchr<true>(tree, black, depth, -beta, -alpha);

			if (value > alpha && value < beta) {
				return value;
			}

			if (value <= astat.alpha) {
				return value;
			}

			if (astat.upper == wcnt-1 && astat.lower == wcnt-1) {
				return value;
			}

			while (value <= alphas[astat.lower]) {
				astat.lower++;
				assert(astat.lower < wcnt);
			}

			while (value >= betas[astat.upper]) {
				astat.upper++;
				assert(astat.upper < wcnt);
			}

		}

	}

	/**
	 * search from root node
	 */
	bool Searcher::search(int depth, Move& best, bool gen /* = true */, Value* prevval /* = nullptr */) {

		// tree
		auto& tree = _trees[0];

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
			if (newDepth >= Depth1Ply && count != 1 &&
					!isCheck && isPriorMove(tree, move) &&
					(!move.promote() || move.piece() != Piece::Silver)) {

				reduced = Depth1Ply;
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
				if (currval >= value + 1) {
					// full window search
					newDepth += reduced;
					currval = -searchr<true>(tree, !black, newDepth - Depth1Ply, -Value::Inf, -value);
				}

			}

			// unmake move
			tree.unmakeMove(move);

			move.setExt(currval.int32(), -10000, 10000);

			// 値更新
			if (currval > value) {
				best = move;
				value = currval;
				tree.updatePv(move);
			}
		}

		moves.sortDesc();

		_info.eval = value;
		if (prevval != nullptr) {
			*prevval = value;
		}

		if (value == -Value::Inf) {
			return false;
		}

		return true;

	}

	/**
	 * iterative deepening search from root node
	 */
	bool Searcher::idsearch(Move& best) {

		auto& tree = _trees[0];
		const auto& board = tree.getBoard();
		bool black = board.isBlack();
		bool result = true;
		bool gen = true;

		// 前処理
		before();

		// 最大深さ
		int maxDepth = _config.maxDepth * Depth1Ply;

		Value value = -Value::Inf;

		for (int depth = Depth1Ply; depth <= maxDepth && result; depth += Depth1Ply) {
			result = search(depth, best, gen, &value);
			gen = false;
			Loggers::message << tree.getPv().toString() << ": " << (black ? value.int32() : -value.int32());
		}

		// 後処理
		after();

		return result;

	}

	/**
	 * 指定した局面に対して探索を実行します。
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
