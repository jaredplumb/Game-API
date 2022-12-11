#ifndef G_SYSTEM_H_
#define G_SYSTEM_H_

#include "GTypes.h"
#include <chrono>
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
	
	/// Returns the epoch time in milliseconds.
	static int64_t GetMilliseconds () { return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()); }
	
	/// Returns the epoch time in microseconds.
	static int64_t GetMicroseconds () { return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()); }
	
	/// Returns the epoch time in nanoseconds.
	static int64_t GetNanoseconds () { return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()); }
	
	
	
	
	
	
	
	
	
	
	
	static bool PackageOpen (const GString& resource = "Main.pkg");
	
	static bool PackageOpenForWrite (const GString& resource = "Main.pkg"); // Typically is not used and will fail on most platforms.  Used for tool creation.  Must call PackageCloseForWrite() to write the header.
	
	static bool PackageClose (); // Typically does not need to be called, the package data is automatically released at program termination.
	
	static bool PackageCloseForWrite (); // Must be called if package was opened with PackageOpenForWrite.
	
	static int64_t ResourceSize (const GString& name);
	
	static int64_t ResourceSizeFromFile (const GString& path);
	
	static bool ResourceRead (const GString& name, void* data, int64_t size);
	
	static bool ResourceReadFromFile (const GString& path, void* data, int64_t size);
	
	static bool ResourceWrite (const GString& name, void* data, int64_t size); // Will fail if package was not opened with PackageOpenForWrite.
	
	
	
	
	
	
	
	
	/// Read save data from the appropriate place
	static bool SaveRead (const GString& name, void* data, int64_t size);
	
	/// Write save data to the appropriate place
	static bool SaveWrite (const GString& name, const void* data, int64_t size);
	
	/// Returns a list of file names in the directory provided by the path including all sub directories
	static std::vector<GString> GetFileNamesInDirectory (const GString& path);
	
	/// Sets the preferred screen width and height, fps, or arg, must be called before calling Run(), these functions may do nothing on some platforms
	static void RunPreferredSize (int width, int height);
	static void RunPreferredFPS (int fps);
	static void RunPreferredArgs (int argc, char* argv[]);
	
	/// Runs the core game system until finished (or in some cases indefinitely) returning the exit code
	static int Run ();
	
	// Typical usage is to set the matrix to default, then scale, then rotate, then translate, then update the matrix
	static inline void MatrixSetModelDefault (); // Set model to identity
	static void MatrixSetProjectionDefault (); // Set projection to an ortho 2D view
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
	static void Print (const char* message, ...);
	
	/// Prints a formatted string to the console in debug builds only.
	static void Debug (const char* message, ...);
	
	static inline int NewStartupCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		STARTUP_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewShutdownCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		SHUTDOWN_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline int NewDrawCallback (void (* callback) ()) {
		int ref = GetUniqueRef();
		DRAW_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	// Touch locations are relative to the preferred rect
	static inline int NewTouchCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		TOUCH_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	// Touch locations are relative to the preferred rect
	static inline int NewTouchUpCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		TOUCHUP_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	// Touch locations are relative to the preferred rect
	static inline int NewTouchMoveCallback (void (* callback) (int x, int y)) {
		int ref = GetUniqueRef();
		TOUCHMOVE_CALLBACKS.insert(std::make_pair(ref, callback));
		return ref;
	}
	
	static inline void DeleteStartupCallback (int ref) {
		STARTUP_CALLBACKS.erase(STARTUP_CALLBACKS.find(ref));
	}
	
	static inline void DeleteShutdownCallback (int ref) {
		SHUTDOWN_CALLBACKS.erase(SHUTDOWN_CALLBACKS.find(ref));
	}
	
	static inline void DeleteDrawCallback (int ref) {
		DRAW_CALLBACKS.erase(DRAW_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchCallback (int ref) {
		TOUCH_CALLBACKS.erase(TOUCH_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchUpCallback (int ref) {
		TOUCHUP_CALLBACKS.erase(TOUCHUP_CALLBACKS.find(ref));
	}
	
	static inline void DeleteTouchMoveCallback (int ref) {
		TOUCHMOVE_CALLBACKS.erase(TOUCHMOVE_CALLBACKS.find(ref));
	}
	
	static inline void RunStartupCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = STARTUP_CALLBACKS.begin(); i != STARTUP_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunShutdownCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = SHUTDOWN_CALLBACKS.begin(); i != SHUTDOWN_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunDrawCallbacks () {
		for(std::map<int, void (*) ()>::iterator i = DRAW_CALLBACKS.begin(); i != DRAW_CALLBACKS.end(); i++)
			i->second();
	}
	
	static inline void RunTouchCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = TOUCH_CALLBACKS.begin(); i != TOUCH_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
	static inline void RunTouchUpCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = TOUCHUP_CALLBACKS.begin(); i != TOUCHUP_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
	static inline void RunTouchMoveCallbacks (int x, int y) {
		for(std::map<int, void (*) (int, int)>::iterator i = TOUCHMOVE_CALLBACKS.begin(); i != TOUCHMOVE_CALLBACKS.end(); i++)
			i->second(x, y);
	}
	
private:
	static inline std::map<int, void (*) ()>				STARTUP_CALLBACKS;
	static inline std::map<int, void (*) ()>				SHUTDOWN_CALLBACKS;
	static inline std::map<int, void (*) ()>				DRAW_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	TOUCH_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	TOUCHUP_CALLBACKS;
	static inline std::map<int, void (*) (int x, int y)>	TOUCHMOVE_CALLBACKS;
};

#endif // G_SYSTEM_H_
