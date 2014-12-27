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

		static bool write(const char* filename, const Record& record);
		static bool write(const std::string& filename, const Record& record) {
			return write(filename.c_str(), record);
		}
		static void write(std::ostream& os, const Record& record);
		static void writeBoard(std::ostream& os, const Board& board);

	};

}

#endif // __SUNFISH_CSAWRITER__
