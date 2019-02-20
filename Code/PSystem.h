#ifndef _P_SYSTEM_H_
#define _P_SYSTEM_H_

#include "GTypes.h"

/// This class is used to access global data internally created to run a core game shell.  This
/// class internally contains everything needed to automatically launches and create a game
/// including hardware accelerated graphics and audio.  Main has deliberately been
/// moved from this class to allow for a full inclusion of this library when creating tools.
/// Several functions and all the callbacks will only work after calling Run(), which can be
/// ignored when working with tools.
/// The design of this class should avoid including any other library headers except GPlatform.h.
class PSystem {
public:
	
	/// Prints a formatted string to the console.
	static void Print (const char* message, ...);
	
	/// Prints a formatted string to the console in debug builds only.
	static void Debug (const char* message, ...);
	
	/// Returns the width of the system area in pixels.
	static int_t GetWidth ();
	
	/// Returns the height of the system area in pixels.
	static int_t GetHeight ();
	
    /// Returns the "safe" width for interaction, mostly useful for mobile or console
    static int_t GetSafeWidth ();
    
    /// Returns the "safe" height for interaction, mostly useful for mobile or console
    static int_t GetSafeHeight ();
    
	/// Returns the the current FPS.
	static int_t GetFPS ();
	
	/// Returns a unique integer, per application session.
	static int_t GetUniqueRef ();
	
	/// Returns the startup time in milliseconds.
	static uint64 GetMilliseconds ();
	
	/// Returns the startup time in microseconds.
	static uint64 GetMicroseconds ();
	
	/// Returns the startup time in nanoseconds.
	static uint64 GetNanoseconds ();
	
	/// Returns a value from 0 to one minus range, unless range is 0, then the maximum uint32 size is used
	static uint32 GetRandom (uint32 range = 0);
	
	/// Sets the global random seed, returning the old seed, if seed is 0, then a random time value is used
	static uint32 SetRandomSeed (uint32 seed = 0);
	
	/// Returns a FNV-1 64-bit hash given the bytes
	static uint64 GetHash (const uint8* bytes);
	
	/// Sets the default working directory to the Resources directory.
	static void SetDefaultWD ();
	
	/// Sets the default screen widht and height, must be called before calling Run(), will not work on most platforms (do not use this function)
	static void SetDefaultScreenSize (int_t width, int_t height);
	
	/// Runs the core game system until finished (or in some cases indefinitely) returning the exit code
	static int_t Run (int_t argc, char* argv[]);
	
	// Typical usage is to set the matrix to default, then tranlate/scale/rotate, then update the matrix
	static void MatrixSetModelDefault ();		// Set model to identity
	static void MatrixSetProjectionDefault ();	// Set projection to an ortho 2D view
	static void MatrixTranslateModel (float x, float y);
	static void MatrixTranslateProjection (float x, float y);
	static void MatrixScaleModel (float x, float y);
	static void MatrixScaleProjection (float x, float y);
	static void MatrixRotateModel (float degrees);
	static void MatrixRotateProjection (float degrees);
	static void MatrixUpdate ();
	
	// Never assume these callbacks are called in any specific order
	static int_t NewStartupCallback (void (* callback) ());
	static int_t NewShutdownCallback (void (* callback) ());
	static int_t NewDrawCallback (void (* callback) ()); // This is called "Draw" and not "Timer", because it is always the speed of the framerate, and drawing is only allowed in these callbacks
	static int_t NewMouseCallback (void (* callback) (int_t x, int_t y, int_t button));
	static int_t NewMouseUpCallback (void (* callback) (int_t x, int_t y, int_t button));
	static int_t NewMouseMoveCallback (void (* callback) (int_t x, int_t y));
	static int_t NewMouseDragCallback (void (* callback) (int_t x, int_t y, int_t button));
	static int_t NewMouseWheelCallback(void(*callback) (float xdelta, float ydelta)); // >=1 or <= -1 for mouse wheel scrolling
	static int_t NewKeyCallback (void (* callback) (vkey_t key));
	static int_t NewKeyUpCallback (void (* callback) (vkey_t key));
	static int_t NewASCIICallback (void (* callback) (char key)); // ASCII events allow for autokey
	static int_t NewTouchCallback (void (* callback) (int_t x, int_t y));
	static int_t NewTouchUpCallback (void (* callback) (int_t x, int_t y));
	static int_t NewTouchMoveCallback (void (* callback) (int_t x, int_t y));
	
	// Callbacks do not need to be deleted, they will automatically be deleted after shutdown
	static void DeleteStartupCallback (int_t ref);
	static void DeleteShutdownCallback (int_t ref);
	static void DeleteDrawCallback (int_t ref);
	static void DeleteMouseCallback (int_t ref);
	static void DeleteMouseUpCallback (int_t ref);
	static void DeleteMouseMoveCallback (int_t ref);
	static void DeleteMouseDragCallback (int_t ref);
	static void DeleteMouseWheelCallback (int_t ref);
	static void DeleteKeyCallback (int_t ref);
	static void DeleteKeyUpCallback (int_t ref);
	static void DeleteASCIICallback (int_t ref);
	static void DeleteTouchCallback (int_t ref);
	static void DeleteTouchUpCallback (int_t ref);
	static void DeleteTouchMoveCallback (int_t ref);
	static void DeleteAllCallbacks (); // Very dangerous to call, used internally
	
	// These should not be called without a good reason (they are used internally)
	static void RunStartupCallbacks ();
	static void RunShutdownCallbacks ();
	static void RunDrawCallbacks ();
	static void RunMouseCallbacks (int_t x, int_t y, int_t button);
	static void RunMouseUpCallbacks (int_t x, int_t y, int_t button);
	static void RunMouseMoveCallbacks (int_t x, int_t y);
	static void RunMouseDragCallbacks (int_t x, int_t y, int_t button);
	static void RunMouseWheelCallbacks(float xdelta, float ydelta);
	static void RunKeyCallbacks (vkey_t key);
	static void RunKeyUpCallbacks (vkey_t key);
	static void RunASCIICallbacks (char key);
	static void RunTouchCallbacks (int_t x, int_t y);
	static void RunTouchUpCallbacks (int_t x, int_t y);
	static void RunTouchMoveCallbacks (int_t x, int_t y);
};

#endif // _P_SYSTEM_H_
