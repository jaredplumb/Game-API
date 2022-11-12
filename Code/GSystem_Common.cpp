#include "GSystem.h"
#include <map>

static std::map<int, void (*) ()>*				_STARTUP_CALLBACKS = NULL;
static std::map<int, void (*) ()>*				_SHUTDOWN_CALLBACKS = NULL;
static std::map<int, void (*) ()>*				_DRAW_CALLBACKS = NULL;
static std::map<int, void (*) (int x, int y)>*	_TOUCH_CALLBACKS = NULL;
static std::map<int, void (*) (int x, int y)>*	_TOUCHUP_CALLBACKS = NULL;
static std::map<int, void (*) (int x, int y)>*	_TOUCHMOVE_CALLBACKS = NULL;

//uint64 GSystem::GetHash (const uint8* bytes) {
	// en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
//	uint64 hash = 0xcbf29ce484222325;
//	while(*bytes)
//		hash = (hash * 1099511628211) ^ *bytes++;
//	return hash;
//}

int GSystem::NewStartupCallback (void (* callback) ()) {
	if(_STARTUP_CALLBACKS == NULL) _STARTUP_CALLBACKS = new std::map<int, void (*) ()>;
	int ref = GetUniqueRef();
	_STARTUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int GSystem::NewShutdownCallback (void (* callback) ()) {
	if(_SHUTDOWN_CALLBACKS == NULL) _SHUTDOWN_CALLBACKS = new std::map<int, void (*) ()>;
	int ref = GetUniqueRef();
	_SHUTDOWN_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int GSystem::NewDrawCallback (void (* callback) ()) {
	if(_DRAW_CALLBACKS == NULL) _DRAW_CALLBACKS = new std::map<int, void (*) ()>;
	int ref = GetUniqueRef();
	_DRAW_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int GSystem::NewTouchCallback (void (* callback) (int x, int y)) {
	if(_TOUCH_CALLBACKS == NULL) _TOUCH_CALLBACKS = new std::map<int, void (*) (int x, int y)>;
	int ref = GetUniqueRef();
	_TOUCH_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int GSystem::NewTouchUpCallback (void (* callback) (int x, int y)) {
	if(_TOUCHUP_CALLBACKS == NULL) _TOUCHUP_CALLBACKS = new std::map<int, void (*) (int x, int y)>;
	int ref = GetUniqueRef();
	_TOUCHUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int GSystem::NewTouchMoveCallback (void (* callback) (int x, int y)) {
	if(_TOUCHMOVE_CALLBACKS == NULL) _TOUCHMOVE_CALLBACKS = new std::map<int, void (*) (int x, int y)>;
	int ref = GetUniqueRef();
	_TOUCHMOVE_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

void GSystem::DeleteStartupCallback (int ref) {
	if(_STARTUP_CALLBACKS) _STARTUP_CALLBACKS->erase(_STARTUP_CALLBACKS->find(ref));
}

void GSystem::DeleteShutdownCallback (int ref) {
	if(_SHUTDOWN_CALLBACKS) _SHUTDOWN_CALLBACKS->erase(_SHUTDOWN_CALLBACKS->find(ref));
}

void GSystem::DeleteDrawCallback (int ref) {
	if(_DRAW_CALLBACKS) _DRAW_CALLBACKS->erase(_DRAW_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchCallback (int ref) {
	if(_TOUCH_CALLBACKS) _TOUCH_CALLBACKS->erase(_TOUCH_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchUpCallback (int ref) {
	if(_TOUCHUP_CALLBACKS) _TOUCHUP_CALLBACKS->erase(_TOUCHUP_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchMoveCallback (int ref) {
	if(_TOUCHMOVE_CALLBACKS) _TOUCHMOVE_CALLBACKS->erase(_TOUCHMOVE_CALLBACKS->find(ref));
}

void GSystem::DeleteAllCallbacks () {
	if(_STARTUP_CALLBACKS) {
		delete _STARTUP_CALLBACKS;
		_STARTUP_CALLBACKS = NULL;
	}
	if(_SHUTDOWN_CALLBACKS) {
		delete _SHUTDOWN_CALLBACKS;
		_SHUTDOWN_CALLBACKS = NULL;
	}
	if(_DRAW_CALLBACKS) {
		delete _DRAW_CALLBACKS;
		_DRAW_CALLBACKS = NULL;
	}
	if(_TOUCH_CALLBACKS) {
		delete _TOUCH_CALLBACKS;
		_TOUCH_CALLBACKS = NULL;
	}
	if(_TOUCHUP_CALLBACKS) {
		delete _TOUCHUP_CALLBACKS;
		_TOUCHUP_CALLBACKS = NULL;
	}
	if(_TOUCHMOVE_CALLBACKS) {
		delete _TOUCHMOVE_CALLBACKS;
		_TOUCHMOVE_CALLBACKS = NULL;
	}
}

void GSystem::RunStartupCallbacks () {
	if(_STARTUP_CALLBACKS)
		for(std::map<int, void (*) ()>::iterator i = _STARTUP_CALLBACKS->begin(); i != _STARTUP_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunShutdownCallbacks () {
	if(_SHUTDOWN_CALLBACKS)
		for(std::map<int, void (*) ()>::iterator i = _SHUTDOWN_CALLBACKS->begin(); i != _SHUTDOWN_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunDrawCallbacks () {
	if(_DRAW_CALLBACKS)
		for(std::map<int, void (*) ()>::iterator i = _DRAW_CALLBACKS->begin(); i != _DRAW_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunTouchCallbacks (int x, int y) {
	if(_TOUCH_CALLBACKS)
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCH_CALLBACKS->begin(); i != _TOUCH_CALLBACKS->end(); i++)
			i->second(x, y);
}

void GSystem::RunTouchUpCallbacks (int x, int y) {
	if(_TOUCHUP_CALLBACKS)
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCHUP_CALLBACKS->begin(); i != _TOUCHUP_CALLBACKS->end(); i++)
			i->second(x, y);
}

void GSystem::RunTouchMoveCallbacks (int x, int y) {
	if(_TOUCHMOVE_CALLBACKS)
		for(std::map<int, void (*) (int, int)>::iterator i = _TOUCHMOVE_CALLBACKS->begin(); i != _TOUCHMOVE_CALLBACKS->end(); i++)
			i->second(x, y);
}
