/* CsaWriter.cpp
 *
 * Kubo Ryosuke
 */

#include "CsaWriter.h"
#include <fstream>

namespace sunfish {

/**
 * 棋譜をファイルに書き出します。
 */
bool CsaWriter::write(const char* filename, const Record& record, const RecordInfo* info/* = nullptr*/) {
  std::ofstream file(filename);

  if (!file) {
    return false;
  }

  write(file, record, info);

  file.close();

  return true;
}

/**
 * 棋譜をストリームに書き出します。
 */
void CsaWriter::write(std::ostream& os, const Record& record, const RecordInfo* info/* = nullptr*/) {

  os << "V2.2\n";

  if (info != nullptr) {
    writeInfo(os, *info);
  }

  // 開始局面
  writeBoard(os, record.getInitialBoard());

  // 指し手
  writeMoves(os, record);

}

/**
 * 対局情報をストリームに書き出します。
 */
void CsaWriter::writeInfo(std::ostream& os, const RecordInfo& info) {

  os << "N+" << info.blackName << '\n';
  os << "N-" << info.whiteName << '\n';
  os << "$EVENT:" << info.title << '\n';
  os << "$TIME_LIMIT:"
    << std::setw(2) << std::setfill('0') << info.timeLimitHour << ':'
    << std::setw(2) << std::setfill('0') << info.timeLimitMinutes << '+'
    << std::setw(2) << std::setfill('0') << info.timeLimitReadoff << '\n';

}

/**
 * 局面をストリームに書き出します。
 */
void CsaWriter::writeBoard(std::ostream& os, const Board& board) {

  os << board.toStringCsa();

}

/**
 * 手順をストリームに書き出します。
 */
void CsaWriter::writeMoves(std::ostream& os, const Record& record) {

  // 手順
  for (size_t i = 0; i < record.getTotalCount(); i++) {
    bool black = record.isBlackAt(i);
    os << record.getMoveAt(i).toStringCsa(black) << '\n';
  }

}

} // namespace sunfish
