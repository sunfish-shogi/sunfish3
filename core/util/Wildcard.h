/* Wildcard.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_WILDCARD__
#define __SUNFISH_WILDCARD__

#include <string>

namespace sunfish {

class Wildcard {
private:
  std::string _pattern;

  static std::string normalize(const char* pattern);
  static std::string normalize(const std::string& pattern) {
    return normalize(pattern.c_str());
  }

  static bool match(const char* pp, const char* pt);

public:
  Wildcard(const char* pattern) : _pattern(normalize(pattern)) {
  }
  Wildcard(const std::string& pattern) : _pattern(normalize(pattern)) {
  }

  bool match(const char* str) const;
  bool match(const std::string& str) const {
    return match(str.c_str());
  }

};

} // namespace sunfish

#endif // __SUNFISH_WILDCARD__
