/* Bitboard.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BITBOARD__
#define SUNFISH_BITBOARD__

#include "../def.h"
#include "../base/Square.h"
#include "../util/StringUtil.h"
#include <cstdint>

#if SSE2__
# define USE_SSE2 1
#endif

#if USE_SSE2
# include <emmintrin.h>
#endif

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
 *  low --------> <----- high
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
 * 4 1: 45:  0 - 0x0000000000000001
 * 4 2: 46:  1 - 0x0000000000000002
 * 4 3: 47:  2 - 0x0000000000000004
 * 4 4: 48:  3 - 0x0000000000000008
 * 4 5: 49:  4 - 0x0000000000000010
 * 4 6: 50:  5 - 0x0000000000000020
 * 4 7: 51:  6 - 0x0000000000000040
 * 4 8: 52:  7 - 0x0000000000000080
 * 4 9: 53:  8 - 0x0000000000000100
 *
 * 3 1: 54:  9 - 0x0000000000000200
 * 3 2: 55: 10 - 0x0000000000000400
 * 3 3: 56: 11 - 0x0000000000000800
 * 3 4: 57: 12 - 0x0000000000001000
 * 3 5: 58: 13 - 0x0000000000002000
 * 3 6: 59: 14 - 0x0000000000004000
 * 3 7: 60: 15 - 0x0000000000008000
 * 3 8: 61: 16 - 0x0000000000010000
 * 3 9: 62: 17 - 0x0000000000020000
 *
 * 2 1: 63: 18 - 0x0000000000040000
 * 2 2: 64: 19 - 0x0000000000080000
 * 2 3: 65: 20 - 0x0000000000100000
 * 2 4: 66: 21 - 0x0000000000200000
 * 2 5: 67: 22 - 0x0000000000400000
 * 2 6: 68: 23 - 0x0000000000800000
 * 2 7: 69: 24 - 0x0000000001000000
 * 2 8: 70: 25 - 0x0000000002000000
 * 2 9: 71: 26 - 0x0000000004000000
 *
 * 1 1: 73: 27 - 0x0000000008000000
 * 1 2: 74: 28 - 0x0000000010000000
 * 1 3: 75: 29 - 0x0000000020000000
 * 1 4: 76: 30 - 0x0000000040000000
 * 1 5: 77: 31 - 0x0000000080000000
 * 1 6: 78: 32 - 0x0000000100000000
 * 1 7: 79: 33 - 0x0000000200000000
 * 1 8: 80: 34 - 0x0000000400000000
 * 1 9: 81: 35 - 0x0000000800000000
 */

#define LOW_RANGE__  0x00001fffffffffffLL
#define HIGH_RANGE__ 0x0000000fffffffffLL

namespace sunfish {

class Bitboard {
public:
  static const int LowFiles = 5;
  static const int HighFiles = 4;
  static const int LowBits = 9 * LowFiles;
  static const int HighBits = 9 * HighFiles;

private:
  static const int8_t bfirst_[256];
  static const int8_t blast_[256];
  static const Bitboard file_[9];
  static const Bitboard notFile_[9];

  // sse2
  union {
    uint32_t i32[4];
    uint64_t i64[2];
#if USE_SSE2
    __m128i m;
#endif
  } bb_;
# define low_ bb_.i64[0]
# define high_ bb_.i64[1]
# define low0_ bb_.i32[0]
# define low1_ bb_.i32[1]
# define high0_ bb_.i32[2]
# define high1_ bb_.i32[3]

  explicit Bitboard(int sq) {
    init(sq);
  }
  explicit Bitboard(const Square& sq) {
    init(sq);
  }

  Bitboard& init(int sq) {
    if (sq < LowBits) {
      high_ = 0x00LL;
      low_ |= 0x01LL << (sq);
    } else {
      high_ |= 0x01LL << (sq-LowBits);
      low_ = 0x00LL;
    }
    return *this;
  }
  Bitboard& init(const Square& sq) {
    return init(sq.index());
  }

public:

  static const Bitboard Zero;
  static const Bitboard BPawnMovable;
  static const Bitboard& BLanceMovable;
  static const Bitboard BKnightMovable;
  static const Bitboard WPawnMovable;
  static const Bitboard& WLanceMovable;
  static const Bitboard WKnightMovable;
  static const Bitboard BPromotable;
  static const Bitboard WPromotable;
  static const Bitboard BPromotable2;
  static const Bitboard WPromotable2;

  Bitboard() {
  }
#if USE_SSE2
  explicit Bitboard(__m128i m) {
    bb_.m = m;
  }
#endif
  explicit Bitboard(uint64_t high, uint64_t low) {
    high_ = high;
    low_ = low;
  }

  static const Bitboard& mask(int sq);
  static const Bitboard& mask(const Square& sq) {
    return mask(sq.index());
  }
  static const Bitboard& file(int file) {
    return file_[file-1];
  }
  static const Bitboard& notFile(int file) {
    return notFile_[file-1];
  }

  // initialization
#if USE_SSE2
  void init() {
    bb_.m = Zero.bb_.m;
  }
  void init(const Bitboard& src) {
    bb_.m = src.bb_.m;
  }
#else
  void init() {
    high_ = 0x00LL;
    low_ = 0x00LL;
  }
  void init(const Bitboard& src) {
    high_ = src.high_;
    low_ = src.low_;
  }
#endif
  void init(uint64_t high, uint64_t low) {
    high_ = high;
    low_ = low;
  }

  // shift operation
#if USE_SSE2
  void cheapLeftShift(int n) {
    bb_.m = _mm_slli_epi64(bb_.m, n);
  }
  void cheapRightShift(int n) {
    bb_.m = _mm_srli_epi64(bb_.m, n);
  }
#else
  void cheapLeftShift(int n) {
    high_ <<= n;
    low_ <<= n;
  }
  void cheapRightShift(int n) {
    high_ >>= n;
    low_ >>= n;
  }
#endif
  void leftShift(int n) {
    if (n < LowBits) {
      high_ <<= n;
      high_ = (high_ | (low_ >> (LowBits - n))) & HIGH_RANGE__;
      low_ <<= n;
      low_ = low_ & LOW_RANGE__;
    } else {
      high_ = (low_ << (n - LowBits)) & HIGH_RANGE__;
      low_ = 0x00LL;
    }
  }
  void rightShift(int n) {
    if (n < LowBits) {
      low_ >>= n;
      low_ = (low_ | high_ << (LowBits - n)) & LOW_RANGE__;
      high_ >>= n;
    } else {
      low_ = (low_ | high_ >> (n - LowBits)) & LOW_RANGE__;
      high_ = 0x00;
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
    high_ = bb.high_;
    low_ = bb.low_;
    return *this;
  }
  const Bitboard& operator|=(const Bitboard& bb) {
    high_ |= bb.high_;
    low_ |= bb.low_;
    return *this;
  }
  const Bitboard& operator&=(const Bitboard& bb) {
    high_ &= bb.high_;
    low_ &= bb.low_;
    return *this;
  }
  const Bitboard& operator^=(const Bitboard& bb) {
    high_ ^= bb.high_;
    low_ ^= bb.low_;
    return *this;
  }
#endif
  const Bitboard& operator<<=(int n){
    leftShift(n);
    return *this;
  }
  const Bitboard& operator>>=(int n){
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
  Bitboard operator|(const Bitboard& bb) const {
    return Bitboard(high_ | bb.high_, low_ | bb.low_);
  }
  Bitboard operator&(const Bitboard& bb) const {
    return Bitboard(high_ & bb.high_, low_ & bb.low_);
  }
  Bitboard operator^(const Bitboard& bb) const {
    return Bitboard(high_ ^ bb.high_, low_ ^ bb.low_);
  }
  Bitboard operator~() const{
    return Bitboard(high_ ^ HIGH_RANGE__, low_ ^ LOW_RANGE__);
  }
  Bitboard andNot(const Bitboard& bb) const{
    return Bitboard((~high_) & bb.high_, (~low_) & bb.low_);
  }
#endif
  Bitboard operator<<(int n) const {
    return Bitboard(*this) <<= n;
  }
  Bitboard operator>>(int n) const {
    return Bitboard(*this) >>= n;
  }

  // comparation operators
  bool operator==(const Bitboard& bb) const {
    return (high_ == bb.high_) && (low_ == bb.low_);
  }
  operator bool() const {
    return (high_ || low_);
  }

  Bitboard up(int distance = 1) const {
    return *this >> distance;
  }
  Bitboard down(int distance = 1) const {
    return *this << distance;
  }
  Bitboard left(int distance = 1) const {
    return *this >> (distance * Square::RankN);
  }
  Bitboard right(int distance = 1) const {
    return *this << (distance * Square::RankN);
  }

  Bitboard& set(int sq) {
    *this |= mask(sq);
    return *this;
  }
  Bitboard& set(const Square& sq) {
    return set(sq.index());
  }
  Bitboard& unset(int sq) {
    *this &= ~mask(sq);
    return *this;
  }
  Bitboard& unset(const Square& sq) {
    return unset(sq.index());
  }
  bool check(int sq) const {
    return *this & mask(sq);
  }
  bool check(const Square& sq) const {
    return check(sq.index());
  }
  uint64_t low() const {
    return low_;
  }
  uint64_t high() const {
    return high_;
  }

  uint32_t low0() const {
    return low0_;
  }
  uint32_t low1() const {
    return low1_;
  }
  uint32_t high0() const {
    return high0_;
  }
  uint32_t high1() const {
    return high1_;
  }

  static bool isLow(const Square& sq) {
    return sq.index() < LowBits;
  }
  static bool isHigh(const Square& sq) {
    return !isLow(sq);
  }

private:
  static int count_(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555LL);
    x = (x & 0x3333333333333333LL ) + ((x >> 2) & 0x3333333333333333LL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FLL;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return (int)(x & 0x000000000000007FLL);
  }

  static int getFirst_(uint32_t bits) {
#if defined(WIN32) && !defined(__MINGW32__)
    int b;
    return _BitScanForward((DWORD*)&b, bits) ? (b+1) : 0;
#elif defined(UNIX)
    return bits == 0x00 ? 0 : __builtin_ffs(bits);
#else
    int b;
    if (bits == 0x00) { return 0; }
    if ((b = bfirst_[ bits     &0xff]) != 0) { return b; }
    if ((b = bfirst_[(bits>> 8)&0xff]) != 0) { return b + 8; }
    if ((b = bfirst_[(bits>>16)&0xff]) != 0) { return b + 16; }
    return   bfirst_[(bits>>24)     ] + 24;
#endif
  }
  static int getLast_(uint32_t bits){
#if defined(WIN32) && !defined(__MINGW32__)
    int b;
    return _BitScanReverse((DWORD*)&b, bits) ? (b+1) : 0;
#elif defined(UNIX)
    return bits == 0x00 ? 0 : (32 - __builtin_clz(bits));
#else
    int b;
    if (bits == 0x00){ return 0; }
    if ((b = blast_[(bits>>24)     ]) != 0){ return b + 24; }
    if ((b = blast_[(bits>>16)&0xff]) != 0){ return b + 16; }
    if ((b = blast_[(bits>> 8)&0xff]) != 0){ return b + 8; }
    return   blast_[ bits     &0xff];
#endif
  }

public:

  int count() const {
    return count_(high_) + count_(low_);
  }

  int getFirst() const {
    if (low_) {
      int b = getFirst_((uint32_t)low_);
      if (b) {
        return b - 1;
      } else {
        return getFirst_(low_ >> 32) + 32 - 1;
      }
    } else if (high_) {
      int b = getFirst_((uint32_t)high_);
      if (b) {
        return b + LowBits - 1;
      } else {
        return getFirst_(high_ >> 32) + 32 + LowBits - 1;
      }
    }
    return Square::Invalid;
  }
  int getLast() const {
    if (high_) {
      int b = getLast_(high_ >> 32);
      if (b) {
        return b + 32 + LowBits - 1;
      } else {
        return getLast_((uint32_t)high_) + LowBits - 1;
      }
    } else if(low_) {
      int b = getLast_(low_ >> 32);
      if (b) {
        return b + 32 - 1;
      } else {
        return getLast_((uint32_t)low_) - 1;
      }
    }
    return Square::Invalid;
  }

  int pickFirst() {
    int b;
    if (low_) {
      b = getFirst_((uint32_t)low_);
      if (!b) {
        b = getFirst_((low_ >> 32)) + 32;
      }
      b--;
      low_ &= ~(0x01LL << b);
    } else if (high_) {
      b = getFirst_((uint32_t)high_);
      if (!b) {
        b = getFirst_((high_ >> 32)) + 32;
      }
      b--;
      high_ &= ~(0x01LL << b);
      b += LowBits;
    } else {
      b = Square::Invalid;
    }
    return b;
  }

  int pickLow0First() {
    int b = getFirst_(low0_) - 1;
    low0_ &= ~(0x01 << b);
    return b;
  }
  int pickLow1First() {
    int b = getFirst_(low1_) - 1;
    low1_ &= ~(0x01 << b);
    return b + 32;
  }
  int pickHigh0First() {
    int b = getFirst_(high0_) - 1;
    high0_ &= ~(0x01 << b);
    return b + LowBits;
  }
  int pickHigh1First() {
    int b = getFirst_(high1_) - 1;
    high1_ &= ~(0x01 << b);
    return b + LowBits + 32;
  }

  std::string toString() const {
    return StringUtil::stringify(high_) + StringUtil::stringify(low_);
  }
  std::string toString2D() const;
};

#if USE_SSE2
# undef high_
# undef low_
#endif

} // namespace sunfish

// for-each
#define BB_EACH(sq, bb) \
for (sunfish::Square sq = bb.pickFirst(); sq.isValid(); sq = bb.pickFirst())

// for-each
#define BB_EACH_OPE(sq, bb, ope) \
do { \
  BB_EACH_OPE_LOW0(sq, bb, ope); \
  BB_EACH_OPE_LOW1(sq, bb, ope); \
  BB_EACH_OPE_HIGH0(sq, bb, ope); \
  BB_EACH_OPE_HIGH1(sq, bb, ope); \
} while(false)

#define BB_EACH_OPE_LOW0(sq, bb, ope) \
for (sunfish::Square sq; bb.low0(); ) { sq = bb.pickLow0First(); { ope } }

#define BB_EACH_OPE_LOW1(sq, bb, ope) \
for (sunfish::Square sq; bb.low1(); ) { sq = bb.pickLow1First(); { ope } }

#define BB_EACH_OPE_HIGH0(sq, bb, ope) \
for (sunfish::Square sq; bb.high0(); ) { sq = bb.pickHigh0First(); { ope } }

#define BB_EACH_OPE_HIGH1(sq, bb, ope) \
for (sunfish::Square sq; bb.high1(); ) { sq = bb.pickHigh1First(); { ope } }

#endif //SUNFISH_BITBOARD__
