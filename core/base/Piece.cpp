/* Piece.cpp
 *
 * Kubo Ryosuke
 */

#include "Piece.h"
#include <cstring>
#include <cctype>

namespace sunfish {

	const char* Piece::names[] = {
		"fu", "ky", "ke", "gi", "ki", "ka", "hi", "ou",
		"to", "ny", "nk", "ng", "  ", "um", "ry", "  ",
		"Fu", "Ky", "Ke", "Gi", "Ki", "Ka", "Hi", "Ou",
		"To", "Ny", "Nk", "Ng", "  ", "Um", "Ry", "  ",
		"  "
	};

	const char* Piece::namesCsa[] = {
		"+FU", "+KY", "+KE", "+GI", "+KI", "+KA", "+HI", "+OU",
		"+TO", "+NY", "+NK", "+NG", "   ", "+UM", "+RY", "   ",
		"-FU", "-KY", "-KE", "-GI", "-KI", "-KA", "-HI", "-OU",
		"-TO", "-NY", "-NK", "-NG", "   ", "-UM", "-RY", "   ",
		"   "
	};

	const char* Piece::namesCsaKindOnly[] = {
		"FU", "KY", "KE", "GI", "KI", "KA", "HI", "OU",
		"TO", "NY", "NK", "NG", "  ", "UM", "RY", "  ",
		"FU", "KY", "KE", "GI", "KI", "KA", "HI", "OU",
		"TO", "NY", "NK", "NG", "  ", "UM", "RY", "  ",
		"  "
	};

	Piece Piece::parse(const char* str) {
		PIECE_EACH(piece) {
			if (strncmp(str, names[piece], 2) == 0) {
				return piece;
			}
		}
		return Piece::Empty;
	}

	Piece Piece::parseCsa(const char* str) {
		if (str[0] == '+' || str[0] == '-') {
			PIECE_EACH(piece) {
				if (strncmp(str, namesCsa[piece], 3) == 0) {
					return piece;
				}
			}
		} else if (isalpha(str[0])) {
			PIECE_KIND_EACH(piece) {
				if (strncmp(str, namesCsaKindOnly[piece], 2) == 0) {
					return piece;
				}
			}
		}
		return Piece::Empty;
	}
}
