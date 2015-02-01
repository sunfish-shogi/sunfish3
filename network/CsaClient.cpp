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

	bool CsaClient::execute() {
		// 設定の読み込み
		if (!_config.read(_configFilename)) {
			return false;
		}
		Loggers::message << _config.toString();

		_con.setHost(_config.getHost());
		_con.setPort(_config.getPort());
		_con.setKeepalive(_config.getKeepalive(), _config.getKeepidle(),
			_config.getKeepintvl(), _config.getKeepcnt());

		// 連続対局
		for (int i = 0; i < _config.getRepeat(); i++) {
			if (!_con.connect()) {
				Loggers::error << "ERROR: can not connect to " << _config.getHost()
						<< ':' << _config.getPort();
				return false;
			}
			init();
			std::thread receiverThread([this]() {
				receiver();
			});

			// login
			if (!login()) {
				Loggers::message << "login failed!";
				goto lab_end;
			}
			Loggers::message << "login ok!!";

			// wait for match-make and agree
			if (waitGameSummary() && agree()) {

				// 棋譜の初期化
				Record record;
				record.init(_board);

				// 探索エージェントの初期化
				// TODO: 評価パラメータのロードを初回のみにする。
				Searcher searcher;
				searcher.init();

				// 探索設定
				auto searchConfigBase = searcher.getConfig();
				searchConfigBase.maxDepth = _config.getDepth();
				searchConfigBase.limitEnable = _config.getLimit() != 0;
				searchConfigBase.limitSeconds = _config.getLimit();

				// 相手番探索設定
				auto searchConfigEnemy = searchConfigBase;
				searchConfigEnemy.maxDepth = 32;
				searchConfigEnemy.limitEnable = false;

				// 残り時間の初期化
				_blackTime.init(gameSummary.totalTime, gameSummary.readoff);
				_whiteTime.init(gameSummary.totalTime, gameSummary.readoff);

				while (1) {
#ifndef NDEBUG
					CsaWriter::write("debug.csa", record);
#endif
					Loggers::message << "Time(Black):" << _blackTime.toString();
					Loggers::message << "Time(White):" << _whiteTime.toString();
					if (gameSummary.black == record.isBlack()) {
						// 自分の手番
						SendingMove sendingMove;
						Move move;
						auto searchConfig = searchConfigBase;
						buildSearchConfig(searchConfig);
						searcher.setConfig(searchConfig);
						Loggers::message << "begin search: limit(sec)=" << searchConfig.limitSeconds;
						bool ok = searcher.idsearch(record.getBoard(), move);
						Loggers::message << "end search";
						if (ok) {
							sendingMove.set(searcher.getInfo());
						}
						if (ok && record.makeMove(move)) {
							std::string recvStr;
							if (!sendMove(sendingMove, !record.getBoard().isBlack(), &recvStr)) {
								// TODO: エラーの詳細を出力
								Loggers::error << "ERROR:could not send a move";
								break;
							}
							// 消費時間の読み込み
							int usedTime = getUsedTime(recvStr);
							if (gameSummary.black) {
								_blackTime.use(usedTime);
							} else {
								_whiteTime.use(usedTime);
							}
						} else {
							sendResign();
							break;
						}
					} else {
						// 相手番

						// TODO: 相手番中の思考開始

						// 相手番の指し手を受信
						std::string recvStr;
						unsigned mask = gameSummary.black ? RECV_MOVE_W : RECV_MOVE_B;
						unsigned flags = waitReceive(mask | RECV_END_MSK, &recvStr);

						// TODO: 相手番中の思考終了

						if (flags & mask) {
							// 受信した指し手の読み込み
							Move move;
							if (!CsaReader::readMove(recvStr.c_str(), record.getBoard(), move) ||
									!record.makeMove(move)) {
								Loggers::error << "ERROR:illegal move!!";
								break;
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
							break;
						} else {
							// エラー
							Loggers::error << "ERROR:unknown error. :" << __FILE__ << '(' << __LINE__ << ")";
							break;
						}
					}
					Loggers::message << record.getMove().toString();
				}
				// 対局結果の記録
				writeResult(record);
			}

			// logout
			logout();

lab_end:
			_con.disconnect();
			receiverThread.join();
		}
		return true;
	}

	void CsaClient::buildSearchConfig(Searcher::Config& searchConfig) {
		if (searchConfig.limitEnable) {
			int usableTime = gameSummary.black
					? _blackTime.usable() : _whiteTime.usable();
			usableTime = usableTime / 20;
			if (usableTime <= 0) { usableTime = 1; }
			searchConfig.limitSeconds =
					searchConfig.limitSeconds <= usableTime
					? searchConfig.limitSeconds : usableTime;
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

	bool CsaClient::sendMove(const SendingMove& sendingMove, bool black, std::string* str) {
		std::ostringstream oss;
		oss << sendingMove.move.toStringCsa(black);
		if (_config.getFloodgate()) {
			// 評価値
			int sign = gameSummary.black ? 1 : -1;
			oss << ",\'* " << (sendingMove.value * sign).int32();
			// TODO: 読み筋
			//oss << ' ' << sendingMove.pv;
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
