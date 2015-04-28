/* ConsoleManager.cpp
 *
 * Kubo Ryosuke
 */

#include "ConsoleManager.h"
#include "core/record/CsaWriter.h"
#include "core/record/CsaReader.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

#define COMMAND_NUM							((int)Command::__num__)

namespace {

	using namespace sunfish;

	struct CommandSet {
		const char* shortStr;
		const char* longStr;
		Command command;
		const char* description;
	};

	/**
	 * コンソールで利用可能なコマンド一覧
	 */
	const CommandSet commandSet[COMMAND_NUM] = {
		{ "q", "quit", Command::Quit, "quit." },
		{ "h", "help", Command::Help, "show this help." },
		{ "p", "prev", Command::Prev, "go to previous position." },
		{ "n", "next", Command::Next, "go to next position." },
		{ "t", "top", Command::Top, "go to top of this record." },
		{ "e", "end", Command::End, "go to end of this record." },
		{ "s", "search", Command::Search, "search from current position." },
		{ "m", "moves", Command::Moves, "show legal moves." },
		{ "b", "book", Command::Book, "probe book." },
		{ nullptr, "clear-tt", Command::ClearTT, "clear TT." },
		{ nullptr, "clear-history", Command::ClearHistory, "clear history." },
	};

}

namespace sunfish {

	/**
	 * デフォルト設定
	 */
	ConsoleManager::Config ConsoleManager::getDefaultConfig() {
		Config config;
		config.autoBlack = false;
		config.autoWhite = true;
		config.maxDepth = 15;
		config.limitSeconds = 3;
		config.worker = 1;
		config.inFileName = "";
		config.outFileName = "console.csa";
		return config;
	}

	/**
	 * ユーザが入力したコマンドを読み込みます。
	 */
	Command ConsoleManager::parseCommand(const char* str) {
		if (str[0] == '\0') {
			return Command::Empty;
		}
		for (int i = 0; i < COMMAND_NUM; i++) {
			const char* s = commandSet[i].shortStr;
			const char* l = commandSet[i].longStr;
			if ((s != nullptr && 0 == strcmp(str, s)) ||
					(l != nullptr && 0 == strcmp(str, l))) {
				return commandSet[i].command;
			}
		}
		return Command::Unknown;
	}

	/**
	 * ユーザが入力した指し手を読み込みます。
	 */
	bool ConsoleManager::inputMove(const char* str, const Board& board, Move& move) const {

		// CSA形式で読み込んでみる
		if (CsaReader::readMove(str, _record.getBoard(), move)) {
			return true;
		}

		if (strlen(str) < 4) {
			return false;
		}

		// 移動元
		auto to = Position::parse(&str[2]);
		if (!to.isValid()) {
			return false;
		}

		// 移動先
		auto from = Position::parse(&str[0]);

		if (from.isValid()) {
			// 盤上の駒を動かす手

			// 駒種
			auto piece = board.getBoardPiece(from);
			if (piece.isEmpty()) {
				return false;
			}

			// 成
			auto promote = str[4] == 'n';
			if (promote && piece.isPromoted()) {
				return false;
			}

			move.set(piece, from, to, promote);

			// 合法手チェック
			if (board.isValidMoveStrict(move)) {
				return true;
			}

			// 成を自動付加
			if (!promote && piece.isUnpromoted()) {
				move.setPromote(true);
				if (board.isValidMoveStrict(move)) {
					return true;
				}
			}

		} else {
			// 持ち駒を打つ手

			// 駒種
			auto piece = Piece::parse(&str[0]);
			if (piece.isEmpty()) {
				return false;
			}

			move.set(piece.black(), to);

			// 合法手チェック
			if (board.isValidMoveStrict(move)) {
				return true;
			}

		}

		return false;

	}

	/**
	 * ヘルプを表示します。
	 */
	void ConsoleManager::showHelp() {
		for (int i = 0; i < COMMAND_NUM; i++) {
			const char* l = commandSet[i].longStr;
			const char* s = commandSet[i].shortStr;
			const char* d = commandSet[i].description;
			if (l != nullptr) { std::cout << l; }
			std::cout << '\t';
			if (s != nullptr) { std::cout << s; }
			std::cout << '\t';
			if (d != nullptr) { std::cout << d; }
			std::cout << std::endl;
		}
	}

	/**
	 * 盤面を表示します。
	 */
	void ConsoleManager::printBoard(const Board& board) {
		std::cout << "'----------------------------\n";
		std::cout << board.toStringCsa();
		std::cout << "'----------------------------\n";
		std::cout << std::endl;

#if 1
		auto& evaluator = _searcher.getEvaluator();
		auto valuePair = evaluator.evaluate(board);
		std::cout << "Evaluator Info\n";
		std::cout << "  eval: " << valuePair.value().int32() <<
  		"(" << valuePair.material().int32() << ","
  		" " << valuePair.positional().int32() << ")\n";
		std::cout << std::endl;
#endif
	}

	/**
	 * 探索を行って結果を出力します。
	 */
	bool ConsoleManager::search(bool withMakeMove) {

		bool ok = false;
		Move move = Move::empty();

		// 定跡検索
		if (!ok) {
  		const auto& board = _record.getBoard();
  		uint64_t hash = board.getHash();
  		BookResult bookResult = _book.selectRandom(hash);
  		if (!bookResult.move.isEmpty() && board.isValidMove(bookResult.move)) {
				move = bookResult.move;
				std::cout << "book hit: " << move.toString() << " (" << bookResult.count << "/" << bookResult.total << ")\n";
				std::cout << std::endl;
				ok = true;
  		}
		}

		// 探索
		if (move.isEmpty()) {
			std::cout << "searching..\n";
			_searcher.setRecord(_record);
			ok = _searcher.idsearch(_record.getBoard(), move);
			_searcher.clearRecord();
			std::cout << "done.\n";
			std::cout << std::endl;

			const auto& info = _searcher.getInfo();
			showSearchInfo(info);
		}

		if (ok) {

			std::cout << "Result:" << std::endl;
			std::cout << "  " << move.toString() << std::endl;
			std::cout << std::endl;

			if (withMakeMove) {
				// 着手
				_record.makeMove(move);
			}

		} else {

			std::cout << "no legal move.\n";
			std::cout << std::endl;

			if (withMakeMove) {
				// 着手をマニュアルモードに切り替え
				if (_config.autoBlack) {
					std::cout << "Change mode:\n";
					std::cout << "  black: auto => manual\n";
					std::cout << std::endl;
					_config.autoBlack = false;
				}
				if (_config.autoWhite) {
					std::cout << "Change mode:\n";
					std::cout << "  white: auto => manual\n";
					std::cout << std::endl;
					_config.autoWhite = false;
				}
			}

		}

		return ok;

	}

	void ConsoleManager::showMoves() const {
		Moves moves;
		MoveGenerator::generate(_record.getBoard(), moves);
		for (auto ite = moves.begin(); ite != moves.end(); ite++) {
			if (_record.getBoard().isValidMoveStrict(*ite)) {
				std::cout << ite->toString() << ' ';
			}
		}
		std::cout << std::endl;
	}

	void ConsoleManager::probeBook() const {
		uint64_t hash = _record.getBoard().getHash();
		auto p = _book.find(hash);
		if (p == nullptr) {
			std::cout << "(empty)" << std::endl;
			return;
		}

		uint32_t totalCount = p->getCount();
		const auto& bookMoves = p->getMoves();

		struct Element {
			Move move;
			uint32_t count;
		};
		std::vector<Element> moves;
		for (const auto& bookMove : bookMoves) {
			moves.push_back({ bookMove.move, bookMove.count });
		}
		std::sort(moves.begin(), moves.end(), [](const Element& l, const Element& r) {
			return l.count > r.count;
		});
		for (const auto& element : moves) {
			float percentage = (float)element.count / totalCount * 100.0f;
			std::cout << element.move.toString() << "(" << std::fixed << std::setprecision(1) << percentage << "%) ";
		}
		std::cout << std::endl;
	}

	/**
	 * ユーザからのコマンド入力を受け付けます。
	 */
	ConsoleManager::CommandResult ConsoleManager::inputCommand() {

		char line[1024];

		std::cout << "> ";

		// コマンド受付
		std::cin.getline(line, sizeof(line));
		if (std::cin.eof()) {
			return CommandResult::Quit;
		}

		// コマンドの解析
		Command command = parseCommand(line);

		// quit
		if (command == Command::Quit) {
			return CommandResult::Quit;
		}

		// 入力がないときは直前のコマンドを繰り返す。
		if (command == Command::Empty) {
			command = _prevCommand;
		}

		_prevCommand = Command::Empty;

		switch (command) {
			case Command::Empty:
				// 入力なし
				return CommandResult::None;

			case Command::Help:
				// ヘルプを表示
				showHelp();
				return CommandResult::None;

			case Command::Prev:
				// 1手戻る。
				if (!_record.unmakeMove()) {
					std::cout << "There is no previous move.\n";
					return CommandResult::None;
				}

				if (_config.autoBlack || _config.autoWhite) {
					// コンピュータの手番はスキップ
					_record.unmakeMove();
				}
				_prevCommand = Command::Prev;
				return CommandResult::Changed;

			case Command::Next:
				// 1手進む。
				if (!_record.makeMove()) {
					std::cout << "There is no next move.\n";
					return CommandResult::None;
				}

				if (_config.autoBlack || _config.autoWhite) {
					// コンピュータの手番はスキップ
					_record.makeMove();
				}
				_prevCommand = Command::Next;
				return CommandResult::Changed;

			case Command::Top:
				// 開始局面に戻る。
				if (!_record.unmakeMove()) {
					std::cout << "There is no previous move.\n";
					return CommandResult::None;
				}

				while (_record.unmakeMove())
					;
				return CommandResult::Changed;

			case Command::End:
				// 最終局面まで進む。
				if (!_record.makeMove()) {
					std::cout << "There is no next move.\n";
					return CommandResult::None;
				}

				while (_record.makeMove())
					;
				return CommandResult::Changed;

			case Command::Search:
				// 探索
				search(false);
				return CommandResult::None;

			case Command::Moves:
				// 合法手生成
				showMoves();
				return CommandResult::None;

			case Command::Book:
				// 定跡検索
				probeBook();
				return CommandResult::None;

			case Command::ClearTT:
				// TTクリア
				_searcher.clearTT();
				return CommandResult::None;

			case Command::ClearHistory:
				// Historyクリア
				_searcher.clearHistory();
				return CommandResult::None;

			default: {
				// 指し手入力
				Move move;
				if (!inputMove(line, _record.getBoard(), move)) {
  			std::cout << "unknown command!!\n";
  			return CommandResult::None;
				}

				if (!_record.makeMove(move)) {
					std::cout << "illegal move!!\n";
					return CommandResult::None;
				}

				std::cout << "Move:\n";
				std::cout << "  " << move.toString() << '\n';
				std::cout << std::endl;
				return CommandResult::Changed;
			}
		}

	}

	/**
	 * コンソール上で対局を行います。
	 */
	bool ConsoleManager::play() {

		// 定跡の読み込み
		if (!_book.readFile()) {
			Loggers::warning << "Could not read book.";
		}

		_record.init(Board::Handicap::Even);

		// 棋譜の読み込み
		if (!_config.inFileName.empty()) {
			CsaReader::read(_config.inFileName, _record);
		}

		auto searcherConfig = buildSearcherConfig(_searcher.getConfig(), _config);
		_searcher.setConfig(searcherConfig);

		printBoard(_record.getBoard());

		while (true) {

			if (!_config.outFileName.empty()) {
				CsaWriter::write(_config.outFileName, _record);
			}

			if (isAuto()) {

				// コンピュータによる着手
				search(true);

				// 盤面表示
				printBoard(_record.getBoard());

			} else {

				// ユーザからのコマンド入力
				CommandResult result = inputCommand();

				if (result == CommandResult::Changed) {
					// 盤面表示
					printBoard(_record.getBoard());

				} else if (result == CommandResult::Quit) {
					// quit
					break;

				}

			}
		}

		return true;
	}

	void ConsoleManager::showSearchInfo(const SearchInfo& info) {
		auto format = [](int64_t value) {
			std::ostringstream oss;
			oss << std::setw(8) << (value);
			return oss.str();
		};
		auto format2 = [](int64_t value, int64_t total) {
			std::ostringstream oss;
			oss << std::setw(8) << (value) << '/' << std::setw(8) << (total)
				<< " (" << std::setw(5) << std::fixed << std::setprecision(1)<< ((double)(value) / ((total)!=0?(total):1) * 100.0) << "%)";
			return oss.str();
		};

		std::vector<std::pair<std::string, std::string>> lines;
		lines.emplace_back("nodes          ", format (info.node));
		lines.emplace_back("quies-nodes    ", format (info.qnode));
		lines.emplace_back("all-nodes      ", format ((info.node + info.qnode)));
		lines.emplace_back("time           ", format (info.time));
		lines.emplace_back("nps            ", format (std::ceil(info.nps)));
		lines.emplace_back("eval           ", format (info.eval.int32()));
		lines.emplace_back("split          ", format (info.split));
		lines.emplace_back("fail high first", format2(info.failHighFirst, info.failHigh));
		lines.emplace_back("fail high hash ", format2(info.failHighIsHash, info.failHigh));
		lines.emplace_back("fail high kill1", format2(info.failHighIsKiller1, info.failHigh));
		lines.emplace_back("fail high kill2", format2(info.failHighIsKiller2, info.failHigh));
		lines.emplace_back("expand hash    ", format2(info.expandHashMove, info.expand));
		lines.emplace_back("hash hit       ", format2(info.hashHit, info.hashProbed));
		lines.emplace_back("hash extract   ", format2(info.hashExact, info.hashProbed));
		lines.emplace_back("hash lower     ", format2(info.hashLower, info.hashProbed));
		lines.emplace_back("hash upper     ", format2(info.hashUpper, info.hashProbed));
		lines.emplace_back("hash new       ", format2(info.hashNew, info.hashStore));
		lines.emplace_back("hash update    ", format2(info.hashUpdate, info.hashStore));
		lines.emplace_back("hash collide   ", format2(info.hashCollision, info.hashStore));
		lines.emplace_back("hash reject    ", format2(info.hashReject, info.hashStore));
		lines.emplace_back("mate hit       ", format2(info.mateHit, info.mateProbed));
		lines.emplace_back("shek superior  ", format2(info.shekSuperior, info.shekProbed));
		lines.emplace_back("shek inferior  ", format2(info.shekInferior, info.shekProbed));
		lines.emplace_back("shek equal     ", format2(info.shekEqual, info.shekProbed));
		lines.emplace_back("null mv pruning", format2(info.nullMovePruning, info.nullMovePruningTried));
		lines.emplace_back("fut pruning    ", format (info.futilityPruning));
		lines.emplace_back("ext fut pruning", format (info.extendedFutilityPruning));
		lines.emplace_back("mov cnt pruning", format (info.moveCountPruning));
		lines.emplace_back("razoring       ", format2(info.razoring, info.razoringTried));
		lines.emplace_back("probcut        ", format2(info.probcut, info.probcutTried));
		lines.emplace_back("check extension", format2(info.checkExtension, info.expanded));
		lines.emplace_back("1rep extension ", format2(info.onerepExtension, info.expanded));
		lines.emplace_back("recap extension", format2(info.recapExtension, info.expanded));

		int columns = 2;
		int rows = (lines.size() + columns - 1) / columns;
		int maxLength[columns];
		memset(maxLength, 0, sizeof(int) * columns);

		for (int row = 0; row < rows; row++) {
			for (int column = 0; column < columns; column++) {
				int index = column * rows + row;
				if (index >= (int)lines.size()) { continue; }
				int length = lines[index].first.length() + lines[index].second.length();
				maxLength[column] = std::max(maxLength[column], length);
			}
		}

		std::cout << "Search Info:\n";
		for (int row = 0; row < rows; row++) {
			std::cout << "  ";
			for (int column = 0; column < columns; column++) {
				int index = column * rows + row;
				if (index >= (int)lines.size()) { continue; }
				int length = lines[index].first.length() + lines[index].second.length();
				int padding = maxLength[column] - length + 1;
				std::cout << " * " << lines[index].first << ":" << lines[index].second;
				bool isLastColumn = column == columns - 1;
				if (isLastColumn) { break; }
				for (int i = 0; i < padding; i++) {
					std::cout << ' ';
				}
			}
			std::cout << '\n';
		}
		std::cout << std::endl;
	}
}
