#include "UINode.h"
#include "GSystem.h"






UINode::UINode ()
:	_ref(GSystem::GetUniqueRef())
,	_delete(false)
,	_rect(0, 0, GSystem::GetWidth(), GSystem::GetHeight())
,	_visible(true)
,	_active(true)
,	_focus(false)
,	_exit(false)
,	_parent(NULL)
{
}

UINode::~UINode () {
	
	if(_parent)
		_parent->Remove(*this);
	
	while(_alloc.empty() == false) {
		delete _alloc.back();
		_alloc.pop_back();
	}
}






int_t UINode::GetWidth () const {
	return _rect.width;
}

int_t UINode::GetHeight () const {
	return _rect.height;
}

GRect UINode::GetRect() const {
	return _rect;
}

void UINode::SetRect (const GRect& rect) {
	_rect = rect;
}



uint64 UINode::GetMilliseconds () {
	return _MILLISECONDS;
}

uint64 UINode::GetElapse () {
	return _ELAPSE;
}






void UINode::Run (const GString& name) {
	
	// Using the _FACTORY_LIST, add a new instance of name to the roots node list
	
	// Use default transition
	
	// Mark this node for exit from the roots node list
	
	// Automatically calls Exit()?
	
	//Exit();
	_ROOT->Run(name);
	
}

//void UINode::RunAsChild (const GString& name) {
	
	// Uses a different default transition (none)
	
//}

//void UINode::RunLast () {
	
//}

void UINode::Exit () {
	_exit = true;
}



void UINode::Add (UINode& node) {
	
	if(&node == this)
		return;
	
	_children.remove(&node);
	
	_children.push_back(&node);
	
	
	if(node._parent != NULL)
		node._parent->Remove(node);
	
	node._parent = this;
	
}

void UINode::Add (UINode* node) {
	node->_delete = true;
	_alloc.push_back(node);
	Add(*node);
}

void UINode::Add (const GString& name) {
	std::map<GString, UINode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(name);
	if(factory != _FACTORY_LIST->end()) {
		UINode* node = factory->second();
		node->_delete = true;
		_alloc.push_back(node);
		Add(*node);
	}
}

void UINode::Remove (UINode& node) {
	_children.remove(&node);
	if(node._delete)
		_alloc.remove(&node);
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

// All of the focus based events check focus first, for in case the event deletes the node and is no longer valid
void UINode::SendMouse (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouse(x, y, button);
	}
}

void UINode::SendMouseUp (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouseUp(x, y, button);
	}
}

void UINode::SendMouseMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnMouseMove(x, y);
	}
}

void UINode::SendMouseDrag (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouseDrag(x, y, button);
	}
}

void UINode::SendMouseWheel (float xdelta, float ydelta) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseWheel(xdelta, ydelta);
			else
				(*i)->SendMouseWheel(xdelta, ydelta);
		OnMouseWheel(xdelta, ydelta);
	}
}

void UINode::SendKey (vkey_t key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendKey(key);
			else
				(*i)->SendKey(key);
		OnKey(key);
	}
}

void UINode::SendKeyUp (vkey_t key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendKeyUp(key);
			else
				(*i)->SendKeyUp(key);
		OnKeyUp(key);
	}
}

void UINode::SendASCII (char key) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendASCII(key);
			else
				(*i)->SendASCII(key);
		OnASCII(key);
	}
}

void UINode::SendTouch (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouch(x, y);
	}
}

void UINode::SendTouchUp (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouchUp(x, y);
	}
}

void UINode::SendTouchMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouchMove(x, y);
	}
}









UINode::_Root::_Root ()
:	transitionIn(0)
,	transitionOut(0)
{
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
	while(!nodes.empty()) {
		delete nodes.back();
		nodes.pop_back();
	}
}

void UINode::_Root::Run (const GString& name) {
	// This allows this function to be called from the draw callbacks without causing problems
	if(transitionIn || transitionOut)
		return;
	
	to = name;
	transitionOut = FADE_TIME;
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
#if DEBUG
			GSystem::Debug("----------------------------------------------------------------\n");
			GSystem::Debug("- %s\n", (const char*)_AUTORUN_LIST->begin()->first);
#endif
			std::map<GString, UINode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(_AUTORUN_LIST->begin()->first);
			if(factory != _FACTORY_LIST->end() && _ROOT)
				_ROOT->nodes.push_front(factory->second());
#if DEBUG
			GSystem::Debug("----------------------------------------------------------------\n");
#endif
			_AUTORUN_LIST->erase(_AUTORUN_LIST->begin());
		}
		delete _AUTORUN_LIST;
		_AUTORUN_LIST = NULL;
	}
	
	if(_ROOT) {
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++) {
			
			if(!(*i)->_exit) {
				(*i)->SendDraw();
			} else {
				delete *i;
				i = _ROOT->nodes.erase(i)--;
			}
			
		}
		
		
		
		if(_ROOT && _ROOT->transitionIn) {
			
			//_ROOT->fade.DrawRect(PRect(0, 0, _ROOT->GetWidth(), _ROOT->GetHeight()), PColor(0.0f, 0.0f, 0.0f, (float)_ROOT->transitionIn / (float)FADE_TIME));
			
			if(_ROOT->transitionIn > _ELAPSE)
				_ROOT->transitionIn -= _ELAPSE;
			else
				_ROOT->transitionIn = 0;
			
			
			
			if(_ROOT->to) {
				
				
				while(!_ROOT->nodes.empty()) {
					delete _ROOT->nodes.front();
					_ROOT->nodes.pop_front();
				}
				
#if DEBUG
				GSystem::Debug("----------------------------------------------------------------\n");
				GSystem::Debug("- %s\n", (const char*)_ROOT->to);
#endif
				
				std::map<GString, UINode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(_ROOT->to);
				if(factory != _FACTORY_LIST->end())
					_ROOT->nodes.push_front(factory->second());
				
#if DEBUG
				GSystem::Debug("----------------------------------------------------------------\n");
#endif
				
				_ROOT->to = NULL;
				
				
			}
			
			
		}
		
		
		
		if(_ROOT && _ROOT->transitionOut) {
			
			//_ROOT->fade.Draw(0, 0, 1.0f - (float)_ROOT->transitionOut / (float)FADE_TIME);
			
			
			if(_ROOT->transitionOut > _ELAPSE) {
				_ROOT->transitionOut -= _ELAPSE;
			} else {
				_ROOT->transitionOut = 0;
				_ROOT->transitionIn = FADE_TIME;
			}
			
			
		}
		
		
		
		
		
	}
	
}

void UINode::_Root::MouseCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void UINode::_Root::MouseUpCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void UINode::_Root::MouseMoveCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void UINode::_Root::MouseDragCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void UINode::_Root::MouseWheelCallback (float xdelta, float ydelta) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseWheel(xdelta, ydelta);
}

void UINode::_Root::KeyCallback (vkey_t key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendKey(key);
}

void UINode::_Root::KeyUpCallback (vkey_t key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendKeyUp(key);
}

void UINode::_Root::ASCIICallback (char key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendASCII(key);
}

void UINode::_Root::TouchCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void UINode::_Root::TouchUpCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void UINode::_Root::TouchMoveCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
}







std::map<GString, UINode* (*) ()>*		UINode::_FACTORY_LIST = NULL;
std::map<GString, bool>*				UINode::_AUTORUN_LIST = NULL;
UINode::_Root*							UINode::_ROOT = NULL;
uint64									UINode::_MILLISECONDS = 0;
uint64									UINode::_ELAPSE = 0;




