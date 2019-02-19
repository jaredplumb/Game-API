#ifndef _P_RECT_H_
#define _P_RECT_H_

#include "PPlatform.h"
#include "PPoint.h"
#include "PSize.h"

class PRect {
public:
	int_t x, y, width, height;
	
	inline PRect ()															: x(0), y(0), width(0), height(0) {}
	inline PRect (const PRect& r)											: x(r.x) , y(r.y), width(r.width), height(r.height) {}
	inline PRect (int_t x_, int_t y_, int_t w, int_t h)						: x(x_), y(y_), width(w), height(h) {}
	inline int_t GetLeft () const											{ return x; }
	inline int_t GetRight () const											{ return x + width; }
	inline int_t GetTop () const											{ return y; }
	inline int_t GetBottom () const											{ return y + height; }
	inline bool IsPointInRect (const PPoint& p) const						{ return p.x >= x && p.y >= y && p.x < (x + width) && p.y < (y + height); }
	inline bool IsPointInRect (int_t x_, int_t y_) const					{ return x_ >= x && y_ >= y && x_ < (x + width) && y_ < (y + height); }
	inline bool IsCollision (const PRect& r) const							{ return x < (r.x + r.width) && y < (r.y + r.height) && (x + width) > r.x && (y + height) > r.y; }
	inline bool IsCollision (int_t x_, int_t y_, int_t w, int_t h) const	{ return x < (x_ + w) && y < (y_ + h) && (x + width) > x_ && (y + height) > y_; }
	inline PRect& SetLoc (const PPoint& p)									{ x = p.x; y = p.y; return *this; }
	inline PRect& SetLoc (int_t x_, int_t y_)								{ x = x_; y = y_; return *this; }
	inline PRect& SetSize (const PSize& s)									{ width = s.width; height = s.height; return *this; }
	inline PRect& SetSize (int_t w, int_t h)								{ width = w; height = h; return *this; }
	inline PRect& Center (const PRect& r)									{ x = r.x + (r.width - width) / 2; y = r.y + (r.height - height) / 2; return *this; }
	inline PRect& Center (int_t x_, int_t y_, int_t w, int_t h)				{ x = x_ + (w - width) / 2; y = y_ + (h - height) / 2; return *this; }
	inline PRect& Offset (const PPoint& p)									{ x += p.x; y += p.y; return *this; }
	inline PRect& Offset (int_t x_, int_t y_)								{ x += x_; y += y_; return *this; }
	inline bool operator== (const PRect& r) const							{ return x == r.x && y == r.y && width == r.width && height == r.height; }
	inline bool operator!= (const PRect& r) const							{ return x != r.x || y != r.y || width != r.width || height != r.height; }
};

struct PRect16 {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
};

#endif // _P_RECT_H_
