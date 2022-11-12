#include "UINode.h"
#include "GSystem.h"

UINode::UINode ()
:	_ref(GSystem::GetUniqueRef())
,	_rect(GSystem::GetPreferredRect()) // UINode default is the prefered rect, which is centered in the safe area, this can always be changed later by setting the node to parent and such
,	_visible(true)
,	_active(true)
,	_passive(false)
,	_exit(false)
,	_parent(NULL)
{
}

UINode::~UINode () {
	if(_parent)
		_parent->Remove(*this);
	
	if(!_next.IsEmpty() && _ROOT)
		_ROOT->RunOnRoot(_next);
}

int UINode::GetUniqueRef () const {
	return _ref;
}

UINode* UINode::GetParent() const {
	return _parent;
}

int UINode::GetWidth () const {
	return _rect.width;
}

int UINode::GetHeight () const {
	return _rect.height;
}

GRect UINode::GetRect() const {
	return GRect(_rect.x, _rect.y, _rect.width, _rect.height);
}

GRect UINode::GetRectInParent() const {
	return _parent ? GRect(_rect).Offset(-_parent->_rect.x, -_parent->_rect.y) : _rect;
}

GRect UINode::GetScreenRect () const {
	return GSystem::GetRect().Offset(-_rect.x, -_rect.y);
}

GRect UINode::GetSafeRect () const {
	return GSystem::GetSafeRect().Offset(-_rect.x, -_rect.y);
}

GRect UINode::GetPreferredRect () const {
	return GSystem::GetPreferredRect().Offset(-_rect.x, -_rect.y);
}

void UINode::SetRect (const GRect& rect) {
	int x = _rect.x;
	int y = _rect.y;
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

void UINode::SetRectCenterInParent () {
	GRect rect;
	if(_parent) {
		rect.width = _parent->_rect.width;
		rect.height = _parent->_rect.height;
	} else {
		rect = GSystem::GetRect();
	}
	SetRect(GRect(rect.x + rect.width / 2 - GetWidth() / 2, rect.y + rect.height / 2 - GetHeight() / 2, GetWidth(), GetHeight()));
}

void UINode::SetPosition (const GPoint& pos) {
	SetRect(GRect(pos.x, pos.y, _rect.width, _rect.height));
}

void UINode::SetVisible (bool visible) {
	_visible = visible;
}

void UINode::SetActive (bool active) {
	_active = active;
}

void UINode::SetPassive (bool passive) {
	_passive = passive;
}

bool UINode::IsVisible () const {
	return _visible;
}

bool UINode::IsActive () const {
	return _active;
}

bool UINode::IsPassive () const {
	return _passive;
}

UINode* UINode::NewNode (const GString& name) {
	std::map<GString, UINode* (*) ()>::const_iterator factory = _FACTORY_LIST->find(name);
	if(factory != _FACTORY_LIST->end())
		return factory->second();
	GConsole::Debug("UINode factory \"%s\" does not exist!\n", (const char*)name);
	return NULL;
}

int64_t UINode::GetMilliseconds () {
	return _MILLISECONDS;
}

int64_t UINode::GetElapse () {
	return _ELAPSE;
}

// This random generator is from the BSD random, which is from:
// "Random number generators: good ones are hard to find"
// Park and Miller, Communications of the ACM, vol. 31, no. 10, October 1988, p. 1195.
static uint32_t _GET_SEED_RANDOM () {
	return (uint32_t)GSystem::GetMilliseconds() + 1;
}

uint32_t UINode::GetRandom (uint32_t range) {
	_RANDOM_SEED = (_RANDOM_SEED != 0) ? (16807 * (_RANDOM_SEED % 127773) - 2836 * (_RANDOM_SEED / 127773)) : _GET_SEED_RANDOM();
	if((int32_t)_RANDOM_SEED <= 0)
		_RANDOM_SEED += 0x7fffffff;
	return range ? _RANDOM_SEED % range : _RANDOM_SEED % 1073741824;
}

uint32_t UINode::GetRandomSeed () {
	return _RANDOM_SEED;
}

void UINode::SetRandomSeed (uint32_t seed) {
	_RANDOM_SEED = (seed != 0) ? (seed) : _GET_SEED_RANDOM();
}

void UINode::Run (const GString& name) {
	
	// If this node is exiting, do nothing, this prevents RunOnRoot from being called many times during a transition
	if(_exit)
		return;
	
	// If the UINode factory does not exist for the given name, do nothing
	if(_FACTORY_LIST == NULL || _FACTORY_LIST->find(name) == _FACTORY_LIST->end()) {
		GConsole::Debug("UINode factory \"%s\" does not exist!\n", (const char*)name);
		return;
	}
	
	// Running a new node will exit this line of nodes by exiting to the root node
	UINode* parent = this;
	while(parent->_parent)
		parent = parent->_parent;
	parent->Exit();
	
	// Run the new node when this node is deleted
	_next = name;
}

void UINode::Exit () {
	_exit = true;
}

void UINode::ExitCancel() {
	_exit = false;
	_next.Delete();
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
	
	// Safety to avoid adding the same node multiple times
	_children.remove(&node);
	_children.push_front(&node);
	
	// A node can only be the child of one parent
	if(node._parent != NULL)
		node._parent->Remove(node);
	node._parent = this;
	
	// Adjust the child's rect to be relative to this node by calling SetRect with it's own rect
	node.SetRect(node._rect);
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
			(*i)->SendDraw();
	}
}

void UINode::SendTouch (int x, int y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouch(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		if(!_passive)
			OnTouch(x, y);
	}
}

void UINode::SendTouchUp (int x, int y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouchUp(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		if(!_passive)
			OnTouchUp(x, y);
	}
}

void UINode::SendTouchMove (int x, int y) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendTouchMove(x + _rect.x - (*i)->_rect.x, y + _rect.y - (*i)->_rect.y);
		if(!_passive)
			OnTouchMove(x, y);
	}
}

void UINode::SendEvent (UINode* node) {
	if(_visible && _active) {
		for(std::list<UINode*>::iterator i = _children.begin(); i != _children.end(); i++)
			(*i)->SendEvent(node);
		if(!_passive)
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
	GSystem::NewTouchCallback(TouchCallback);
	GSystem::NewTouchUpCallback(TouchUpCallback);
	GSystem::NewTouchMoveCallback(TouchMoveCallback);
}

UINode::_Root::~_Root () {
	while(nodes.begin() != nodes.end()) {
		delete (*(nodes.begin()));
		nodes.erase(nodes.begin());
	}
}

void UINode::_Root::RunOnRoot (const GString& name) {
	GConsole::Debug("----------------------------------------------------------------\n");
	GConsole::Debug("%s\n", (const char*)name);
	if(_ROOT != NULL) {
		UINode* node = NewNode(name);
		if(node) _ROOT->nodes.push_back(node);
	}
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
	
	static uint64_t FRAMES = 0;
	static uint64_t FPS = 0;
	FRAMES++;
	FPS += GSystem::GetFPS();
	_ELAPSE = FRAMES * 1000 / FPS;
	_MILLISECONDS += _ELAPSE;
	
	if(_AUTORUN_LIST) {
		while(_AUTORUN_LIST->begin() != _AUTORUN_LIST->end()) {
			RunOnRoot(_AUTORUN_LIST->begin()->first);
			_AUTORUN_LIST->erase(_AUTORUN_LIST->begin());
		}
		delete _AUTORUN_LIST;
		_AUTORUN_LIST = NULL;
	}
	
	if(_ROOT) {
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); ) {
			if((*i)->_exit && (*i)->SendExit()) {
				delete (*i);
				i = _ROOT->nodes.erase(i);
				if(i != _ROOT->nodes.begin())
					i--;
			} else {
				(*i)->SendDraw();
				i++;
			}
		}
	}
}

void UINode::_Root::TouchCallback (int x, int y) {
	if(_ROOT)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void UINode::_Root::TouchUpCallback (int x, int y) {
	if(_ROOT)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

void UINode::_Root::TouchMoveCallback (int x, int y) {
	if(_ROOT)
		for(std::list<UINode*>::iterator i = _ROOT->nodes.begin(); _ROOT && i != _ROOT->nodes.end(); i++)
			(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
}

std::map<GString, UINode* (*) ()>*		UINode::_FACTORY_LIST = NULL;
std::map<GString, bool>*				UINode::_AUTORUN_LIST = NULL;
UINode::_Root*							UINode::_ROOT = NULL;
int64_t									UINode::_MILLISECONDS = 1;
int64_t									UINode::_ELAPSE = 1;
uint32_t								UINode::_RANDOM_SEED = 0;
