/* CsaReader.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_CSAREADER__
#define __SUNFISH_CSAREADER__

#include "Record.h"
#include <iostream>

namespace sunfish {

	class CsaReader {
	private:

		CsaReader();

		static bool _readBoard(std::istream& is, Board& board);
		static bool _readBoard(const char* line, Board& board);
		static bool _readBoardPieces(const char* line, Board& board);
		static bool _readHand(const char* line, Board& board, bool black);

	public:

		static bool read(const char* filename, Record& record);
		static bool read(const std::string& filename, Record& record) {
			return read(filename.c_str(), record);
		}
		static bool read(std::istream& is, Record& record);
		static bool readBoard(std::istream& is, Board& board);
		static bool readBoard(const char* line, Board& board);
		static bool readMoves(std::istream& is, Record& record);
		static bool readMove(const char* line, const Board& board, Move& move);

	};

}

#endif // __SUNFISH_CSAREADER__
