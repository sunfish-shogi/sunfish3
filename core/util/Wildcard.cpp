/* Wildcard.cpp
 *
 * Kubo Ryosuke
 */

#include "Wildcard.h"
#include <sstream>

namespace sunfish {

std::string Wildcard::normalize(const char* pattern) {
  if (pattern[0] == '\0') {
    return "";
  }

  std::ostringstream oss;

  oss << *pattern;
  for (const char* p = pattern+1; *p != '\0'; p++) {
    if (*p != '*' || *(p-1) != '*') {
      oss << *p;
    }
  }

  return oss.str();
}

bool Wildcard::match(const char* pp, const char* pt) {
  while (true) {
    switch (*pp) {
      case '\0':
        if (*pt == '\0') {
          return true;
        } else {
          return false;
        }

      case '*':
        if ((pp[1] == '?' && pt[0] != '\0' && match(pp+1, pt)) ||
            (pp[1] == pt[0] && match(pp+1, pt)) ||
            (pt[0] != '\0' && pp[1] == '?' && pt[1] != '\0' && match(pp+1, pt+1)) ||
            (pt[0] != '\0' && pp[1] == pt[1] && match(pp+1, pt+1)) ||
            (pt[0] != '\0' && match(pp, pt+1))) {
          return true;
        }
        return false;

      case '?':
        if (*pt == '\0') {
          return false;
        }
        break;

      case '\\':
        pp++;
        if (*pp == '\0') {
          return false; // syntax error
        } else if (*pp != *pt) {
          return false;
        }
        break;

      default:
        if (*pp != *pt) {
          return false;
        }
        break;
    }
    pp++;
    pt++;
  }
}

bool Wildcard::match(const char* str) const {
  return match(pattern_.c_str(), str);
}

} // namespace sunfish
