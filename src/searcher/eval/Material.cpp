/* Material.cpp
 *
 * Kubo Ryosuke
 */

#include "Material.h"
#include <fstream>

namespace sunfish {

namespace material {

#if !defined(NLEARN)
int16_t Pawn       = 100;
int16_t Lance      = 200;
int16_t Knight     = 300;
int16_t Silver     = 400;
int16_t Gold       = 400;
int16_t Bishop     = 600;
int16_t Rook       = 700;
int16_t Tokin      = 500;
int16_t Pro_lance  = 500;
int16_t Pro_knight = 500;
int16_t Pro_silver = 500;
int16_t Horse      = 800;
int16_t Dragon     = 900;

int16_t PawnEx       = Pawn * 2;
int16_t LanceEx      = Lance * 2;
int16_t KnightEx     = Knight * 2;
int16_t SilverEx     = Silver * 2;
int16_t GoldEx       = Gold * 2;
int16_t BishopEx     = Bishop * 2;
int16_t RookEx       = Rook * 2;
int16_t TokinEx      = Tokin + Pawn;
int16_t Pro_lanceEx  = Pro_lance + Lance;
int16_t Pro_knightEx = Pro_knight + Knight;
int16_t Pro_silverEx = Pro_silver + Silver;
int16_t HorseEx      = Horse + Bishop;
int16_t DragonEx     = Dragon + Rook;

/**
 * 駒割を取得します。
 */
Value piece(const Piece& piece) {
  switch (piece.index()) {
    case Piece::BPawn     : case Piece::WPawn     : return material::Pawn;
    case Piece::BLance    : case Piece::WLance    : return material::Lance;
    case Piece::BKnight   : case Piece::WKnight   : return material::Knight;
    case Piece::BSilver   : case Piece::WSilver   : return material::Silver;
    case Piece::BGold     : case Piece::WGold     : return material::Gold;
    case Piece::BBishop   : case Piece::WBishop   : return material::Bishop;
    case Piece::BRook     : case Piece::WRook     : return material::Rook;
    case Piece::BKing     : case Piece::WKing     : return Value::PieceInf;
    case Piece::BTokin    : case Piece::WTokin    : return material::Tokin;
    case Piece::BProLance : case Piece::WProLance : return material::Pro_lance;
    case Piece::BProKnight: case Piece::WProKnight: return material::Pro_knight;
    case Piece::BProSilver: case Piece::WProSilver: return material::Pro_silver;
    case Piece::BHorse    : case Piece::WHorse    : return material::Horse;
    case Piece::BDragon   : case Piece::WDragon   : return material::Dragon;
  }
  return 0;
}

/**
 * 駒を取った時の変化値を取得します。
 */
Value pieceExchange(const Piece& piece) {
  switch (piece.index()) {
    case Piece::BPawn     : case Piece::WPawn     : return material::PawnEx;
    case Piece::BLance    : case Piece::WLance    : return material::LanceEx;
    case Piece::BKnight   : case Piece::WKnight   : return material::KnightEx;
    case Piece::BSilver   : case Piece::WSilver   : return material::SilverEx;
    case Piece::BGold     : case Piece::WGold     : return material::GoldEx;
    case Piece::BBishop   : case Piece::WBishop   : return material::BishopEx;
    case Piece::BRook     : case Piece::WRook     : return material::RookEx;
    case Piece::BKing     : case Piece::WKing     : return Value::PieceInfEx;
    case Piece::BTokin    : case Piece::WTokin    : return material::TokinEx;
    case Piece::BProLance : case Piece::WProLance : return material::Pro_lanceEx;
    case Piece::BProKnight: case Piece::WProKnight: return material::Pro_knightEx;
    case Piece::BProSilver: case Piece::WProSilver: return material::Pro_silverEx;
    case Piece::BHorse    : case Piece::WHorse    : return material::HorseEx;
    case Piece::BDragon   : case Piece::WDragon   : return material::DragonEx;
  }
  return 0;
}

/**
 * 駒が成った時の変化値を取得します。
 */
Value piecePromote(const Piece& piece) {
  switch (piece.index()) {
    case Piece::BPawn     : case Piece::WPawn     : return material::Tokin - material::Pawn;
    case Piece::BLance    : case Piece::WLance    : return material::Pro_lance - material::Lance;
    case Piece::BKnight   : case Piece::WKnight   : return material::Pro_knight - material::Knight;
    case Piece::BSilver   : case Piece::WSilver   : return material::Pro_silver - material::Silver;
    case Piece::BGold     : case Piece::WGold     : return 0;
    case Piece::BBishop   : case Piece::WBishop   : return material::Horse - material::Bishop;
    case Piece::BRook     : case Piece::WRook     : return material::Dragon - material::Rook;
    case Piece::BKing     : case Piece::WKing     : return 0;
    case Piece::BTokin    : case Piece::WTokin    : return 0;
    case Piece::BProLance : case Piece::WProLance : return 0;
    case Piece::BProKnight: case Piece::WProKnight: return 0;
    case Piece::BProSilver: case Piece::WProSilver: return 0;
    case Piece::BHorse    : case Piece::WHorse    : return 0;
    case Piece::BDragon   : case Piece::WDragon   : return 0;
  }
  return 0;
}

void updateEx() {
  PawnEx       = Pawn * 2;
  LanceEx      = Lance * 2;
  KnightEx     = Knight * 2;
  SilverEx     = Silver * 2;
  GoldEx       = Gold * 2;
  BishopEx     = Bishop * 2;
  RookEx       = Rook * 2;
  TokinEx      = Tokin + Pawn;
  Pro_lanceEx  = Pro_lance + Lance;
  Pro_knightEx = Pro_knight + Knight;
  Pro_silverEx = Pro_silver + Silver;
  HorseEx      = Horse + Bishop;
  DragonEx     = Dragon + Rook;
}

void writeFile() {
  std::ofstream file("material_values.h");

  if (file) {
    file << "/* material_values.h        \n";
    file << " *                          \n";
    file << " * generated by Material.cpp\n";
    file << " */                         \n";
    file << "\n";
    file << "#define MATERIAL_VALUE_PAWN       " << Pawn << "\n";
    file << "#define MATERIAL_VALUE_LANCE      " << Lance << "\n";
    file << "#define MATERIAL_VALUE_KNIGHT     " << Knight << "\n";
    file << "#define MATERIAL_VALUE_SILVER     " << Silver << "\n";
    file << "#define MATERIAL_VALUE_GOLD       " << Gold << "\n";
    file << "#define MATERIAL_VALUE_BISHOP     " << Bishop << "\n";
    file << "#define MATERIAL_VALUE_ROOK       " << Rook << "\n";
    file << "#define MATERIAL_VALUE_TOKIN      " << Tokin << "\n";
    file << "#define MATERIAL_VALUE_PRO_LANCE  " << Pro_lance << "\n";
    file << "#define MATERIAL_VALUE_PRO_KNIGHT " << Pro_knight << "\n";
    file << "#define MATERIAL_VALUE_PRO_SILVER " << Pro_silver << "\n";
    file << "#define MATERIAL_VALUE_HORSE      " << Horse << "\n";
    file << "#define MATERIAL_VALUE_DRAGON     " << Dragon << "\n";
    file.close();
  }
}
#endif // !defined(NLEARN)

} // namespace material

} // namespace sunfish
