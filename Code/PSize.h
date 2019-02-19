#ifndef _P_SIZE_H_
#define _P_SIZE_H_

#include "PPlatform.h"

class PSize {
public:
	int_t width, height;
	
	inline PSize ()															: width(0), height(0) {}
	inline PSize (const PSize& s)											: width(s.width), height(s.height) {}
	inline PSize (int_t width_, int_t height_)								: width(width_), height(height_) {}
	inline bool operator== (const PSize& s) const							{ return width == s.width && height == s.height; }
	inline bool operator!= (const PSize& s) const							{ return width != s.width || height != s.height; }
};

#endif // _P_SIZE_H_
