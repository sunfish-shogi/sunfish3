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
#include <iomanip>
#include <queue>
#include <regex>
#include <thread>
#include <mutex>

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

		struct SendingMove {
			Move move;
			Value value;
			void set(const Move& move) {
				this->move = move;
				this->value = 0;
			}
			void set(const Searcher::Info& info) {
				this->move = info.move;
				this->value = info.eval;
			}
		};

		struct ReceiveFlagSet {
			std::regex regex;
			RECV_FLAG flag;
			void (*func)(CsaClient*);
			const char* name;
		};
		static const ReceiveFlagSet FlagSets[RECV_NUM];

		std::queue<RECV_DATA> _recvQueue; // 受信データ
		unsigned _endFlags; // 対局終了フラグ

		const char* _configFilename; // 設定ファイル
		CsaClientConfig _config; // 設定

		// 開始局面
		Board _board;

		Connection _con; // サーバとのコネクション

		RemainingTime _blackTime; // 先手の持ち時間
		RemainingTime _whiteTime; // 後手の持ち時間

		struct GameSummary {
			bool black; // 自分の手番が黒か
			std::string gameId; // 対局ID
			std::string blackName; // 先手の名前
			std::string whiteName; // 後手の名前
			int totalTime; // 持ち時間
			int readoff; // 秒読み
		} gameSummary;

		void buildSearchConfig(Searcher::Config& searchConfig);

		void receiver();

		bool enqueue(const std::string& recvStr);

		bool login();

		bool logout();

		bool agree();

		bool sendMove(const SendingMove& move, bool black, std::string* str);

		bool sendResign();

		bool send(const char* str) {
			printSentString(str);
			return _con.sendln(str);
		}

		bool waitGameSummary() {
			return waitReceive(RECV_SUMMARY) == RECV_SUMMARY;
		}

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

		~CsaClient() {
		}

		void setConfigFile(const char* filename) {
			_configFilename = filename;
		}

		bool execute();

	};
}

#endif // __SUNFISH_CSACLIENT__
