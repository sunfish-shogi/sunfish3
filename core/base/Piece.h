/* Piece.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_PIECE__
#define __SUNFISH_PIECE__

namespace sunfish {

	class Piece {
	public:
		static const unsigned char Promotion = 0x08;
		static const unsigned char Empty = 0x20;
		static const unsigned char White = 0x10;
           
		static const unsigned char HandMask = 0x07;
		static const unsigned char KindMask = 0x0f;

		static const unsigned char Pawn = 0;
		static const unsigned char Lance = 1;
		static const unsigned char Knight = 2;
		static const unsigned char Silver = 3;
		static const unsigned char Gold = 4;
		static const unsigned char Bishop = 5;
		static const unsigned char Rook = 6;
		static const unsigned char King = 7;
		static const unsigned char Tokin = Promotion | Pawn;
		static const unsigned char ProLance = Promotion | Lance;
		static const unsigned char ProKnight = Promotion | Knight;
		static const unsigned char ProSilver = Promotion | Silver;
		static const unsigned char Horse = Promotion | Bishop;
		static const unsigned char Dragon = Promotion | Rook;
           
		static const unsigned char BPawn = Pawn;
		static const unsigned char BLance = Lance;
		static const unsigned char BKnight = Knight;
		static const unsigned char BSilver = Silver;
		static const unsigned char BGold = Gold;
		static const unsigned char BBishop = Bishop;
		static const unsigned char BRook = Rook;
		static const unsigned char BKing = King;
		static const unsigned char BTokin = Tokin;
		static const unsigned char BProLance = ProLance;
		static const unsigned char BProKnight = ProKnight;
		static const unsigned char BProSilver = ProSilver;
		static const unsigned char BHorse = Horse;
		static const unsigned char BDragon = Dragon;
           
		static const unsigned char WPawn = White | Pawn;
		static const unsigned char WLance = White | Lance;
		static const unsigned char WKnight = White | Knight;
		static const unsigned char WSilver = White | Silver;
		static const unsigned char WGold = White | Gold;
		static const unsigned char WBishop = White | Bishop;
		static const unsigned char WRook = White | Rook;
		static const unsigned char WKing = White | King;
		static const unsigned char WTokin = White | Tokin;
		static const unsigned char WProLance = White | ProLance;
		static const unsigned char WProKnight = White | ProKnight;
		static const unsigned char WProSilver = White | ProSilver;
		static const unsigned char WHorse = White | Horse;
		static const unsigned char WDragon = White | Dragon;
           
		static const unsigned char Num = WDragon + 1;
		static const unsigned char Begin = BPawn;
		static const unsigned char End = WDragon + 1;
           
		static const unsigned char HandNum = Rook + 1;
		static const unsigned char HandBegin = Pawn;
		static const unsigned char HandEnd = Rook + 1;

		static const unsigned char KindNum = Dragon + 1;
		static const unsigned char KindBegin = Pawn;
		static const unsigned char KindEnd = Dragon + 1;
           
		static const char* names[0x21];
		static const char* namesCsa[0x21];
		static const char* namesCsaKindOnly[0x21];

	private:

		unsigned char _index;

	public:

		Piece() : _index(Empty) {
		}

		Piece(const Piece& piece) : _index(piece._index) {
		}

		Piece(unsigned char index) : _index(index) {
		}

		operator unsigned char() const {
			return _index;
		}

		bool exists() const {
			return _index != Empty;
		}
		bool isEmpty() const {
			return _index == Empty;
		}

		Piece hand() const {
			return Piece(_index & HandMask);
		}
		Piece promote() const {
			return Piece(_index | Promotion);
		}
		Piece unpromote() const {
			return Piece(_index & ~Promotion);
		}
		Piece kindOnly() const {
			return Piece(_index & KindMask);
		}
		Piece black() const {
			return Piece(_index & ~White);
		}
		Piece white() const {
			return Piece(_index | White);
		}

		bool isUnpromoted() const {
			return !isPromoted();
		}
		bool isPromoted() const {
			return _index & Promotion;
		}
		bool isBlack() const {
			return !(_index & (Empty | White));
		}
		bool isWhite() const {
			return _index & White;
		}

		Piece next() const {
			unsigned char nextIndex = _index + 1U;
			if ((nextIndex == (Promotion | BGold)) ||
					(nextIndex == (Promotion | WGold)) ||
					(nextIndex == (Promotion | BKing))) {
				nextIndex++;
			}
			return Piece(nextIndex);
		}
		Piece nextUnsafe() const {
			return Piece(_index + 1U);
		}

		const char* toString() const {
			return names[_index];
		}
		const char* toStringCsa(bool kind_only = false) const {
			return kind_only ? namesCsaKindOnly[kindOnly()._index] : namesCsa[_index];
		}
		static Piece parse(const char* str);
		static Piece parseCsa(const char* str);

	};

}

#define PIECE_EACH(piece)						for (sunfish::Piece (piece) = sunfish::Piece::Begin; (piece) != sunfish::Piece::End; (piece) = (piece).next())
#define PIECE_EACH_UNSAFE(piece)		for (sunfish::Piece (piece) = sunfish::Piece::Begin; (piece) != sunfish::Piece::End; (piece) = (piece).nextUnsafe())
#define PIECE_KIND_EACH(piece)			for (sunfish::Piece (piece) = sunfish::Piece::KindBegin; (piece) != sunfish::Piece::KindEnd; (piece) = (piece).nextUnsafe())
#define HAND_EACH(piece)						for (sunfish::Piece (piece) = sunfish::Piece::HandBegin; (piece) != sunfish::Piece::HandEnd; (piece) = (piece).nextUnsafe())

#endif //__SUNFISH_PIECE__
