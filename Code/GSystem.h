#ifndef _GSYSTEM_H_
#define _GSYSTEM_H_

#include "GTypes.h"

/// This class is used to access global data internally created to run a core game shell.  This
/// class internally contains everything needed to automatically launches and create a game
/// including hardware accelerated graphics and audio.  Main has deliberately been
/// moved from this class to allow for a full inclusion of this library when creating tools.
/// Several functions and all the callbacks will only work after calling Run(), which can be
/// ignored when working with tools.
/// The design of this class should avoid including any other library headers except GPlatform.h.

class GSystem {
public:
	
	/// Returns the system rect in pixels
	static GRect GetRect ();
	
	/// Returns the safe rect in pixels, which is the area with no obstructed visuals or interactions
	static GRect GetSafeRect ();
	
	/// Returns the the current FPS.
	static int_t GetFPS ();
	
	/// Returns a new unique integer, per application session.
	static int_t GetUniqueRef ();
	
	/// Returns the startup time in milliseconds.
	static uint64 GetMilliseconds ();
	
	/// Returns the startup time in microseconds.
	static uint64 GetMicroseconds ();
	
	/// Returns the startup time in nanoseconds.
	static uint64 GetNanoseconds ();
	
	/// Sets the default working directory to the Resources directory.
	static void SetDefaultWD ();
	
	/// Sets the preferred screen width and height, fps, or arg, must be called before calling Run(), some
	/// options will not work on all platforms
	static void RunPreferredSize (int_t width, int_t height);
	static void RunPreferredFPS (int_t fps);
	static void RunPreferredArgs (int_t argc, char* argv[]);
	
	/// Runs the core game system until finished (or in some cases indefinitely) returning the exit code
	static int_t Run ();
	
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
	static int_t NewEventCallback (void (* callback) (int_t event, void* data));
	
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
	static void DeleteEventCallback (int_t ref);
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
	static void RunEventCallbacks (int_t event, void* data);
};

#endif // _GSYSTEM_H_
