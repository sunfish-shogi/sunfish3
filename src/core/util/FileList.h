/* fileList.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_FILELIST__
#define SUNFISH_FILELIST__

#include <list>
#include <string>
#include "../def.h"

namespace sunfish {

class FileList{
public:
  using iterator = std::list<std::string>::iterator;

private:
  std::list<std::string> flist_;

public:
  FileList() = default;
  FileList(const FileList&) = default;
  //FileList(FileList&&) = default; // this code is error on msvc
  FileList(FileList&& src) NOEXCEPT : flist_(std::move(src.flist_)) {
  }
  ~FileList() = default;

  size_t enumerate(const char* directory, const char* extension);

  void clear(){ flist_.clear(); }
  size_t size() { return flist_.size(); }

  iterator begin() { return flist_.begin(); }
  iterator end() { return flist_.end(); }

};

} // namespace sunfish

#endif // SUNFISH_FILELIST__
