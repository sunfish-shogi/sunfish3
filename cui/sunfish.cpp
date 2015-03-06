/* sunfish.cpp
 * 
 * Kubo Ryosuke
 */

#include "sunfish.h"
#include "console/ConsoleManager.h"
#include "program_options/ProgramOptions.h"
#include "logger/Logger.h"
#include <iostream>
#include <sstream>
#include <map>
#include <string>

using namespace sunfish;

// book
int generateBook(const std::string& directory);

// network
int network();

// solve
int solve(const std::vector<std::string>& problems);

// test.cpp
int test();

// dev.cpp
int exprMoveGenSpeed();
int generateZobrist();
int checkMoveGen();

/**
 * entry point
 */
int main(int argc, char** argv, char** /*envp*/) {
	std::cout << SUNFISH_NAME << ' ';
	std::cout << SUNFISH_VERSION << '\n';
	std::cout << SUNFISH_COPYRIGHT << '\n';
	std::cout << std::endl;

	// program options
	ProgramOptions po;
	po.addOption("in", "i", "record file name to load", true);
	po.addOption("out", "o", "record file name to save", true);
	po.addOption("black", "b", "[auto/manual]", true);
	po.addOption("white", "w", "[auto/manual]", true);
	po.addOption("depth", "d", "max depth (default: 15)", true);
	po.addOption("time", "t", "max time for 1 move (default: 3)", true);
	po.addOption("book", "generate book", true);
	po.addOption("network", "n", "network mode");
	po.addOption("problem", "p", "solve problems");
	po.addOption("help", "h", "show this help.");
#ifndef NDEBUG
	po.addOption("test", "unit test");
	po.addOption("dev", "development method", true);
#endif
	po.parse(argc, argv);

	for (const auto& invalidArg : po.getInvalidArgs()) {
		std::cerr << "WARNING: `" << invalidArg.arg << "' is invalid argument: " << invalidArg.reason << std::endl;
	}

	if (po.has("help")) {
		// show help
		std::cerr << po.help() << std::endl;
		return 0;

	} else if (po.has("book")) {
		std::string directory = po.getValue("book");
		return generateBook(directory);

	} else if (po.has("network")) {
		return network();

	} else if (po.has("problem")) {
		return solve(po.getStdArgs());

#ifndef NDEBUG
	} else if (po.has("test")) {
		// unit test
		return test();

	} else if (po.has("dev")) {
		// development
		std::string code = po.getValue("dev");

		if (code == "gen_speed_test") {
			return exprMoveGenSpeed();

		} else if (code == "zobrist") {
			return generateZobrist();

		} else if (code == "gen_check") {
			return checkMoveGen();

		} else {
			std::cerr << '"' << code << "\" is unknown code." << std::endl;
			return 1;

		}

#endif
	}

	// init loggers
	Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
	Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
	Loggers::message.addStream(std::cerr);
	Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
	Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
#ifndef NDEBUG
	Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
	Loggers::test.addStream(std::cerr, "\x1b[32m", "\x1b[39m");
	Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");
#endif

	ConsoleManager console;

	// 起動時に読み込む棋譜ファイル
	if (po.has("in")) {
		console.setInFileName(po.getValue("in"));
		console.setAutoBlack(false);
		console.setAutoWhite(false);
	}

	// 自動保存する棋譜ファイル
	if (po.has("out")) {
		console.setOutFileName(po.getValue("out"));
	}

	// 先手番自動対局 or マニュアル
	if (po.has("black")) {
		std::string value = po.getValue("black");
		if (value == "auto") {
			console.setAutoBlack(true);
		} else if (value == "manual") {
			console.setAutoBlack(false);
		} else {
			Loggers::warning << value << " is unknown value for --black option.";
		}
	}

	// 後手番自動対局 or マニュアル
	if (po.has("white")) {
		std::string value = po.getValue("white");
		if (value == "auto") {
			console.setAutoWhite(true);
		} else if (value == "manual") {
			console.setAutoWhite(false);
		} else {
			Loggers::warning << value << " is unknown value for --white option.";
		}
	}

    // 最大探索深さ
	if (po.has("depth")) {
		int depth = std::stoi(po.getValue("depth"));
		console.setMaxDepth(depth);
	}

    // 最大思考時間
    if (po.has("time")) {
        int time = std::stoi(po.getValue("time"));
        console.setLimitSeconds(time);
    }

	bool ok = console.play();

	return ok ? 0 : 1;
}
