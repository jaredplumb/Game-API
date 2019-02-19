#ifndef _P_POINT_H_
#define _P_POINT_H_

#include "PPlatform.h"

class PPoint {
public:
	int_t x, y;
	
	inline PPoint ()														: x(0), y(0) {}
	inline PPoint (const PPoint& p)											: x(p.x), y(p.y) {}
	inline PPoint (int_t x_, int_t y_)										: x(x_), y(y_) {}
	inline PPoint& Offset (const PPoint& p)									{ x += p.x; y += p.y; return *this; }
	inline PPoint& Offset (int_t x_, int_t y_)								{ x += x_; y += y_; return *this; }
	inline bool operator== (const PPoint& p) const							{ return x == p.x && y == p.y; }
	inline bool operator!= (const PPoint& p) const							{ return x != p.x || y != p.y; }
	inline const PPoint operator+ (const PPoint& p) const					{ return PPoint(x + p.x, y + p.y); }
	inline const PPoint operator- () const									{ return PPoint(-x, -y); }
	inline const PPoint operator- (const PPoint& p) const					{ return PPoint(x - p.x, y - p.y); }
	inline const PPoint operator* (int_t t) const							{ return PPoint(x * t, y * t); }
	inline const PPoint operator/ (int_t t) const							{ return PPoint(x / t, y / t); }
	inline PPoint& operator= (const PPoint& p)								{ x = p.x; y = p.y; return *this; }
	inline PPoint& operator+= (const PPoint& p)								{ x += p.x; y += p.y; return *this; }
	inline PPoint& operator-= (const PPoint& p)								{ x -= p.x; y -= p.y; return *this; }
	inline PPoint& operator*= (int_t t)										{ x *= t; y *= t; return *this; }
	inline PPoint& operator/= (int_t t)										{ x /= t; y /= t; return *this; }
};

struct PPoint16 {
	int16 x;
	int16 y;
};

#endif // _P_POINT_H_
