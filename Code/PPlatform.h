#ifndef _P_PLATFORM_H_
#define _P_PLATFORM_H_

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
#else
#error Unknown Platform
#endif

#if defined(G_PLATFORM_IOS)
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
#else
#error Unknown Hardware
#endif

#define ENDIAN_SWAP_16(x) ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8)
#define ENDIAN_SWAP_32(x) ((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >>  8) | ((x & 0x0000ff00) <<  8) | ((x & 0x000000ff) << 24)
#define ENDIAN_SWAP_64(x) ((x & 0xff00000000000000ULL) >> 56) | ((x & 0x00ff000000000000ULL) >> 40) | ((x & 0x0000ff0000000000ULL) >> 24) | ((x & 0x000000ff00000000ULL) >>  8) | ((x & 0x00000000ff000000ULL) <<  8) | ((x & 0x0000000000ff0000ULL) << 24) | ((x & 0x000000000000ff00ULL) << 40) | ((x & 0x00000000000000ffULL) << 56)

#if defined(_DEBUG) || defined(DEBUG)
#define DEBUG 1
#endif
/*
////////////////////////////////////////////////////////////////
#if PLATFORM_IOS
////////////////////////////////////////////////////////////////
// Core Objective-C imports
#ifdef __OBJC__
//#import <Foundation/Foundation.h>
//#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
//#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>
#endif

// Platform includes
//#include <CoreFoundation/CoreFoundation.h>
//#include <CFNetwork/CFNetwork.h>
#include <OpenGLES/ES2/gl.h>
//#include <OpenGLES/ES3/glext.h>
#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
//#include <OpenAL/oalStaticBufferExtension.h>

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
typedef int8_t				int8;
typedef uint8_t				uint8;
typedef int16_t				int16;
typedef uint16_t			uint16;
typedef int32_t				int32;
typedef uint32_t			uint32;
typedef int64_t				int64;
typedef uint64_t			uint64;

enum vkey_t {
	VKEY_NONE		= 0x0000,
	VKEY_LEFT		= 0x0001,
	VKEY_RIGHT		= 0x0002,
	VKEY_UP			= 0x0003,
	VKEY_DOWN		= 0x0004,
	VKEY_UNKNOWN	= 0xffff,
};

#endif
*/
////////////////////////////////////////////////////////////////
#if PLATFORM_MACOSX || PLATFORM_IOS
////////////////////////////////////////////////////////////////

// Core Objective-C imports
#ifdef __OBJC__
#if PLATFORM_IOS
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
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
typedef int8_t				int8;
typedef uint8_t				uint8;
typedef int16_t				int16;
typedef uint16_t			uint16;
typedef int32_t				int32;
typedef uint32_t			uint32;
typedef int64_t				int64;
typedef uint64_t			uint64;

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
typedef INT8				int8;
typedef UINT8				uint8;
typedef INT16				int16;
typedef UINT16				uint16;
typedef INT32				int32;
typedef UINT32				uint32;
typedef INT64				int64;
typedef UINT64				uint64;

enum vkey_t {
	VKEY_NONE		= 0x0000,
	VKEY_LEFT		= VK_LEFT,
	VKEY_RIGHT		= VK_RIGHT,
	VKEY_UP			= VK_UP,
	VKEY_DOWN		= VK_DOWN,
	VKEY_UNKNOWN	= 0xffff,
};

#endif

#endif // _P_PLATFORM_H_
