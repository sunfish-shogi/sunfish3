/* CsaReader.cpp
 *
 * Kubo Ryosuke
 */

#include "CsaReader.h"
#include "logger/Logger.h"
#include <fstream>

#define LINE_BUFFER_SIZE							1024

namespace sunfish {

	bool CsaReader::read(const char* filename, Record& record) {

		std::ifstream file(filename);
		if (!file) {
			return false;
		}

		bool ok = read(file, record);

		file.close();
		return ok;

	}

	bool CsaReader::read(std::istream& is, Record& record) {
		Board board;

		// 局面の読み込み
		if (!readBoard(is, board)) {
			return false;
		}

		record.init(board);

		// 指し手の読み込み
		if (!readMoves(is, record)) {
			return false;
		}

		return true;
	}

	/**
	 * 局面の読み込み
	 */
	bool CsaReader::readBoard(std::istream& is, Board& board) {
		bool ok = _readBoard(is, board);
		board.refreshHash();
		return ok;
	}

	/**
	 * 局面の読み込み
	 */
	bool CsaReader::readBoard(const char* line, Board& board) {
		bool ok = _readBoard(line, board);
		board.refreshHash();
		return ok;
	}

	/**
	 * 局面の読み込み
	 */
	bool CsaReader::_readBoard(std::istream& is, Board& board) {
		char line[LINE_BUFFER_SIZE];

		board.init();

		while (true) {
			is.getline(line, sizeof(line));
			if (is.eof()) {
				return true;
			}
			if (is.fail()) {
				Loggers::warning << "file io error. " << __FILE__ << "(" << __LINE__ << ")";
				return false;
			}
			if (!_readBoard(line, board)) {
				Loggers::warning << "invalid board format. " << __FILE__ << "(" << __LINE__ << ")";
				return false;
			}
			if (line[0] == '+' || line[0] == '-') {
				return true;
			}
		}
	}

	/**
	 * 局面の読み込み
	 */
	bool CsaReader::_readBoard(const char* line, Board& board) {
		switch (line[0]) {
		case 'P':
			if (line[1] >= '1' && line[1] <= '9') {
				return _readBoardPieces(line, board);
			} else if (line[1] == '+') {
				return _readHand(line, board, true);
			} else if (line[1] == '-') {
				return _readHand(line, board, false);
			}
			Loggers::warning << __THIS__ << ": unknown command";
			Loggers::warning << line;
			return false;
		case '+':
			board.setBlack();
			return true;
		case '-':
			board.setWhite();
			return true;
		case 'V': case 'N': case '$': case '\'': case '\0':
			return true;
		default:
			Loggers::warning << __THIS__ << ": unknown command";
			Loggers::warning << line;
			return false;
		}
	}

	/**
	 * 盤面の読み込み
	 */
	bool CsaReader::_readBoardPieces(const char* line, Board& board) {
		if (strlen(line) < 2 + 3 * Position::FileN) {
			Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
			return false;
		}
		int rank = line[1] - '0';
		for (int file = 1; file <= Position::FileN; file++) {
			Piece piece = Piece::parseCsa(line + 2 + 3 * (9 - file));
			board.setBoardPiece(Position(file, rank), piece);
		}
		return true;
	}

	/**
	 * 持ち駒の読み込み
	 */
	bool CsaReader::_readHand(const char* line, Board& board, bool black) {
		unsigned length = (unsigned)strlen(line);
		for (unsigned i = 2; i + 4 <= length; i += 4) {
			unsigned file = line[i+0] - '0';
			unsigned rank = line[i+1] - '0';
			Piece piece = Piece::parseCsa(&line[i+2]);
			if (piece != Piece::Empty) {
				if (Position::isValidFile(file) && Position::isValidRank(rank)) {
					board.setBoardPiece(Position(file, rank), black ? piece.black() : piece.white());
				} else if (file == 0 && rank == 0 && piece.isUnpromoted() && piece != Piece::King) {
					if (black) {
						board.incBlackHand(piece);
					} else {
						board.incWhiteHand(piece);
					}
				} else {
					Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
					return false;
				}
			} else {
				Loggers::warning << "invalid format. " << __FILE__ << "(" << __LINE__ << ")";
				return false;
			}
		}
		return true;
	}

	/**
	 * 手順の読み込み
	 */
	bool CsaReader::readMoves(std::istream& is, Record& record) {
		char line[LINE_BUFFER_SIZE];
		Move move;

		while (true) {
			is.getline(line, sizeof(line));
			if (is.eof()) {
				return true;
			}
			if (is.fail()) {
				Loggers::warning << "file io error. " << __FILE__ << "(" << __LINE__ << ")";
				return false;
			}
			if (!readMove(line, record.getBoard(), move)) {
				Loggers::warning << "invalid move format. " << __FILE__ << "(" << __LINE__ << ")";
				Loggers::warning << "> " << line;
				return false;
			}
			record.makeMove(move);
		}
	}

	/**
	 * 指し手の読み込み
	 */
	bool CsaReader::readMove(const char* line, const Board& board, Move& move) {
		if (line[0] == '\0' || line[0] == 'T' || line[0] == '\'') {
			return true;
		}

		if (strlen(line) < 7) {
			return false;
		}

		bool black;
		if (line[0] == '+') {
			black = true;
		} else if (line[0] == '-') {
			black = false;
		} else {
			return false;
		}

		if (black != board.isBlack()) {
			return false;
		}

		Position to = Position::parse(&line[3]);
		Piece piece = Piece::parseCsa(&line[5]);
		if (line[1] == '0' && line[2] == '0') {
			move = Move(piece, to);
		} else {
			Position from = Position::parse(&line[1]);
			Piece pieceOrg = board.getBoardPiece(from).kindOnly();
			bool promote;
			if (piece == pieceOrg) {
				promote = false;
			} else if (piece == pieceOrg.promote()) {
				promote = true;
			} else {
				return false;
			}
			move = Move(pieceOrg, from, to, promote);
		}

		return true;
	}
}
