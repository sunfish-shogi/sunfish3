/* NodeStat.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_NODESTAT__
#define __SUNFISH_NODESTAT__

namespace sunfish {

	class NodeStat {
	public:

		// 項目の数が変わった場合 => TTE::_
		enum {
			NullMove  = 0x01,
			Recapture = 0x02,
			Mate      = 0x04,
			HashCut   = 0x08,

			Default = NullMove | Recapture | Mate | HashCut,
		};

	private:

		unsigned _stat;

		void set(unsigned flag) {
			_stat |= flag;
		}

		void unset(unsigned flag) {
			_stat &= ~flag;
		}

		bool is(unsigned flag) const {
			return _stat & flag;
		}

	public:
		NodeStat() : _stat(Default) {
		}

		NodeStat(unsigned stat) : _stat(stat) {
		}

		NodeStat(const NodeStat& src) : _stat(src._stat) {
		}

#define __METHOD__(s)												\
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

		__METHOD__(NullMove);
		__METHOD__(Recapture);
		__METHOD__(Mate);
		__METHOD__(HashCut);

		operator unsigned() const {
			return _stat;
		}
	};

}

#endif // __SUNFISH_NODESTAT__
