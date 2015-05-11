/* CsaReader.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_CSAREADER__
#define __SUNFISH_CSAREADER__

#include "Record.h"
#include <iostream>

namespace sunfish {

class CsaReader {
private:

  CsaReader();

  static bool _readBoard(std::istream& is, Board& board, RecordInfo* info = nullptr);
  static bool _readBoard(const char* line, Board& board, RecordInfo* info = nullptr);
  static bool _readBoardPieces(const char* line, Board& board);
  static bool _readInfo(const char* line, RecordInfo& info);
  static bool _readHand(const char* line, Board& board, bool black);
  static bool _readComment(const char* line);
  static bool _readTime(const char* line);
  static bool _readCommand(const char* line);

public:

  static bool read(const char* filename, Record& record, RecordInfo* info = nullptr);
  static bool read(const std::string& filename, Record& record, RecordInfo* info = nullptr) {
    return read(filename.c_str(), record, info);
  }
  static bool read(std::istream& is, Record& record, RecordInfo* info = nullptr);
  static bool readBoard(std::istream& is, Board& board, RecordInfo* info = nullptr);
  static bool readBoard(const char* line, Board& board, RecordInfo* info = nullptr);
  static bool readMoves(std::istream& is, Record& record);
  static bool readMove(const char* line, const Board& board, Move& move);

};

} // namespace sunfish

#endif // __SUNFISH_CSAREADER__
