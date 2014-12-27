/* CsaWriter.cpp
 *
 * Kubo Ryosuke
 */

#include "CsaWriter.h"
#include <fstream>

namespace sunfish {

	/**
	 * 棋譜をファイルに書き出します。
	 */
	bool CsaWriter::write(const char* filename, const Record& record) {
		std::ofstream file(filename);
		
		if (!file) {
			return false;
		}

		write(file, record);

		file.close();

		return true;
	}

	/**
	 * 棋譜をストリームに書き出します。
	 */
	void CsaWriter::write(std::ostream& os, const Record& record) {

		// 開始局面
		writeBoard(os, record.getInitialBoard());

		// 指し手
		writeMoves(os, record);

	}

	/**
	 * 局面をストリームに書き出します。
	 */
	void CsaWriter::writeBoard(std::ostream& os, const Board& board) {

		os << board.toStringCsa();

	}

	/**
	 * 手順をストリームに書き出します。
	 */
	void CsaWriter::writeMoves(std::ostream& os, const Record& record) {

		// 手番
		os << (record.isBlackAt(0) ? '+' : '-') << '\n';

		// 手順
		for (int i = 0; i < record.getSize(); i++) {
			bool black = record.isBlackAt(i);
			os << record.getMoveAt(i).toStringCsa(black) << '\n';
		}

	}

}
