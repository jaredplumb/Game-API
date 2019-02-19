#include "UINode.h"
#include "PSystem.h"






PNode::PNode ()
:	_ref(PSystem::GetUniqueRef())
,	_delete(false)
,	_rect(0, 0, PSystem::GetWidth(), PSystem::GetHeight())
,	_visible(true)
,	_active(true)
,	_focus(false)
,	_exit(false)
,	_parent(NULL)
{
}

PNode::~PNode () {
	
	if(_parent)
		_parent->Remove(*this);
	
	while(_alloc.empty() == false) {
		delete _alloc.back();
		_alloc.pop_back();
	}
}






int_t PNode::GetWidth () const {
	return _rect.width;
}

int_t PNode::GetHeight () const {
	return _rect.height;
}

PRect PNode::GetRect() const {
	return _rect;
}

void PNode::SetRect (const PRect& rect) {
	_rect = rect;
}



uint64 PNode::GetMilliseconds () {
	return _MILLISECONDS;
}

uint64 PNode::GetElapse () {
	return _ELAPSE;
}






void PNode::Run (const PString& name) {
	
	// Using the _FACTORY_LIST, add a new instance of name to the roots node list
	
	// Use default transition
	
	// Mark this node for exit from the roots node list
	
	// Automatically calls Exit()?
	
	//Exit();
	_ROOT->Run(name);
	
}

//void PNode::RunAsChild (const PString& name) {
	
	// Uses a different default transition (none)
	
//}

//void PNode::RunLast () {
	
//}

void PNode::Exit () {
	_exit = true;
}



void PNode::Add (PNode& node) {
	
	if(&node == this)
		return;
	
	_children.remove(&node);
	
	_children.push_back(&node);
	
	
	if(node._parent != NULL)
		node._parent->Remove(node);
	
	node._parent = this;
	
}

void PNode::Add (PNode* node) {
	node->_delete = true;
	_alloc.push_back(node);
	Add(*node);
}

void PNode::Add (const PString& name) {
	std::map<PString, PNode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(name);
	if(factory != _FACTORY_LIST->end()) {
		PNode* node = factory->second();
		node->_delete = true;
		_alloc.push_back(node);
		Add(*node);
	}
}

void PNode::Remove (PNode& node) {
	_children.remove(&node);
	if(node._delete)
		_alloc.remove(&node);
	if(node._parent == this)
		node._parent = NULL;
}





void PNode::SendDraw () {
	if(_visible) {
		
		PSystem::MatrixSetProjectionDefault();
		PSystem::MatrixTranslateProjection((float)_rect.x, (float)_rect.y);
		PSystem::MatrixUpdate();
		
		OnDraw();
		for(std::list<PNode*>::reverse_iterator i = _children.rbegin(); i != _children.rend(); i++)
			if(!(*i)->_exit)
				(*i)->SendDraw();
			else
				delete *i;
	}
}

// All of the focus based events check focus first, for in case the event deletes the node and is no longer valid
void PNode::SendMouse (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouse(x, y, button);
	}
}

void PNode::SendMouseUp (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouseUp(x, y, button);
	}
}

void PNode::SendMouseMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnMouseMove(x, y);
	}
}

void PNode::SendMouseDrag (int_t x, int_t y, int_t button) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
			else
				(*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
		OnMouseDrag(x, y, button);
	}
}

void PNode::SendMouseWheel (float xdelta, float ydelta) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendMouseWheel(xdelta, ydelta);
			else
				(*i)->SendMouseWheel(xdelta, ydelta);
		OnMouseWheel(xdelta, ydelta);
	}
}

void PNode::SendKey (vkey_t key) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendKey(key);
			else
				(*i)->SendKey(key);
		OnKey(key);
	}
}

void PNode::SendKeyUp (vkey_t key) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendKeyUp(key);
			else
				(*i)->SendKeyUp(key);
		OnKeyUp(key);
	}
}

void PNode::SendASCII (char key) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendASCII(key);
			else
				(*i)->SendASCII(key);
		OnASCII(key);
	}
}

void PNode::SendTouch (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouch(x, y);
	}
}

void PNode::SendTouchUp (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouchUp(x, y);
	}
}

void PNode::SendTouchMove (int_t x, int_t y) {
	if(_visible && _active) {
		for(std::list<PNode*>::iterator i = _children.begin(); i != _children.end(); i++)
			if((*i)->_focus)
				return (*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
			else
				(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
		OnTouchMove(x, y);
	}
}









PNode::_Root::_Root ()
:	transitionIn(0)
,	transitionOut(0)
{
	PSystem::NewStartupCallback(StartupCallback);
	PSystem::NewShutdownCallback(ShutdownCallback);
	PSystem::NewDrawCallback(DrawCallback);
	PSystem::NewMouseCallback(MouseCallback);
	PSystem::NewMouseUpCallback(MouseUpCallback);
	PSystem::NewMouseMoveCallback(MouseMoveCallback);
	PSystem::NewMouseDragCallback(MouseDragCallback);
	PSystem::NewMouseWheelCallback(MouseWheelCallback);
	PSystem::NewKeyCallback(KeyCallback);
	PSystem::NewKeyUpCallback(KeyUpCallback);
	PSystem::NewASCIICallback(ASCIICallback);
	PSystem::NewTouchCallback(TouchCallback);
	PSystem::NewTouchUpCallback(TouchUpCallback);
	PSystem::NewTouchMoveCallback(TouchMoveCallback);
}

PNode::_Root::~_Root () {
	while(!nodes.empty()) {
		delete nodes.back();
		nodes.pop_back();
	}
}

void PNode::_Root::Run (const PString& name) {
	// This allows this function to be called from the draw callbacks without causing problems
	if(transitionIn || transitionOut)
		return;
	
	to = name;
	transitionOut = FADE_TIME;
}

void PNode::_Root::StartupCallback () {
}

void PNode::_Root::ShutdownCallback () {
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

void PNode::_Root::DrawCallback () {
	
	static uint64 FRAMES = 0;
	FRAMES++;
	_ELAPSE = FRAMES * 1000 / PSystem::GetFPS() - _MILLISECONDS;
	_MILLISECONDS = FRAMES * 1000 / PSystem::GetFPS();
	
	
	
	
	if(_AUTORUN_LIST) {
		while(_AUTORUN_LIST->begin() != _AUTORUN_LIST->end()) {
#if DEBUG
			PSystem::Debug("----------------------------------------------------------------\n");
			PSystem::Debug("- %s\n", (const char*)_AUTORUN_LIST->begin()->first);
#endif
			std::map<PString, PNode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(_AUTORUN_LIST->begin()->first);
			if(factory != _FACTORY_LIST->end() && _ROOT)
				_ROOT->nodes.push_front(factory->second());
#if DEBUG
			PSystem::Debug("----------------------------------------------------------------\n");
#endif
			_AUTORUN_LIST->erase(_AUTORUN_LIST->begin());
		}
		delete _AUTORUN_LIST;
		_AUTORUN_LIST = NULL;
	}
	
	if(_ROOT) {
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++) {
			
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
				PSystem::Debug("----------------------------------------------------------------\n");
				PSystem::Debug("- %s\n", (const char*)_ROOT->to);
#endif
				
				std::map<PString, PNode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(_ROOT->to);
				if(factory != _FACTORY_LIST->end())
					_ROOT->nodes.push_front(factory->second());
				
#if DEBUG
				PSystem::Debug("----------------------------------------------------------------\n");
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

void PNode::_Root::MouseCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouse(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void PNode::_Root::MouseUpCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseUp(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void PNode::_Root::MouseMoveCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void PNode::_Root::MouseDragCallback (int_t x, int_t y, int_t button) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseDrag(x - (*i)->_rect.x, y - (*i)->_rect.y, button);
}

void PNode::_Root::MouseWheelCallback (float xdelta, float ydelta) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendMouseWheel(xdelta, ydelta);
}

void PNode::_Root::KeyCallback (vkey_t key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendKey(key);
}

void PNode::_Root::KeyUpCallback (vkey_t key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendKeyUp(key);
}

void PNode::_Root::ASCIICallback (char key) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendASCII(key);
}

void PNode::_Root::TouchCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void PNode::_Root::TouchUpCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void PNode::_Root::TouchMoveCallback (int_t x, int_t y) {
	if(_ROOT && !_ROOT->transitionIn && !_ROOT->transitionOut)
		for(std::list<PNode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
}







std::map<PString, PNode* (*) ()>*		PNode::_FACTORY_LIST = NULL;
std::map<PString, bool>*				PNode::_AUTORUN_LIST = NULL;
PNode::_Root*							PNode::_ROOT = NULL;
uint64									PNode::_MILLISECONDS = 0;
uint64									PNode::_ELAPSE = 0;




