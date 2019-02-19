#ifndef _P_VECTOR_H_
#define _P_VECTOR_H_

#include "PPlatform.h"

class PVector {
public:
	double x, y;
	
	inline PVector ()												: x((double)0), y((double)0) {}
	inline PVector (const PVector& p)								: x(p.x), y(p.y) {}
	inline PVector (double x_, double y_)							: x(x_), y(y_) {}
	inline bool operator== (const PVector& p) const					{ return x == p.x && y == p.y; }
	inline bool operator!= (const PVector& p) const					{ return x != p.x || y != p.y; }
	inline const PVector operator+ (const PVector& p) const			{ return PVector(x + p.x, y + p.y); }
	inline const PVector operator- () const							{ return PVector(-x, -y); }
	inline const PVector operator- (const PVector& p) const			{ return PVector(x - p.x, y - p.y); }
	inline const PVector operator* (double t) const					{ return PVector(x * t, y * t); }
	inline const PVector operator/ (double t) const					{ return PVector(x / t, y / t); }
	inline PVector& operator= (const PVector& p)					{ x = p.x; y = p.y; return *this; }
	inline PVector& operator+= (const PVector& p)					{ x += p.x; y += p.y; return *this; }
	inline PVector& operator-= (const PVector& p)					{ x -= p.x; y -= p.y; return *this; }
	inline PVector& operator*= (double t)							{ x *= t; y *= t; return *this; }
	inline PVector& operator/= (double t)							{ x /= t; y /= t; return *this; }
	
	inline double GetDistance (const PVector& v) const				{ return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y)); }
	inline double GetDot (const PVector& v) const					{ return x * v.x + y * v.y; }
	inline double GetLength () const								{ return sqrt(x * x + y * y); } // GetMagnitude
	
	inline PVector& Offset (const PVector& p)						{ x += p.x; y += p.y; return *this; }
	inline PVector& Offset (double x_, double y_)					{ x += x_; y += y_; return *this; }
	inline PVector& Normalize ()									{ double t = GetLength(); if(t) { x /= t; y /= t; } return *this; }
	inline PVector& Reflect (const PVector& n)						{ *this -= (n * ((double)2 * GetDot(n))); return *this; }
};

#endif // _P_VECTOR_H_
