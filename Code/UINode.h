#ifndef _UI_NODE_H_
#define _UI_NODE_H_

#include "PPlatform.h"
#include "PRect.h"
#include "PString.h"

#define		P_NODE_NAME(c,n)			static PNode::Factory<c> _P_NODE_ ## c ## _NAME(n, false);
#define		P_NODE_AUTORUN(c,n)			static PNode::Factory<c> _P_NODE_ ## c ## _NAME(n, true);

class PNode {
public:
	
	
	
	PNode ();
	virtual ~PNode ();
	
	int_t GetWidth () const;
	int_t GetHeight () const;
	PRect GetRect () const;
	
	void SetRect (const PRect& rect);
	
	static uint64 GetMilliseconds ();	// Returns frame locked milliseconds
	static uint64 GetElapse ();		// Returns frame locked elapse time
	
	
	
	
	
	
	void Run (const PString& name);				// Exits the current root PNode and runs the named PNode
	//void RunAsChild (const PString& name);		// Run the named PNode as a child of this PNode
	//void RunLast ();							// Runs the name of the previous PNode
	void Exit ();								// Will delete this PNode as soon as possible
	
	void Add (PNode& node);
	void Add (PNode* node); // This function will delete the node after use
	void Add (const PString& name);
	void Remove (PNode& node);
	
	
	virtual void OnDraw () {} // OnDraw is special in that it will be called when a child has focus
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
	
	
	
	
	
	
	
	
private:
	
	int_t				_ref;
	bool				_delete; // This node will be deleted when it's parent is deleted
	PRect				_rect;
	bool				_visible;
	bool				_active;
	bool				_focus; // A node in focus will not pass events, more than one node could have focus, all node by default have no focus
	bool				_exit;
	PNode*				_parent;
	std::list<PNode*>	_children;
	std::list<PNode*> 	_alloc; // These are children created with the Add(PString) or Add(Node*) functions, they will be deleted when this node is deleted
	
	struct _Root {
		std::list<PNode*> nodes;
		
		
		PString current;
		PString last;
		PString from;
		PString to;
		uint64 transitionIn;
		uint64 transitionOut;
		//PImage fade;
		
		
		_Root ();
		~_Root ();
		
		void Run (const PString& name);
		
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
		static const int FADE_TIME = 500; // Milliseconds;
	};
	
	static std::map<PString, PNode* (*) ()>*	_FACTORY_LIST;
	static std::map<PString, bool>*				_AUTORUN_LIST;
	static _Root*								_ROOT;
	static uint64								_MILLISECONDS;
	static uint64								_ELAPSE;
	
public:
	
	template <class T>
	struct Factory {
		static PNode* New () { return new T; };
		Factory (const char* name, bool autorun) {
			if(_ROOT == NULL) _ROOT = new _Root;
			if(_FACTORY_LIST == NULL) _FACTORY_LIST = new std::map<PString, PNode* (*) ()>;
			(*_FACTORY_LIST)[name] = New;
			if(!autorun) return;
			if(_AUTORUN_LIST == NULL) _AUTORUN_LIST = new std::map<PString, bool>;
			(*_AUTORUN_LIST)[name] = true;
		}
	};
	
};

#endif // _UI_NODE_H_
