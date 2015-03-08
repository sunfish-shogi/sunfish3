/* SeeTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../shek/ShekTable.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(ShekTest, test) {
	{
		std::string src =
"P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
"P2 * -HI *  *  *  * -KI-KA * \n"
"P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
"P8 * +KA+KI *  *  *  * +HI * \n"
"P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"+\n";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		ShekTable table;

		Move moves[] = {
			{ Piece::Pawn, P24 },
			{ Piece::Pawn, P23, P24, false },
			{ Piece::Rook, P28, P24, false },
			{ Piece::Pawn, P23 },
		};

		for (auto& move : moves) {
			table.set(board);
			bool ok = board.makeMove(move);
			ASSERT(ok);
			ShekStat stat = table.check(board);
			ASSERT_EQ((int)ShekStat::None, (int)stat);
		}

		table.set(board);
		Move move = { Piece::Rook, P24, P28, false };
		bool ok = board.makeMove(move);
		ASSERT(ok);
		ShekStat stat = table.check(board);
		ASSERT_EQ((int)ShekStat::Superior, (int)stat);

		Move moves2[] = {
			{ Piece::Pawn, P86 },
			{ Piece::Pawn, P87, P86, false },
			{ Piece::Rook, P82, P86, false },
			{ Piece::Pawn, P87 },
		};

		for (auto& move : moves2) {
			table.set(board);
			bool ok = board.makeMove(move);
			ASSERT(ok);
			ShekStat stat = table.check(board);
			ASSERT_EQ((int)ShekStat::None, (int)stat);
		}

		table.set(board);
		move = { Piece::Rook, P86, P82, false };
		ok = board.makeMove(move);
		ASSERT(ok);
		stat = table.check(board);
		ASSERT_EQ((int)ShekStat::Equal, (int)stat);
	}

	{
		std::string src =
"P1-KY-KE-GI-KI-OU *  * -KE-KY\n"
"P2 * -HI *  *  *  * -KI *  * \n"
"P3-FU * -FU-FU-FU-FU-GI * -FU\n"
"P4 *  *  *  *  * -KA-FU *  * \n"
"P5 *  *  *  *  * +FU *  *  * \n"
"P6 *  * +FU+FU *  *  * -FU * \n"
"P7+FU+FU * +GI+FU * +FU * +FU\n"
"P8 * +KA+KI *  * +GI * +HI * \n"
"P9+KY+KE *  * +OU+KI * +KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"-\n";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		Move moves[] = {
			{ Piece::Pawn, P26, P27, true },
			{ Piece::Rook, P28, P27, false },
			{ Piece::Pawn, P26 },
		};

		ShekTable table;
		for (auto& move : moves) {
			table.set(board);
			bool ok = board.makeMove(move);
			ASSERT(ok);
			ShekStat stat = table.check(board);
			ASSERT_EQ((int)ShekStat::None, (int)stat);
		}

		table.set(board);
		Move move = { Piece::Rook, P27, P28, false };
		bool ok = board.makeMove(move);
		ASSERT(ok);
		ShekStat stat = table.check(board);
		ASSERT_EQ((int)ShekStat::Inferior, (int)stat);

		table.set(board);
		move = { Piece::Pawn, P26, P27, true };
		ok = board.makeMove(move);
		ASSERT(ok);
		stat = table.check(board);
		ASSERT_EQ((int)ShekStat::Superior, (int)stat);
	}
}

#endif // !defined(NDEBUG)
