/* Progression.cpp
 *
 * Kubo Ryosuke
 */

#include "Progression.h"
#include <utility>

namespace sunfish {

int Progression::evaluate(const Board& board) {
	const int Scale = Progression::Scale;
  int invader0 = 0;
  int invader1 = 0;
  int invader2 = 0;
  int hand0 = 0;
  int hand1 = 0;
  int hand2 = 0;
  int value = 0;

  for (int file = 1; file <= 9; file++) {
    for (int rank = 1; rank <= 3; rank++) {
      Position pos(file, rank);
      Piece piece = board.getBoardPiece(pos);
      switch (piece) {
      case Piece::BPawn:
        invader0++;
      case Piece::BLance: case Piece::BKnight: case Piece::BSilver: case Piece::BGold:
      case Piece::BTokin: case Piece::BProLance: case Piece::BProKnight: case Piece::BProSilver:
        invader1++;
      case Piece::BBishop: case Piece::BRook: case Piece::BHorse: case Piece::BDragon:
        invader2++;
        break;
      default:
        break;
      }
    }

    for (int rank = 7; rank <= 9; rank++) {
      Position pos(file, rank);
      Piece piece = board.getBoardPiece(pos);
      switch (piece) {
      case Piece::WPawn:
        invader0++;
      case Piece::WLance: case Piece::WKnight: case Piece::WSilver: case Piece::WGold:
      case Piece::WTokin: case Piece::WProLance: case Piece::WProKnight: case Piece::WProSilver:
        invader1++;
      case Piece::WBishop: case Piece::WRook: case Piece::WHorse: case Piece::WDragon:
        invader2++;
        break;
      default:
        break;
      }
    }
  }

  hand0 += board.getBlackHand(Piece::Pawn);
  hand1 += board.getBlackHand(Piece::Lance);
  hand1 += board.getBlackHand(Piece::Knight);
  hand1 += board.getBlackHand(Piece::Silver);
  hand1 += board.getBlackHand(Piece::Gold);
  hand2 += board.getBlackHand(Piece::Bishop);
  hand2 += board.getBlackHand(Piece::Rook);
  hand0 += board.getWhiteHand(Piece::Pawn);
  hand1 += board.getWhiteHand(Piece::Lance);
  hand1 += board.getWhiteHand(Piece::Knight);
  hand1 += board.getWhiteHand(Piece::Silver);
  hand1 += board.getWhiteHand(Piece::Gold);
  hand2 += board.getWhiteHand(Piece::Bishop);
  hand2 += board.getWhiteHand(Piece::Rook);

  value += 60 * std::min(invader2, 2) / 2;
  value += 40 * std::min(invader1, 3) / 3;
  value += 20 * std::min(invader0, 5) / 5;

  value += 40 * std::min(hand2, 2) / 2;
  value += 30 * std::min(hand1, 3) / 3;
  value += 10 * std::min(hand0, 5) / 5;

  return std::min(value, Scale);
}

} // namespace sunfish
