/*
 * CsaClient.h
 */

#ifndef __SUNFISH_CSACLIENT__
#define __SUNFISH_CSACLIENT__

#include "RemainingTime.h"
#include "CsaClientConfig.h"
#include "Connection.h"
#include "logger/Logger.h"
#include "core/record/Record.h"
#include "core/util/StringUtil.h"
#include "searcher/Searcher.h"
#include "book/Book.h"
#include <iomanip>
#include <queue>
#include <regex>
#include <thread>
#include <mutex>
#include <atomic>

namespace sunfish {

	class CsaClient {
	private:
		std::mutex _recvMutex;

		enum RECV_FLAG {
			RECV_NULL      = 0x000000,

			RECV_LOGIN_OK  = 0x000001,
			RECV_LOGIN_INC = 0x000002,
			RECV_LOGIN_MSK = RECV_LOGIN_OK | RECV_LOGIN_INC,

			RECV_LOGOUT    = 0x000004,

			RECV_MOVE_B    = 0x000008,
			RECV_MOVE_W    = 0x000010,
			RECV_MOVE_EX   = 0x000020,

			RECV_SUMMARY   = 0x000040,

			RECV_START     = 0x000080,
			RECV_REJECT    = 0x000100,
			RECV_AGREE_MSK = RECV_START | RECV_REJECT,

			RECV_WIN       = 0x000200,
			RECV_LOSE      = 0x000400,
			RECV_DRAW      = 0x000800,
			RECV_INTERRUPT = 0x001000,
			RECV_REPEAT    = 0x002000,
			RECV_CHECK_REP = 0x004000,
			RECV_ILLEGAL   = 0x008000,
			RECV_TIME_UP   = 0x010000,
			RECV_RESIGN    = 0x020000,
			RECV_JISHOGI   = 0x040000,

			RECV_WIN_LOSE  = RECV_WIN | RECV_LOSE, // CSA将棋付属の簡易サーバ用

			RECV_END_MSK   = RECV_WIN | RECV_LOSE
					| RECV_DRAW | RECV_INTERRUPT
					//| RECV_REPEAT | RECV_CHECK_REP
					//| RECV_ILLEGAL | RECV_TIME_UP
					//| RECV_RESIGN | RECV_JISHOGI
					,

			RECV_ERROR     = 0x800000,

			RECV_NUM       = 20,
		};

		struct RECV_DATA {
			unsigned flag;
			std::string str;
		};

		struct MyMove {
			Move move;
			Value value;
		};

		struct ReceiveFlagSet {
			std::regex regex;
			RECV_FLAG flag;
			void (*func)(CsaClient*);
			const char* name;
		};
		static const ReceiveFlagSet FlagSets[RECV_NUM];

		/** 受信データ */
		std::queue<RECV_DATA> _recvQueue;

		/** 対局終了フラグ */
		unsigned _endFlags;

		/** 設定ファイル */
		const char* _configFilename;

		/** 通信設定 */
		CsaClientConfig _config;

		/** 探索設定 */
		Searcher::Config _searchConfigBase;

		/** 開始局面 */
		Board _board;

		/** 棋譜 */
		Record _record;

		/** 思考部 */
		Searcher _searcher;

		/** 定跡 */
		Book _book;

		/** サーバとのコネクション */
		Connection _con;

		/** 先手の持ち時間 */
		RemainingTime _blackTime;
		/** 後手の持ち時間 */
		RemainingTime _whiteTime;

		std::atomic<bool> _ponderCompleted;

		struct GameSummary {
			/** 自分の手番が黒か */
			bool black;
			/** 対局ID */
			std::string gameId;
			/** 先手の名前 */
			std::string blackName;
			/** 後手の名前 */
			std::string whiteName;
			/** 持ち時間 */
			int totalTime;
			/** 秒読み */
			int readoff;
		} gameSummary;

		void init() {
			while (!_recvQueue.empty()) {
				_recvQueue.pop();
			}
			_endFlags = RECV_NULL;
			gameSummary.gameId = "";
			gameSummary.blackName = "";
			gameSummary.whiteName = "";
			gameSummary.totalTime = 0;
		}

		/**
		 * 対局
		 */
		bool game();

		/**
		 * 対局を進める
		 */
		bool nextTurn();

		/**
		 * 自分の手番
		 */
		bool myTurn();

		/**
		 * 相手の手番
		 */
		bool enemyTurn();

		/**
		 * Ponder
		 */
		void ponder();

		/**
		 * 探索設定を構築
		 */
		void buildSearchConfig(Searcher::Config& searchConfig);

		void receiver();

		bool enqueue(const std::string& recvStr);

		bool login();

		bool logout();

		bool agree();

		bool sendMove(const MyMove& move, bool black, std::string* str);

		bool sendResign();

		bool send(const char* str) {
			printSentString(str);
			return _con.sendln(str);
		}

		bool waitGameSummary() {
			return waitReceive(RECV_SUMMARY) == RECV_SUMMARY;
		}

		unsigned waitReceive(unsigned flags, std::string* str = NULL);

		int getUsedTime(const std::string& recvStr);

		static void _recvGameSummary(CsaClient* p) {
			p->recvGameSummary();
		}

		void recvGameSummary();

		bool inputGameSummary(std::string recvStr);

		void recvTime();

		bool inputTime(std::string recvStr);

		void recvBoard();

		bool inputBoard(std::string recvStr);

		void sleep(unsigned msec) {
			std::this_thread::sleep_for(std::chrono::milliseconds(msec));
		}

		void printSentString(const char* str) {
			Loggers::send << '<' << StringUtil::chomp(str);
		}

		void printReceivedString(std::string recvStr) {
			Loggers::receive << '>' << StringUtil::chomp(recvStr);
		}

		void writeResult(const Record& record);

		void printSearchInfo(Value value,
				uint64_t nodes, int depth, double seconds) {
			std::cout << std::setw(2) << depth << std::setw(10) << nodes
					<< ':' << value.int32() << " (" << seconds << "sec)\n";
		}

	public:
		static const char* DEFAULT_CONFIG_FILE;

		CsaClient() {
			_configFilename = DEFAULT_CONFIG_FILE;
		}
		CsaClient(const CsaClient&) = delete;
		CsaClient(CsaClient&&) = delete;

		~CsaClient() {
		}

		void setConfigFile(const char* filename) {
			_configFilename = filename;
		}

		/**
		 * 対局の実行
		 */
		bool execute();

	};
}

#endif // __SUNFISH_CSACLIENT__
