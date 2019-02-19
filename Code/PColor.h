#ifndef _P_COLOR_H_
#define _P_COLOR_H_

#include "PPlatform.h"

class PColor {
public:
	/// RGBA format
	uint32 color;
	
	inline PColor ()														: color(0) {}
	inline PColor (const PColor& c)											: color(c.color) {}
	inline PColor (uint32 hex)												: color(hex) {}
	inline PColor (uint8 r, uint8 g, uint8 b, uint8 a = 0xff)				: color((a) | (b << 8) | (g << 16) | (r << 24)) {}
	inline PColor (float r, float g, float b, float a = 1.0f)				: color((uint8)(255.0f * a) | ((uint8)(255.0f * b) << 8) | ((uint8)(255.0f * g) << 16) | ((uint8)(255.0f * r) << 24)) {}
	inline uint8 GetRed () const											{ return (uint8)((color & 0xff000000) >> 24); }
	inline uint8 GetGreen () const											{ return (uint8)((color & 0x00ff0000) >> 16); }
	inline uint8 GetBlue () const											{ return (uint8)((color & 0x0000ff00) >> 8); }
	inline uint8 GetAlpha () const											{ return (uint8)(color & 0x000000ff); }
	inline float GetRedF () const											{ return (float)((uint8)((color & 0xff000000) >> 24) / 255.0f); }
	inline float GetGreenF () const											{ return (float)((uint8)((color & 0x00ff0000) >> 16) / 255.0f); }
	inline float GetBlueF () const											{ return (float)((uint8)((color & 0x0000ff00) >> 8) / 255.0f); }
	inline float GetAlphaF () const											{ return (float)((uint8)(color & 0x000000ff) / 255.0f); }
	inline void SetRed (uint8 r)											{ color = (color & 0x00ffffff) | (r << 24); }
	inline void SetGreen (uint8 g)											{ color = (color & 0xff00ffff) | (g << 16); }
	inline void SetBlue (uint8 b)											{ color = (color & 0xffff00ff) | (b << 8); }
	inline void SetAlpha (uint8 a)											{ color = (color & 0xffffff00) | (a); }
	inline bool operator== (const PColor& c) const							{ return color == c.color; }
	inline bool operator!= (const PColor& c) const							{ return color != c.color; }
	inline PColor& operator= (uint32 hex)									{ color = hex; return *this; }
	
	static const uint32 BLACK = 0x000000ff;
	static const uint32 WHITE = 0xffffffff;
};

#endif // _P_COLOR_H_
