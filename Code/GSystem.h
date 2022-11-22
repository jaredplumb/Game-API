#ifndef _GSYSTEM_H_
#define _GSYSTEM_H_

#include "GTypes.h"
#include <map>

/// This class is used to access global data internally created to run a core game shell.  This
/// class internally contains everything needed to automatically launches and create a game
/// including hardware accelerated graphics and audio.  Main has deliberately been
/// moved from this class to allow for a full inclusion of this library when creating tools.
/// Several functions and all the callbacks will only work after calling Run(), which can be
/// ignored when working with tools.
/// The design of this class should avoid including any other library headers except GPlatform.h.

class GSystem {
public:
	/// Returns  the entire screen in pixels, including hidden pixels behind curved corners and screen notches
	static GRect GetScreenRect ();
	
	/// Returns the safe rect in pixels, which is the area with no obstructed visuals or interactions containing a minimum of the preferred size
	static GRect GetSafeRect ();
	
	/// Returns the preferred rect centered within the screen rect and then adjusted to fit within the safe rect, the preferred rect has a width and height always equal to the preferred size set in RunPreferredSize
	static GRect GetPreferredRect ();
	
	/// Returns the the current FPS.
	static int GetFPS ();
	
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
	
	/// Sets the preferred screen width and height, fps, or arg, must be called before calling Run(), these functions may do nothing on some platforms
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
	
	/// Returns a new unique integer, per application session.
	static inline int GetUniqueRef () {
		static int REF = 1;
		return REF++;
	}
	
	/// Prints a formatted string to the console.
	static inline void Print (const char* message, ...) {
		if (message) {
			va_list args;
			va_start(args, message);
			vprintf(message, args);
			va_end(args);
		}
	}
	
	/// Prints a formatted string to the console in debug builds only.
	static inline void Debug (const char* message, ...) {
#if DEBUG
		if (message) {
			va_list args;
			va_start(args, message);
			vprintf(message, args);
			va_end(args);
		}
#endif
	}
	
	static inline int NewStartupCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		_STARTUP_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewShutdownCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		_SHUTDOWN_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewDrawCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		_DRAW_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewTouchCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		_TOUCH_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewTouchUpCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		_TOUCHUP_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewTouchMoveCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		_TOUCHMOVE_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline void DeleteStartupCallback (int ref) {
		_STARTUP_CALLBACKS.erase(_STARTUP_CALLBACKS.find(ref));
	}
	
	static inline void DeleteShutdownCallback (int ref) {
		_SHUTDOWN_CALLBACKS.erase(_SHUTDOWN_CALLBACKS.find(ref));
	}
	
	static inline void DeleteDrawCallback (int ref) {
		_DRAW_CALLBACKS.erase(_DRAW_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchCallback (int ref) {
		_TOUCH_CALLBACKS.erase(_TOUCH_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchUpCallback (int ref) {
		_TOUCHUP_CALLBACKS.erase(_TOUCHUP_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchMoveCallback (int ref) {
		_TOUCHMOVE_CALLBACKS.erase(_TOUCHMOVE_CALLBACKS.find(ref));
	}
	
	static inline void RunStartupCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = _STARTUP_CALLBACKS.begin(); i != _STARTUP_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunShutdownCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = _SHUTDOWN_CALLBACKS.begin(); i != _SHUTDOWN_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunDrawCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = _DRAW_CALLBACKS.begin(); i != _DRAW_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunTouchCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCH_CALLBACKS.begin(); i != _TOUCH_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
	static inline void RunTouchUpCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCHUP_CALLBACKS.begin(); i != _TOUCHUP_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
	static inline void RunTouchMoveCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCHMOVE_CALLBACKS.begin(); i != _TOUCHMOVE_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
private:
	static inline std::map<int, void (*) ()>				_STARTUP_CALLBACKS;
	static inline std::map<int, void (*) ()>				_SHUTDOWN_CALLBACKS;
	static inline std::map<int, void (*) ()>				_DRAW_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	_TOUCH_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	_TOUCHUP_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	_TOUCHMOVE_CALLBACKS;
};

#endif // _GSYSTEM_H_
