/* Data.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_DATA__
#define __SUNFISH_DATA__

namespace sunfish {

	struct MovableFlags {
		bool up;
		bool down;
		bool left;
		bool right;
		bool leftUp;
		bool leftDown;
		bool rightUp;
		bool rightDown;
	};
	extern MovableFlags MovableTable[32];

}

#endif // __SUNFISH_DATA__
