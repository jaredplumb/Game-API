#ifndef _UINODE_H_
#define _UINODE_H_

#include "GTypes.h"
#include <list>
#include <map>

#define		UINODE_NAME(c,n)			static const UINode::UIFactory<c> _UI_NODE_ ## c ## _NAME(n, false);
#define		UINODE_AUTORUN(c,n)			static const UINode::UIFactory<c> _UI_NODE_ ## c ## _NAME(n, true);
#define		UINODE_PACKAGE(n)			_UINODEPACKAGE_UNIQUE(n, __COUNTER__)
#define		_UINODEPACKAGE_UNIQUE(n,u)	_UINODEPACKAGE_STATIC(n,u)
#define		_UINODEPACKAGE_STATIC(n,u)	static const GPackage _GPACKAGE_ ## u ## _NAME(n, true);

class UINode {
public:
	UINode ();										// The default rect is the safe area
	virtual ~UINode ();
	
	int GetUniqueRef () const;						// Returns this UINode's unique reference
	UINode* GetParent () const;						// Returns this nodes parent
	int GetWidth () const;
	int GetHeight () const;
	GRect GetRect () const;							// Returns the rect of this UINode
	GRect GetRectInParent () const;					// Returns the rect of this UINode, relative to the parent
	GRect GetScreenRect () const;					// Returns the rect of the screen relative to self
	GRect GetSafeRect () const;						// Returns the rect of the safe area relative to self
	GRect GetPreferredRect () const;				// Returns the preferred rect relative to self
	
	void SetRect (const GRect& rect);				// Sets the rect of this node relative to the parent
	void SetRectCenterInParent ();					// Centers this node in the parent, or safe area if parent is null
	void SetPosition (const GPoint& pos);			// Set this nodes position relative to parent (NOTE: "location" would be absolute position)
	void SetVisible (bool visible);
	void SetActive (bool active);
	void SetPassive (bool passive);					// A passive object only receives OnDraw and OnExit events (although events are still passed along to children), the default is false
	
	bool IsVisible () const;
	bool IsActive () const;
	bool IsPassive () const;
	
	static UINode* NewNode (const GString& name);	// Create a new node using the given name
	
	static int64_t GetMilliseconds ();				// Returns frame locked milliseconds
	static int64_t GetElapse ();						// Returns frame locked elapse time
	static uint32_t GetRandom (uint32_t range = 0);		// Returns a value from 0 to one minus range, unless range is 0, then the maximum 2^30 size is used (1073741824)
	static uint32_t GetRandomSeed ();					// Returns the current global random seed
	static void SetRandomSeed (uint32_t seed = 0);	// Sets the global random seed, if seed is 0, then a random time value is used
	
	void Run (const GString& name);					// Exits the current root UINode and runs the named UINode
	void RunChild (const GString& name);			// Runs the named node on as a child of this node, and delete the node when this node is deleted
	void Exit ();									// Will delete this UINode as soon as possible
	void ExitCancel ();								// Cancels an exit
	void Add (UINode& node);						// Adds a node to the front of the this nodes children
	void Remove (UINode& node);						// Removes a node from this nodes children
	
	virtual void OnDraw () {}
	virtual void OnTouch (int x, int y) {}
	virtual void OnTouchUp (int x, int y) {}
	virtual void OnTouchMove (int x, int y) {}
	virtual void OnEvent (UINode* node) {}
	virtual bool OnExit () { return true; } // Once Exit is called (or Run on a different Node), this function will be called repeatedly until true is returned
	
	void SendDraw ();
	void SendTouch (int x, int y);
	void SendTouchUp (int x, int y);
	void SendTouchMove (int x, int y);
	void SendEvent (UINode* node);
	bool SendExit ();
	
	inline bool operator== (const UINode& n) const                            { return _ref == n._ref; }
	inline bool operator!= (const UINode& n) const                            { return _ref != n._ref; }
	
private:
	int					_ref;
	GRect				_rect;		// This is the rect relative to the parent, initially set to the screen's preferred rect, the root node uses the actual screen locations from GSystem as it's parent
	bool				_visible;
	bool				_active;
	bool				_passive;
	bool				_exit;		// Node will exit as soon as possible, _exit will block most events when true
	GString				_next;		// This is the name of the node to run when this node is deleted, used from Run
	UINode*				_parent;
	std::list<UINode*>	_children;
	
	struct _Root {
		std::list<UINode*> nodes;
		
		_Root ();
		~_Root ();
		static void RunOnRoot (const GString& name);
		static void StartupCallback ();
		static void ShutdownCallback ();
		static void DrawCallback ();
		static void TouchCallback (int x, int y);
		static void TouchUpCallback (int x, int y);
		static void TouchMoveCallback (int x, int y);
	};
	
	static std::map<GString, UINode* (*) ()>*	_FACTORY_LIST;
	static std::map<GString, bool>*				_AUTORUN_LIST;
	static _Root*								_ROOT;
	static int64_t								_MILLISECONDS;
	static int64_t								_ELAPSE;
	static uint32_t								_RANDOM_SEED;
	
public:
	template <class T>
	struct UIFactory {
		static UINode* New () { return new T; };
		UIFactory (const char* name, bool autorun) {
			if(_ROOT == NULL) _ROOT = new _Root;
			if(_FACTORY_LIST == NULL) _FACTORY_LIST = new std::map<GString, UINode* (*) ()>;
			(*_FACTORY_LIST)[name] = New;
			if(!autorun) return;
			if(_AUTORUN_LIST == NULL) _AUTORUN_LIST = new std::map<GString, bool>;
			(*_AUTORUN_LIST)[name] = true;
		}
	};
};

#endif // _UINODE_H_
