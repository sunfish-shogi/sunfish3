/*
 * CsaClient.cpp
 */

#include "CsaClient.h"
#include "core/record/Record.h"
#include "core/record/CsaReader.h"
#include "core/record/CsaWriter.h"
#include <regex>
#include <mutex>
#include <fstream>
#include <sstream>
#include <thread>

#define WARN_IGNORED(key, value) Loggers::warning << __THIS__ << ": not supported: key=[" << (key) << "] value=[" << (value) << "]"

namespace sunfish {

	const char* CsaClient::DEFAULT_CONFIG_FILE = "network.conf";

	const CsaClient::ReceiveFlagSet CsaClient::FlagSets[RECV_NUM] = {
		{ std::regex("^LOGIN:.* OK$"), RECV_LOGIN_OK, NULL, NULL },
		{ std::regex("^LOGIN:incorect$"), RECV_LOGIN_INC, NULL, NULL },
		{ std::regex("^LOGOUT:completed$"), RECV_LOGOUT, NULL, NULL },
		{ std::regex("^%.*"), RECV_MOVE_EX, NULL, NULL },
		{ std::regex("^\\+.*"), RECV_MOVE_B, NULL, NULL },
		{ std::regex("^-.*"), RECV_MOVE_W, NULL, NULL },
		{ std::regex("^BEGIN Game_Summary$"), RECV_SUMMARY, _recvGameSummary, NULL },
		{ std::regex("^START:.*"), RECV_START, NULL, NULL },
		{ std::regex("^REJECT:.* by .*"), RECV_REJECT, NULL, NULL },
		{ std::regex("^#WIN$"), RECV_WIN, NULL, "win" },
		{ std::regex("^#LOSE$"), RECV_LOSE, NULL, "lose" },
		{ std::regex("^#WIN\\(LOSE\\)$"), RECV_WIN_LOSE, NULL, "unknown" }, // CSA将棋付属の簡易サーバ用
		{ std::regex("^#DRAW$"), RECV_DRAW, NULL, "draw" },
		{ std::regex("^#CHUDAN$"), RECV_INTERRUPT, NULL, "chudan" },
		{ std::regex("^#SENNICHITE$"), RECV_REPEAT, NULL, "sennichite" },
		{ std::regex("^#OUTE_SENNICHITE$"), RECV_CHECK_REP, NULL, "oute sennichite" },
		{ std::regex("^#ILLEGAL_MOVE$"), RECV_ILLEGAL, NULL, "illegal move" },
		{ std::regex("^#TIME_UP$"), RECV_TIME_UP, NULL, "time up" },
		{ std::regex("^#RESIGN$"), RECV_RESIGN, NULL, "resign" },
		{ std::regex("^#JISHOGI$"), RECV_JISHOGI, NULL, "jishogi" },
	};

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
		_con.setHost(_config.getHost());
		_con.setPort(_config.getPort());
		_con.setKeepalive(_config.getKeepalive(), _config.getKeepidle(),
			_config.getKeepintvl(), _config.getKeepcnt());

		// 連続対局
		for (int i = 0; i < _config.getRepeat(); i++) {
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
			Loggers::error << "ERROR: can not connect to " << _config.getHost()
					<< ':' << _config.getPort();
			return false;
		}

		// 初期化
		init();

		// 定跡読み込み
		_book.readFile();

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

			// 探索エージェントの初期化
			// TODO: 評価パラメータのロードを初回のみにする。
			_searcher.init();

			// 探索設定
			_searchConfigBase = _searcher.getConfig();
			_searchConfigBase.maxDepth = _config.getDepth();
			_searchConfigBase.limitEnable = _config.getLimit() != 0;
			_searchConfigBase.limitSeconds = _config.getLimit();

			// 残り時間の初期化
			_blackTime.init(gameSummary.totalTime, gameSummary.readoff);
			_whiteTime.init(gameSummary.totalTime, gameSummary.readoff);

			while (1) {
				bool ok = nextTurn();
				if (!ok) {
					break;
				}
			}
			// 対局結果の記録
			writeResult(_record);
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
#ifndef NDEBUG
		CsaWriter::write("debug.csa", _record);
#endif

		// 残り時間を表示
		Loggers::message << "Time(Black):" << _blackTime.toString();
		Loggers::message << "Time(White):" << _whiteTime.toString();

		bool ok;
		if (gameSummary.black == _record.isBlack()) {
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
  		ok = _searcher.idsearch(_record.getBoard(), myMove.move);
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
			if (gameSummary.black) {
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
		// 相手番中の思考開始
		_ponderCompleted = false;
		std::thread th([this]() {
			ponder();
		});

		// 相手番の指し手を受信
		std::string recvStr;
		unsigned mask = gameSummary.black ? RECV_MOVE_W : RECV_MOVE_B;
		unsigned flags = waitReceive(mask | RECV_END_MSK, &recvStr);

		// 探索が開始されていることを確認
		while (!_searcher.isRunning() && !_ponderCompleted) {
			std::this_thread::yield();
		}

		// 相手番中の思考終了
		_searcher.forceInterrupt();
		th.join();

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
			if (gameSummary.black) {
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
		assert(_ponderCompleted == false);

		// 相手番探索設定
		auto searchConfig = _searchConfigBase;
		searchConfig.maxDepth = 32;
		searchConfig.limitEnable = false;
		_searcher.setConfig(searchConfig);

		// 探索
		Loggers::message << "begin ponder";
		Move move;
		_searcher.idsearch(_record.getBoard(), move);
		Loggers::message << "end ponder";

		_ponderCompleted = true;
	}

	/**
	 * 探索設定を構築
	 */
	void CsaClient::buildSearchConfig(Searcher::Config& searchConfig) {
		// 思考時間設定
		if (searchConfig.limitEnable) {
			const auto& myTime = gameSummary.black ? _blackTime : _whiteTime;

			// 次の一手で利用可能な最大時間
			int usableTime = myTime.usable();

			// 最大思考時間を確定
			usableTime = std::min(usableTime - 1, std::max(usableTime / 20 + 1, myTime.getReadoff() * 2));
			searchConfig.limitSeconds = std::min(searchConfig.limitSeconds, usableTime);
		}
	}

	bool CsaClient::login() {
		std::ostringstream os;
		os << "LOGIN " << _config.getUser() << ' ' << _config.getPass();
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
		if (_config.getFloodgate()) {
			// 評価値
			int sign = gameSummary.black ? 1 : -1;
			oss << ",\'* " << (myMove.value * sign).int32();
			// TODO: 読み筋
			//oss << ' ' << myMove.pv;
		}
		if (!send(oss.str().c_str())) {
			return false;
		}
		unsigned mask = gameSummary.black ? RECV_MOVE_B : RECV_MOVE_W;
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
				Loggers::warning << __THIS__ << ": parse error!!";
			}
		}
	}

	bool CsaClient::enqueue(const std::string& recvStr) {
		for (int i = 0; i < RECV_NUM; i++) {
			if (std::regex_match(recvStr, FlagSets[i].regex)) {
				if (FlagSets[i].func != NULL) {
					FlagSets[i].func(this);
				}
				std::lock_guard<std::mutex> lock(_recvMutex);
				RECV_DATA data;
				data.flag = FlagSets[i].flag;
				data.str = recvStr;
				_recvQueue.push(data);
				_endFlags |= FlagSets[i].flag & RECV_END_MSK;
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
				Loggers::warning << __THIS__ << ": parse error!!";
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
				gameSummary.black = true;
			} else if (value == "-") {
				gameSummary.black = false;
			} else {
				Loggers::warning << __THIS__ << ": unknown value [" << value << "]";
				return false;
			}
		} else if (key == "Game_ID") {
			gameSummary.gameId = value;
		} else if (key == "Name+") {
			gameSummary.blackName = value;
		} else if (key == "Name-") {
			gameSummary.whiteName = value;
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
			Loggers::warning << __THIS__ << ": unknown key [" << key << "]";
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
				Loggers::warning << __THIS__ << ": parse error!!";
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
			gameSummary.totalTime = std::stoi(value);
		} else if (key == "Byoyomi") {
			// 秒読み
			gameSummary.readoff = std::stoi(value);
		} else {
			Loggers::warning << __THIS__ << ": unknown key [" << key << "]";
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
				Loggers::warning << __THIS__ << ": parse error!!";
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
				Loggers::warning << __THIS__ << ": unknown key [" << key << "]";
				return false;
			}
		}

		return CsaReader::readBoard(recvStr.c_str(), _board);
	}

	void CsaClient::writeResult(const Record& record) {
		// 結果の保存
		// TODO: ファイル名を指定可能に
		std::ofstream fout("csaClient.csv", std::ios::out | std::ios::app);
		std::ostringstream endStatus;
		for (int i = 0; i < RECV_NUM; i++) {
			if (_endFlags & FlagSets[i].flag) {
				endStatus << FlagSets[i].name << ' ';
			}
		}
		fout << gameSummary.gameId << ','
				<< gameSummary.blackName << ','
				<< gameSummary.whiteName << ','
				<< endStatus.str() << '\n';
		fout.close();

		// 棋譜の保存
		std::ostringstream path;
		path << _config.getKifu();
		path << gameSummary.gameId << ".csa";
		CsaWriter::write(path.str().c_str(), record);
	}

}
