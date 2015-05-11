/* CsaReader.cpp
 *
 * Kubo Ryosuke
 */

#include "CsaReader.h"
#include "logger/Logger.h"
#include <fstream>
#include <string>
#include <cstring>

#define LINE_BUFFER_SIZE 1024

namespace sunfish {

bool CsaReader::read(const char* filename, Record& record, RecordInfo* info/* = nullptr*/) {

  std::ifstream file(filename);
  if (!file) {
    return false;
  }

  bool ok = read(file, record, info);

  file.close();
  return ok;

}

bool CsaReader::read(std::istream& is, Record& record, RecordInfo* info/* = nullptr*/) {
  Board board;

  // 局面の読み込み
  if (!readBoard(is, board, info)) {
    return false;
  }

  record.init(board);

  // 指し手の読み込み
  if (!readMoves(is, record)) {
    return false;
  }

  return true;
}

/**
 * 局面の読み込み
 */
bool CsaReader::readBoard(std::istream& is, Board& board, RecordInfo* info/* = nullptr*/) {
  bool ok = _readBoard(is, board, info);
  board.refreshHash();
  return ok;
}

/**
 * 局面の読み込み
 */
bool CsaReader::readBoard(const char* line, Board& board, RecordInfo* info/* = nullptr*/) {
  bool ok = _readBoard(line, board, info);
  board.refreshHash();
  return ok;
}

/**
 * 局面の読み込み
 */
bool CsaReader::_readBoard(std::istream& is, Board& board, RecordInfo* info/* = nullptr*/) {
  char line[LINE_BUFFER_SIZE];

  board.init();

  while (true) {
    is.getline(line, sizeof(line));
    if (is.eof()) {
      break;
    }
    if (is.fail()) {
      Loggers::warning << "file io error. " << __FILE__ << "(" << __LINE__ << ")";
      return false;
    }
    if (!_readBoard(line, board, info)) {
      Loggers::warning << "invalid board format. " << __FILE__ << "(" << __LINE__ << ")";
      return false;
    }
    if (line[0] == '+' || line[0] == '-') {
      break;
    }
  }

  assert(board.getBKingPosition() != Position::Invalid);
  assert(board.getWKingPosition() != Position::Invalid);

  return true;
}

/**
 * 局面の読み込み
 */
bool CsaReader::_readBoard(const char* line, Board& board, RecordInfo* info/* = nullptr*/) {
  switch (line[0]) {
  case 'P':
    if (line[1] >= '1' && line[1] <= '9') {
      return _readBoardPieces(line, board);
    } else if (line[1] == '+') {
      return _readHand(line, board, true);
    } else if (line[1] == '-') {
      return _readHand(line, board, false);
    }
    Loggers::warning << __FILE_LINE__ << ": unknown command";
    Loggers::warning << line;
    return false;
  case '+':
    board.setBlack();
    return true;
  case '-':
    board.setWhite();
    return true;
  case '$': case 'N':
    return info != nullptr ? _readInfo(line, *info) : true;
  case 'V': case '\'': case '\0':
    return true;
  default:
    Loggers::warning << __FILE_LINE__ << ": unknown command";
    Loggers::warning << line;
    return false;
  }
}

/**
 * 盤面の読み込み
 */
bool CsaReader::_readBoardPieces(const char* line, Board& board) {
  if (strlen(line) < 2 + 3 * Position::FileN) {
    Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
    return false;
  }
  int rank = line[1] - '0';
  for (int file = 1; file <= Position::FileN; file++) {
    Piece piece = Piece::parseCsa(line + 2 + 3 * (9 - file));
    board.setBoardPiece(Position(file, rank), piece);
  }
  return true;
}

bool CsaReader::_readInfo(const char* line, RecordInfo& info) {
  if (strncmp(line, "$EVENT:", 7) == 0) {
    info.title = &line[7];

  } else if (strncmp(line, "N+", 2) == 0) {
    info.blackName = &line[2];

  } else if (strncmp(line, "N-", 2) == 0) {
    info.whiteName = &line[2];

  } else if (strncmp(line, "$TIME_LIMIT:", 12) == 0 && strlen(line) >= 20) {
    info.timeLimitHour = std::stoi(&line[12]);
    info.timeLimitMinutes = std::stoi(&line[15]);
    info.timeLimitReadoff = std::stoi(&line[18]);

  } else {
    Loggers::warning << "unknown command: [" << line << "]: " << __FILE__ << "(" << __LINE__ << ")";

  }

  return true;
}

/**
 * 持ち駒の読み込み
 */
bool CsaReader::_readHand(const char* line, Board& board, bool black) {
  unsigned length = (unsigned)strlen(line);
  for (unsigned i = 2; i + 4 <= length; i += 4) {
    unsigned file = line[i+0] - '0';
    unsigned rank = line[i+1] - '0';
    Piece piece = Piece::parseCsa(&line[i+2]);
    if (piece != Piece::Empty) {
      if (Position::isValidFile(file) && Position::isValidRank(rank)) {
        board.setBoardPiece(Position(file, rank), black ? piece.black() : piece.white());
      } else if (file == 0 && rank == 0 && piece.isUnpromoted() && piece != Piece::King) {
        if (black) {
          board.incBlackHand(piece);
        } else {
          board.incWhiteHand(piece);
        }
      } else {
        Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
        return false;
      }
    } else {
      Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
      return false;
    }
  }
  return true;
}

/**
 * 手順の読み込み
 */
bool CsaReader::readMoves(std::istream& is, Record& record) {
  char line[LINE_BUFFER_SIZE];
  Move move;

  while (true) {
    is.getline(line, sizeof(line));
    if (is.eof()) {
      return true;
    }
    if (is.fail()) {
      Loggers::warning << "file io error. " << __FILE__ << "(" << __LINE__ << ")";
      return false;
    }
    if (_readComment(line) || _readCommand(line) || _readTime(line)) {
      continue;
    }
    if (!readMove(line, record.getBoard(), move)) {
      Loggers::warning << "invalid move format. " << __FILE__ << "(" << __LINE__ << ")";
      Loggers::warning << "> " << line;
      return false;
    }
    record.makeMove(move);
  }
}

bool CsaReader::_readComment(const char* line) {
  if (line[0] == '\0' || line[0] == '\'') {
    return true;
  }
  return false;
}

bool CsaReader::_readTime(const char* line) {
  if (line[0] == 'T') {
    return true;
  }
  return false;
}

bool CsaReader::_readCommand(const char* line) {
  if (line[0] == '%') {
    return true;
  }
  return false;
}

/**
 * 指し手の読み込み
 */
bool CsaReader::readMove(const char* line, const Board& board, Move& move) {
  if (strlen(line) < 7) {
    return false;
  }

  bool black;
  if (line[0] == '+') {
    black = true;
  } else if (line[0] == '-') {
    black = false;
  } else {
    return false;
  }

  if (black != board.isBlack()) {
    return false;
  }

  Position to = Position::parse(&line[3]);
  Piece piece = Piece::parseCsa(&line[5]);
  if (line[1] == '0' && line[2] == '0') {
    move = Move(piece, to);
  } else {
    Position from = Position::parse(&line[1]);
    Piece pieceOrg = board.getBoardPiece(from).kindOnly();
    bool promote;
    if (piece == pieceOrg) {
      promote = false;
    } else if (piece == pieceOrg.promote()) {
      promote = true;
    } else {
      return false;
    }
    move = Move(pieceOrg, from, to, promote);
  }

  return true;
}

} // namespace sunfish
