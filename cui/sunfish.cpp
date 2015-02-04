/* sunfish.cpp
 * 
 * Kubo Ryosuke
 */

#include "sunfish.h"
#include "console/ConsoleManager.h"
#include "logger/Logger.h"
#include <iostream>
#include <sstream>
#include <map>
#include <string>

using namespace sunfish;

// network
int network();

// test.cpp
int test();

// dev.cpp
int exprMoveGenSpeed();
int generateZobrist();

class ProgramOptions {
private:
	struct Option {
		std::string key;
		std::string shortKey;
		std::string description;
		bool arg;
		bool exists;
		std::string value;
	};
	std::vector<Option> _options;
	void setValue(const char* key, const char* value) {
		for (auto& option : _options) {
			if (option.key == key || option.shortKey == key) {
				option.exists = true;
				option.value = value;
				return;
			}
		}
	}
public:
	void addOption(const char* key, const char* description, bool arg = false) {
		_options.push_back(std::move(Option{ key, "", description, arg, false, "" }));
	}
	void addOption(const char* key, const char* shortKey, const char* description, bool arg = false) {
		_options.push_back(std::move(Option{ key, shortKey, description, arg, false, "" }));
	}
	void parse(int argc, char** argv) {
		const char* key = nullptr;
		for (int i = 1; i <= argc; i++) {
			bool isLast = (i == argc);
			const char* opt = !isLast ? argv[i] : "";
			if (opt[0] == '-') {
				bool isFullSpell = (opt[1] == '-');
				if (key != nullptr) {
					setValue(key, "");
				}
				key = &opt[isFullSpell?2:1];
			} else {
				if (key != nullptr) {
					setValue(key, opt);
					key = nullptr;
				}
			}
		}
	}
	bool has(const char* key) const {
		for (const auto& option : _options) {
			if (option.key == key || option.shortKey == key) {
				if (option.exists) {
  				return true;
				}
			}
		}
		return false;
	}
	const char* getValue(const char* key) const {
		for (const auto& option : _options) {
			if (option.key == key || option.shortKey == key) {
				if (option.exists) {
  				return option.value.c_str();
				}
			}
		}
		return "";
	}
	std::string help() const {
		std::ostringstream oss;
		oss << "Option:\n";
		for (const auto& option : _options) {
			int length = 0;
			oss << "  ";
			if (!option.shortKey.empty()) {
				oss << '-' << option.shortKey << " [ --" << option.key << ']';
				length += (int)(option.shortKey.length() + option.key.length() + 7);
			} else {
				oss << "--" << option.key;
				length += (int)(option.key.length() + 2);
			}
			if (option.arg) {
				oss << " arg";
				length += 4;
			}
			int indent = std::max(1, (int)(24 - length));
			for (int i = 0; i < indent; i++) {
				oss << ' ';
			}
			oss << option.description << '\n';
		}
		return oss.str();
	}
};

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
	po.addOption("black", "b", "auto/manual (default: manual)", true);
	po.addOption("white", "w", "auto/manual (default: auto)", true);
	po.addOption("depth", "d", "max depth (default: 15)", true);
	po.addOption("time", "t", "max time for 1 move [sec] (default: 3)", true);
	po.addOption("network", "n", "network mode");
	po.addOption("help", "h", "show this help.");
#ifndef NDEBUG
	po.addOption("test", "unit test");
	po.addOption("dev", "development method", true);
#endif
	po.parse(argc, argv);

	if (po.has("help")) {
		// show help
		std::cerr << po.help() << std::endl;
		return 0;

	} else if (po.has("network")) {
		return network();

#ifndef NDEBUG
	} else if (po.has("test")) {
		// unit test
		return test();

	} else if (po.has("dev")) {
		// development
		const std::string code = po.getValue("dev");

		if (code == "gen_speed_test") {
			return exprMoveGenSpeed();

		} else if (code == "zobrist") {
			return generateZobrist();

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
