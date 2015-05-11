/* CsaWriter.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_CSAWRITER__
#define __SUNFISH_CSAWRITER__

#include "Record.h"
#include <iostream>

namespace sunfish {

class CsaWriter {
private:

  CsaWriter();

  static void writeMoves(std::ostream& os, const Record& record);

public:

  static bool write(const char* filename, const Record& record, const RecordInfo* info = nullptr);
  static bool write(const std::string& filename, const Record& record, const RecordInfo* info = nullptr) {
    return write(filename.c_str(), record, info);
  }
  static void write(std::ostream& os, const Record& record, const RecordInfo* info = nullptr);
  static void writeInfo(std::ostream& os, const RecordInfo& info);
  static void writeBoard(std::ostream& os, const Board& board);

};

} // namespace sunfish

#endif // __SUNFISH_CSAWRITER__
