/* Wildcard.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_WILDCARD__
#define SUNFISH_WILDCARD__

#include <string>

namespace sunfish {

class Wildcard {
private:
  std::string pattern_;

  static std::string normalize(const char* pattern);
  static std::string normalize(const std::string& pattern) {
    return normalize(pattern.c_str());
  }

  static bool match(const char* pp, const char* pt);

public:
  Wildcard(const char* pattern) : pattern_(normalize(pattern)) {
  }
  Wildcard(const std::string& pattern) : pattern_(normalize(pattern)) {
  }

  bool match(const char* str) const;
  bool match(const std::string& str) const {
    return match(str.c_str());
  }

};

} // namespace sunfish

#endif // SUNFISH_WILDCARD__
