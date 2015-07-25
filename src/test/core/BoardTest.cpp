/* BoardTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(BoardTest, makemoveTest) {
  {
    // 合法手の場合
    Board board;
    board.init(Board::Handicap::Even);

    board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false)); // 76歩

    const char* correct = "\
KyKeGiKiOuKiGiKeKy\n\
  Hi          Ka  \n\
FuFuFuFuFuFuFuFuFu\n\
                  \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
  ka          hi  \n\
kykegikioukigikeky\n\
black: \n\
white: \n\
next: white\n\
";
    ASSERT_EQ(correct, board.toString(false));
  }

  {
    // 王手放置の場合
    std::string src = "\
P1-KY-KE-GI-KI * -OU-GI-KE-KY\n\
P2 * -HI *  *  *  * -KI *  * \n\
P3-FU * -FU-FU-FU-FU * -FU-FU\n\
P4 *  *  *  *  *  * -FU *  * \n\
P5 * -FU *  *  *  *  *  *  * \n\
P6 *  * +FU *  *  *  * +FU * \n\
P7+FU+FU-UM+FU+FU+FU+FU * +FU\n\
P8 * +GI+KI *  *  *  * +HI * \n\
P9+KY+KE * +OU * +KI+GI+KE+KY\n\
P+\n\
P-00KA\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 68玉
    Move move6968(Piece::King, P69, P68, false);
    ASSERT_EQ(false, board.makeMove(move6968));

    // 59玉
    Move move6959(Piece::King, P69, P59, false);
    ASSERT_EQ(false, board.makeMove(move6959));

    // 58玉
    Move move6958(Piece::King, P69, P58, false);
    ASSERT_EQ(true, board.makeMove(move6958));
    board.unmakeMove(move6958);
  }

  {
    // 香車によるPIN
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  * -KY *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  * +KI *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  * +OU *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 56金
    Move move6656(Piece::Gold, P66, P56, false);
    ASSERT_EQ(false, board.makeMove(move6656));

    // 75金
    Move move6675(Piece::Gold, P66, P75, false);
    ASSERT_EQ(false, board.makeMove(move6675));

    // 67金
    Move move6667(Piece::Gold, P66, P67, false);
    ASSERT_EQ(true, board.makeMove(move6667));
    board.unmakeMove(move6667);

    // 65金
    Move move6665(Piece::Gold, P66, P65, false);
    ASSERT_EQ(true, board.makeMove(move6665));
    board.unmakeMove(move6665);
  }

  {
    // 角によるPIN
    std::string src = "\
P1-KY-KE-GI-KI-OU * -GI-KE-KY\n\
P2 * -HI *  *  *  * -KI *  * \n\
P3-FU * -FU-FU * -FU * -FU-FU\n\
P4 *  *  *  * -FU * -FU *  * \n\
P5 *  *  *  *  *  *  * +FU * \n\
P6 * -KA+FU+FU+FU *  *  *  * \n\
P7+FU *  *  *  * +FU+FU * +FU\n\
P8 * +KA+KI+GI *  *  * +HI * \n\
P9+KY+KE *  * +OU+KI+GI+KE+KY\n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 57銀
    Move move6857(Piece::Silver, P68, P57, false);
    ASSERT_EQ(false, board.makeMove(move6857));

    // 79銀
    Move move6879(Piece::Silver, P68, P79, false);
    ASSERT_EQ(false, board.makeMove(move6879));

    // 67銀
    Move move6867(Piece::Silver, P68, P67, false);
    ASSERT_EQ(false, board.makeMove(move6867));

    // 77銀
    Move move6877(Piece::Silver, P68, P77, false);
    ASSERT_EQ(true, board.makeMove(move6877));
    board.unmakeMove(move6877);
  }

  {
    // 飛車によるPIN
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  * -HI *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  * +UM *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  * +OU+KI *  * -RY * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 48馬
    Move move6648(Piece::Horse, P66, P48, false);
    ASSERT_EQ(false, board.makeMove(move6648));

    // 67馬
    Move move6667(Piece::Horse, P66, P67, false);
    ASSERT_EQ(true, board.makeMove(move6667));
    board.unmakeMove(move6667);

    // 47金
    Move move5847(Piece::Gold, P58, P47, false);
    ASSERT_EQ(false, board.makeMove(move5847));

    // 48金
    Move move5848(Piece::Gold, P58, P48, false);
    ASSERT_EQ(true, board.makeMove(move5848));
    board.unmakeMove(move5848);
  }
}

TEST(BoardTest, unmakemoveTest) {
  Board board;
  board.init(Board::Handicap::Even);

  Move moves[] = {
    Move(Piece::Pawn, P77, P76, false),
    Move(Piece::Pawn, P33, P34, false),
    Move(Piece::Bishop, P88, P22, true),
    Move(Piece::Silver, P31, P22, false),
    Move(Piece::Bishop, P45),
  };

  // make move

  // 76歩 34歩 22角成 同銀 45角
  for (unsigned i = 0; i < sizeof(moves) / sizeof(moves[0]); i++) {
    board.makeMove(moves[i]);
  }

  // unmake move

  {
    // 45角
    board.unmakeMove(moves[4]);
    const char* correct = "\
KyKeGiKiOuKi  KeKy\n\
  Hi          Gi  \n\
FuFuFuFuFuFu  FuFu\n\
            Fu    \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
              hi  \n\
kykegikioukigikeky\n\
black: ka\n\
white: ka\n\
next: black\n\
";
    ASSERT_EQ(correct, board.toString(false));
  }

  {
    // 22同銀
    board.unmakeMove(moves[3]);
    const char* correct = "\
KyKeGiKiOuKiGiKeKy\n\
  Hi          um  \n\
FuFuFuFuFuFu  FuFu\n\
            Fu    \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
              hi  \n\
kykegikioukigikeky\n\
black: ka\n\
white: \n\
next: white\n\
";
    ASSERT_EQ(correct, board.toString(false));
  }

  {
    // 22角成
    board.unmakeMove(moves[2]);
    const char* correct = "\
KyKeGiKiOuKiGiKeKy\n\
  Hi          Ka  \n\
FuFuFuFuFuFu  FuFu\n\
            Fu    \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
  ka          hi  \n\
kykegikioukigikeky\n\
black: \n\
white: \n\
next: black\n\
";
    ASSERT_EQ(correct, board.toString(false));
  }
}

TEST(BoardTest, promoteTest) {
  Board board;
  board.init(Board::Handicap::Even);

  board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false)); // 76歩
  board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false)); // 34歩
  board.makeMoveIrr(Move(Piece::Bishop, P88, P33, true)); // 33角成

  const char* correct = "\
KyKeGiKiOuKiGiKeKy\n\
  Hi          Ka  \n\
FuFuFuFuFuFuumFuFu\n\
            Fu    \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
              hi  \n\
kykegikioukigikeky\n\
black: \n\
white: \n\
next: white\n\
";
  ASSERT_EQ(correct, board.toString(false));
}

TEST(BoardTest, captureTest) {
  Board board;
  board.init(Board::Handicap::Even);

  board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false)); // 76歩
  board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false)); // 34歩
  board.makeMoveIrr(Move(Piece::Bishop, P88, P22, true)); // 22角成

  const char* correct = "\
KyKeGiKiOuKiGiKeKy\n\
  Hi          um  \n\
FuFuFuFuFuFu  FuFu\n\
            Fu    \n\
                  \n\
    fu            \n\
fufu  fufufufufufu\n\
              hi  \n\
kykegikioukigikeky\n\
black: ka\n\
white: \n\
next: white\n\
";
  ASSERT_EQ(correct, board.toString(false));
}

TEST(BoardTest, dropTest) {
  Board board;
  board.init(Board::Handicap::Even);

  board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false)); // 76歩
  board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false)); // 34歩
  board.makeMoveIrr(Move(Piece::Bishop, P88, P22, true)); // 22角成
  board.makeMoveIrr(Move(Piece::Silver, P31, P22, false)); // 同銀
  board.makeMoveIrr(Move(Piece::Bishop, P45)); // 45角

  const char* correct = "\
KyKeGiKiOuKi  KeKy\n\
  Hi          Gi  \n\
FuFuFuFuFuFu  FuFu\n\
            Fu    \n\
          ka      \n\
    fu            \n\
fufu  fufufufufufu\n\
              hi  \n\
kykegikioukigikeky\n\
black: \n\
white: ka\n\
next: white\n\
";
  ASSERT_EQ(correct, board.toString(false));
}

TEST(BoardTest, isCheckingTest) {
  {
    // 王手がかかっていない場合
    std::string src = "\
P1-KY-KE-GI-KI-OU * -GI-KE-KY\n\
P2 * -HI *  *  *  * -KI-KA * \n\
P3-FU * -FU-FU-FU-FU * -FU-FU\n\
P4 *  *  *  *  *  * -FU *  * \n\
P5 * -FU *  *  *  *  *  *  * \n\
P6 *  * +FU *  *  *  * +FU * \n\
P7+FU+FU+KA+FU+FU+FU+FU * +FU\n\
P8 *  * +KI *  *  *  * +HI * \n\
P9+KY+KE+GI * +OU+KI+GI+KE+KY\n\
P+\n\
P-\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(false, board.isChecking());
  }

  {
    // 王手がかかっていない場合
    std::string src = "\
P1-KY-KE-GI-KI-OU * -GI-KE-KY\n\
P2 * -HI *  *  *  * -KI-KA * \n\
P3-FU * -FU-FU-FU-FU * -FU-FU\n\
P4 *  *  *  *  *  * -FU *  * \n\
P5 * -FU *  *  *  *  *  *  * \n\
P6 *  * +FU *  *  *  * +FU * \n\
P7+FU+FU+KA+FU+FU+FU+FU * +FU\n\
P8 * +GI+KI *  *  *  * +HI * \n\
P9+KY+KE *  * +OU+KI+GI+KE+KY\n\
P+\n\
P-\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(false, board.isChecking());
  }

  {
    // 王手がかかっている場合
    std::string src = "\
P1-KY-KE-GI-KI-OU * -GI-KE-KY\n\
P2 * -HI *  *  *  * -KI *  * \n\
P3-FU * -FU-FU-FU-FU * -FU-FU\n\
P4 *  *  *  *  *  * -FU *  * \n\
P5 * -FU *  *  *  *  *  *  * \n\
P6 *  * +FU *  *  *  * +FU * \n\
P7+FU+FU-UM+FU+FU+FU+FU * +FU\n\
P8 * +GI+KI *  *  *  * +HI * \n\
P9+KY+KE *  * +OU+KI+GI+KE+KY\n\
P+\n\
P-00KA\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }

  {
    // 飛車による王手
    std::string src = "\
P1 *  *  * -HI *  *  *  *  * \n\
P2 *  *  *  *  * -OU *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }

  {
    // 角による王手
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  * -OU *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  * -KA *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }

  {
    // 香車による王手
    std::string src = "\
P1 *  *  * -KY *  *  *  *  * \n\
P2 *  *  *  *  * -OU *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }

  {
    // 跳び駒以外の王手
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  * -OU *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * +KE *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }

  {
    // と金の利き
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  * -TO *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isChecking());
  }
}

TEST(BoardTest, isValidMoveTest) {
  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  * -FU *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P76, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P66, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P68, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P56, false)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  * -KY *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P76, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P66, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P68, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P56, false)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  * -KA *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P66, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P56, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P78, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P68, false)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +OU * -RY *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P77, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P56, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P57, false)));
    ASSERT_EQ(false, board.isValidMove(Move(Piece::King, P67, P58, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P78, false)));
    ASSERT_EQ(true, board.isValidMove(Move(Piece::King, P67, P66, false)));
  }

  {
    std::string src = "\
P1 *  *  *  * +KA *  *  *  * \n\
P2 *  *  *  * -FU *  *  *  * \n\
P3 *  *  * -OU *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  * +RY * +KY *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(false, board.isValidMove(Move(Piece::Pawn, P64)));
  }

  {
    std::string src = "\
P1 *  *  *  * +KA *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * -OU-FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  * +RY * +KY *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::Pawn, P64)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  * -KY *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 * -UM *  *  *  *  *  *  * \n\
P8 *  *  * +GI *  *  *  *  * \n\
P9 * +FU+OU *  *  *  *  *  * \n\
P+\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::Pawn, P78)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  * -KY *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 * -UM *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 * +FU+OU+GI *  *  *  *  * \n\
P+\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMove(Move(Piece::Pawn, P78)));
  }
}

TEST(BoardTest, isValidMoveStrictTest) {
  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +GI *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Silver, P63, P54, false)));
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Silver, P63, P54, true)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Silver, P63, P64, true)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Silver, P63, P61, true)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * -FU * +HI *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  * +FU *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Rook, P33, P53, true)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Rook, P33, P63, true)));
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Rook, P33, P36, true)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Rook, P33, P37, true)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Rook, P23, P13, true)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * -FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  * +KY *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Lance, P56, P53, true)));
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Lance, P56, P54, false)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Lance, P56, P54, true)));
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 * -OU *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  * -RY * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  * +OU\n\
P+\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::Pawn, P18)));
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 * -OU *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  * -RY *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  * +OU+KY\n\
P+\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::Pawn, P18)));
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * -OU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  * +OU *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isValidMoveStrict(Move(Piece::King, P46, P35, false)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::King, P46, P45, false)));
    ASSERT_EQ(false, board.isValidMoveStrict(Move(Piece::King, P54, P65, false)));
  }
}

TEST(BoardTest, isCheckTest) {
  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  * -OU *  *  *  * \n\
P3 *  *  * -FU *  *  *  *  * \n\
P4 *  *  *  *  * +FU *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isCheck(Move(Piece::Pawn, P44, P43, true)));
    ASSERT_EQ(false, board.isCheck(Move(Piece::Pawn, P44, P43, false)));
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  * -KY *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +FU *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  * +OU *  *  *  *  * \n\
P+\n\
P-\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isCheck(Move(Piece::Lance, P62, P67, false)));
    ASSERT_EQ(false, board.isCheck(Move(Piece::Lance, P62, P67, true)));
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  * -OU *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  * +GI *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  * +UM *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    ASSERT_EQ(true, board.isCheck(Move(Piece::Silver, P55, P44, false)));
    ASSERT_EQ(true, board.isCheck(Move(Piece::Silver, P55, P54, false)));
    ASSERT_EQ(false, board.isCheck(Move(Piece::Silver, P55, P66, false)));
  }
}

TEST(BoardTest, compactBoardTest) {
  {
    std::string src = "\
P1 *  * +TO *  *  *  *  * -KY\n\
P2+UM *  *  *  *  * -KI-OU * \n\
P3+TO *  *  *  * -GI-KE * -GI\n\
P4-FU *  *  *  * -FU-HI * -FU\n\
P5 *  *  *  *  *  * -FU+FU * \n\
P6 *  * -FU-UM * +FU * +HI+FU\n\
P7+FU *  *  *  *  * +FU * +KE\n\
P8+KI *  * +FU-TO+GI+KI *  * \n\
P9+KY *  *  *  *  * +OU * +KY\n\
P+00KY00FU\n\
P-00KI00GI00KE00KE00FU00FU00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    CompactBoard cb = board.getCompactBoard();

    Board board2(cb);

    ASSERT_EQ(board.toStringCsa(), board2.toStringCsa());
    ASSERT_EQ(true, board2.validate());
    ASSERT_EQ(P39, board2.getBKingSquare());
    ASSERT_EQ(P22, board2.getWKingSquare());
  }
}

#endif // !defined(NDEBUG)
