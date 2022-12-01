#ifndef _GNODE_H_
#define _GNODE_H_

#include "GTypes.h"
#include "GSystem.h"
#include <list>
#include <string>
#include <unordered_map>

#define		GNODE_PACKAGE(n)			_GNODEPACKAGE_UNIQUE(n, __COUNTER__)
#define		_GNODEPACKAGE_UNIQUE(n,u)	_GNODEPACKAGE_STATIC(n,u)
#define		_GNODEPACKAGE_STATIC(n,u)	static const GPackage _GPACKAGE_ ## u ## _NAME(n);

class GNode {
public:
	GNode ();										// The default rect is the preferred rect
	inline bool operator== (const GNode& n) const	{ return _ref == n._ref; }
	inline bool operator!= (const GNode& n) const	{ return _ref != n._ref; }
	virtual ~GNode ();
	
	int GetUniqueRef () const;						// Returns this GNode's unique reference
	GNode* GetParent () const;						// Returns this nodes parent
	int GetWidth () const;
	int GetHeight () const;
	int GetX () const;
	int GetY () const;
	GRect GetRect () const;							// Returns this node's rect (the default rect is at 0,0 with a width and height equal to the system's preferred rect, all node locations are relative to the system's preferred rect)
	GRect GetScreenRect () const;					// Returns the screen rect relative to this node
	GRect GetSafeRect () const;						// Returns the safe rect relative to this node
	GRect GetPreferredRect () const;				// Returns the preferred rect relative to this node
	
	void SetRect (const GRect& rect);				// Sets this node's rect
	void SetRectCenterInParent ();					// Sets this node's rect to be centered in the parent or the system's preferred rect if no parent
	void SetPosition (const GPoint& pos);			// Sets this node's x,y in the rect
	void SetVisible (bool visible);					// A non-visible node only receives and sends to children OnExit (default true)
	void SetActive (bool active);					// A non-active node only receives and sends to children OnDraw and OnExit (default true)
	void SetBlocking (bool blocking);				// Sets the node to blocking preventing interactive events from being passed further and automatically calls SetNodeAsFront to ensure the blocking happens (default false)
	void SetNodeAsFront ();							// Sets the node to draw last and receive events first
	
	bool IsVisible () const;
	bool IsActive () const;
	bool IsBlocking () const;
	
	static GNode* NewNode (const GString& name);	// Find the factory by name and create a new node
	void RunNewNode (const GString& name);			// Exits the current node line down to the root and runs a new root node, can be called from the root or child and will exit all the way to the root
	void RunNewNodeAsChild (const GString& name);	// Runs the node as a child of this node with managed memory, deleting child node when the parent is deleted
	void ExitNode ();								// Exits the current node as soon as possible (usually the next frame), this will not exit up to the parent but will still exit all this node's children
	void ExitNodeCancel ();							// If this node is marked to exit, this function will cancel that exit and mark this node for continued running
	void AddNode (GNode& node);						// Adds a node to the front of the this nodes children
	void RemoveNode (GNode& node);					// Removes a node from this nodes children
	
	virtual void OnDraw () {}
	virtual void OnTouch (int x, int y) {}
	virtual void OnTouchUp (int x, int y) {}
	virtual void OnTouchMove (int x, int y) {}
	virtual void OnEvent (GNode* node) {}
	virtual bool OnExit () { return true; }
	
	void SendDraw ();
	void SendTouch (int x, int y);
	void SendTouchUp (int x, int y);
	void SendTouchMove (int x, int y);
	void SendEvent (GNode* node);
	bool SendExit ();
	
	/// Returns frame locked milliseconds
	static inline int64_t GetMilliseconds () {
		return _MILLISECONDS;
	}
	
	/// Returns frame locked elapse time
	static inline int64_t GetElapse () {
		return _ELAPSE;
	}
	
	// This random generator is from the BSD random, which is from: "Random number generators: good ones are hard to find" Park and Miller, Communications of the ACM, vol. 31, no. 10, October 1988, p. 1195.
	/// Returns a value from 0 to one minus range, unless range is 0, then the maximum 2^30 size is used (1073741824)
	static inline uint32_t GetRandom (uint32_t range = 0) {
		_RANDOM_SEED = 16807 * (_RANDOM_SEED % 127773) - 2836 * (_RANDOM_SEED / 127773);
		if((int32_t)_RANDOM_SEED <= 0)
			_RANDOM_SEED += 0x7fffffff;
		return range ? _RANDOM_SEED % range : _RANDOM_SEED % 1073741824;
	}
	
	/// Returns a random float value equal to or greater than min and equal to or less than max
	static inline float GetRandom (float min, float max) {
		return ((float)GetRandom(1000000) / (float)999999) * (max - min) + min;
	}
	
	/// Returns the current global random seed
	static inline uint32_t GetRandomSeed () {
		return _RANDOM_SEED;
	}
	
	/// Sets the global random seed, if seed is 0, then a random time value is used
	static inline void SetRandomSeed (uint32_t seed = 0) {
		_RANDOM_SEED = seed ? seed : ((uint32_t)GSystem::GetMilliseconds() + 1);
	}
	
private:
	int					_ref;
	GRect				_rect;		// This is the rect relative to the parent
	bool				_alloc;		// If this node is allocated, then it should be deleted internally when the parent goes out of scope
	bool				_visible;
	bool				_active;
	bool				_blocking;
	bool				_blocked;	// This is set to true if the node had interactive events blocked during the last interactive event (root nodes are never set to blocked and will always send events)
	bool				_exit;		// This node will exit as soon as possible blocking interactive events until complete, only the root node needs to be triggered as exiting becaue this is handled in the system callbacks
	GString				_next;		// This is the name of the node to run when this node is deleted, only the root node needs to know what node will run next because this is handled in the system callbacks
	GNode*				_parent;
	std::list<GNode*>	_children;
	
	static inline std::list<GNode*>									_ROOT_NODE_LIST;
	static inline std::unordered_map<std::string, GNode* (*) ()>	_FACTORY_MAP;
	static inline std::list<std::string>							_AUTO_RUN_LIST;
	static inline std::list<GNode*>									_AUTO_DELETE_LIST;
	static inline int64_t											_FRAMES = 0;
	static inline int64_t											_MILLISECONDS = 1;
	static inline int64_t											_ELAPSE = 1;
	static inline uint32_t											_RANDOM_SEED = (uint32_t)GSystem::GetMilliseconds() + 1;
	
	static void _DrawCallback ();
	
	static inline void _TouchCallback (int x, int y) {
		for(std::list<GNode*>::iterator i = _ROOT_NODE_LIST.begin(); i != _ROOT_NODE_LIST.end(); i++)
			(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
	}
	
	static inline void _TouchUpCallback (int x, int y) {
		for(std::list<GNode*>::iterator i = _ROOT_NODE_LIST.begin(); i != _ROOT_NODE_LIST.end(); i++)
			(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
	}
	
	static inline void _TouchMoveCallback (int x, int y) {
		for(std::list<GNode*>::iterator i = _ROOT_NODE_LIST.begin(); i != _ROOT_NODE_LIST.end(); i++)
			(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
	}
	
	struct _STATIC_CONSTRUCTOR { inline _STATIC_CONSTRUCTOR () {
		GSystem::NewDrawCallback(_DrawCallback);
		GSystem::NewTouchCallback(_TouchCallback);
		GSystem::NewTouchUpCallback(_TouchUpCallback);
		GSystem::NewTouchMoveCallback(_TouchMoveCallback);
	} }; static inline _STATIC_CONSTRUCTOR _CONSTRUCTOR;
	
protected:
	class _Factory {
	public:
		inline _Factory (const char* name, GNode* (*function) ()) {
			_FACTORY_MAP[name] = function;
		}
	};
	
	class _AutoRun {
	public:
		inline _AutoRun (const char* name) {
			_AUTO_RUN_LIST.push_back(name);
		}
	};
	
}; // class GNode


// These templace classes use the curiously recurring template pattern
// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern


template <class T, const GString::Literal S>
class GNodeWithName : public GNode {
public:
	static inline GNode* _NewNodeFromTemplate () { return new T; }
	static inline std::unique_ptr<GNode::_Factory> _factory = std::unique_ptr<GNode::_Factory>(new GNode::_Factory(S, _NewNodeFromTemplate));
	inline GNodeWithName () { assert(_factory); } // This is a hack to force static initialization
};


template <class T, GString::Literal S>
class GNodeAutoRun : public GNodeWithName<T, S> {
public:
	static inline std::unique_ptr<GNode::_AutoRun> _auto = std::unique_ptr<GNode::_AutoRun>(new GNode::_AutoRun(S));
	inline GNodeAutoRun () { assert(_auto); } // This is a hack to force static initialization
};


#endif // _GNODE_H_
