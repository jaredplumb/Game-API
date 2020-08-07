#ifndef _GTYPES_H_
#define _GTYPES_H_

// The idea behind this file is that all core Computer Science topics are included for a strong
// foundation to any program.  This includes basic variable definitions, file handling, basic
// threading, and even compression.  If all programs benefit from the functionality, it should
// be added to this file.

// The follwing macros may exist:
// DEBUG
// ENDIAN_BIG
// ENDIAN_LITTLE
// HARDWARE_ARM
// HARDWARE_PPC
// HARDWARE_X86
// HARDWARD_X64
// PLATFORM_IOS
// PLATFORM_MACOSX
// PLATFORM_WINDOWS

#if defined(__GNUC__) && (defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__))
#ifdef __OBJC__
#import <CoreFoundation/CoreFoundation.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif
#if TARGET_OS_IPHONE
#define PLATFORM_IOS 1
#else
#define PLATFORM_MACOSX 1
#endif
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER) || defined(_WIN32)
#define PLATFORM_WINDOWS 1
#elif defined(__EMSCRIPTEN__)
#define PLATFORM_WEB 1
#else
#error Unknown Platform
#endif

#if defined(PLATFORM_IOS)
#ifdef __arm__
#define HARDWARE_ARM 1
#define ENDIAN_LITTLE 1
#else
#define HARDWARE_X86 1
#define ENDIAN_LITTLE 1
#endif
#elif defined(__ppc__) || defined(ppc) || defined(powerpc) || defined(_M_MPPC)
#define HARDWARE_PPC 1
#define ENDIAN_BIG 1
#elif defined(_M_PPC)
#define HARDWARE_PPC 1
#define ENDIAN_LITTLE 1
#elif defined(__x86_64__)
#define HARDWARD_X64 1
#define ENDIAN_LITTLE 1
#elif defined(__i386__) || defined(i386) || defined(intel) || defined(_M_IX86)
#define HARDWARE_X86 1
#define ENDIAN_LITTLE 1
#elif defined(PLATFORM_WEB)
#define HARDWARE_UNDEFINED 1
#define ENDIAN_LITTLE 1
#else
#error Unknown Hardware
#endif

#define ENDIAN_SWAP_16(x) ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8)
#define ENDIAN_SWAP_32(x) ((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >>  8) | ((x & 0x0000ff00) <<  8) | ((x & 0x000000ff) << 24)
#define ENDIAN_SWAP_64(x) ((x & 0xff00000000000000ULL) >> 56) | ((x & 0x00ff000000000000ULL) >> 40) | ((x & 0x0000ff0000000000ULL) >> 24) | ((x & 0x000000ff00000000ULL) >>  8) | ((x & 0x00000000ff000000ULL) <<  8) | ((x & 0x0000000000ff0000ULL) << 24) | ((x & 0x000000000000ff00ULL) << 40) | ((x & 0x00000000000000ffULL) << 56)

#if defined(_DEBUG) || defined(DEBUG)
#define DEBUG 1
#endif

////////////////////////////////////////////////////////////////
#if PLATFORM_MACOSX || PLATFORM_IOS
////////////////////////////////////////////////////////////////

// Core Objective-C imports
#ifdef __OBJC__
#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
//#import <GLKit/GLKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <AVFoundation/AVFoundation.h>
#endif

// C includes
#include <dirent.h>
#include <pthread.h>

// C++ includes
#include <list>
#include <map>
#include <vector>

// Base data types
typedef ssize_t				int_t;
typedef size_t				uint_t;
typedef float_t				float_t;
typedef int8_t				int8;
typedef uint8_t				uint8;
typedef int16_t				int16;
typedef uint16_t			uint16;
typedef int32_t				int32;
typedef uint32_t			uint32;
typedef int64_t				int64;
typedef uint64_t			uint64;
typedef int8				bool8;

enum vkey_t {
	VKEY_NONE		= 0x0000,
	VKEY_LEFT		= 0x007b, // kVK_LeftArrow
	VKEY_RIGHT		= 0x007c, // kVK_RightArrow
	VKEY_UP			= 0x007d, // kVK_DownArrow
	VKEY_DOWN		= 0x007e, // kVK_UpArrow
	VKEY_UNKNOWN	= 0xffff,
};

#endif

////////////////////////////////////////////////////////////////
#if PLATFORM_WINDOWS
////////////////////////////////////////////////////////////////

// Linked Libraries
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "XmlLite.lib")

// Platform includes
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <gdiplus.h>

// C includes
//#include <stdio.h>
//#include <conio.h>		// _getch
//#include <direct.h>		// _getcwd
//#include <math.h>
#include <io.h>				// _finddata_t, _findfirst, and _findclose
#include <atlbase.h>
#include <Xmllite.h>

// C++ includes
#include <list>
#include <map>
#include <vector>

// Base data types
typedef SSIZE_T				int_t;
typedef SIZE_T				uint_t;
typedef float_t				float_t;
typedef INT8				int8;
typedef UINT8				uint8;
typedef INT16				int16;
typedef UINT16				uint16;
typedef INT32				int32;
typedef UINT32				uint32;
typedef INT64				int64;
typedef UINT64				uint64;
typedef int8				bool8;

enum vkey_t {
	VKEY_NONE		= 0x0000,
	VKEY_LEFT		= VK_LEFT,
	VKEY_RIGHT		= VK_RIGHT,
	VKEY_UP			= VK_UP,
	VKEY_DOWN		= VK_DOWN,
	VKEY_UNKNOWN	= 0xffff,
};

#endif

////////////////////////////////////////////////////////////////
#if PLATFORM_WEB
////////////////////////////////////////////////////////////////

// C includes
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>

// C++ includes
#include <list>
#include <map>
#include <vector>

// Core imports
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// Base data types
typedef ssize_t				int_t;
typedef size_t				uint_t;
typedef float_t				float_t;
typedef int8_t				int8;
typedef uint8_t				uint8;
typedef int16_t				int16;
typedef uint16_t			uint16;
typedef int32_t				int32;
typedef uint32_t			uint32;
typedef int64_t				int64;
typedef uint64_t			uint64;
typedef int8				bool8;

enum vkey_t {
	VKEY_NONE		= 0x0000,
	VKEY_LEFT		= 0x007b, // kVK_LeftArrow
	VKEY_RIGHT		= 0x007c, // kVK_RightArrow
	VKEY_UP			= 0x007d, // kVK_DownArrow
	VKEY_DOWN		= 0x007e, // kVK_UpArrow
	VKEY_UNKNOWN	= 0xffff,
};

#endif






class GColor {
public:
	/// RGBA format
	uint32 color;
	
	inline GColor ()												: color(0) {}
	inline GColor (const GColor& c)									: color(c.color) {}
	inline GColor (uint32 hex)										: color(hex) {}
	inline GColor (uint8 r, uint8 g, uint8 b, uint8 a = 0xff)		: color((a) | (b << 8) | (g << 16) | (r << 24)) {}
	inline GColor (float r, float g, float b, float a = 1.0f)		: color((uint8)(255.0f * a) | ((uint8)(255.0f * b) << 8) | ((uint8)(255.0f * g) << 16) | ((uint8)(255.0f * r) << 24)) {}
	inline uint8 GetRed () const									{ return (uint8)((color & 0xff000000) >> 24); }
	inline uint8 GetGreen () const									{ return (uint8)((color & 0x00ff0000) >> 16); }
	inline uint8 GetBlue () const									{ return (uint8)((color & 0x0000ff00) >> 8); }
	inline uint8 GetAlpha () const									{ return (uint8)(color & 0x000000ff); }
	inline float GetRedF () const									{ return (float)((uint8)((color & 0xff000000) >> 24) / 255.0f); }
	inline float GetGreenF () const									{ return (float)((uint8)((color & 0x00ff0000) >> 16) / 255.0f); }
	inline float GetBlueF () const									{ return (float)((uint8)((color & 0x0000ff00) >> 8) / 255.0f); }
	inline float GetAlphaF () const									{ return (float)((uint8)(color & 0x000000ff) / 255.0f); }
	inline void SetRed (uint8 r)									{ color = (color & 0x00ffffff) | (r << 24); }
	inline void SetGreen (uint8 g)									{ color = (color & 0xff00ffff) | (g << 16); }
	inline void SetBlue (uint8 b)									{ color = (color & 0xffff00ff) | (b << 8); }
	inline void SetAlpha (uint8 a)									{ color = (color & 0xffffff00) | (a); }
	inline bool operator== (const GColor& c) const					{ return color == c.color; }
	inline bool operator!= (const GColor& c) const					{ return color != c.color; }
	inline GColor& operator= (uint32 hex)							{ color = hex; return *this; }
	
	static const uint32 BLACK = 0x000000ff;
	static const uint32 WHITE = 0xffffffff;
	static const uint32 CLEAR = 0x00000000;
};







class GPoint {
public:
	int_t x, y;
	
	inline GPoint ()												: x(0), y(0) {}
	inline GPoint (const GPoint& p)									: x(p.x), y(p.y) {}
	inline GPoint (int_t x_, int_t y_)								: x(x_), y(y_) {}
	inline GPoint& Set (const GPoint& p)							{ x = p.x; y = p.y; return *this; }
	inline GPoint& Set (int_t x_, int_t y_)							{ x = x_; y = y_; return *this; }
	inline GPoint& Offset (const GPoint& p)							{ x += p.x; y += p.y; return *this; }
	inline GPoint& Offset (int_t x_, int_t y_)						{ x += x_; y += y_; return *this; }
	inline bool operator== (const GPoint& p) const					{ return x == p.x && y == p.y; }
	inline bool operator!= (const GPoint& p) const					{ return x != p.x || y != p.y; }
	inline const GPoint operator+ (const GPoint& p) const			{ return GPoint(x + p.x, y + p.y); }
	inline const GPoint operator- () const							{ return GPoint(-x, -y); }
	inline const GPoint operator- (const GPoint& p) const			{ return GPoint(x - p.x, y - p.y); }
	inline const GPoint operator* (int_t t) const					{ return GPoint(x * t, y * t); }
	inline const GPoint operator/ (int_t t) const					{ return GPoint(x / t, y / t); }
	inline GPoint& operator= (const GPoint& p)						{ x = p.x; y = p.y; return *this; }
	inline GPoint& operator+= (const GPoint& p)						{ x += p.x; y += p.y; return *this; }
	inline GPoint& operator-= (const GPoint& p)						{ x -= p.x; y -= p.y; return *this; }
	inline GPoint& operator*= (int_t t)								{ x *= t; y *= t; return *this; }
	inline GPoint& operator/= (int_t t)								{ x /= t; y /= t; return *this; }
};



class GSize {
public:
	int_t width, height;
	
	inline GSize ()													: width(0), height(0) {}
	inline GSize (const GSize& s)									: width(s.width), height(s.height) {}
	inline GSize (int_t width_, int_t height_)						: width(width_), height(height_) {}
	inline GSize& Set (const GSize& s)								{ width = s.width; height = s.height; return *this; }
	inline GSize& Set (int_t width_, int_t height_)					{ width = width_; height = height_; return *this; }
	inline bool operator== (const GSize& s) const					{ return width == s.width && height == s.height; }
	inline bool operator!= (const GSize& s) const					{ return width != s.width || height != s.height; }
};






class GVector {
public:
	float_t x, y;
	
	inline GVector ()												: x((float_t)0), y((float_t)0) {}
	inline GVector (const GVector& p)								: x(p.x), y(p.y) {}
	inline GVector (float_t x_, float_t y_)							: x(x_), y(y_) {}
	inline bool operator== (const GVector& p) const					{ return x == p.x && y == p.y; }
	inline bool operator!= (const GVector& p) const					{ return x != p.x || y != p.y; }
	inline const GVector operator+ (const GVector& p) const			{ return GVector(x + p.x, y + p.y); }
	inline const GVector operator- () const							{ return GVector(-x, -y); }
	inline const GVector operator- (const GVector& p) const			{ return GVector(x - p.x, y - p.y); }
	inline const GVector operator* (float_t t) const				{ return GVector(x * t, y * t); }
	inline const GVector operator/ (float_t t) const				{ return GVector(x / t, y / t); }
	inline GVector& operator= (const GVector& p)					{ x = p.x; y = p.y; return *this; }
	inline GVector& operator+= (const GVector& p)					{ x += p.x; y += p.y; return *this; }
	inline GVector& operator-= (const GVector& p)					{ x -= p.x; y -= p.y; return *this; }
	inline GVector& operator*= (float_t t)							{ x *= t; y *= t; return *this; }
	inline GVector& operator/= (float_t t)							{ x /= t; y /= t; return *this; }
	inline float_t GetDistance (const GVector& v) const				{ return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y)); }
	inline float_t GetDistance2 (const GVector& v) const			{ return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y); }
	inline float_t GetDot (const GVector& v) const					{ return x * v.x + y * v.y; }
	inline float_t GetMagnitude () const							{ return sqrt(x * x + y * y); }
	inline float_t GetMagnitude2 () const							{ return x * x + y * y; } // For those times when you don't need the sqrt
	inline float_t GetLength () const								{ return sqrt(x * x + y * y); } // Yes, Magnitue and Length are the same
	inline GVector& Offset (const GVector& p)						{ x += p.x; y += p.y; return *this; }
	inline GVector& Offset (float_t x_, float_t y_)					{ x += x_; y += y_; return *this; }
	inline GVector& Normalize ()									{ float_t t = GetLength(); if(t) { x /= t; y /= t; } return *this; }
	inline GVector& Reflect (const GVector& n)						{ *this -= (n * ((float_t)2 * GetDot(n))); return *this; }
};







class GRect {
public:
	int_t x, y, width, height;
	
	inline GRect ()															: x(0), y(0), width(0), height(0) {}
	inline GRect (const GRect& r)											: x(r.x) , y(r.y), width(r.width), height(r.height) {}
	inline GRect (int_t x_, int_t y_, int_t w, int_t h)						: x(x_), y(y_), width(w), height(h) {}
	inline int_t GetLeft () const											{ return x; }
	inline int_t GetRight () const											{ return x + width; }
	inline int_t GetTop () const											{ return y; }
	inline int_t GetBottom () const											{ return y + height; }
	inline bool IsPointInRect (const GPoint& p) const						{ return p.x >= x && p.y >= y && p.x <= (x + width) && p.y <= (y + height); }
	inline bool IsPointInRect (int_t x_, int_t y_) const					{ return x_ >= x && y_ >= y && x_ <= (x + width) && y_ <= (y + height); }
	inline bool IsCollision (const GRect& r) const							{ return x < (r.x + r.width) && y < (r.y + r.height) && (x + width) > r.x && (y + height) > r.y; }
	inline bool IsCollision (int_t x_, int_t y_, int_t w, int_t h) const	{ return x < (x_ + w) && y < (y_ + h) && (x + width) > x_ && (y + height) > y_; }
	inline GRect& SetLoc (const GPoint& p)									{ x = p.x; y = p.y; return *this; }
	inline GRect& SetLoc (int_t x_, int_t y_)								{ x = x_; y = y_; return *this; }
	inline GRect& SetSize (const GPoint& s)									{ width = s.x; height = s.y; return *this; }
	inline GRect& SetSize (int_t w, int_t h)								{ width = w; height = h; return *this; }
	inline GRect& Center (const GRect& r)									{ x = r.x + (r.width - width) / 2; y = r.y + (r.height - height) / 2; return *this; }
	inline GRect& Center (int_t x_, int_t y_, int_t w, int_t h)				{ x = x_ + (w - width) / 2; y = y_ + (h - height) / 2; return *this; }
	inline GRect& Offset (const GPoint& p)									{ x += p.x; y += p.y; return *this; }
	inline GRect& Offset (int_t x_, int_t y_)								{ x += x_; y += y_; return *this; }
	inline bool operator== (const GRect& r) const							{ return x == r.x && y == r.y && width == r.width && height == r.height; }
	inline bool operator!= (const GRect& r) const							{ return x != r.x || y != r.y || width != r.width || height != r.height; }
};








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
	int_t GetLength () const;
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
	static int_t strcmp (const char* s1, const char* s2);
	static char* strcpy (char* dst, const char* src);
	static int_t stricmp (const char* s1, const char* s2);
	static char* stristr (const char* s, const char* find);
	static int_t strlen (const char* s);
	static char* strncat (char* dst, const char* src, int_t len);
	static int_t strncmp (const char* s1, const char* s2, int_t len);
	static char* strncpy (char* dst, const char* src, int_t len);
	static int_t strnicmp (const char* s1, const char* s2, int_t len);
	static char* strnistr (const char* s, const char* find, int_t len);
	static char* strnstr (const char* s, const char* find, int_t len);
	static char* strstr (const char* s, const char* find);
	
	// Finds the first accurance of find in s, and returns a pointer to the first character after, or NULL if find failed
	static char* strnnext (const char* s, const char* find, int_t len);
	static char* strnext (const char* s, const char* find);
	static char* strninext (const char* s, const char* find, int_t len);
	static char* strinext (const char* s, const char* find);
	
	static int_t strtoi (const char* s, char** end, int_t base); // Returns an int_t of the string, with an optional end pointer and base
	// strtod https://opensource.apple.com/source/tcl/tcl-10/tcl/compat/strtod.c
	static char tolower (char c);
	static char* tolower (char* s);
	static char toupper (char c);
	static char* toupper (char* s);
	
	// Operator overloads
	inline operator char* (void)									{ _length = 0; return _string; }
	inline operator const char* (void) const						{ *(const_cast<int_t*>(&_length)) = 0; return _string; }
	inline char& operator[] (int_t index)							{ _length = 0; return _string[index]; }
	inline const char& operator[] (int_t index) const				{ return _string[index]; }
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
	int_t _length;
};











/// A GFile is a wrapper class around a C-File function calls.  This class uses the
/// native width versions of the file functions allowing for very large files.
class GFile {
public:
	
	GFile ();
	~GFile ();
	
	/// This constructor calls OpenForRead
	GFile (const GString& path);
	
	/// Opens a file for reading only
	bool OpenForRead (const GString& path);
	
	/// Opens a file for reading and writing
	bool OpenForWrite (const GString& path);
	
	/// Appends a file for reading and writing
	bool OpenForAppend (const GString& path);
	
	/// Closes this file
	void Close ();
	
	/// Read data from this file
	bool Read (void* data, uint_t size);
	
	/// Write data to this file
	bool Write (const void* data, uint_t size);
	
	/// Is this file currently open
	bool IsOpen () const;
	
	/// Returns the current position for reading and writing in the file
	uint_t GetPosition () const;
	
	/// Returns the size of the file
	uint_t GetSize () const;
	
	/// Sets the reading and writing position in the file
	bool SetPosition (uint_t position);
	
private:
	FILE* _file;
};








class GDirectory {
public:
	GDirectory ();
	~GDirectory ();
	GDirectory (const GString& path);
	bool Open (const GString& path);
	void Close ();
	uint_t GetSize () const;
	GString GetFile (uint_t index) const;
private:
	std::vector<GString> _files; // Files are relative to the path used with Open
};







class GThread {
public:
	inline GThread ()											: _thread(NULL) {}
	virtual ~GThread ()											{ Finish(); }
	virtual void Run () = 0;
#if PLATFORM_WINDOWS
	inline void Start ()										{ Finish(); _thread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL); }
	inline void Finish ()										{ if(_thread) { WaitForSingleObject(_thread, INFINITE); CloseHandle(_thread); _thread = NULL; } }
	inline void Sleep (uint64 milliseconds)						{ Sleep(milliseconds); }
private:
	HANDLE _thread;
	static DWORD WINAPI _ThreadProc (LPVOID lpParam)			{ ((GThread *)lpParam)->Run(); return 0; }
#else
	inline void Start ()										{ Finish(); pthread_create(&_thread, NULL, _ThreadProc, this); }
	inline void Finish ()										{ if(_thread) { pthread_join(_thread, NULL); _thread = NULL; } }
	inline void Sleep (uint64 milliseconds)						{ timespec time = { (long)(milliseconds / 1000), (long)(milliseconds % 1000 * 1000000) }; nanosleep(&time, NULL); }
private:
	pthread_t _thread;
	static void* _ThreadProc (void* data)                       { ((GThread *)data)->Run(); return NULL; }
#endif
};




class GMutex {
public:
#if PLATFORM_WINDOWS
	inline GMutex ()											: _mutex(CreateMutex(NULL, FALSE, NULL)) {}
	inline ~GMutex ()											{ if (_mutex) { CloseHandle(_mutex); _mutex = NULL; } }
	inline bool Lock ()											{ return WaitForSingleObject(_mutex, INFINITE) == WAIT_OBJECT_0; }
	inline bool Unlock ()										{ return ReleaseMutex(_mutex) == TRUE; }
private:
	HANDLE _mutex;
#else
	inline GMutex ()											: _mutex(new pthread_mutex_t) { pthread_mutex_init(_mutex, NULL); }
	inline ~GMutex ()											{ if(_mutex) { pthread_mutex_destroy(_mutex); delete _mutex; _mutex = NULL; } }
	inline bool Lock ()											{ return pthread_mutex_lock(_mutex) == 0; }
	inline bool Unlock ()										{ return pthread_mutex_unlock(_mutex) == 0; }
private:
	pthread_mutex_t* _mutex;
#endif
};







class GArchive {
public:
	static const uint8 VERSION = 4;
	static uint_t Compress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize);
	static uint_t Decompress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize);
	static uint_t GetBufferBounds (uint_t srcSize);
};







class GConsole {
public:
	
	/// Prints a formatted string to the console.
	static void Print (const char* message, ...);
	
	/// Prints a formatted string to the console in debug builds only.
	static void Debug (const char* message, ...);
	
};








#endif // _GTYPES_H_
