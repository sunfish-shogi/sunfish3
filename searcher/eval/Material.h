/* Material.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MATERIAL__
#define __SUNFISH_MATERIAL__

#include "core/def.h"
#include "core/base/Piece.h"
#include "Value.h"
#include <cstdint>

namespace sunfish {

namespace material {

	static CONSTEXPR int16_t Pawn       = 87;
	static CONSTEXPR int16_t Lance      = 232;
	static CONSTEXPR int16_t Knight     = 257;
	static CONSTEXPR int16_t Silver     = 369;
	static CONSTEXPR int16_t Gold       = 444;
	static CONSTEXPR int16_t Bishop     = 569;
	static CONSTEXPR int16_t Rook       = 642;
	static CONSTEXPR int16_t Tokin      = 534;
	static CONSTEXPR int16_t Pro_lance  = 489;
	static CONSTEXPR int16_t Pro_knight = 510;
	static CONSTEXPR int16_t Pro_silver = 495;
	static CONSTEXPR int16_t Horse      = 827;
	static CONSTEXPR int16_t Dragon     = 945;

	static CONSTEXPR int16_t PawnEx       = Pawn * 2;
	static CONSTEXPR int16_t LanceEx      = Lance * 2;
	static CONSTEXPR int16_t KnightEx     = Knight * 2;
	static CONSTEXPR int16_t SilverEx     = Silver * 2;
	static CONSTEXPR int16_t GoldEx       = Gold * 2;
	static CONSTEXPR int16_t BishopEx     = Bishop * 2;
	static CONSTEXPR int16_t RookEx       = Rook * 2;
	static CONSTEXPR int16_t TokinEx      = Tokin + Pawn;
	static CONSTEXPR int16_t Pro_lanceEx  = Pro_lance + Lance;
	static CONSTEXPR int16_t Pro_knightEx = Pro_knight + Knight;
	static CONSTEXPR int16_t Pro_silverEx = Pro_silver + Silver;
	static CONSTEXPR int16_t HorseEx      = Horse + Bishop;
	static CONSTEXPR int16_t DragonEx     = Dragon + Rook;

	Value piece(const Piece& piece);
	Value pieceExchange(const Piece& piece);
	Value piecePromote(const Piece& piece);

}

}

#endif // __SUNFISH_MATERIAL__
