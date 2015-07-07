/* CsaReader.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_CSAREADER__
#define SUNFISH_CSAREADER__

#include "Record.h"
#include <iostream>

namespace sunfish {

class CsaReader {
private:

  CsaReader();

  static bool readBoard_(std::istream& is, Board& board, RecordInfo* info = nullptr);
  static bool readBoard_(const char* line, Board& board, RecordInfo* info = nullptr);
  static bool readBoardPieces_(const char* line, Board& board);
  static bool readInfo_(const char* line, RecordInfo& info);
  static bool readHand_(const char* line, Board& board, bool black);
  static bool readComment_(const char* line);
  static bool readTime_(const char* line);
  static bool readCommand_(const char* line);

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

#endif // SUNFISH_CSAREADER__
