#include "GNode.h"

// The default rect for a node is at location 0,0 with a width and height equal to the system's preferred rect.
// All nodes are ultimately relative to the system's preferred rect.

GNode::GNode ()
:	_ref(GSystem::GetUniqueRef())
,	_rect(GSystem::GetPreferredRect().OffsetToZero())
,	_alloc(false)
,	_visible(true)
,	_active(true)
,	_blocking(false)
,	_blocked(false)
,	_exit(false)
,	_parent(nullptr)
{
}

GNode::~GNode () {
	while(!_children.empty()) {
		if(_children.front()->_alloc)
			_AUTO_DELETE_LIST.push_back(_children.front());
		RemoveNode(*_children.front());
	}
	if(_parent)
		_parent->RemoveNode(*this);
}

int GNode::GetUniqueRef () const {
	return _ref;
}

GNode* GNode::GetParent() const {
	return _parent;
}

int GNode::GetWidth () const {
	return _rect.width;
}

int GNode::GetHeight () const {
	return _rect.height;
}

int GNode::GetX () const {
	return _rect.x;
}

int GNode::GetY () const {
	return _rect.y;
}

GRect GNode::GetRect() const {
	return _rect;
}

GRect GNode::GetScreenRect () const {
	GRect screen = GSystem::GetScreenRect();
	GRect preferred = GSystem::GetPreferredRect();
	screen.x -= preferred.x;
	screen.y -= preferred.y;
	for(const GNode* node = this; node != nullptr; node = node->_parent) {
		screen.x -= node->_rect.x;
		screen.y -= node->_rect.y;
	}
	return screen;
}

GRect GNode::GetSafeRect () const {
	GRect safe = GSystem::GetSafeRect();
	GRect preferred = GSystem::GetPreferredRect();
	safe.x -= preferred.x;
	safe.y -= preferred.y;
	for(const GNode* node = this; node != nullptr; node = node->_parent) {
		safe.x -= node->_rect.x;
		safe.y -= node->_rect.y;
	}
	return safe;
}

GRect GNode::GetPreferredRect () const {
	GRect rect = GSystem::GetPreferredRect();
	rect.x = 0;
	rect.y = 0;
	for(const GNode* node = this; node != nullptr; node = node->_parent) {
		rect.x -= node->_rect.x;
		rect.y -= node->_rect.y;
	}
	return rect;
}

void GNode::SetRect (const GRect& rect) {
	_rect = rect;
}

void GNode::SetRectCenterInParent () {
	GRect parent = _parent ? _parent->_rect : GSystem::GetPreferredRect();
	_rect.x = parent.width / 2 - _rect.width / 2;
	_rect.y = parent.height / 2 - _rect.height / 2;
}

void GNode::SetPosition (const GPoint& pos) {
	_rect.x = pos.x;
	_rect.y = pos.y;
}

void GNode::SetVisible (bool visible) {
	_visible = visible;
}

void GNode::SetActive (bool active) {
	_active = active;
}

void GNode::SetBlocking (bool blocking) {
	_blocking = blocking;
	SetNodeAsFront();
}

void GNode::SetNodeAsFront () {
	if(_parent) {
		if(_parent->_children.front() != this) {
			std::list<GNode*>::iterator i = std::find(_parent->_children.begin(), _parent->_children.end(), this);
			if(i != _parent->_children.end())
				_parent->_children.splice(_parent->_children.begin(), _parent->_children, i);
		}
		_parent->SetNodeAsFront();
	} else {
		if(_ROOT_NODE_LIST.front() != this) {
			std::list<GNode*>::iterator i = std::find(_ROOT_NODE_LIST.begin(), _ROOT_NODE_LIST.end(), this);
			if(i != _ROOT_NODE_LIST.end())
				_ROOT_NODE_LIST.splice(_ROOT_NODE_LIST.begin(), _ROOT_NODE_LIST, i);
		}
	}
}

bool GNode::IsVisible () const {
	return _visible;
}

bool GNode::IsActive () const {
	return _active;
}

bool GNode::IsBlocking () const {
	return _blocking;
}

GNode* GNode::NewNode (const GString& name) {
	std::unordered_map<std::string, GNode* (*) ()>::const_iterator i = _FACTORY_MAP.find((const char*)name);
	if(i != _FACTORY_MAP.end())
		return i->second();
	GSystem::Debug("Node factory map does not contain \"%s\"!\n", (const char*)name);
	return nullptr;
}

void GNode::RunNewNode (const GString& name) {
	// If this node is exiting, do nothing to prevent many calls to remove this node
	if(_exit)
		return;
	
	// If the GNode factory does not exist for the given name, do nothing
	if(_FACTORY_MAP.find((const char*)name) == _FACTORY_MAP.end()) {
		GSystem::Debug("Node factory map does not contain \"%s\"!\n", (const char*)name);
		return;
	}
	
	// Only the root node needs an the exit flag set because all children will be checked if an exit should happen and then all children will also be removed, this happends in the SendDraw function
	GNode* parent = this;
	while(parent->_parent)
		parent = parent->_parent;
	parent->_exit = true;
	parent->_next = name;
}

void GNode::RunNewNodeAsChild (const GString& name) {
	GNode* node = NewNode(name);
	if(node) {
		node->_alloc = true;
		node->_parent = this;
		_children.push_front(node);
	}
}

void GNode::ExitNode () {
	_exit = true; // This node (and consequently all it's children) will exit next time SendDraw is called
}

void GNode::ExitNodeCancel() {
	if(_parent) {
		_parent->ExitNodeCancel();
	} else {
		_exit = false;
		_next.Delete();
	}
}

void GNode::AddNode (GNode& node) {
	if(node._parent)
		node._parent->RemoveNode(node);
	node._parent = this;
	_children.push_front(&node);
}

void GNode::RemoveNode (GNode& node) {
	_children.remove(&node);
	node._parent = nullptr;
}

void GNode::SendDraw () {
	if(_visible) {
		GSystem::MatrixSetProjectionDefault();
		GRect screen = GetScreenRect();
		GSystem::MatrixTranslateProjection((float)-screen.x, (float)-screen.y);
		GSystem::MatrixUpdate();
		OnDraw();
		for(std::list<GNode*>::reverse_iterator i = _children.rbegin(); i != _children.rend(); i++)
			(*i)->SendDraw();
	}
	
	if(_exit && SendExit()) {
		if(!_next.IsEmpty())
			_AUTO_RUN_LIST.push_back((const char*)_next);
		if(_parent)
			_parent->RemoveNode(*this);
		if(_alloc)
			_AUTO_DELETE_LIST.push_back(this);
	}
}

void GNode::SendTouch (int x, int y) {
	if(_visible && _active) {
		_blocked = false;
		for(std::list<GNode*>::iterator i = _children.begin(); i != _children.end(); i++) {
			(*i)->SendTouch(x - (*i)->_rect.x, y - (*i)->_rect.y);
			if((*i)->_blocking || (*i)->_blocked) {
				_blocked = true;
				return;
			}
		}
		OnTouch(x, y);
	}
}

void GNode::SendTouchUp (int x, int y) {
	if(_visible && _active) {
		_blocked = false;
		for(std::list<GNode*>::iterator i = _children.begin(); i != _children.end(); i++) {
			(*i)->SendTouchUp(x - (*i)->_rect.x, y - (*i)->_rect.y);
			if((*i)->_blocking || (*i)->_blocked) {
				_blocked = true;
				return;
			}
		}
		OnTouchUp(x, y);
	}
}

void GNode::SendTouchMove (int x, int y) {
	if(_visible && _active) {
		_blocked = false;
		for(std::list<GNode*>::iterator i = _children.begin(); i != _children.end(); i++) {
			(*i)->SendTouchMove(x - (*i)->_rect.x, y - (*i)->_rect.y);
			if((*i)->_blocking || (*i)->_blocked) {
				_blocked = true;
				return;
			}
		}
		OnTouchMove(x, y);
	}
}

void GNode::SendEvent (GNode* node) {
	if(_visible && _active) {
		_blocked = false;
		for(std::list<GNode*>::iterator i = _children.begin(); i != _children.end(); i++) {
			(*i)->SendEvent(node);
			if((*i)->_blocking || (*i)->_blocked) {
				_blocked = true;
				return;
			}
		}
		OnEvent(node);
	}
}

bool GNode::SendExit () {
	// All children must agree on the exit or SendExit returns false and no more checking occurs
	for(std::list<GNode*>::iterator i = _children.begin(); i != _children.end(); i++)
		if(!(*i)->SendExit())
			return false;
	return OnExit();
}

void GNode::_DrawCallback () {
	_MILLISECONDS = ++_FRAMES * 1000 / GSystem::GetFPS();
	_ELAPSE = _MILLISECONDS - (_FRAMES - 1) * 1000 / GSystem::GetFPS();
	
	while(!_AUTO_RUN_LIST.empty()) {
		GSystem::Debug("----------------------------------------------------------------\n");
		GSystem::Debug("%s\n", _AUTO_RUN_LIST.front().c_str());
		GSystem::Debug("----------------------------------------------------------------\n");
		GNode* node = NewNode(_AUTO_RUN_LIST.front().c_str());
		if(node) {
			node->_alloc = true;
			_ROOT_NODE_LIST.push_back(node);
		}
		_AUTO_RUN_LIST.erase(_AUTO_RUN_LIST.begin());
	}
	
	for(std::list<GNode*>::iterator i = _ROOT_NODE_LIST.begin(); i != _ROOT_NODE_LIST.end(); i++)
		(*i)->SendDraw();
	
	while(!_AUTO_DELETE_LIST.empty()) {
		_AUTO_DELETE_LIST.unique();
		GNode* node = _AUTO_DELETE_LIST.front();
		std::list<GNode*>::iterator i = std::find(_ROOT_NODE_LIST.begin(), _ROOT_NODE_LIST.end(), node);
		if(i != _ROOT_NODE_LIST.end())
		_ROOT_NODE_LIST.erase(i);
		delete node;
		_AUTO_DELETE_LIST.erase(_AUTO_DELETE_LIST.begin());
	}
}
