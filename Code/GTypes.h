#ifndef GTYPES_H_
#define GTYPES_H_

#include <cstdint>
#include <cmath>
#include <vector>


class GColor {
public:
	/// RGBA format
	uint32_t color;
	
	inline GColor ()													: color(0) {}
	inline GColor (const GColor& c)										: color(c.color) {}
	inline GColor (uint32_t hex)										: color(hex) {}
	inline GColor (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)	: color((a) | (b << 8) | (g << 16) | (r << 24)) {}
	inline GColor (float r, float g, float b, float a = 1.0f)			: color((uint8_t)(255.0f * a) | ((uint8_t)(255.0f * b) << 8) | ((uint8_t)(255.0f * g) << 16) | ((uint8_t)(255.0f * r) << 24)) {}
	inline uint8_t GetRed () const										{ return (uint8_t)((color & 0xff000000) >> 24); }
	inline uint8_t GetGreen () const									{ return (uint8_t)((color & 0x00ff0000) >> 16); }
	inline uint8_t GetBlue () const										{ return (uint8_t)((color & 0x0000ff00) >> 8); }
	inline uint8_t GetAlpha () const									{ return (uint8_t)(color & 0x000000ff); }
	inline float GetRedF () const										{ return (float)((uint8_t)((color & 0xff000000) >> 24) / 255.0f); }
	inline float GetGreenF () const										{ return (float)((uint8_t)((color & 0x00ff0000) >> 16) / 255.0f); }
	inline float GetBlueF () const										{ return (float)((uint8_t)((color & 0x0000ff00) >> 8) / 255.0f); }
	inline float GetAlphaF () const										{ return (float)((uint8_t)(color & 0x000000ff) / 255.0f); }
	inline void SetRed (uint8_t r)										{ color = (color & 0x00ffffff) | (r << 24); }
	inline void SetGreen (uint8_t g)									{ color = (color & 0xff00ffff) | (g << 16); }
	inline void SetBlue (uint8_t b)										{ color = (color & 0xffff00ff) | (b << 8); }
	inline void SetAlpha (uint8_t a)									{ color = (color & 0xffffff00) | (a); }
	inline bool operator== (const GColor& c) const						{ return color == c.color; }
	inline bool operator!= (const GColor& c) const						{ return color != c.color; }
	inline GColor& operator= (uint32_t hex)								{ color = hex; return *this; }
	
	static const uint32_t BLACK = 0x000000ff;
	static const uint32_t WHITE = 0xffffffff;
	static const uint32_t CLEAR = 0x00000000;
	static const uint32_t RED = 0xff000000;
	static const uint32_t GREEN = 0x00ff0000;
	static const uint32_t BLUE = 0x0000ff00;
	static const uint32_t ALPHA = 0x000000ff;
};


class GPoint {
public:
	int x, y;
	
	inline GPoint ()											: x(0), y(0) {}
	inline GPoint (const GPoint& p)								: x(p.x), y(p.y) {}
	inline GPoint (int x_, int y_)								: x(x_), y(y_) {}
	inline GPoint& Set (const GPoint& p)						{ x = p.x; y = p.y; return *this; }
	inline GPoint& Set (int x_, int y_)							{ x = x_; y = y_; return *this; }
	inline GPoint& Offset (const GPoint& p)						{ x += p.x; y += p.y; return *this; }
	inline GPoint& Offset (int x_, int y_)						{ x += x_; y += y_; return *this; }
	inline bool operator== (const GPoint& p) const				{ return x == p.x && y == p.y; }
	inline bool operator!= (const GPoint& p) const				{ return x != p.x || y != p.y; }
	inline const GPoint operator+ (const GPoint& p) const		{ return GPoint(x + p.x, y + p.y); }
	inline const GPoint operator- () const						{ return GPoint(-x, -y); }
	inline const GPoint operator- (const GPoint& p) const		{ return GPoint(x - p.x, y - p.y); }
	inline const GPoint operator* (int t) const					{ return GPoint(x * t, y * t); }
	inline const GPoint operator/ (int t) const					{ return GPoint(x / t, y / t); }
	inline GPoint& operator= (const GPoint& p)					{ x = p.x; y = p.y; return *this; }
	inline GPoint& operator+= (const GPoint& p)					{ x += p.x; y += p.y; return *this; }
	inline GPoint& operator-= (const GPoint& p)					{ x -= p.x; y -= p.y; return *this; }
	inline GPoint& operator*= (int t)							{ x *= t; y *= t; return *this; }
	inline GPoint& operator/= (int t)							{ x /= t; y /= t; return *this; }
};


class GSize {
public:
	int width, height;
	
	inline GSize ()												: width(0), height(0) {}
	inline GSize (const GSize& s)								: width(s.width), height(s.height) {}
	inline GSize (int width_, int height_)						: width(width_), height(height_) {}
	inline GSize& Set (const GSize& s)							{ width = s.width; height = s.height; return *this; }
	inline GSize& Set (int width_, int height_)					{ width = width_; height = height_; return *this; }
	inline bool operator== (const GSize& s) const				{ return width == s.width && height == s.height; }
	inline bool operator!= (const GSize& s) const				{ return width != s.width || height != s.height; }
};


class GVector {
public:
	float x, y;
	
	inline GVector ()												: x((float)0), y((float)0) {}
	inline GVector (const GVector& p)								: x(p.x), y(p.y) {}
	inline GVector (float x_, float y_)								: x(x_), y(y_) {}
	inline bool operator== (const GVector& p) const					{ return x == p.x && y == p.y; }
	inline bool operator!= (const GVector& p) const					{ return x != p.x || y != p.y; }
	inline const GVector operator+ (const GVector& p) const			{ return GVector(x + p.x, y + p.y); }
	inline const GVector operator- () const							{ return GVector(-x, -y); }
	inline const GVector operator- (const GVector& p) const			{ return GVector(x - p.x, y - p.y); }
	inline const GVector operator* (float t) const					{ return GVector(x * t, y * t); }
	inline const GVector operator/ (float t) const					{ return GVector(x / t, y / t); }
	inline GVector& operator= (const GVector& p)					{ x = p.x; y = p.y; return *this; }
	inline GVector& operator+= (const GVector& p)					{ x += p.x; y += p.y; return *this; }
	inline GVector& operator-= (const GVector& p)					{ x -= p.x; y -= p.y; return *this; }
	inline GVector& operator*= (float t)							{ x *= t; y *= t; return *this; }
	inline GVector& operator/= (float t)							{ x /= t; y /= t; return *this; }
	inline float GetDistance (const GVector& v) const				{ return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y)); }
	inline float GetDistance2 (const GVector& v) const				{ return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y); }
	inline float GetDot (const GVector& v) const					{ return x * v.x + y * v.y; }
	inline float GetMagnitude () const								{ return sqrt(x * x + y * y); }
	inline float GetMagnitude2 () const								{ return x * x + y * y; } // For those times when you don't need the sqrt
	inline float GetLength () const									{ return sqrt(x * x + y * y); } // Yes, Magnitue and Length are the same
	inline GVector& Offset (const GVector& p)						{ x += p.x; y += p.y; return *this; }
	inline GVector& Offset (float x_, float y_)						{ x += x_; y += y_; return *this; }
	inline GVector& Normalize ()									{ float t = GetLength(); if(t) { x /= t; y /= t; } return *this; }
	inline GVector& Reflect (const GVector& n)						{ *this -= (n * ((float)2 * GetDot(n))); return *this; }
};


class GRect {
public:
	int x, y, width, height;
	
	inline GRect ()													: x(0), y(0), width(0), height(0) {}
	inline GRect (const GRect& r)									: x(r.x) , y(r.y), width(r.width), height(r.height) {}
	inline GRect (int x_, int y_, int w, int h)						: x(x_), y(y_), width(w), height(h) {}
	inline int GetLeft () const										{ return x; }
	inline int GetRight () const									{ return x + width; }
	inline int GetTop () const										{ return y; }
	inline int GetBottom () const									{ return y + height; }
	inline bool IsPointInRect (const GPoint& p) const				{ return p.x >= x && p.y >= y && p.x <= (x + width) && p.y <= (y + height); }
	inline bool IsPointInRect (int x_, int y_) const				{ return x_ >= x && y_ >= y && x_ <= (x + width) && y_ <= (y + height); }
	inline bool IsCollision (const GRect& r) const					{ return x < (r.x + r.width) && y < (r.y + r.height) && (x + width) > r.x && (y + height) > r.y; }
	inline bool IsCollision (int x_, int y_, int w, int h) const	{ return x < (x_ + w) && y < (y_ + h) && (x + width) > x_ && (y + height) > y_; }
	inline GRect& Set (int x_, int y_, int w, int h)				{ x = x_; y = y_; width = w; height = h; return *this; }
	inline GRect& SetLoc (const GPoint& p)							{ x = p.x; y = p.y; return *this; }
	inline GRect& SetLoc (int x_, int y_)							{ x = x_; y = y_; return *this; }
	inline GRect& SetSize (const GPoint& s)							{ width = s.x; height = s.y; return *this; }
	inline GRect& SetSize (int w, int h)							{ width = w; height = h; return *this; }
	inline GRect& Center (const GRect& r)							{ x = r.x + (r.width - width) / 2; y = r.y + (r.height - height) / 2; return *this; }
	inline GRect& Center (int x_, int y_, int w, int h)				{ x = x_ + (w - width) / 2; y = y_ + (h - height) / 2; return *this; }
	inline GRect& Offset (const GPoint& p)							{ x += p.x; y += p.y; return *this; }
	inline GRect& Offset (int x_, int y_)							{ x += x_; y += y_; return *this; }
	inline bool operator== (const GRect& r) const					{ return x == r.x && y == r.y && width == r.width && height == r.height; }
	inline bool operator!= (const GRect& r) const					{ return x != r.x || y != r.y || width != r.width || height != r.height; }
};


class GMatrix32_4x4 {
public:
	float numbers[4][4];
	
	// GMatrix deliberately does not have constructors to avoid speed hits
	inline void SetIdentity ()																			{ *this = (GMatrix32_4x4){{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}}; }
	inline void SetOrtho2D (float left, float right, float bottom, float top, float nearZ, float farZ)	{ *this = (GMatrix32_4x4){{{2.0f / (right - left), 0.0f, 0.0f, 0.0f}, {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f}, {(left + right) / (left - right), (top + bottom) / (bottom - top), nearZ / (nearZ - farZ), 1.0f}}}; }
	inline void SetTranslation (float tx, float ty, float tz)											{ *this = (GMatrix32_4x4){{{numbers[0][0], numbers[0][1], numbers[0][2], numbers[0][3]}, {numbers[1][0], numbers[1][1], numbers[1][2], numbers[1][3]}, {numbers[2][0], numbers[2][1], numbers[2][2], numbers[2][3]}, {numbers[0][0] * tx + numbers[1][0] * ty + numbers[2][0] * tz + numbers[3][0], numbers[0][1] * tx + numbers[1][1] * ty + numbers[2][1] * tz + numbers[3][1], numbers[0][2] * tx + numbers[1][2] * ty + numbers[2][2] * tz + numbers[3][2], numbers[0][3] * tx + numbers[1][3] * ty + numbers[2][3] * tz + numbers[3][3]}}}; }
	inline void SetScale (float sx, float sy, float sz)													{ *this = (GMatrix32_4x4){{{numbers[0][0] * sx + numbers[1][0] * sy + numbers[2][0] * sz + numbers[3][0], numbers[0][1], numbers[0][2], numbers[0][3]}, {numbers[1][0], numbers[0][1] * sx + numbers[1][1] * sy + numbers[2][1] * sz + numbers[3][1], numbers[1][2], numbers[1][3]}, {numbers[2][0], numbers[2][1], numbers[0][2] * sx + numbers[1][2] * sy + numbers[2][2] * sz + numbers[3][2], numbers[2][3]}, {numbers[3][0], numbers[3][1], numbers[3][2], numbers[3][3]}}}; }
	inline void SetRotation (float radians)																{ float s = (float)sinf(radians); float c = (float)cosf(radians); *this = (GMatrix32_4x4){{{numbers[0][0] * c + numbers[0][1] * (-s), numbers[0][0] * s + numbers[0][1] * c, numbers[0][2], numbers[0][3]}, {numbers[1][0] * c + numbers[1][1] * (-s), numbers[1][0] * s + numbers[1][1] * c, numbers[1][2], numbers[1][3]}, {numbers[2][0] * c + numbers[2][1] * (-s), numbers[2][0] * s + numbers[2][1] * c, numbers[2][2], numbers[2][3]}, {numbers[3][0] * c + numbers[3][1] * (-s), numbers[3][0] * s + numbers[3][1] * c, numbers[3][2], numbers[3][3]}}}; }
	inline const GMatrix32_4x4 operator* (GMatrix32_4x4 t) const										{ GMatrix32_4x4 r; for(int i = 0; i < 4; i++) { r.numbers[i][0] = 0; r.numbers[i][1] = 0; r.numbers[i][2] = 0; r.numbers[i][3] = 0; for(int j = 0; j < 4; j++) { r.numbers[i][0] = numbers[j][0] * t.numbers[i][j] + r.numbers[i][0]; r.numbers[i][1] = numbers[j][1] * t.numbers[i][j] + r.numbers[i][1]; r.numbers[i][2] = numbers[j][2] * t.numbers[i][j] + r.numbers[i][2]; r.numbers[i][3] = numbers[j][3] * t.numbers[i][j] + r.numbers[i][3]; } } return r; }
};


// Why use this class over a standard c++ string?  Standard c++ strings do a lot
// of copying and re-allocating which causes huge issues in games.  C strings are
// preferable, although somewhat more complex.  This class simplifies a lot of the
// c string management.
class GString {
public:
	GString ();
	GString (const GString& string);
	GString (const char* string);
	~GString ();
	GString& New (const GString& string);
	GString& New (const char* string);
	void Delete ();
	GString& Format (const char* string, ...);
	int GetLength () const;
	bool IsEmpty () const; // Returns true if the string is NULL or ""
	GString& Add (const GString& string);
	GString& Add (const char* string);
	GString& ToLower ();
	GString& ToUpper ();
	GString& TrimSpaces ();
	GString& TrimExtension (); // Removes the extension off of a file path
	GString& TrimToDirectory (); // Removes the file off of a file path
	
	// Standard C string overrides with modifications
	static bool isalnum (char c);
	static bool isalpha (char c);
	static bool isdigit (char c);
	static bool isgraph (char c);
	static bool islower (char c);
	static bool isprint (char c);
	static bool ispunct (char c);
	static bool isspace (char c);
	static bool isupper (char c);
	static bool isxdigit (char c);
	static char* strcat (char* dst, const char* src);
	static int strcmp (const char* s1, const char* s2);
	static char* strcpy (char* dst, const char* src);
	static int stricmp (const char* s1, const char* s2);
	static char* stristr (const char* s, const char* find);
	static int strlen (const char* s);
	static char* strncat (char* dst, const char* src, int len);
	static int strncmp (const char* s1, const char* s2, int len);
	static char* strncpy (char* dst, const char* src, int len);
	static int strnicmp (const char* s1, const char* s2, int len);
	static char* strnistr (const char* s, const char* find, int len);
	static char* strnstr (const char* s, const char* find, int len);
	static char* strstr (const char* s, const char* find);
	
	// Finds the first accurance of find in s, and returns a pointer to the first character after, or NULL if find failed
	static char* strnnext (const char* s, const char* find, int len);
	static char* strnext (const char* s, const char* find);
	static char* strninext (const char* s, const char* find, int len);
	static char* strinext (const char* s, const char* find);
	
	static int strtoi (const char* s, char** end, int base); // Returns an int_t of the string, with an optional end pointer and base
	// strtod https://opensource.apple.com/source/tcl/tcl-10/tcl/compat/strtod.c
	static char tolower (char c);
	static char* tolower (char* s);
	static char toupper (char c);
	static char* toupper (char* s);
	
	// Operator overloads
	inline operator char* (void)									{ _length = 0; return _string; }
	inline operator const char* (void) const						{ *(const_cast<int*>(&_length)) = 0; return _string; }
	inline char& operator[] (int index)								{ _length = 0; return _string[index]; }
	inline const char& operator[] (int index) const					{ return _string[index]; }
	inline GString& operator= (const GString& string)				{ return New(string); }
	inline GString& operator= (const char* string)					{ return New(string); }
	inline const GString operator+ (const GString& string) const	{ return GString(*this).Add(string); }
	inline const GString operator+ (const char* string) const		{ return GString(*this).Add(string); }
	inline GString& operator+= (const GString& string)				{ return Add(string); }
	inline GString& operator+= (const char* string)					{ return Add(string); }
	inline bool operator== (const GString& string) const			{ return strcmp(_string, string._string) == 0; }
	inline bool operator== (const char* string) const				{ return strcmp(_string, string) == 0; }
	inline bool operator!= (const GString& string) const			{ return strcmp(_string, string._string) != 0; }
	inline bool operator!= (const char* string) const				{ return strcmp(_string, string) != 0; }
	inline bool operator< (const GString& string) const				{ return strcmp(_string, string._string) < 0; }
	inline bool operator< (const char* string) const				{ return strcmp(_string, string) < 0; }
	inline bool operator<= (const GString& string) const			{ return strcmp(_string, string._string) <= 0; }
	inline bool operator<= (const char* string) const				{ return strcmp(_string, string) <= 0; }
	inline bool operator> (const GString& string) const				{ return strcmp(_string, string._string) > 0; }
	inline bool operator> (const char* string) const				{ return strcmp(_string, string) > 0; }
	inline bool operator>= (const GString& string) const			{ return strcmp(_string, string._string) >= 0; }
	inline bool operator>= (const char* string) const				{ return strcmp(_string, string) >= 0; }
	
private:
	char* _string;
	int _length;
};


/// A GFile is a wrapper class around C-File function calls.  This class uses the
/// native width versions of the file functions allowing for very large files.
class GFile {
public:
	GFile ();
	
	/// This constructor calls OpenForRead
	GFile (const GString& path);
	
	~GFile ();
	
	/// Opens a file for reading only
	bool OpenForRead (const GString& path);
	
	/// Opens a file for reading and writing
	bool OpenForWrite (const GString& path);
	
	/// Appends a file for reading and writing
	bool OpenForAppend (const GString& path);
	
	/// Closes this file
	void Close ();
	
	/// Read data from this file
	bool Read (void* data, int64_t size);
	
	/// Write data to this file
	bool Write (const void* data, int64_t size);
	
	/// Is this file currently open
	bool IsOpen () const;
	
	/// Returns the current position for reading and writing in the file
	int64_t GetPosition () const;
	
	/// Returns the size of the file
	int64_t GetSize () const;
	
	/// Sets the reading and writing position in the file
	bool SetPosition (int64_t position);
	
	/// Returns a list of file names in the directory relative to the path sent into the function and all sub directories
	static std::vector<GString> GetFileNamesInDirectory (const GString& path);
	
private:
	FILE* _file;
};


class GArchive {
public:
	static const uint8_t VERSION = 4;
	static int64_t Compress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize);
	static int64_t Decompress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize);
	static int64_t GetBufferBounds (int64_t srcSize);
};


class GConsole {
public:
	/// Prints a formatted string to the console.
	static void Print (const char* message, ...);
	
	/// Prints a formatted string to the console in debug builds only.
	static void Debug (const char* message, ...);
	
};


#endif // GTYPES_H_
