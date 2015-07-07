/* CodeGenerator.cpp
 * 
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "CodeGenerator.h"
#include "core/util/Random.h"
#include "core/base/Piece.h"
#include "core/base/Position.h"
#include <iostream>

namespace sunfish {

/**
 * 盤面の駒に対する乱数表を作成
 */
static void generateZobristOnBoard(Random& random) {

  std::cout << "const uint64_t Zobrist::Board[Position::N][Piece::Num] = {\n";
  POSITION_EACH(pos) {
    std::cout << "\t{\n";
    PIECE_EACH_UNSAFE(piece) {
      std::cout << "\t\t0x" << std::hex << random.getInt64() << "ll,\n";
    }
    std::cout << "\t},\n";
  }
  std::cout << "};\n";

}

/**
 * 持ち駒に対する乱数表を作成
 */
static void generateZobristOnHand(Random& random, const char* pieceName, int num) {

  std::cout << "const uint64_t Zobrist::Hand" << pieceName << "[" << std::dec << num << "] = {\n";
  for (int i = 0; i < num; i++) {
    std::cout << "\t0x" << std::hex << random.getInt64() << "ll,\n";
  }
  std::cout << "};\n";

}

/**
 * 黒番にの時の乱数を作成
 */
static void generateZobristBlack(Random& random) {

  std::cout << "const uint64_t Zobrist::Black = 0x" << std::hex << random.getInt64() << "ll;\n";

}

void CodeGenerator::generateZobrist() {

  Random random;

  generateZobristOnBoard(random);

  std::cout << '\n';

  generateZobristOnHand(random, "BPawn", 18);
  generateZobristOnHand(random, "BLance", 4);
  generateZobristOnHand(random, "BKnight", 4);
  generateZobristOnHand(random, "BSilver", 4);
  generateZobristOnHand(random, "BGold", 4);
  generateZobristOnHand(random, "BBishop", 2);
  generateZobristOnHand(random, "BRook", 2);

  generateZobristOnHand(random, "WPawn", 18);
  generateZobristOnHand(random, "WLance", 4);
  generateZobristOnHand(random, "WKnight", 4);
  generateZobristOnHand(random, "WSilver", 4);
  generateZobristOnHand(random, "WGold", 4);
  generateZobristOnHand(random, "WBishop", 2);
  generateZobristOnHand(random, "WRook", 2);

  generateZobristBlack(random);

}

} // namespace sunfish

#endif // !defined(NDEBUG)
