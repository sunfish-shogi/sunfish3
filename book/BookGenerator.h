/* BookGenerator.cpp
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH3__BOOKGENERATOR__
#define __SUNFISH3__BOOKGENERATOR__

#include "Book.h"
#include <cstdint>

namespace sunfish {

class BookGenerator {
private:
  BookGenerator();

public:

  static bool generateByFile(const char* path, Book& book, bool clear = true, bool filtering = true);
  static bool generateByFile(const std::string& path, Book& book, bool clear = true, bool filtering = true) {
    return generateByFile(path.c_str(), book, clear, filtering);
  }
  static size_t generate(const char* directory, Book& book, bool clear = true, bool filtering = true);
  static size_t generate(const std::string& directory, Book& book, bool clear = true, bool filtering = true) {
    return generate(directory.c_str(), book, clear, filtering);
  }

  static void filter(Book& book);

};

} // namespace sunfish

#endif /* defined(__SUNFISH3__BOOKGENERATOR__) */
