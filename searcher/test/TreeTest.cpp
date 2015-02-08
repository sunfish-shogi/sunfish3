/* TreeTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../tree/Tree.h"

using namespace sunfish;

TEST(TreeTest, testSortByValue) {

	Tree tree;
	int32_t values[5];

	tree.initGenPhase();

	auto& moves = tree.getMoves();
	moves.add(Move(Piece::Rook, P28, P58)); values[0] = 7;
	moves.add(Move(Piece::Silver, P39, P48)); values[1] = -48;
	moves.add(Move(Piece::Pawn, P77, P76)); values[2] = 50;
	moves.add(Move(Piece::Rook, P28, P68)); values[3] = -21;
	moves.add(Move(Piece::Pawn, P27, P26)); values[4] = 39;
	tree.setSortValues(values);

	tree.sortAfterCurrent();

	ASSERT_EQ(Move(Piece::Pawn, P77, P76), tree.getMoves()[0]);
	ASSERT_EQ(Move(Piece::Pawn, P27, P26), tree.getMoves()[1]);
	ASSERT_EQ(Move(Piece::Rook, P28, P58), tree.getMoves()[2]);
	ASSERT_EQ(Move(Piece::Rook, P28, P68), tree.getMoves()[3]);
	ASSERT_EQ(Move(Piece::Silver, P39, P48), tree.getMoves()[4]);

	ASSERT_EQ(50, tree.getSortValue(tree.getBegin()+0));
	ASSERT_EQ(39, tree.getSortValue(tree.getBegin()+1));
	ASSERT_EQ(7, tree.getSortValue(tree.getBegin()+2));
	ASSERT_EQ(-21, tree.getSortValue(tree.getBegin()+3));
	ASSERT_EQ(-48, tree.getSortValue(tree.getBegin()+4));

}

#endif // !defined(NDEBUG)
