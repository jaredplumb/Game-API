#ifndef _UINODE_H_
#define _UINODE_H_

#include "GTypes.h"

#define		UINODE_NAME(c,n)			static UINode::Factory<c> _UI_NODE_ ## c ## _NAME(n, false);
#define		UINODE_AUTORUN(c,n)			static UINode::Factory<c> _UI_NODE_ ## c ## _NAME(n, true);

class UINode {
public:
	
	
	
	UINode ();
	virtual ~UINode ();
	
	int_t GetWidth () const;
	int_t GetHeight () const;
	GRect GetRect () const;
	
	void SetRect (const GRect& rect);
	
	static uint64 GetMilliseconds ();	// Returns frame locked milliseconds
	static uint64 GetElapse ();		// Returns frame locked elapse time
	
	
	
	
	
	
	void Run (const GString& name);				// Exits the current root UINode and runs the named UINode
	//void RunAsChild (const GString& name);		// Run the named UINode as a child of this UINode
	//void RunLast ();							// Runs the name of the previous UINode
	void Exit ();								// Will delete this UINode as soon as possible
	
	void Add (UINode& node);
	void Add (UINode* node); // This function will delete the node after use
	void Add (const GString& name);
	void Remove (UINode& node);
	
	
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
	GRect				_rect;
	bool				_visible;
	bool				_active;
	bool				_focus; // A node in focus will not pass events, more than one node could have focus, all node by default have no focus
	bool				_exit;
	UINode*				_parent;
	std::list<UINode*>	_children;
	std::list<UINode*> 	_alloc; // These are children created with the Add(GString) or Add(Node*) functions, they will be deleted when this node is deleted
	
	struct _Root {
		std::list<UINode*> nodes;
		
		
		GString current;
		GString last;
		GString from;
		GString to;
		uint64 transitionIn;
		uint64 transitionOut;
		//PImage fade;
		
		
		_Root ();
		~_Root ();
		
		void Run (const GString& name);
		
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
	
	static std::map<GString, UINode* (*) ()>*	_FACTORY_LIST;
	static std::map<GString, bool>*				_AUTORUN_LIST;
	static _Root*								_ROOT;
	static uint64								_MILLISECONDS;
	static uint64								_ELAPSE;
	
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

#endif // _UINODE_H_
