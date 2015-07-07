/* NodeStat.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_NODESTAT__
#define SUNFISH_NODESTAT__

#include <cstdint>

namespace sunfish {

class NodeStat {
public:

  enum {
    NullMove   = 0x01,
    Recapture  = 0x02,
    Mate       = 0x04,
    HashCut    = 0x08,
    Recursion  = 0x10,
    MateThreat = 0x20,

    Default = NullMove | Recapture | Mate | HashCut | Recursion,
  };

private:

  uint32_t stat_;

  void set(uint32_t flag) {
    stat_ |= flag;
  }

  void unset(uint32_t flag) {
    stat_ &= ~flag;
  }

  bool is(uint32_t flag) const {
    return stat_ & flag;
  }

public:
  NodeStat() : stat_(Default) {
  }

  NodeStat(uint32_t stat) : stat_(stat) {
  }

  NodeStat(const NodeStat& src) : stat_(src.stat_) {
  }

#define METHOD__(s) \
  NodeStat& set ## s() { \
    set(s); \
    return *this; \
  } \
  NodeStat& unset ## s() { \
    unset(s); \
    return *this; \
  } \
  bool is ## s() const { \
    return is(s); \
  }

  METHOD__(NullMove);
  METHOD__(Recapture);
  METHOD__(Mate);
  METHOD__(HashCut);
  METHOD__(Recursion);
  METHOD__(MateThreat);

  operator uint32_t() const {
    return stat_;
  }
};

} // namespace sunfish

#endif // SUNFISH_NODESTAT__
