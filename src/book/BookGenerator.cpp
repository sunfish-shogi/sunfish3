/* BookGenerator.cpp
 *
 * Kubo Ryosuke
 */

#include "BookGenerator.h"
#include "core/record/CsaReader.h"
#include "core/util/FileList.h"
#include "logger/Logger.h"

#define BOOK_MAX_LENGTH 30
#define BOOK_MIN_COUNT  5

namespace sunfish {

bool BookGenerator::generateByFile(const char* path, Book& book, bool/* clear = true*/, bool filtering/* = true*/) {
  Loggers::warning << "Read csa file. [" << path << "]";

  // read
  Record record;
  if (!CsaReader::read(path, record)) {
    Loggers::warning << "Could not read csa file. [" << path << "]";
    return false;
  }

  // check last turn
  bool black = !record.getBoard().isBlack();

  // iterate for moves
  while (record.unmakeMove())
    ;

  for (int i = 0; i < BOOK_MAX_LENGTH; i++) {
    Move move = record.getNextMove();
    if (move.isEmpty()) {
      break;
    }
    if (record.getBoard().isBlack() == black &&
        !book.add(record.getBoard().getHash(), move)) {
      Loggers::warning << "Could not insert a move to book.";
    }
    if (!record.makeMove()) {
      break;
    }
  }

  if (filtering) {
    filter(book);
  }

  return true;
}

size_t BookGenerator::generate(const char* directory, Book& book, bool clear/* = true*/, bool filtering/* = true*/) {
  size_t files = 0;

  // enumerate csa files
  FileList fileList;
  fileList.enumerate(directory, "csa");

  if (clear) {
    book.clear();
  }

  for (const auto& filename : fileList) {
    bool ok = generateByFile(filename, book, false, false);
    if (ok) {
      files++;
    }
  }

  if (filtering) {
    filter(book);
  }

  return files;
}

void BookGenerator::filter(Book& book) {
  book.filter([](const BookMove& bookMove) {
    if (bookMove.count < BOOK_MIN_COUNT) {
      return false;
    }
    return true;
  });
}

} // namespace sunfish
