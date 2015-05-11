/* StringUtil.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_STRINGUTIL__
#define __SUNFISH_STRINGUTIL__

#include <sstream>
#include <iomanip>
#include <cstdint>

namespace sunfish {

class StringUtil {
private:

  StringUtil() {}

public:

  static std::string stringify(unsigned u32) {
    std::ostringstream oss;
    oss << std::setw(8) << std::setfill('0') << std::hex << u32;
    return oss.str();
  }

  static std::string stringify(uint64_t u64) {
    return stringify((unsigned)(u64>>32)) + stringify((unsigned)u64);
  }

  static std::string chomp(const std::string& line) {
    for (int index = (int)line.length()-1; index >= 0; index--) {
      if (line.at(index) != '\n') {
        return line.substr(0, index + 1);
      }
    }
    return line;
  }

};

} // namespace sunfish

#endif //__SUNFISH_STRINGUTIL__
