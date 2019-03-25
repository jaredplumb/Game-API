#ifndef _UINODE_H_
#define _UINODE_H_

#include "GTypes.h"

#define		UINODE_NAME(c,n)			static UINode::Factory<c> _UI_NODE_ ## c ## _NAME(n, false);
#define		UINODE_AUTORUN(c,n)			static UINode::Factory<c> _UI_NODE_ ## c ## _NAME(n, true);
#define		UINODE_PACKAGE(n)			_UINODEPACKAGE_UNIQUE(n, __COUNTER__)

class UINode {
public:
	UINode ();										// The default rect is the safe area
	virtual ~UINode ();
	
	int_t GetUniqueRef () const;					// Returns this UINode's unique reference
	UINode* GetParent () const;						// Returns this nodes parent
	int_t GetWidth () const;
	int_t GetHeight () const;
	GRect GetRect () const;							// Returns the rect of this UINode (0,0 location)
	GRect GetRectInParent () const;					// Returns the rect of this UINode, relative to the parent
	GRect GetScreenRect () const;					// Returns the rect of the screen relative to self
	GRect GetSafeRect () const;						// Returns the rect of the safe area relative to self
	
	void SetRect (const GRect& rect);				// Sets the rect of this node relative to the parent
	void SetRectCenterInParent ();					// Centers this node in the parent, or safe area if parent is null
	void SetVisible (bool visible);
	void SetActive (bool active);
	
	bool IsVisible () const;
	bool IsActive () const;
	
	static UINode* NewNode (const GString& name);	// Create a new node using the given name
	
	static uint64 GetMilliseconds ();				// Returns frame locked milliseconds
	static uint64 GetElapse ();						// Returns frame locked elapse time
	static uint32 GetRandom (uint32 range = 0);		// Returns a value from 0 to one minus range, unless range is 0, then the maximum 2^31-1 size is used
	static uint32 GetRandomSeed ();					// Returns the current global random seed
	static void SetRandomSeed (uint32 seed = 0);	// Sets the global random seed, if seed is 0, then a random time value is used
	
	void Run (const GString& name);					// Exits the current root UINode and runs the named UINode
	void Exit ();									// Will delete this UINode as soon as possible
	void ExitCancel ();								// Cancels an exit
	void Add (UINode& node);						// Adds a node to the front of the this nodes children
	void Remove (UINode& node);						// Removes a node from this nodes children
	
	virtual void OnDraw () {}
	virtual void OnMouse (int_t x, int_t y, int_t button) {}
	virtual void OnMouseUp (int_t x, int_t y, int_t button) {}
	virtual void OnMouseMove (int_t x, int_t y) {}
	virtual void OnMouseDrag (int_t x, int_t y, int_t button) {}
	virtual void OnMouseWheel (float xdelta, float ydelta) {}
	virtual void OnKey (vkey_t key) {}
	virtual void OnKeyUp (vkey_t key) {}
	virtual void OnASCII (char key) {}
	virtual void OnTouch (int_t x, int_t y) {}
	virtual void OnTouchUp (int_t x, int_t y) {}
	virtual void OnTouchMove (int_t x, int_t y) {}
	virtual void OnEvent (UINode* node) {}
	virtual bool OnExit () { return true; } // Once Exit is called (or Run on a different Node), this function will be called repeatedly until true is returned
	
	void SendDraw ();
	void SendMouse (int_t x, int_t y, int_t button);
	void SendMouseUp (int_t x, int_t y, int_t button);
	void SendMouseMove (int_t x, int_t y);
	void SendMouseDrag (int_t x, int_t y, int_t button);
	void SendMouseWheel (float xdelta, float ydelta);
	void SendKey (vkey_t key);
	void SendKeyUp (vkey_t key);
	void SendASCII (char key);
	void SendTouch (int_t x, int_t y);
	void SendTouchUp (int_t x, int_t y);
	void SendTouchMove (int_t x, int_t y);
	void SendEvent (UINode* node);
	bool SendExit ();
	
	inline bool operator== (const UINode& n) const                            { return _ref == n._ref; }
	inline bool operator!= (const UINode& n) const                            { return _ref != n._ref; }
	
private:
	int_t				_ref;
	GRect				_rect;		// This is the screen coordinates of this node initially set to the entire screen
	bool				_visible;
	bool				_active;
	bool				_exit;		// Node will exit as soon as possible, _exit will block most events when true
	GString				_next;		// This is the name of the node to run when this node is deleted, used from Run
	UINode*				_parent;
	std::list<UINode*>	_children;
	
	struct _Root {
		std::map<GString, UINode*> nodes;
		
		_Root ();
		~_Root ();
		static void RunOnRoot (const GString& name);
		
		static void StartupCallback ();
		static void ShutdownCallback ();
		static void DrawCallback ();
		static void MouseCallback (int_t x, int_t y, int_t button);
		static void MouseUpCallback (int_t x, int_t y, int_t button);
		static void MouseMoveCallback (int_t x, int_t y);
		static void MouseDragCallback (int_t x, int_t y, int_t button);
		static void MouseWheelCallback (float xdelta, float ydelta);
		static void KeyCallback (vkey_t key);
		static void KeyUpCallback (vkey_t key);
		static void ASCIICallback (char key);
		static void TouchCallback (int_t x, int_t y);
		static void TouchUpCallback (int_t x, int_t y);
		static void TouchMoveCallback (int_t x, int_t y);
	};
	
	static std::map<GString, UINode* (*) ()>*	_FACTORY_LIST;
	static std::map<GString, bool>*				_AUTORUN_LIST;
	static _Root*								_ROOT;
	static uint64								_MILLISECONDS;
	static uint64								_ELAPSE;
	static uint32								_RANDOM_SEED;
	
public:
	template <class T>
	struct Factory {
		static UINode* New () { return new T; };
		Factory (const char* name, bool autorun) {
			if(_ROOT == NULL) _ROOT = new _Root;
			if(_FACTORY_LIST == NULL) _FACTORY_LIST = new std::map<GString, UINode* (*) ()>;
			(*_FACTORY_LIST)[name] = New;
			if(!autorun) return;
			if(_AUTORUN_LIST == NULL) _AUTORUN_LIST = new std::map<GString, bool>;
			(*_AUTORUN_LIST)[name] = true;
		}
	};
};

#define		_UINODEPACKAGE_UNIQUE(n,u)		_UINODEPACKAGE_STATIC(n,u)
#define		_UINODEPACKAGE_STATIC(n,u)		static GPackage _GPACKAGE_ ## u ## _NAME(n, true);

#endif // _UINODE_H_
