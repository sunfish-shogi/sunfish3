/* Data.cpp
 *
 * Kubo Ryosuke
 */

#include "Data.h"

namespace sunfish {

	MovableFlags MovableTable[] = {
		//   up,  down,  left, right,   l-u,   l-d,   r-u,   r-d
		{  true, false, false, false, false, false, false, false }, // black pawn
		{  true, false, false, false, false, false, false, false }, // black lance
		{ false, false, false, false, false, false, false, false }, // black knight
		{  true, false, false, false,  true,  true,  true,  true }, // black silver
		{  true,  true,  true,  true,  true, false,  true, false }, // black gold
		{ false, false, false, false,  true,  true,  true,  true }, // black bishop
		{  true,  true,  true,  true, false, false, false, false }, // black rook
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black king
		{  true,  true,  true,  true,  true, false,  true, false }, // black tokin
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-lance
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-knight
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black horse
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black dragon
		{ false, false, false, false, false, false, false, false }, // n/a
		{ false,  true, false, false, false, false, false, false }, // white pawn
		{ false,  true, false, false, false, false, false, false }, // white lance
		{ false, false, false, false, false, false, false, false }, // white knight
		{ false,  true, false, false,  true,  true,  true,  true }, // white silver
		{  true,  true,  true,  true, false,  true, false,  true }, // white gold
		{ false, false, false, false,  true,  true,  true,  true }, // white bishop
		{  true,  true,  true,  true, false, false, false, false }, // white rook
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white king
		{  true,  true,  true,  true, false,  true, false,  true }, // white tokin
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-lance
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-knight
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white horse
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white dragon
		{ false, false, false, false, false, false, false, false }, // n/a
	};

}
