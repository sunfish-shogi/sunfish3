/*
 * ConsoleManager.h
 *
 * Kubo Ryosuke
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "core/record/Record.h"
#include "searcher/Searcher.h"
#include "book/Book.h"

namespace sunfish {

	/**
	 * コンソールコマンド
	 */
	enum class Command : int {
		Empty = 0,
		Quit,
		Help,
		Prev,
		Next,
		Top,
		End,
		Search,
		Moves,
		Book,
		ClearTT,
		ClearHistory,

		__num__,

		Unknown,
	};

	/**
	 * ConsoleManager
	 */
	class ConsoleManager {
	public:

		struct Config {
			bool autoBlack;
			bool autoWhite;
			int maxDepth;
			int limitSeconds;
			int worker;
			std::string inFileName;
			std::string outFileName;
		};

	private:

		enum class CommandResult {
			Quit, Changed, None
		};

		Searcher _searcher;
		Book _book;
		Record _record;
		Command _prevCommand;
		Config _config;

		void showHelp();

		Command parseCommand(const char* str);

		bool inputMove(const char* str, const Board& board, Move& move) const;

		void printBoard(const Board& board);

		bool isAuto() const {
			return (_record.getBoard().isBlack() && _config.autoBlack)
					|| (_record.getBoard().isWhite() && _config.autoWhite);
		}

		bool search(bool withMakeMove);

		void showMoves() const;

		void probeBook() const;

		CommandResult inputCommand();

	public:

		ConsoleManager() {
			_config = getDefaultConfig();
			_prevCommand = Command::Empty;
		}
		ConsoleManager(const ConsoleManager&) = delete;
		ConsoleManager(ConsoleManager&&) = delete;

		void setAutoBlack(bool enable) {
			_config.autoBlack = enable;
		}
		bool getAutoBlack() const {
			return _config.autoBlack;
		}

		void setAutoWhite(bool enable) {
			_config.autoWhite = enable;
		}
		bool getAutoWhite() const {
			return _config.autoWhite;
		}

		void setMaxDepth(int depth) {
			_config.maxDepth = depth;
		}
		int getMaxDepth() const  {
			return _config.maxDepth;
		}

		void setLimitSeconds(int limitSeconds) {
			_config.limitSeconds = limitSeconds;
		}
		int getLimitSeconds() const {
			return _config.limitSeconds;
		}

		void setWorker(int worker) {
			_config.worker = worker;
		}
		int getWorker() const {
			return _config.worker;
		}

		void setInFileName(const std::string& inFileName) {
			_config.inFileName = inFileName;
		}
		const std::string& getInFileName() const {
			return _config.inFileName;
		}

		void setOutFileName(const std::string& outFileName) {
			_config.outFileName = outFileName;
		}
		const std::string& getOutFileName() const {
			return _config.outFileName;
		}

		void setConfig(Config config) {
			_config = std::move(config);
		}
		const Config& getConfig() const {
			return _config;
		}
		static Config getDefaultConfig();
		static Searcher::Config buildSearcherConfig(Searcher::Config searcherConfigOrg, const Config& config) {
			Searcher::Config searcherConfig = std::move(searcherConfigOrg);
			searcherConfig.maxDepth = config.maxDepth;
			searcherConfig.limitSeconds = config.limitSeconds;
			searcherConfig.workerSize = config.worker;
			searcherConfig.treeSize = Searcher::standardTreeSize(config.worker);
			return searcherConfig;
		}

		bool play();

		static void showSearchInfo(const SearchInfo& info);

	};
}

#endif // CONTROLLER_H_
