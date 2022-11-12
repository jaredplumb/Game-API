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
	
	/// Returns the system rect in pixels, which is the entire screen, including hidden pixels behind curved corners and screen notches
	static GRect GetRect ();
	
	/// Returns the safe rect in pixels, which is the area with no obstructed visuals or interactions containing a minimum of the preferred size
	static GRect GetSafeRect ();
	
	/// Returns the preferred rect centered within the screen rect and then adjusted to fit within the safe rect, the preferred rect has a width and height always equal to the preferred size set in RunPreferredSize
	static GRect GetPreferredRect ();
	
	/// Returns the the current FPS.
	static int GetFPS ();
	
	/// Returns a new unique integer, per application session.
	static int GetUniqueRef ();
	
	/// Returns the startup time in milliseconds.
	static int64_t GetMilliseconds ();
	
	/// Returns the startup time in microseconds.
	static int64_t GetMicroseconds ();
	
	/// Returns the startup time in nanoseconds.
	static int64_t GetNanoseconds ();
	
	/// Sets the default working directory to the Resources directory.
	static void SetDefaultWD ();
	
	/// Returns the full path to the location to save data.
	static const GString& GetSaveDirectory ();
	
	/// Sets the preferred screen width and height, fps, or arg, must be called before calling Run(), some
	/// options will not work on all platforms
	static void RunPreferredSize (int width, int height);
	static void RunPreferredFPS (int fps);
	static void RunPreferredArgs (int argc, char* argv[]);
	
	/// Runs the core game system until finished (or in some cases indefinitely) returning the exit code
	static int Run ();
	
	// Typical usage is to set the matrix to default, then scale, then rotate, then translate, then update the matrix
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
	static int NewStartupCallback (void (* callback) ());
	static int NewShutdownCallback (void (* callback) ());
	static int NewDrawCallback (void (* callback) ()); // This is called "Draw" and not "Timer", because it is always the speed of the framerate, and drawing is only allowed in these callbacks
	static int NewTouchCallback (void (* callback) (int x, int y));
	static int NewTouchUpCallback (void (* callback) (int x, int y));
	static int NewTouchMoveCallback (void (* callback) (int x, int y));
	
	// Callbacks do not need to be deleted, they will automatically be deleted after shutdown
	static void DeleteStartupCallback (int ref);
	static void DeleteShutdownCallback (int ref);
	static void DeleteDrawCallback (int ref);
	static void DeleteTouchCallback (int ref);
	static void DeleteTouchUpCallback (int ref);
	static void DeleteTouchMoveCallback (int ref);
	static void DeleteAllCallbacks (); // Very dangerous to call, used internally
	
	// These should not be called without a good reason (they are used internally)
	static void RunStartupCallbacks ();
	static void RunShutdownCallbacks ();
	static void RunDrawCallbacks ();
	static void RunTouchCallbacks (int x, int y);
	static void RunTouchUpCallbacks (int x, int y);
	static void RunTouchMoveCallbacks (int x, int y);
};

#endif // _GSYSTEM_H_
