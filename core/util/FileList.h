/* fileList.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_FILELIST__
#define __SUNFISH_FILELIST__

#include <list>
#include <string>

namespace sunfish {

	class FileList{
	public:
		using iterator = std::list<std::string>::iterator;

	private:
		std::list<std::string> _flist;

	public:
		FileList() = default;
		FileList(const FileList&) = default;
		FileList(FileList&&) = default;
		~FileList() = default;

		size_t enumerate(const char* directory, const char* extension);

		void clear(){ _flist.clear(); }
		size_t size() { return _flist.size(); }

		iterator begin() { return _flist.begin(); }
		iterator end() { return _flist.end(); }

	};
}

#endif // __SUNFISH_FILELIST__
