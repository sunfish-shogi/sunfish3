/*
 * CsaClient.cpp
 */

#include "CsaClient.h"
#include "core/def.h"
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
  configFilename_ = DEFAULT_CONFIG_FILE;

  config_.addDef(CONF_HOST, "localhost");
  config_.addDef(CONF_PORT, "4081");
  config_.addDef(CONF_USER, "test");
  config_.addDef(CONF_PASS, "");
  config_.addDef(CONF_DEPTH, "32");
  config_.addDef(CONF_LIMIT, "10");
  config_.addDef(CONF_REPEAT, "1");
  config_.addDef(CONF_WORKER, "1");
  config_.addDef(CONF_PONDER, "1");
  config_.addDef(CONF_KEEPALIVE, "1");
  config_.addDef(CONF_KEEPIDLE, "120");
  config_.addDef(CONF_KEEPINTVL, "60");
  config_.addDef(CONF_KEEPCNT, "10");
  config_.addDef(CONF_FLOODGATE, "0");
  config_.addDef(CONF_KIFU, "Kifu");
  config_.addDef(CONF_MONITOR, "");
}

const CsaClient::ReceiveFlagSet* CsaClient::getFlagSets() {
  static const ReceiveFlagSet flagSets[RECV_NUM] = {
    { Wildcard("LOGIN:* OK"), RECV_LOGIN_OK, NULL, NULL },
    { Wildcard("LOGIN:incorrect"), RECV_LOGIN_INC, NULL, NULL },
    { Wildcard("LOGOUT:completed"), RECV_LOGOUT, NULL, NULL },
    { Wildcard("%*"), RECV_MOVE_EX, NULL, NULL },
    { Wildcard("+*"), RECV_MOVE_B, NULL, NULL },
    { Wildcard("-*"), RECV_MOVE_W, NULL, NULL },
    { Wildcard("BEGIN Game_Summary"), RECV_SUMMARY, recvGameSummary_, NULL },
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
  if (!config_.read(configFilename_)) {
    return false;
  }
  Loggers::message << config_.toString();

  // 通信設定
  con_.setHost(config_.getString(CONF_HOST));
  con_.setPort(config_.getInt(CONF_PORT));
  con_.setKeepalive(config_.getInt(CONF_KEEPALIVE), config_.getInt(CONF_KEEPIDLE),
  config_.getInt(CONF_KEEPINTVL), config_.getInt(CONF_KEEPCNT));

  // 定跡読み込み
  book_.readFile();

  // 探索設定
  searchConfigBase_ = searcher_.getConfig();
  searchConfigBase_.maxDepth = config_.getInt(CONF_DEPTH);
  searchConfigBase_.limitSeconds = config_.getDouble(CONF_LIMIT);
  searchConfigBase_.enableLimit = searchConfigBase_.limitSeconds != 0.0;
  searchConfigBase_.workerSize = std::max(config_.getInt(CONF_WORKER), 1);
  searchConfigBase_.treeSize = Searcher::standardTreeSize(searchConfigBase_.workerSize);
  searcher_.setConfig(searchConfigBase_);

  // 連続対局
  int repeatCount = config_.getInt(CONF_REPEAT);
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
  if (!con_.connect()) {
    Loggers::error << "ERROR: can not connect to " << config_.getString(CONF_HOST)
        << ':' << config_.getInt(CONF_PORT);
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
    record_.init(board_);

    // 残り時間の初期化
    blackTime_.init(gameSummary_.totalTime, gameSummary_.readoff);
    whiteTime_.init(gameSummary_.totalTime, gameSummary_.readoff);

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
  con_.disconnect();
  receiverThread.join();

  return success;
}

/**
 * 対局を進める
 */
bool CsaClient::nextTurn() {
  std::string monitor = config_.getString(CONF_MONITOR);
  if (!monitor.empty()) {
    RecordInfo info = getRecordInfo();
    CsaWriter::write(monitor, record_, &info);
  }

  // 残り時間を表示
  Loggers::message << "Time(Black):" << blackTime_.toString();
  Loggers::message << "Time(White):" << whiteTime_.toString();

  bool ok;
  if (gameSummary_.black == record_.isBlack()) {
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
  Loggers::message << record_.getMove().toString();

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
    const auto& board = record_.getBoard();
    uint64_t hash = board.getHash();
    BookResult bookResult = book_.selectRandom(hash);
    if (!bookResult.move.isEmpty() && board.isValidMove(bookResult.move)) {
      myMove.move = bookResult.move;
      myMove.value = Value::Zero;
      myMove.pv.init();
      Loggers::message << "book hit: " << myMove.move.toString() << " (" << bookResult.count << "/" << bookResult.total << ")";
      ok = true;
    }
  }

  // 探索設定
  if (!ok) {
    auto searchConfig = searchConfigBase_;
    buildSearchConfig(searchConfig);
    searcher_.setConfig(searchConfig);

    // 探索
    Loggers::message << "begin search: limit(sec)=" << searchConfig.limitSeconds;
    searcher_.setRecord(record_);
    ok = searcher_.idsearch(record_.getBoard(), myMove.move);
    searcher_.clearRecord();
    Loggers::message << "end search";

    Loggers::message << searcher_.getInfoString();

    if (ok) {
      myMove.value = searcher_.getInfo().eval;
      myMove.pv = searcher_.getInfo().pv;
    }
  }

  if (ok && record_.makeMove(myMove.move)) {
    // 指し手を送信
    std::string recvStr;
    if (!sendMove(myMove, !record_.getBoard().isBlack(), &recvStr)) {
      // TODO: エラーの詳細を出力
      Loggers::error << "ERROR:could not send a move";
      return false;
    }

    // 消費時間の読み込み
    int usedTime = getUsedTime(recvStr);
    if (gameSummary_.black) {
      blackTime_.use(usedTime);
    } else {
      whiteTime_.use(usedTime);
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
  bool enablePonder = config_.getBool(CONF_PONDER);

  std::thread ponderThread;

  if (enablePonder) {
    // 相手番中の思考開始
    ponderCompleted_.store(false);
    ponderThread = std::thread(std::bind(std::mem_fn(&CsaClient::ponder), this));
  }

  // 相手番の指し手を受信
  std::string recvStr;
  unsigned mask = gameSummary_.black ? RECV_MOVE_W : RECV_MOVE_B;
  unsigned flags = waitReceive(mask | RECV_END_MSK, &recvStr);

  if (enablePonder) {
    // 探索が開始されていることを確認
    while (!searcher_.isRunning() && !ponderCompleted_.load()) {
      std::this_thread::yield();
    }

    // 相手番中の思考終了
    Loggers::message << "force interrupt";
    searcher_.forceInterrupt();
    Loggers::message << "join...";
    ponderThread.join();
    Loggers::message << "completed";
  }

  if (flags & mask) {
    // 受信した指し手の読み込み
    Move move;
    if (!CsaReader::readMove(recvStr.c_str(), record_.getBoard(), move) ||
        !record_.makeMove(move)) {
      Loggers::error << "ERROR:illegal move!!";
      return false;
    }

    // 消費時間の読み込み
    int usedTime = getUsedTime(recvStr);
    if (gameSummary_.black) {
      whiteTime_.use(usedTime);
    } else {
      blackTime_.use(usedTime);
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
  assert(ponderCompleted_.load() == false);

  // 相手番探索設定
  auto searchConfig = searchConfigBase_;
  searchConfig.maxDepth = 32;
  searchConfig.enableLimit = false;
  searchConfig.ponder = true;
  searcher_.setConfig(searchConfig);

  // 探索
  Loggers::message << "begin ponder";
  Move move;
  searcher_.setRecord(record_);
  searcher_.idsearch(record_.getBoard(), move);
  searcher_.clearRecord();
  Loggers::message << "end ponder";

  ponderCompleted_.store(true);
}

/**
 * 探索設定を構築
 */
void CsaClient::buildSearchConfig(Searcher::Config& searchConfig) {
  // 思考時間設定
  if (searchConfig.enableLimit) {
    const auto& myTime = gameSummary_.black ? blackTime_ : whiteTime_;

    // 次の一手で利用可能な最大時間
    float usableTime = myTime.usable();

    // マージン
    CONSTEXPR_CONST float marginTime = 1.0f;

    // 最大思考時間を確定
    usableTime = std::min(usableTime - marginTime, std::max(usableTime / 5.0f, myTime.getReadoff() * 3.0f));
    searchConfig.limitSeconds = std::min(searchConfig.limitSeconds, usableTime);

    // 時間を使いきっている場合は最大まで使う
    if (myTime.getRemain() == 0) {
      searchConfig.enableTimeManagement = false;
    }
  }
}

bool CsaClient::login() {
  std::ostringstream os;
  os << "LOGIN " << config_.getString(CONF_USER) << ' ' << config_.getString(CONF_PASS);
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
  if (config_.getBool(CONF_FLOODGATE)) {
    // 評価値
    int sign = gameSummary_.black ? 1 : -1;
    oss << ",\'* " << (myMove.value * sign).int32();
    // 読み筋
    oss << ' ' << myMove.pv.toStringCsa(black);
  }
  if (!send(oss.str().c_str())) {
    return false;
  }
  unsigned mask = gameSummary_.black ? RECV_MOVE_B : RECV_MOVE_W;
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
      std::lock_guard<std::mutex> lock(recvMutex_);
      if (!recvQueue_.empty()) {
        RECV_DATA data = recvQueue_.front();
        recvQueue_.pop();
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
  while (con_.receive()) {
    std::string recvStr = con_.getReceivedString();
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
      std::lock_guard<std::mutex> lock(recvMutex_);
      RECV_DATA data;
      data.flag = getFlagSets()[i].flag;
      data.str = recvStr;
      recvQueue_.push(data);
      endFlags_ |= getFlagSets()[i].flag & RECV_END_MSK;
      return true;
    }
  }
  return false;
}

void CsaClient::recvGameSummary() {
  while (con_.receive()) {
    std::string recvStr = con_.getReceivedString();
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
      gameSummary_.black = true;
    } else if (value == "-") {
      gameSummary_.black = false;
    } else {
      Loggers::warning << __FILE_LINE__ << ": unknown value [" << value << "]";
      return false;
    }
  } else if (key == "Game_ID") {
    gameSummary_.gameId = value;
  } else if (key == "Name+") {
    gameSummary_.blackName = value;
  } else if (key == "Name-") {
    gameSummary_.whiteName = value;
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
  while (con_.receive()) {
    std::string recvStr = con_.getReceivedString();
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
    gameSummary_.totalTime = std::stoi(value);
  } else if (key == "Byoyomi") {
    // 秒読み
    gameSummary_.readoff = std::stoi(value);
  } else {
    Loggers::warning << __FILE_LINE__ << ": unknown key [" << key << "]";
    return false;
  }
  return true;
}

void CsaClient::recvBoard() {
  while (con_.receive()) {
    std::string recvStr = con_.getReceivedString();
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

  return CsaReader::readBoard(recvStr.c_str(), board_);
}

RecordInfo CsaClient::getRecordInfo() const {
  return RecordInfo{
    gameSummary_.gameId,
    gameSummary_.blackName,
    gameSummary_.whiteName,
    gameSummary_.totalTime / 60 / 60,
    gameSummary_.totalTime / 60 % 60,
    gameSummary_.readoff,
  };
}

void CsaClient::writeResult() {
  // 結果の保存
  // TODO: ファイル名を指定可能に
  std::ofstream fout("csaClient.csv", std::ios::out | std::ios::app);
  std::ostringstream endStatus;
  for (int i = 0; i < RECV_NUM; i++) {
    if (endFlags_ & getFlagSets()[i].flag) {
      endStatus << getFlagSets()[i].name << ' ';
    }
  }
  fout << gameSummary_.gameId << ','
      << gameSummary_.blackName << ','
      << gameSummary_.whiteName << ','
      << endStatus.str() << '\n';
  fout.close();

  // 棋譜の保存
  std::string dir = config_.getString(CONF_KIFU);
  std::ostringstream path;
  path << dir;
  if (dir[dir.length()-1] != '/') {
    path << '/';
  }
  path << gameSummary_.gameId << ".csa";
  RecordInfo info = getRecordInfo();
  CsaWriter::write(path.str().c_str(), record_, &info);
}

} // namespace sunfish
