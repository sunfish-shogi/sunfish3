/*
 * CsaClient.cpp
 */

#include "CsaClient.h"
#include "core/record/Record.h"
#include "core/record/CsaReader.h"
#include "core/record/CsaWriter.h"
#include <mutex>
#include <fstream>
#include <sstream>
#include <thread>

#define WARN_IGNORED(key, value) Loggers::warning << __FILE_LINE__ << ": not supported: key=[" << (key) << "] value=[" << (value) << "]"

#define CONF_HOST      "host"
#define CONF_PORT      "port"
#define CONF_USER      "user"
#define CONF_PASS      "pass"
#define CONF_DEPTH     "depth"
#define CONF_LIMIT     "limit"
#define CONF_REPEAT    "repeat"
#define CONF_WORKER    "worker"
#define CONF_PONDER    "ponder"
#define CONF_KEEPALIVE "keepalive"
#define CONF_KEEPIDLE  "keepidle"
#define CONF_KEEPINTVL "keepintvl"
#define CONF_KEEPCNT   "keepcnt"
#define CONF_FLOODGATE "floodgate"
#define CONF_KIFU      "kifu"
#define CONF_MONITOR   "monitor"

namespace sunfish {

	const char* CsaClient::DEFAULT_CONFIG_FILE = "network.conf";

	CsaClient::CsaClient() {
		_configFilename = DEFAULT_CONFIG_FILE;

		_config.addDef(CONF_HOST, "localhost");
		_config.addDef(CONF_PORT, "4081");
		_config.addDef(CONF_USER, "test");
		_config.addDef(CONF_PASS, "");
		_config.addDef(CONF_DEPTH, "32");
		_config.addDef(CONF_LIMIT, "10");
		_config.addDef(CONF_REPEAT, "1");
		_config.addDef(CONF_WORKER, "1");
		_config.addDef(CONF_PONDER, "1");
		_config.addDef(CONF_KEEPALIVE, "1");
		_config.addDef(CONF_KEEPIDLE, "120");
		_config.addDef(CONF_KEEPINTVL, "60");
		_config.addDef(CONF_KEEPCNT, "10");
		_config.addDef(CONF_FLOODGATE, "0");
		_config.addDef(CONF_KIFU, "Kifu");
		_config.addDef(CONF_MONITOR, "");
	}

	const CsaClient::ReceiveFlagSet* CsaClient::getFlagSets() {
		static const ReceiveFlagSet flagSets[RECV_NUM] = {
			{ Wildcard("LOGIN:* OK"), RECV_LOGIN_OK, NULL, NULL },
			{ Wildcard("LOGIN:incorrect"), RECV_LOGIN_INC, NULL, NULL },
			{ Wildcard("LOGOUT:completed"), RECV_LOGOUT, NULL, NULL },
			{ Wildcard("%*"), RECV_MOVE_EX, NULL, NULL },
			{ Wildcard("+*"), RECV_MOVE_B, NULL, NULL },
			{ Wildcard("-*"), RECV_MOVE_W, NULL, NULL },
			{ Wildcard("BEGIN Game_Summary"), RECV_SUMMARY, _recvGameSummary, NULL },
			{ Wildcard("START:*"), RECV_START, NULL, NULL },
			{ Wildcard("REJECT:* by *"), RECV_REJECT, NULL, NULL },
			{ Wildcard("#WIN"), RECV_WIN, NULL, "win" },
			{ Wildcard("#LOSE"), RECV_LOSE, NULL, "lose" },
			{ Wildcard("#WIN(LOSE)"), RECV_WIN_LOSE, NULL, "unknown" }, // CSA将棋付属の簡易サーバ用
			{ Wildcard("#DRAW"), RECV_DRAW, NULL, "draw" },
			{ Wildcard("#CHUDAN"), RECV_INTERRUPT, NULL, "chudan" },
			{ Wildcard("#SENNICHITE"), RECV_REPEAT, NULL, "sennichite" },
			{ Wildcard("#OUTE_SENNICHITE"), RECV_CHECK_REP, NULL, "oute sennichite" },
			{ Wildcard("#ILLEGAL_MOVE"), RECV_ILLEGAL, NULL, "illegal move" },
			{ Wildcard("#TIME_UP"), RECV_TIME_UP, NULL, "time up" },
			{ Wildcard("#RESIGN"), RECV_RESIGN, NULL, "resign" },
			{ Wildcard("#JISHOGI"), RECV_JISHOGI, NULL, "jishogi" },
			{ Wildcard("#MAX_MOVES"), RECV_MAX_MOVE, NULL, "max move" },
			{ Wildcard("#CENSORED"), RECV_CENSORED, NULL, "censored" },
		};
		return flagSets;
	}

	/**
	 * 対局の実行
	 */
	bool CsaClient::execute() {
		// 設定の読み込み
		if (!_config.read(_configFilename)) {
			return false;
		}
		Loggers::message << _config.toString();

		// 通信設定
		_con.setHost(_config.getString(CONF_HOST));
		_con.setPort(_config.getInt(CONF_PORT));
		_con.setKeepalive(_config.getInt(CONF_KEEPALIVE), _config.getInt(CONF_KEEPIDLE),
		_config.getInt(CONF_KEEPINTVL), _config.getInt(CONF_KEEPCNT));

		// 定跡読み込み
		_book.readFile();

		// 探索設定
		_searchConfigBase = _searcher.getConfig();
		_searchConfigBase.maxDepth = _config.getInt(CONF_DEPTH);
		_searchConfigBase.limitEnable = _config.getInt(CONF_LIMIT) != 0;
		_searchConfigBase.limitSeconds = _config.getInt(CONF_LIMIT);
		_searchConfigBase.workerSize = std::max(_config.getInt(CONF_WORKER), 1);
		_searchConfigBase.treeSize = Searcher::standardTreeSize(_searchConfigBase.workerSize);
		_searcher.setConfig(_searchConfigBase);

		// 連続対局
		int repeatCount = _config.getInt(CONF_REPEAT);
		for (int i = 0; i < repeatCount; i++) {
			bool noError = game();
			if (!noError) {
				return false;
			}
		}

		return true;
	}

	/**
	 * 対局
	 */
	bool CsaClient::game() {
		// 接続を確立
		if (!_con.connect()) {
			Loggers::error << "ERROR: can not connect to " << _config.getString(CONF_HOST)
					<< ':' << _config.getInt(CONF_PORT);
			return false;
		}

		// 初期化
		init();

		// 受信スレッドを開始
		std::thread receiverThread([this]() {
			receiver();
		});

		bool success = true;

		// login
		if (!login()) {
			Loggers::message << "login failed!";
			success = false;
			goto lab_end;
		}
		Loggers::message << "login ok!!";

		// wait for match-make and agree
		if (waitGameSummary() && agree()) {

			// 棋譜の初期化
			_record.init(_board);

			// 残り時間の初期化
			_blackTime.init(_gameSummary.totalTime, _gameSummary.readoff);
			_whiteTime.init(_gameSummary.totalTime, _gameSummary.readoff);

			while (1) {
				bool ok = nextTurn();
				if (!ok) {
					break;
				}
			}
			// 対局結果の記録
			writeResult();
		}

		// logout
		logout();

lab_end:
		_con.disconnect();
		receiverThread.join();

		return success;
	}

	/**
	 * 対局を進める
	 */
	bool CsaClient::nextTurn() {
		std::string monitor = _config.getString(CONF_MONITOR);
		if (!monitor.empty()) {
			RecordInfo info = getRecordInfo();
			CsaWriter::write(monitor, _record, &info);
		}

		// 残り時間を表示
		Loggers::message << "Time(Black):" << _blackTime.toString();
		Loggers::message << "Time(White):" << _whiteTime.toString();

		bool ok;
		if (_gameSummary.black == _record.isBlack()) {
			// 自分の手番
			ok = myTurn();
		} else {
			// 相手番
			ok = enemyTurn();
		}

		if (!ok) {
			return false;
		}

		// 指し手を表示
		Loggers::message << _record.getMove().toString();

		return true;
	}

	/**
	 * 自分の手番
	 */
	bool CsaClient::myTurn() {

		bool ok = false;
		MyMove myMove;

		// 定跡検索
		if (!ok) {
  		const auto& board = _record.getBoard();
  		uint64_t hash = board.getHash();
  		BookResult bookResult = _book.selectRandom(hash);
  		if (!bookResult.move.isEmpty() && board.isValidMove(bookResult.move)) {
				myMove.move = bookResult.move;
				Loggers::message << "book hit: " << myMove.move.toString() << " (" << bookResult.count << "/" << bookResult.total << ")";
				ok = true;
  		}
		}

		// 探索設定
		if (!ok) {
  		auto searchConfig = _searchConfigBase;
  		buildSearchConfig(searchConfig);
  		_searcher.setConfig(searchConfig);

  		// 探索
  		Loggers::message << "begin search: limit(sec)=" << searchConfig.limitSeconds;
			_searcher.setRecord(_record);
  		ok = _searcher.idsearch(_record.getBoard(), myMove.move);
			_searcher.clearRecord();
  		Loggers::message << "end search";

			if (ok) {
				myMove.value = _searcher.getInfo().eval;
			}
		}

		if (ok && _record.makeMove(myMove.move)) {
			// 指し手を送信
			std::string recvStr;
			if (!sendMove(myMove, !_record.getBoard().isBlack(), &recvStr)) {
				// TODO: エラーの詳細を出力
				Loggers::error << "ERROR:could not send a move";
				return false;
			}

			// 消費時間の読み込み
			int usedTime = getUsedTime(recvStr);
			if (_gameSummary.black) {
				_blackTime.use(usedTime);
			} else {
				_whiteTime.use(usedTime);
			}

		} else {
			// 投了
			sendResign();
			return false;

		}

		return true;
	}

	/**
	 * 相手の手番
	 */
	bool CsaClient::enemyTurn() {
		bool enablePonder = _config.getBool(CONF_PONDER);

		std::thread ponderThread;

		if (enablePonder) {
			// 相手番中の思考開始
			_ponderCompleted.store(false);
			ponderThread = std::thread(std::bind(std::mem_fn(&CsaClient::ponder), this));
		}

		// 相手番の指し手を受信
		std::string recvStr;
		unsigned mask = _gameSummary.black ? RECV_MOVE_W : RECV_MOVE_B;
		unsigned flags = waitReceive(mask | RECV_END_MSK, &recvStr);

		if (enablePonder) {
			// 探索が開始されていることを確認
			while (!_searcher.isRunning() && !_ponderCompleted.load()) {
				std::this_thread::yield();
			}

			// 相手番中の思考終了
			Loggers::message << "force interrupt";
			_searcher.forceInterrupt();
			Loggers::message << "join...";
			ponderThread.join();
			Loggers::message << "completed";
		}

		if (flags & mask) {
			// 受信した指し手の読み込み
			Move move;
			if (!CsaReader::readMove(recvStr.c_str(), _record.getBoard(), move) ||
					!_record.makeMove(move)) {
				Loggers::error << "ERROR:illegal move!!";
				return false;
			}

			// 消費時間の読み込み
			int usedTime = getUsedTime(recvStr);
			if (_gameSummary.black) {
				_whiteTime.use(usedTime);
			} else {
				_blackTime.use(usedTime);
			}

		} else if (flags & RECV_END_MSK) {
			// 対局の終了
			return false;

		} else {
			// エラー
			Loggers::error << "ERROR:unknown error. :" << __FILE__ << '(' << __LINE__ << ")";
			return false;

		}

		return true;
	}

	/**
	 * Ponder
	 */
	void CsaClient::ponder() {
		assert(_ponderCompleted.load() == false);

		// 相手番探索設定
		auto searchConfig = _searchConfigBase;
		searchConfig.maxDepth = 32;
		searchConfig.limitEnable = false;
		searchConfig.ponder = true;
		_searcher.setConfig(searchConfig);

		// 探索
		Loggers::message << "begin ponder";
		Move move;
		_searcher.setRecord(_record);
		_searcher.idsearch(_record.getBoard(), move);
		_searcher.clearRecord();
		Loggers::message << "end ponder";

		_ponderCompleted.store(true);
	}

	/**
	 * 探索設定を構築
	 */
	void CsaClient::buildSearchConfig(Searcher::Config& searchConfig) {
		// 思考時間設定
		if (searchConfig.limitEnable) {
			const auto& myTime = _gameSummary.black ? _blackTime : _whiteTime;

			// 次の一手で利用可能な最大時間
			int usableTime = myTime.usable();

			// 最大思考時間を確定
			usableTime = std::min(usableTime - 1, std::max(usableTime / 5 + 1, myTime.getReadoff() * 2));
			searchConfig.limitSeconds = std::min(searchConfig.limitSeconds, usableTime);
		}
	}

	bool CsaClient::login() {
		std::ostringstream os;
		os << "LOGIN " << _config.getString(CONF_USER) << ' ' << _config.getString(CONF_PASS);
		if (!send(os.str().c_str())) { return false; }
		unsigned response = waitReceive(RECV_LOGIN_MSK);
		return (response & RECV_LOGIN_OK) != 0U;
	}

	bool CsaClient::logout() {
		if (!send("LOGOUT")) { return false; }
#if 0
		unsigned response = waitReceive(RECV_LOGOUT);
		return (response & RECV_LOGOUT) != 0U;
#else
		// floodgateがLOGOUT:completedを送信してくれない。
		return true;
#endif
	}

	bool CsaClient::agree() {
		if (!send("AGREE")) { return false; }
		unsigned response = waitReceive(RECV_AGREE_MSK);
		return (response & RECV_START) != 0U;
	}

	bool CsaClient::sendMove(const MyMove& myMove, bool black, std::string* str) {
		std::ostringstream oss;
		oss << myMove.move.toStringCsa(black);
		if (_config.getBool(CONF_FLOODGATE)) {
			// 評価値
			int sign = _gameSummary.black ? 1 : -1;
			oss << ",\'* " << (myMove.value * sign).int32();
			// TODO: 読み筋
			//oss << ' ' << myMove.pv;
		}
		if (!send(oss.str().c_str())) {
			return false;
		}
		unsigned mask = _gameSummary.black ? RECV_MOVE_B : RECV_MOVE_W;
		unsigned response = waitReceive(mask | RECV_END_MSK, str);
		return (response & mask) != 0U;
	}

	bool CsaClient::sendResign() {
		if (!send("%TORYO")) { return false; }
		unsigned response = waitReceive(RECV_END_MSK);
		return (response & RECV_END_MSK) != 0U;
	}

	unsigned CsaClient::waitReceive(unsigned flags, std::string* str) {
		while (true) {
			{
				std::lock_guard<std::mutex> lock(_recvMutex);
				if (!_recvQueue.empty()) {
					RECV_DATA data = _recvQueue.front();
					_recvQueue.pop();
					unsigned masked = data.flag & flags;
					if (masked) {
						if (str != NULL) {
							(*str) = data.str;
						}
						return masked;
					} else if (data.flag & RECV_LOGOUT) {
						return 0U;
					}
				}
			}
			sleep(20);
		}
	}

	int CsaClient::getUsedTime(const std::string& recvStr) {
		std::string::size_type index = recvStr.find_last_of('T');
		if (index == std::string::npos) {
			return 0;
		}
		return std::stoi(recvStr.substr(index+1));
	}

	void CsaClient::receiver() {
		while (_con.receive()) {
			std::string recvStr = _con.getReceivedString();
			printReceivedString(recvStr);
			bool ok = enqueue(recvStr);
			if (!ok) {
				Loggers::warning << __FILE_LINE__ << ": parse error!!";
			}
		}
	}

	bool CsaClient::enqueue(const std::string& recvStr) {
		for (int i = 0; i < RECV_NUM; i++) {
			if (getFlagSets()[i].wildcard.match(recvStr)) {
				if (getFlagSets()[i].func != NULL) {
					getFlagSets()[i].func(this);
				}
				std::lock_guard<std::mutex> lock(_recvMutex);
				RECV_DATA data;
				data.flag = getFlagSets()[i].flag;
				data.str = recvStr;
				_recvQueue.push(data);
				_endFlags |= getFlagSets()[i].flag & RECV_END_MSK;
				return true;
			}
		}
		return false;
	}

	void CsaClient::recvGameSummary() {
		while (_con.receive()) {
			std::string recvStr = _con.getReceivedString();
			printReceivedString(recvStr);
			bool ok;
			if (recvStr == "BEGIN Time") {
				recvTime();
				ok = true;
			} else if (recvStr == "BEGIN Position") {
				recvBoard();
				ok = true;
			} else if (recvStr == "END Game_Summary") {
				return;
			} else {
				ok = inputGameSummary(recvStr);
			}
			if (!ok) {
				Loggers::warning << __FILE_LINE__ << ": parse error!!";
				Loggers::warning << recvStr;
			}
		}
	}

	bool CsaClient::inputGameSummary(std::string recvStr) {
		auto sep = recvStr.find_first_of(':');
		if (sep == std::string::npos) {
			return false;
		}
		std::string key = recvStr.substr(0, sep);
		std::string value = recvStr.substr(sep + 1);

		if (key == "Your_Turn") {
			if (value == "+") {
				_gameSummary.black = true;
			} else if (value == "-") {
				_gameSummary.black = false;
			} else {
				Loggers::warning << __FILE_LINE__ << ": unknown value [" << value << "]";
				return false;
			}
		} else if (key == "Game_ID") {
			_gameSummary.gameId = value;
		} else if (key == "Name+") {
			_gameSummary.blackName = value;
		} else if (key == "Name-") {
			_gameSummary.whiteName = value;
		} else if (key == "Protocol_Version") {
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Protocol_Mode") {
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Format") {
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Declaration") {
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Rematch_On_Draw") {
			// 自動再試合(CSAプロトコル仕様1.1.3では無視してよい。)
			WARN_IGNORED(key, value);
		} else if (key == "To_Move") {
			// TODO
		} else {
			Loggers::warning << __FILE_LINE__ << ": unknown key [" << key << "]";
			return false;
		}
		return true;
	}

	void CsaClient::recvTime() {
		while (_con.receive()) {
			std::string recvStr = _con.getReceivedString();
			printReceivedString(recvStr);
			if (recvStr == "END Time") {
				return;
			}
			bool ok = inputTime(recvStr);
			if (!ok) {
				Loggers::warning << __FILE_LINE__ << ": parse error!!";
			}
		}
	}

	bool CsaClient::inputTime(std::string recvStr) {
		auto sep = recvStr.find_first_of(':');
		if (sep == std::string::npos) {
			return false;
		}
		std::string key = recvStr.substr(0, sep);
		std::string value = recvStr.substr(sep + 1);

		if (key == "Time_Unit") {
			// 計測単位
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Least_Time_Per_Move") {
			// 1手毎の最小消費時間
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Time_Roundup") {
			// YES : 時間切り上げ
			// NO : 時間切り捨て
			// TODO
			WARN_IGNORED(key, value);
		} else if (key == "Total_Time") {
			// 持ち時間(省略時無制限)
			_gameSummary.totalTime = std::stoi(value);
		} else if (key == "Byoyomi") {
			// 秒読み
			_gameSummary.readoff = std::stoi(value);
		} else {
			Loggers::warning << __FILE_LINE__ << ": unknown key [" << key << "]";
			return false;
		}
		return true;
	}

	void CsaClient::recvBoard() {
		while (_con.receive()) {
			std::string recvStr = _con.getReceivedString();
			printReceivedString(recvStr);
			if (recvStr == "END Position") {
				return;
			}
			bool ok = inputBoard(recvStr);
			if (!ok) {
				Loggers::warning << __FILE_LINE__ << ": parse error!!";
			}
		}
	}

	bool CsaClient::inputBoard(std::string recvStr) {
		auto sep = recvStr.find_first_of(':');

		if (sep != std::string::npos) {
			std::string key = recvStr.substr(0, sep);
			std::string value = recvStr.substr(sep + 1);

			if (key == "Jishogi_Declaration") {
				// TODO: 宣言法対応
				WARN_IGNORED(key, value);
				return true;
			} else {
				Loggers::warning << __FILE_LINE__ << ": unknown key [" << key << "]";
				return false;
			}
		}

		return CsaReader::readBoard(recvStr.c_str(), _board);
	}

	RecordInfo CsaClient::getRecordInfo() const {
		return RecordInfo{
			_gameSummary.gameId,
			_gameSummary.blackName,
			_gameSummary.whiteName,
			_gameSummary.totalTime / 60 / 60,
			_gameSummary.totalTime / 60 % 60,
			_gameSummary.readoff,
		};
	}

	void CsaClient::writeResult() {
		// 結果の保存
		// TODO: ファイル名を指定可能に
		std::ofstream fout("csaClient.csv", std::ios::out | std::ios::app);
		std::ostringstream endStatus;
		for (int i = 0; i < RECV_NUM; i++) {
			if (_endFlags & getFlagSets()[i].flag) {
				endStatus << getFlagSets()[i].name << ' ';
			}
		}
		fout << _gameSummary.gameId << ','
				<< _gameSummary.blackName << ','
				<< _gameSummary.whiteName << ','
				<< endStatus.str() << '\n';
		fout.close();

		// 棋譜の保存
		std::string dir = _config.getString(CONF_KIFU);
		std::ostringstream path;
		path << dir;
		if (dir[dir.length()-1] != '/') {
			path << '/';
		}
		path << _gameSummary.gameId << ".csa";
		RecordInfo info = getRecordInfo();
		CsaWriter::write(path.str().c_str(), _record, &info);
	}

}
