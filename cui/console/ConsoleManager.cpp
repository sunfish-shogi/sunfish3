/* ConsoleManager.cpp
 *
 * Kubo Ryosuke
 */

#include "ConsoleManager.h"
#include "core/record/CsaWriter.h"
#include "core/record/CsaReader.h"
#include "logger/Logger.h"
#include <iostream>

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
		{ "p", "prev", Command::Prev, "go to a previous position." },
		{ "n", "next", Command::Next, "go to a next position." },
		{ "t", "top", Command::Top, "go to a top of this record." },
		{ "e", "end", Command::End, "go to a end of this record." },
		{ "s", "search", Command::Search, "search from current position." },
	};

}

namespace sunfish {

	/**
	 * 設定の初期化
	 */
	void ConsoleManager::initConfig() {
		_config.autoBlack = false;
		_config.autoWhite = true;
		_config.maxDepth = 5;
		_config.inFileName = "";
#ifndef NDEBUG
		_config.outFileName = ".debug.csa";
#else
		_config.outFileName = "";
#endif
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
			if ((s != NULL && 0 == strcmp(str, s)) ||
					(l != NULL && 0 == strcmp(str, l))) {
				return commandSet[i].command;
			}
		}
		return Command::Unknown;
	}

	/**
	 * ユーザが入力した指し手を読み込みます。
	 */
	bool ConsoleManager::inputMove(const char* str, const Board& board, Move& move) const {

		if (strlen(str) < 4) {
			return false;
		}

		auto to = Position::parse(&str[2]);
		if (!to.isValid()) {
			return false;
		}

		auto from = Position::parse(&str[0]);
		if (from.isValid()) {
			// 盤上の駒を動かす手
			auto piece = board.getBoardPiece(from);
			auto promote = str[4] == 'n';
			if (piece.isEmpty()) {
				return false;
			}
			move.set(piece, from, to, promote);
			if (!board.isValidMoveStrict(move) && !promote) {
				Move mtemp(piece, from, to, true);
				if (board.isValidMoveStrict(mtemp)) {
					move = mtemp;
				}
			}
			return true;

		} else {
			// 持ち駒を打つ手
			auto piece = Piece::parse(&str[0]);
			if (piece != Piece::Empty) {
				move.set(piece.black(), to);
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
			if (l != NULL) { std::cout << l; }
			std::cout << '\t';
			if (s != NULL) { std::cout << s; }
			std::cout << '\t';
			if (d != NULL) { std::cout << d; }
			std::cout << std::endl;
		}
	}

	/**
	 * 盤面を表示します。
	 */
	void ConsoleManager::printBoard(const Board& board) const {
		std::cout << "===================\n";
		std::cout << board.toString();
		std::cout << "===================\n";
		std::cout << std::endl;

#if 1
		const auto& evaluator = _searcher.getEvaluator();
		auto valuePair = evaluator.evaluate(board);
		std::cout << "Evaluator Info\n";
		std::cout << "  eval: " << valuePair.value().int32() << '\n';
		std::cout << std::endl;
#endif
	}

	/**
	 * 探索を行って結果を出力します。
	 */
	bool ConsoleManager::search(bool withMakeMove) {

		// 探索
		Move move;
		std::cout << "searching..\n";
		bool ok = _searcher.idsearch(_record.getBoard(), move);
		std::cout << "done.\n";
		std::cout << std::endl;

		const auto& info = _searcher.getInfo();
		std::cout << "Search Info:\n";
		std::cout << "  node: " << info.node << '\n';
		std::cout << "  time: " << info.time << '\n';
		std::cout << "  nps : " << (int)info.nps << '\n';
		std::cout << "  eval: " << info.eval.int32() << '\n';
		std::cout << "  tt  : " << info.hashPruning << '\n';
		std::cout << "  null: " << info.nullMovePruning << '\n';
		std::cout << "  fut : " << info.futilityPruning << '\n';
		std::cout << "  efut: " << info.extendedFutilityPruning << '\n';
		std::cout << std::endl;

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

		if (command == Command::Empty) {
			// 入力なし
			return CommandResult::None;

		} else if (command == Command::Help) {
			// ヘルプを表示
			showHelp();
			return CommandResult::None;

		} else if (command == Command::Prev) {
			// 1手戻る。
			if (_record.unmakeMove()) {
				if (_config.autoBlack || _config.autoWhite) {
					// コンピュータの手番はスキップ
					_record.unmakeMove();
				}
				_prevCommand = Command::Prev;
				return CommandResult::Changed;
			}
			std::cout << "There is no previous move.\n";
			return CommandResult::None;

		} else if (command == Command::Next) {
			// 1手進む。
			if (_record.makeMove()) {
				if (_config.autoBlack || _config.autoWhite) {
					_record.makeMove();
				}
				_prevCommand = Command::Next;
				return CommandResult::Changed;
			}
			std::cout << "There is no next move.\n";
			return CommandResult::None;

		} else if (command == Command::Top) {
			// 開始局面に戻る。
			if (!_record.unmakeMove()) {
				std::cout << "There is no previous move.\n";
				return CommandResult::None;
			}
			while (_record.unmakeMove())
				;
			return CommandResult::Changed;

		} else if (command == Command::End) {
			// 最終局面まで進む。
			if (!_record.makeMove()) {
				std::cout << "There is no next move.\n";
				return CommandResult::None;
			}
			while (_record.makeMove())
				;
			return CommandResult::Changed;

		} else if (command == Command::Search) {
			// 探索
			search(false);
			return CommandResult::None;

		} else {
			// 指し手入力
			Move move;
			if (CsaReader::readMove(line, _record.getBoard(), move) ||
					inputMove(line, _record.getBoard(), move)) {
				if (_record.makeMove(move)) {
					std::cout << "Move:\n";
					std::cout << "  " << move.toString() << '\n';
					std::cout << std::endl;
					return CommandResult::Changed;
				}
				std::cout << "illegal move!!\n";
				return CommandResult::None;
			}
			std::cout << "unknown command!!\n";
			return CommandResult::None;
		}

	}

	/**
	 * コンソール上で対局を行います。
	 */
	bool ConsoleManager::play() {
		
		_record.init(Board::Handicap::Even);

		// 棋譜の読み込み
		if (!_config.inFileName.empty()) {
			CsaReader::read(_config.inFileName, _record);
		}

		auto searchConfig = _searcher.getConfig();
		searchConfig.maxDepth = _config.maxDepth;
		searchConfig.treeSize = 1;
		searchConfig.wokerSize = 1;
		_searcher.setConfig(searchConfig);
		_searcher.init();

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
}
