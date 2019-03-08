#include "UINode.h"
#include "GSystem.h"






UINode::UINode ()
:	_ref(GSystem::GetUniqueRef())
,	_rect(GSystem::GetRect())
,	_visible(true)
,	_active(true)
,	_exit(false)
,	_parent(NULL)
{
}

UINode::~UINode () {
	if(_parent)
		_parent->Remove(*this);
}





int_t UINode::GetUniqueRef () const {
	return _ref;
}

UINode* UINode::GetParent() const {
	return _parent;
}

int_t UINode::GetWidth () const {
	return _rect.width;
}

int_t UINode::GetHeight () const {
	return _rect.height;
}

GRect UINode::GetRect() const {
	return _parent ? GRect(_rect).Offset(-_parent->_rect.x, -_parent->_rect.y) : _rect;
}

void UINode::SetRect (const GRect& rect) {
	int_t x = _rect.x;
	int_t y = _rect.y;
	if(_parent) {
		_rect.x = _parent->_rect.x + rect.x;
		_rect.y = _parent->_rect.y + rect.y;
	} else {
		_rect.x = rect.x;
		_rect.y = rect.y;
	}
	_rect.width = rect.width;
	_rect.height = rect.height;
	for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
		(*i)->SetRect(GRect((*i)->_rect).Offset(-x, -y));
}

void UINode::SetVisible (bool visible) {
	_visible = visible;
}

void UINode::SetActive (bool active) {
	_active = active;
}

bool UINode::IsVisible () const {
	return _visible;
}

bool UINode::IsActive () const {
	return _active;
}




UINode* UINode::NewNode (const GString& name) {
	// TODO: This needs thought, should we allow creation of a node, or use UINodeFactory to create the node using a name
	return NULL;
}

uint64 UINode::GetMilliseconds () {
	return _MILLISECONDS;
}

uint64 UINode::GetElapse () {
	return _ELAPSE;
}

// This random generator is from the BSD random, which is from:
// "Random number generators: good ones are hard to find"
// Park and Miller, Communications of the ACM, vol. 31, no. 10, October 1988, p. 1195.
static uint32 _RANDOM_SEED = 0;

uint32 UINode::GetRandom (uint32 range) {
	_RANDOM_SEED = (_RANDOM_SEED != 0) ? (16807 * (_RANDOM_SEED % 127773) - 2836 * (_RANDOM_SEED / 127773)) : ((uint32)GSystem::GetMilliseconds() + 1);
	if((int32)_RANDOM_SEED <= 0)
		_RANDOM_SEED += 0x7fffffff;
	return range ? _RANDOM_SEED % range : _RANDOM_SEED;
}

uint32 UINode::GetRandomSeed () {
	return _RANDOM_SEED;
}

void UINode::SetRandomSeed (uint32 seed) {
	_RANDOM_SEED = (seed != 0) ? (seed) : ((uint32)GSystem::GetMilliseconds() + 1);
}







void UINode::Run (const GString& name) {
	
	// Running a new node will exit this line of nodes up to the root
	UINode* parent = this;
	while(parent->_parent)
		parent = parent->_parent;
	parent->Exit();
	
	// Run the new node on the root list
	_ROOT->RunOnRoot(name);
	
}

//void UINode::RunAsChild (const GString& name) {
	
	// Uses a different default transition (none)
	
//}

//void UINode::RunLast () {
	
//}

void UINode::Exit () {
	_exit = true;
}

void UINode::ExitCancel() {
	_exit = false;
}



void UINode::Add (UINode& node) {
	
	if(node == *this) {
		GConsole::Debug("Cannot add a UINode to itself!\n");
		return;
	}
	
	for(UINode* parent = _parent; _parent != NULL; _parent = _parent->_parent)
		if(*parent == *this) {
			GConsole::Debug("UINode cannot be a child to itself!\n");
			return;
		}
	
	_children.remove(&node); // Safety to avoid adding the same node multiple times
	_children.push_back(&node);
	
	if(node._parent != NULL) // A node can only be the child of one parent
		node._parent->Remove(node);
	node._parent = this;
	
	node.SetRect(node._rect); // Adjust the child's rect to be relative to this node by calling SetRect with it's own rect
}

void UINode::Remove (UINode& node) {
	_children.remove(&node);
	if(node._parent == this)
		node._parent = NULL;
}





void UINode::SendDraw () {
	if(_visible) {
		
		GSystem::MatrixSetProjectionDefault();
		GSystem::MatrixTranslateProjection((float)_rect.x, (float)_rect.y);
		GSystem::MatrixUpdate();
		
		OnDraw();
		for(std::list<UINode*>::reverse_iterator i = _children.rbegin(); i != _children.rend(); i++)
			if(!(*i)->_exit)
				(*i)->SendDraw();
			else
				delete *i;
	}
}

void UINode::SendMouse (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendMouse(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y, button);
		OnMouse(x, y, button);
	}
}

void UINode::SendMouseUp (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendMouseUp(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y, button);
		OnMouseUp(x, y, button);
	}
}

void UINode::SendMouseMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendMouseMove(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		OnMouseMove(x, y);
	}
}

void UINode::SendMouseDrag (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendMouseDrag(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y, button);
		OnMouseDrag(x, y, button);
	}
}

void UINode::SendMouseWheel (float xdelta, float ydelta) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendMouseWheel(xdelta, ydelta);
		OnMouseWheel(xdelta, ydelta);
	}
}

void UINode::SendKey (vkey_t key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendKey(key);
		OnKey(key);
	}
}

void UINode::SendKeyUp (vkey_t key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendKeyUp(key);
		OnKeyUp(key);
	}
}

void UINode::SendASCII (char key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendASCII(key);
		OnASCII(key);
	}
}

void UINode::SendTouch (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouch(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		OnTouch(x, y);
	}
}

void UINode::SendTouchUp (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouchUp(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		OnTouchUp(x, y);
	}
}

void UINode::SendTouchMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouchMove(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		OnTouchMove(x, y);
	}
}

void UINode::SendEvent (UINode* node) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendEvent(node);
		OnEvent(node);
	}
}

bool UINode::SendExit () {
	for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
		if(!(*i)->SendExit())
			return false;
	return OnExit();
}








UINode::_Root::_Root () {
	GSystem::NewStartupCallback(StartupCallback);
	GSystem::NewShutdownCallback(ShutdownCallback);
	GSystem::NewDrawCallback(DrawCallback);
	GSystem::NewMouseCallback(MouseCallback);
	GSystem::NewMouseUpCallback(MouseUpCallback);
	GSystem::NewMouseMoveCallback(MouseMoveCallback);
	GSystem::NewMouseDragCallback(MouseDragCallback);
	GSystem::NewMouseWheelCallback(MouseWheelCallback);
	GSystem::NewKeyCallback(KeyCallback);
	GSystem::NewKeyUpCallback(KeyUpCallback);
	GSystem::NewASCIICallback(ASCIICallback);
	GSystem::NewTouchCallback(TouchCallback);
	GSystem::NewTouchUpCallback(TouchUpCallback);
	GSystem::NewTouchMoveCallback(TouchMoveCallback);
}

UINode::_Root::~_Root () {
	while(nodes.begin() != nodes.end()) {
		delete nodes.begin()->second;
		nodes.erase(nodes.begin());
	}
}

void UINode::_Root::RunOnRoot (const GString& name) {
	GConsole::Debug("----------------------------------------------------------------\n");
	GConsole::Debug("- %s\n", (const char*)name);
	std::map<GString, UINode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(name);
	if(_ROOT != NULL && factory != _FACTORY_LIST->end())
		_ROOT->nodes[name] = factory->second();
	GConsole::Debug("----------------------------------------------------------------\n");
}

void UINode::_Root::StartupCallback () {
}

void UINode::_Root::ShutdownCallback () {
	if(_AUTORUN_LIST) {
		delete _AUTORUN_LIST;
		_AUTORUN_LIST = NULL;
	}
	if(_FACTORY_LIST) {
		delete _FACTORY_LIST;
		_FACTORY_LIST = NULL;
	}
	if(_ROOT) {
		delete _ROOT;
		_ROOT = NULL;
	}
}

void UINode::_Root::DrawCallback () {
	
	static uint64 FRAMES = 0;
	FRAMES++;
	_ELAPSE = FRAMES * 1000 / GSystem::GetFPS() - _MILLISECONDS;
	_MILLISECONDS = FRAMES * 1000 / GSystem::GetFPS();
	
	if(_AUTORUN_LIST) {
		while(_AUTORUN_LIST->begin() != _AUTORUN_LIST->end()) {
			RunOnRoot(_AUTORUN_LIST->begin()->first);
			_AUTORUN_LIST->erase(_AUTORUN_LIST->begin());
		}
		delete _AUTORUN_LIST;
		_AUTORUN_LIST = NULL;
	}
	
	if(_ROOT) {
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); ) {
			if(i->second->_exit && i->second->SendExit()) {
				delete i->second;
				i = _ROOT->nodes.erase(i);
			} else {
				i->second->SendDraw();
				i++;
			}
		}
	}
	
}

void UINode::_Root::MouseCallback (int_t x, int_t y, int_t button) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendMouse(x - i->second->_rect.x, y - i->second->_rect.y, button);
}

void UINode::_Root::MouseUpCallback (int_t x, int_t y, int_t button) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendMouseUp(x - i->second->_rect.x, y - i->second->_rect.y, button);
}

void UINode::_Root::MouseMoveCallback (int_t x, int_t y) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendMouseMove(x - i->second->_rect.x, y - i->second->_rect.y);
}

void UINode::_Root::MouseDragCallback (int_t x, int_t y, int_t button) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendMouseDrag(x - i->second->_rect.x, y - i->second->_rect.y, button);
}

void UINode::_Root::MouseWheelCallback (float xdelta, float ydelta) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendMouseWheel(xdelta, ydelta);
}

void UINode::_Root::KeyCallback (vkey_t key) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendKey(key);
}

void UINode::_Root::KeyUpCallback (vkey_t key) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendKeyUp(key);
}

void UINode::_Root::ASCIICallback (char key) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendASCII(key);
}

void UINode::_Root::TouchCallback (int_t x, int_t y) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendTouch(x - i->second->_rect.x, y - i->second->_rect.y);
}

void UINode::_Root::TouchUpCallback (int_t x, int_t y) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendTouchUp(x - i->second->_rect.x, y - i->second->_rect.y);
}

void UINode::_Root::TouchMoveCallback (int_t x, int_t y) {
	if(_ROOT)
		for(std::map<GString, UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			i->second->SendTouchMove(x - i->second->_rect.x, y - i->second->_rect.y);
}






std::map<GString, UINode* (*) ()>*		UINode::_FACTORY_LIST = NULL;
std::map<GString, bool>*				UINode::_AUTORUN_LIST = NULL;
UINode::_Root*							UINode::_ROOT = NULL;
uint64									UINode::_MILLISECONDS = 0;
uint64									UINode::_ELAPSE = 0;




