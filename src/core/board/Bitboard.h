/* Bitboard.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BITBOARD__
#define SUNFISH_BITBOARD__

#include "../def.h"
#include "../sse.h"
#include "../base/Square.h"
#include "../util/StringUtil.h"
#include <cstdint>

#if WIN32
# include "windows.h"
#endif

/*
 *  9  8  7  6  5  4  3  2  1 |
 * ---------------------------+--
 *  0  9 18 27 36 45 54 63 72 | 1
 *  1 10 19 28 37 46 55 64 73 | 2
 *  2 11 20 29 38 47 56 65 74 | 3
 *  3 12 21 30 39 48 57 66 75 | 4
 *  4 13 22 31 40 49 58 67 76 | 5
 *  5 14 23 32 41 50 59 68 77 | 6
 *  6 15 24 33 42 51 60 69 78 | 7
 *  7 16 25 34 43 52 61 70 79 | 8
 *  8 17 26 35 44 53 62 71 80 | 9
 *  low --------------> <- high
 */

/*
 * 9 1:  0:  0 - 0x0000000000000001
 * 9 2:  1:  1 - 0x0000000000000002
 * 9 3:  2:  2 - 0x0000000000000004
 * 9 4:  3:  3 - 0x0000000000000008
 * 9 5:  4:  4 - 0x0000000000000010
 * 9 6:  5:  5 - 0x0000000000000020
 * 9 7:  6:  6 - 0x0000000000000040
 * 9 8:  7:  7 - 0x0000000000000080
 * 9 9:  8:  8 - 0x0000000000000100
 *
 * 8 1:  9:  9 - 0x0000000000000200
 * 8 2: 10: 10 - 0x0000000000000400
 * 8 3: 11: 11 - 0x0000000000000800
 * 8 4: 12: 12 - 0x0000000000001000
 * 8 5: 13: 13 - 0x0000000000002000
 * 8 6: 14: 14 - 0x0000000000004000
 * 8 7: 15: 15 - 0x0000000000008000
 * 8 8: 16: 16 - 0x0000000000010000
 * 8 9: 17: 17 - 0x0000000000020000
 *
 * 7 1: 18: 18 - 0x0000000000040000
 * 7 2: 19: 19 - 0x0000000000080000
 * 7 3: 20: 20 - 0x0000000000100000
 * 7 4: 21: 21 - 0x0000000000200000
 * 7 5: 22: 22 - 0x0000000000400000
 * 7 6: 23: 23 - 0x0000000000800000
 * 7 7: 24: 24 - 0x0000000001000000
 * 7 8: 25: 25 - 0x0000000002000000
 * 7 9: 26: 26 - 0x0000000004000000
 *
 * 6 1: 27: 27 - 0x0000000008000000
 * 6 2: 28: 28 - 0x0000000010000000
 * 6 3: 29: 29 - 0x0000000020000000
 * 6 4: 30: 30 - 0x0000000040000000
 * 6 5: 31: 31 - 0x0000000080000000
 * 6 6: 32: 32 - 0x0000000100000000
 * 6 7: 33: 33 - 0x0000000200000000
 * 6 8: 34: 34 - 0x0000000400000000
 * 6 9: 35: 35 - 0x0000000800000000
 *
 * 5 1: 36: 36 - 0x0000001000000000
 * 5 2: 37: 37 - 0x0000002000000000
 * 5 3: 38: 38 - 0x0000004000000000
 * 5 4: 39: 39 - 0x0000008000000000
 * 5 5: 40: 40 - 0x0000010000000000
 * 5 6: 41: 41 - 0x0000020000000000
 * 5 7: 42: 42 - 0x0000040000000000
 * 5 8: 43: 43 - 0x0000080000000000
 * 5 9: 44: 44 - 0x0000100000000000
 *
 * 4 1: 45: 45 - 0x0000200000000000
 * 4 2: 46: 46 - 0x0000400000000000
 * 4 3: 47: 47 - 0x0000800000000000
 * 4 4: 48: 48 - 0x0001000000000000
 * 4 5: 49: 49 - 0x0002000000000000
 * 4 6: 50: 50 - 0x0004000000000000
 * 4 7: 51: 51 - 0x0008000000000000
 * 4 8: 52: 52 - 0x0010000000000000
 * 4 9: 53: 53 - 0x0020000000000000
 *
 * 3 1: 54: 54 - 0x0040000000000000
 * 3 2: 55: 55 - 0x0080000000000000
 * 3 3: 56: 56 - 0x0100000000000000
 * 3 4: 57: 57 - 0x0200000000000000
 * 3 5: 58: 58 - 0x0400000000000000
 * 3 6: 59: 59 - 0x0800000000000000
 * 3 7: 60: 60 - 0x1000000000000000
 * 3 8: 61: 61 - 0x2000000000000000
 * 3 9: 62: 62 - 0x4000000000000000
 *
 * 2 1: 63:  0 - 0x0000000000000001
 * 2 2: 64:  1 - 0x0000000000000002
 * 2 3: 65:  2 - 0x0000000000000004
 * 2 4: 66:  3 - 0x0000000000000008
 * 2 5: 67:  4 - 0x0000000000000010
 * 2 6: 68:  5 - 0x0000000000000020
 * 2 7: 69:  6 - 0x0000000000000040
 * 2 8: 70:  7 - 0x0000000000000080
 * 2 9: 71:  8 - 0x0000000000000100
 *
 * 1 1: 73:  9 - 0x0000000000000200
 * 1 2: 74: 10 - 0x0000000000000400
 * 1 3: 75: 11 - 0x0000000000000800
 * 1 4: 76: 12 - 0x0000000000001000
 * 1 5: 77: 13 - 0x0000000000002000
 * 1 6: 78: 14 - 0x0000000000004000
 * 1 7: 79: 15 - 0x0000000000008000
 * 1 8: 80: 16 - 0x0000000000010000
 * 1 9: 81: 17 - 0x0000000000020000
 */

#define LOW_RANGE__  0x7fffffffffffffffLL
#define HIGH_RANGE__ 0x000000000003ffffLL

namespace sunfish {

class Bitboard {
public:
  static CONSTEXPR_CONST int LowFiles = 7;
  static CONSTEXPR_CONST int HighFiles = 2;
  static CONSTEXPR_CONST int LowBits = 9 * LowFiles;
  static CONSTEXPR_CONST int HighBits = 9 * HighFiles;

private:
  // sse2
  union u128 {
    uint64_t i64[2];
#if BYTE_ORDER == BIG_ENDIAN
    CONSTEXPR u128(uint64_t high, uint64_t low) : i64{high, low} {}
#elif BYTE_ORDER == LITTLE_ENDIAN
    CONSTEXPR u128(uint64_t high, uint64_t low) : i64{low, high} {}
#endif
#if USE_SSE2
    __m128i m;
    CONSTEXPR u128(const __m128i& m) : m(m) {}
#endif
    u128() {}
  };
  static_assert(sizeof(u128) == 16, "invalid size");
  u128 bb_;
#if BYTE_ORDER == BIG_ENDIAN
# define BB_LOW_ bb_.i64[1]
# define BB_HIGH_ bb_.i64[0]
#elif BYTE_ORDER == LITTLE_ENDIAN
# define BB_LOW_ bb_.i64[0]
# define BB_HIGH_ bb_.i64[1]
#endif

public:

  Bitboard() {
  }
#if USE_SSE2
  explicit CONSTEXPR Bitboard(__m128i m) : bb_(m) {
  }
#endif
  explicit CONSTEXPR Bitboard(uint64_t high, uint64_t low) : bb_(high, low) {
  }

  explicit CONSTEXPR Bitboard(int sq) :
    bb_(sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x00LL         : 0x01LL << (sq-LowBits),
        sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x01LL << (sq) : 0x00LL) {
  }
  explicit CONSTEXPR Bitboard(const Square& sq) :
    bb_(sq.index() == Square::Invalid ? 0x00LL : isLow(sq) ? 0x00LL                 : 0x01LL << (sq.index()-LowBits),
        sq.index() == Square::Invalid ? 0x00LL : isLow(sq) ? 0x01LL << (sq.index()) : 0x00LL) {
  }

  static CONSTEXPR Bitboard Zero() {
    return Bitboard(0x00LL, 0x00LL);
  }

  static Bitboard file(int fileIndex) {
    static CONSTEXPR_CONST Bitboard t[9] = {
      Bitboard{ 0x000000000003fe00LL, 0x0000000000000000LL },
      Bitboard{ 0x00000000000001ffLL, 0x0000000000000000LL },
      Bitboard{ 0x0000000000000000LL, 0x7fc0000000000000LL },
      Bitboard{ 0x0000000000000000LL, 0x003fe00000000000LL },
      Bitboard{ 0x0000000000000000LL, 0x00001ff000000000LL },
      Bitboard{ 0x0000000000000000LL, 0x0000000ff8000000LL },
      Bitboard{ 0x0000000000000000LL, 0x0000000007fc0000LL },
      Bitboard{ 0x0000000000000000LL, 0x000000000003fe00LL },
      Bitboard{ 0x0000000000000000LL, 0x00000000000001ffLL },
    };
    return t[fileIndex-1];
  }
  static Bitboard notFile(int fileIndex) {
    static CONSTEXPR_CONST Bitboard t[9] = {
      Bitboard{ 0x00000000000001ffLL, 0x7fffffffffffffffLL },
      Bitboard{ 0x000000000003fe00LL, 0x7fffffffffffffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x003fffffffffffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7fc01fffffffffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7fffe00fffffffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7ffffff007ffffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7ffffffff803ffffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7ffffffffffc01ffLL },
      Bitboard{ 0x000000000003ffffLL, 0x7ffffffffffffe00LL },
    };
    return t[fileIndex-1];
  }

  // initialization
#if USE_SSE2
  void init() {
    bb_.m = Zero().bb_.m;
  }
  void init(const Bitboard& src) {
    bb_.m = src.bb_.m;
  }
#else
  void init() {
    BB_HIGH_ = 0x00LL;
    BB_LOW_ = 0x00LL;
  }
  void init(const Bitboard& src) {
    BB_HIGH_ = src.BB_HIGH_;
    BB_LOW_ = src.BB_LOW_;
  }
#endif
  void init(uint64_t high, uint64_t low) {
    BB_HIGH_ = high;
    BB_LOW_ = low;
  }

  // shift operation
#if USE_SSE2
  void leftShift64(int n) {
    bb_.m = _mm_slli_epi64(bb_.m, n);
  }
  void rightShift64(int n) {
    bb_.m = _mm_srli_epi64(bb_.m, n);
  }
#else
  void leftShift64(int n) {
    BB_HIGH_ <<= n;
    BB_LOW_ <<= n;
  }
  void rightShift64(int n) {
    BB_HIGH_ >>= n;
    BB_LOW_ >>= n;
  }
#endif
  void leftShift(int n) {
    if (n < LowBits) {
      BB_HIGH_ <<= n;
      BB_HIGH_ = (BB_HIGH_ | (BB_LOW_ >> (LowBits - n))) & HIGH_RANGE__;
      BB_LOW_ <<= n;
      BB_LOW_ = BB_LOW_ & LOW_RANGE__;
    } else {
      BB_HIGH_ = (BB_LOW_ << (n - LowBits)) & HIGH_RANGE__;
      BB_LOW_ = 0x00LL;
    }
  }
  void rightShift(int n) {
    if (n < LowBits) {
      BB_LOW_ >>= n;
      BB_LOW_ = (BB_LOW_ | BB_HIGH_ << (LowBits - n)) & LOW_RANGE__;
      BB_HIGH_ >>= n;
    } else {
      BB_LOW_ = (BB_LOW_ | BB_HIGH_ >> (n - LowBits)) & LOW_RANGE__;
      BB_HIGH_ = 0x00;
    }
  }

  // substitution operators
#if USE_SSE2
  const Bitboard& operator=(const Bitboard& bb) {
    bb_.m = bb.bb_.m;
    return *this;
  }
  const Bitboard& operator|=(const Bitboard& bb) {
    bb_.m = _mm_or_si128(bb_.m, bb.bb_.m);
    return *this;
  }
  const Bitboard& operator&=(const Bitboard& bb) {
    bb_.m = _mm_and_si128(bb_.m, bb.bb_.m);
    return *this;
  }
  const Bitboard& operator^=(const Bitboard& bb) {
    bb_.m = _mm_xor_si128(bb_.m, bb.bb_.m);
    return *this;
  }
#else
  const Bitboard& operator=(const Bitboard& bb) {
    BB_HIGH_ = bb.BB_HIGH_;
    BB_LOW_ = bb.BB_LOW_;
    return *this;
  }
  const Bitboard& operator|=(const Bitboard& bb) {
    BB_HIGH_ |= bb.BB_HIGH_;
    BB_LOW_ |= bb.BB_LOW_;
    return *this;
  }
  const Bitboard& operator&=(const Bitboard& bb) {
    BB_HIGH_ &= bb.BB_HIGH_;
    BB_LOW_ &= bb.BB_LOW_;
    return *this;
  }
  const Bitboard& operator^=(const Bitboard& bb) {
    BB_HIGH_ ^= bb.BB_HIGH_;
    BB_LOW_ ^= bb.BB_LOW_;
    return *this;
  }
#endif
  const Bitboard& operator<<=(int n) {
    leftShift(n);
    return *this;
  }
  const Bitboard& operator>>=(int n) {
    rightShift(n);
    return *this;
  }

  // bit operators
#if USE_SSE2
  Bitboard operator|(const Bitboard& bb) const {
    return Bitboard(_mm_or_si128(bb_.m, bb.bb_.m));
  }
  Bitboard operator&(const Bitboard& bb) const {
    return Bitboard(_mm_and_si128(bb_.m, bb.bb_.m));
  }
  Bitboard operator^(const Bitboard& bb) const {
    return Bitboard(_mm_xor_si128(bb_.m, bb.bb_.m));
  }
  Bitboard operator~() const{
    return Bitboard(_mm_andnot_si128(bb_.m, _mm_set_epi64((__m64)HIGH_RANGE__, (__m64)LOW_RANGE__)));
  }
  Bitboard andNot(const Bitboard& bb) const{
    return Bitboard(_mm_andnot_si128(bb_.m, bb.bb_.m));
  }
#else
  CONSTEXPR Bitboard operator|(const Bitboard& bb) const {
    return Bitboard(BB_HIGH_ | bb.BB_HIGH_, BB_LOW_ | bb.BB_LOW_);
  }
  CONSTEXPR Bitboard operator&(const Bitboard& bb) const {
    return Bitboard(BB_HIGH_ & bb.BB_HIGH_, BB_LOW_ & bb.BB_LOW_);
  }
  CONSTEXPR Bitboard operator^(const Bitboard& bb) const {
    return Bitboard(BB_HIGH_ ^ bb.BB_HIGH_, BB_LOW_ ^ bb.BB_LOW_);
  }
  CONSTEXPR Bitboard operator~() const{
    return Bitboard(BB_HIGH_ ^ HIGH_RANGE__, BB_LOW_ ^ LOW_RANGE__);
  }
  CONSTEXPR Bitboard andNot(const Bitboard& bb) const{
    return Bitboard((~BB_HIGH_) & bb.BB_HIGH_, (~BB_LOW_) & bb.BB_LOW_);
  }
#endif
  Bitboard operator<<(int n) const {
    return Bitboard(*this) <<= n;
  }
  Bitboard operator>>(int n) const {
    return Bitboard(*this) >>= n;
  }

  // comparation operators
  CONSTEXPR bool operator==(const Bitboard& bb) const {
    return (BB_HIGH_ == bb.BB_HIGH_) && (BB_LOW_ == bb.BB_LOW_);
  }
  CONSTEXPR operator bool() const {
    return (BB_HIGH_ || BB_LOW_);
  }

  Bitboard up(int distance = 1) const {
    Bitboard tmp(*this);
    tmp.rightShift64(distance);
    return tmp;
  }
  Bitboard down(int distance = 1) const {
    Bitboard tmp(*this);
    tmp.leftShift64(distance);
    return tmp;
  }

  Bitboard& set(int sq) {
    if (Square(sq).isValid()) {
      if (isLow(sq)) {
        BB_LOW_ |= 0x01LL << (sq);
      } else {
        BB_HIGH_ |= 0x01LL << (sq-LowBits);
      }
    }
    return *this;
  }
  Bitboard& set(const Square& sq) {
    return set(sq.index());
  }
  Bitboard& unset(int sq) {
    if (Square(sq).isValid()) {
      if (isLow(sq)) {
        BB_LOW_ &= ~(0x01LL << (sq));
      } else {
        BB_HIGH_ &= ~(0x01LL << (sq-LowBits));
      }
    }
    return *this;
  }
  Bitboard& unset(const Square& sq) {
    return unset(sq.index());
  }
  bool check(int sq) const {
    if (Square(sq).isValid()) {
      if (isLow(sq)) {
        return BB_LOW_ & (0x01LL << (sq));
      } else {
        return BB_HIGH_ & (0x01LL << (sq-LowBits));
      }
    }
    return false;
  }
  bool check(const Square& sq) const {
    return check(sq.index());
  }

  CONSTEXPR Bitboard copyWithSet(int sq) const {
    return Bitboard(
        BB_HIGH_ | (sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x00LL         : 0x01LL << (sq-LowBits)),
        BB_LOW_  | (sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x01LL << (sq) : 0x00LL));
  }
  CONSTEXPR Bitboard copyWithSet(const Square& sq) const {
    return copyWithSet(sq.index());
  }
  CONSTEXPR Bitboard copyWithUnset(int sq) const {
    return Bitboard(
        BB_HIGH_ & ~(sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x00LL         : 0x01LL << (sq-LowBits)),
        BB_LOW_  & ~(sq == Square::Invalid ? 0x00LL : isLow(sq) ? 0x01LL << (sq) : 0x00LL));
  }
  CONSTEXPR Bitboard copyWithUnset(const Square& sq) const {
    return copyWithUnset(sq.index());
  }

  CONSTEXPR uint64_t low() const {
    return BB_LOW_;
  }
  CONSTEXPR uint64_t high() const {
    return BB_HIGH_;
  }

  static CONSTEXPR bool isLow(const Square& sq) {
    return sq.index() < LowBits;
  }
  static CONSTEXPR bool isHigh(const Square& sq) {
    return !isLow(sq);
  }

private:
  static int getFirst_(uint64_t bits) {
#if defined(WIN32) && !defined(__MINGW32__)
    int b;
    return _BitScanForward64((DWORD*)&b, bits) ? (b+1) : 0;
#elif defined(UNIX)
    return bits == 0x00LL ? 0 : __builtin_ffsll(bits);
#else
    static const int8_t bfirst_[256] = {
       0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1,
       3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1,
       4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
       3, 1, 2, 1, 7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
       5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1,
       3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1,
       4, 1, 2, 1, 3, 1, 2, 1, 8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
       3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
       6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1,
       3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 7, 1, 2, 1, 3, 1, 2, 1,
       4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
       3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
       5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    };
    int b;
    if (bits == 0x00) { return 0; }
    if ((b = bfirst_[ bits     &0xff]) != 0) { return b; }
    if ((b = bfirst_[(bits>> 8)&0xff]) != 0) { return b +  8; }
    if ((b = bfirst_[(bits>>16)&0xff]) != 0) { return b + 16; }
    if ((b = bfirst_[(bits>>24)&0xff]) != 0) { return b + 24; }
    if ((b = bfirst_[ bits>>32 &0xff]) != 0) { return b + 32; }
    if ((b = bfirst_[(bits>>40)&0xff]) != 0) { return b + 40; }
    if ((b = bfirst_[(bits>>48)&0xff]) != 0) { return b + 48; }
    return   bfirst_[(bits>>56)     ] + 56;
#endif
  }
  static int getLast_(uint64_t bits){
#if defined(WIN32) && !defined(__MINGW32__)
    int b;
    return _BitScanReverse64((DWORD*)&b, bits) ? (b+1) : 0;
#elif defined(UNIX)
    return bits == 0x00LL ? 0 : (64 - __builtin_clzll(bits));
#else
    static const int8_t blast_[256] = {
       0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
       5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,
       6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
       6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
       7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
       7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
       7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    };
    int b;
    if (bits == 0x00LL){ return 0; }
    if ((b = blast_[(bits>>56)       ]) != 0){ return b + 56; }
    if ((b = blast_[(bits>>48)&0xffLL]) != 0){ return b + 48; }
    if ((b = blast_[(bits>>40)&0xffLL]) != 0){ return b + 40; }
    if ((b = blast_[(bits>>32)&0xffLL]) != 0){ return b + 32; }
    if ((b = blast_[(bits>>24)&0xffLL]) != 0){ return b + 24; }
    if ((b = blast_[(bits>>16)&0xffLL]) != 0){ return b + 16; }
    if ((b = blast_[(bits>> 8)&0xffLL]) != 0){ return b +  8; }
    return   blast_[ bits     &0xffLL];
#endif
  }

public:

  int count() const {
#if defined(UNIX)
    return __builtin_popcountll(BB_LOW_)
         + __builtin_popcountll(BB_HIGH_);
#else
    auto count64 = [](uint64_t x) {
      x = x - ((x >> 1) & 0x5555555555555555LL);
      x = (x & 0x3333333333333333LL ) + ((x >> 2) & 0x3333333333333333LL);
      x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FLL;
      x = x + (x >> 8);
      x = x + (x >> 16);
      x = x + (x >> 32);
      return (int)(x & 0x000000000000007FLL);
    };
    return count64(BB_HIGH_) + count64(BB_LOW_);
#endif
  }

  int getFirst() const {
    if (BB_LOW_) {
      return getFirst_(BB_LOW_) - 1;
    } else if (BB_HIGH_) {
      return getFirst_(BB_HIGH_) + (LowBits - 1);
    }
    return Square::Invalid;
  }
  int getLast() const {
    if (BB_HIGH_) {
      return getLast_(BB_HIGH_) + (LowBits - 1);
    } else if(BB_LOW_) {
      return getLast_(BB_LOW_) - 1;
    }
    return Square::Invalid;
  }

  int pickFirst() {
    int b;
    if (BB_LOW_) {
      b = getFirst_(BB_LOW_);
      b--;
      BB_LOW_ &= ~(0x01LL << b);
    } else if (BB_HIGH_) {
      b = getFirst_(BB_HIGH_);
      b--;
      BB_HIGH_ &= ~(0x01LL << b);
      b += LowBits;
    } else {
      b = Square::Invalid;
    }
    return b;
  }

  int pickLowFirst() {
    int b = getFirst_(BB_LOW_) - 1;
    BB_LOW_ &= ~(0x01LL << b);
    return b;
  }
  int pickHighFirst() {
    int b = getFirst_(BB_HIGH_) - 1;
    BB_HIGH_ &= ~(0x01LL << b);
    return b + LowBits;
  }

  std::string toString() const {
    return StringUtil::stringify(BB_HIGH_) + StringUtil::stringify(BB_LOW_);
  }
  std::string toString2D() const;
};

#if USE_SSE2
# undef BB_LOW_
# undef BB_HIGH_
#endif

CONSTEXPR_CONST Bitboard BPawnMovable(
  HIGH_RANGE__ - 0x0000000000000201LL,
  LOW_RANGE__  - 0x0040201008040201LL
);
CONSTEXPR_CONST Bitboard BLanceMovable = BPawnMovable;
CONSTEXPR_CONST Bitboard BKnightMovable(
  HIGH_RANGE__ - 0x0000000000000603LL,
  LOW_RANGE__  - 0x00c06030180c0603LL
);
CONSTEXPR_CONST Bitboard WPawnMovable(
  HIGH_RANGE__ - 0x0000000000020100LL,
  LOW_RANGE__  - 0x4020100804020100LL
);
CONSTEXPR_CONST Bitboard WLanceMovable = WPawnMovable;
CONSTEXPR_CONST Bitboard WKnightMovable(
  HIGH_RANGE__ - 0x0000000000030180LL,
  LOW_RANGE__  - 0x6030180c06030180LL
);
CONSTEXPR_CONST Bitboard BPromotable(0x0000000000000e07LL, 0x01c0e070381c0e07LL);
CONSTEXPR_CONST Bitboard WPromotable(0x00000000000381c0LL, 0x70381c0e070381c0LL);
CONSTEXPR_CONST Bitboard BPromotable2(0x0000000000000603LL, 0x00c06030180c0603LL);
CONSTEXPR_CONST Bitboard WPromotable2(0x0000000000030180LL, 0x6030180c06030180LL);

} // namespace sunfish

// for-each
#define BB_EACH(sq, bb) \
for (sunfish::Square sq = bb.pickFirst(); sq.isValid(); sq = bb.pickFirst())

// for-each
#define BB_EACH_OPE(sq, bb, ope) \
do { \
  BB_EACH_OPE_LOW(sq, bb, ope); \
  BB_EACH_OPE_HIGH(sq, bb, ope); \
} while(false)

#define BB_EACH_OPE_LOW(sq, bb, ope) \
for (sunfish::Square sq; bb.low(); ) { sq = bb.pickLowFirst(); { ope } }

#define BB_EACH_OPE_HIGH(sq, bb, ope) \
for (sunfish::Square sq; bb.high(); ) { sq = bb.pickHighFirst(); { ope } }

#endif //SUNFISH_BITBOARD__
