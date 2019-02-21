#include "GSystem.h"

static uint32															_RANDOM_SEED = 0;
static std::map<int_t, void (*) ()>*									_STARTUP_CALLBACKS = NULL;
static std::map<int_t, void (*) ()>*									_SHUTDOWN_CALLBACKS = NULL;
static std::map<int_t, void (*) ()>*									_DRAW_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y, int_t button)>*		_MOUSE_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y, int_t button)>*		_MOUSEUP_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y)>*					_MOUSEMOVE_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y, int_t button)>*		_MOUSEDRAG_CALLBACKS = NULL;
static std::map<int_t, void (*) (float xdelta, float ydelta)>*			_MOUSEWHEEL_CALLBACKS = NULL;
static std::map<int_t, void (*) (vkey_t key)>*							_KEY_CALLBACKS = NULL;
static std::map<int_t, void (*) (vkey_t key)>*							_KEYUP_CALLBACKS = NULL;
static std::map<int_t, void (*) (char key)>*							_ASCII_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y)>*					_TOUCH_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y)>*					_TOUCHUP_CALLBACKS = NULL;
static std::map<int_t, void (*) (int_t x, int_t y)>*					_TOUCHMOVE_CALLBACKS = NULL;

// This random generator is from the BSD random, which is from:
// "Random number generators: good ones are hard to find"
// Park and Miller, Communications of the ACM, vol. 31, no. 10, October 1988, p. 1195.
uint32 GSystem::GetRandom (uint32 range) {
	if(_RANDOM_SEED == 0)
		_RANDOM_SEED = (uint32)GetMilliseconds() + 1;
	_RANDOM_SEED = 16807 * (_RANDOM_SEED % 127773) - 2836 * (_RANDOM_SEED / 127773);
	if((int32)_RANDOM_SEED <= 0)
		_RANDOM_SEED += 0x7fffffff;
	return range ? _RANDOM_SEED % range : _RANDOM_SEED;
}

uint32 GSystem::SetRandomSeed (uint32 seed) {
	uint32 original = _RANDOM_SEED;
	_RANDOM_SEED = seed;
	return original;
}

uint64 GSystem::GetHash (const uint8* bytes) {
	// en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
	uint64 hash = 0xcbf29ce484222325;
	while(*bytes)
		hash = (hash * 1099511628211) ^ *bytes++;
	return hash;
}

int_t GSystem::NewStartupCallback (void (* callback) ()) {
	if(_STARTUP_CALLBACKS == NULL) _STARTUP_CALLBACKS = new std::map<int_t, void (*) ()>;
	int_t ref = GetUniqueRef();
	_STARTUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewShutdownCallback (void (* callback) ()) {
	if(_SHUTDOWN_CALLBACKS == NULL) _SHUTDOWN_CALLBACKS = new std::map<int_t, void (*) ()>;
	int_t ref = GetUniqueRef();
	_SHUTDOWN_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewDrawCallback (void (* callback) ()) {
	if(_DRAW_CALLBACKS == NULL) _DRAW_CALLBACKS = new std::map<int_t, void (*) ()>;
	int_t ref = GetUniqueRef();
	_DRAW_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewMouseCallback (void (* callback) (int_t x, int_t y, int_t button)) {
	if(_MOUSE_CALLBACKS == NULL) _MOUSE_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y, int_t button)>;
	int_t ref = GetUniqueRef();
	_MOUSE_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewMouseUpCallback (void (* callback) (int_t x, int_t y, int_t button)) {
	if(_MOUSEUP_CALLBACKS == NULL) _MOUSEUP_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y, int_t button)>;
	int_t ref = GetUniqueRef();
	_MOUSEUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewMouseMoveCallback (void (* callback) (int_t x, int_t y)) {
	if(_MOUSEMOVE_CALLBACKS == NULL) _MOUSEMOVE_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y)>;
	int_t ref = GetUniqueRef();
	_MOUSEMOVE_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewMouseDragCallback (void (* callback) (int_t x, int_t y, int_t button)) {
	if(_MOUSEDRAG_CALLBACKS == NULL) _MOUSEDRAG_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y, int_t button)>;
	int_t ref = GetUniqueRef();
	_MOUSEDRAG_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewMouseWheelCallback (void (* callback) (float xdelta, float ydelta)) {
	if(_MOUSEWHEEL_CALLBACKS == NULL) _MOUSEWHEEL_CALLBACKS = new std::map<int_t, void (*) (float xdelta, float ydelta)>;
	int_t ref = GetUniqueRef();
	_MOUSEWHEEL_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewKeyCallback (void (* callback) (vkey_t key)) {
	if(_KEY_CALLBACKS == NULL) _KEY_CALLBACKS = new std::map<int_t, void (*) (vkey_t key)>;
	int_t ref = GetUniqueRef();
	_KEY_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewKeyUpCallback (void (* callback) (vkey_t key)) {
	if(_KEYUP_CALLBACKS == NULL) _KEYUP_CALLBACKS = new std::map<int_t, void (*) (vkey_t key)>;
	int_t ref = GetUniqueRef();
	_KEYUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewASCIICallback (void (* callback) (char key)) {
	if(_ASCII_CALLBACKS == NULL) _ASCII_CALLBACKS = new std::map<int_t, void (*) (char key)>;
	int_t ref = GetUniqueRef();
	_ASCII_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewTouchCallback (void (* callback) (int_t x, int_t y)) {
	if(_TOUCH_CALLBACKS == NULL) _TOUCH_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y)>;
	int_t ref = GetUniqueRef();
	_TOUCH_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewTouchUpCallback (void (* callback) (int_t x, int_t y)) {
	if(_TOUCHUP_CALLBACKS == NULL) _TOUCHUP_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y)>;
	int_t ref = GetUniqueRef();
	_TOUCHUP_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

int_t GSystem::NewTouchMoveCallback (void (* callback) (int_t x, int_t y)) {
	if(_TOUCHMOVE_CALLBACKS == NULL) _TOUCHMOVE_CALLBACKS = new std::map<int_t, void (*) (int_t x, int_t y)>;
	int_t ref = GetUniqueRef();
	_TOUCHMOVE_CALLBACKS->insert(std::make_pair(ref, callback));
	return ref;
}

void GSystem::DeleteStartupCallback (int_t ref) {
	if(_STARTUP_CALLBACKS) _STARTUP_CALLBACKS->erase(_STARTUP_CALLBACKS->find(ref));
}

void GSystem::DeleteShutdownCallback (int_t ref) {
	if(_SHUTDOWN_CALLBACKS) _SHUTDOWN_CALLBACKS->erase(_SHUTDOWN_CALLBACKS->find(ref));
}

void GSystem::DeleteDrawCallback (int_t ref) {
	if(_DRAW_CALLBACKS) _DRAW_CALLBACKS->erase(_DRAW_CALLBACKS->find(ref));
}

void GSystem::DeleteMouseCallback (int_t ref) {
	if(_MOUSE_CALLBACKS) _MOUSE_CALLBACKS->erase(_MOUSE_CALLBACKS->find(ref));
}

void GSystem::DeleteMouseUpCallback (int_t ref) {
	if(_MOUSEUP_CALLBACKS) _MOUSEUP_CALLBACKS->erase(_MOUSEUP_CALLBACKS->find(ref));
}

void GSystem::DeleteMouseMoveCallback (int_t ref) {
	if(_MOUSEMOVE_CALLBACKS) _MOUSEMOVE_CALLBACKS->erase(_MOUSEMOVE_CALLBACKS->find(ref));
}

void GSystem::DeleteMouseDragCallback (int_t ref) {
	if(_MOUSEDRAG_CALLBACKS) _MOUSEDRAG_CALLBACKS->erase(_MOUSEDRAG_CALLBACKS->find(ref));
}

void GSystem::DeleteMouseWheelCallback (int_t ref) {
	if(_MOUSEWHEEL_CALLBACKS) _MOUSEWHEEL_CALLBACKS->erase(_MOUSEWHEEL_CALLBACKS->find(ref));
}

void GSystem::DeleteKeyCallback (int_t ref) {
	if(_KEY_CALLBACKS) _KEY_CALLBACKS->erase(_KEY_CALLBACKS->find(ref));
}

void GSystem::DeleteKeyUpCallback (int_t ref) {
	if(_KEYUP_CALLBACKS) _KEYUP_CALLBACKS->erase(_KEYUP_CALLBACKS->find(ref));
}

void GSystem::DeleteASCIICallback (int_t ref) {
	if(_ASCII_CALLBACKS) _ASCII_CALLBACKS->erase(_ASCII_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchCallback (int_t ref) {
	if(_TOUCH_CALLBACKS) _TOUCH_CALLBACKS->erase(_TOUCH_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchUpCallback (int_t ref) {
	if(_TOUCHUP_CALLBACKS) _TOUCHUP_CALLBACKS->erase(_TOUCHUP_CALLBACKS->find(ref));
}

void GSystem::DeleteTouchMoveCallback (int_t ref) {
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
	if(_MOUSE_CALLBACKS) {
		delete _MOUSE_CALLBACKS;
		_MOUSE_CALLBACKS = NULL;
	}
	if(_MOUSEUP_CALLBACKS) {
		delete _MOUSEUP_CALLBACKS;
		_MOUSEUP_CALLBACKS = NULL;
	}
	if(_MOUSEMOVE_CALLBACKS) {
		delete _MOUSEMOVE_CALLBACKS;
		_MOUSEMOVE_CALLBACKS = NULL;
	}
	if(_MOUSEDRAG_CALLBACKS) {
		delete _MOUSEDRAG_CALLBACKS;
		_MOUSEDRAG_CALLBACKS = NULL;
	}
	if(_MOUSEWHEEL_CALLBACKS) {
		delete _MOUSEWHEEL_CALLBACKS;
		_MOUSEWHEEL_CALLBACKS = NULL;
	}
	if(_KEY_CALLBACKS) {
		delete _KEY_CALLBACKS;
		_KEY_CALLBACKS = NULL;
	}
	if(_KEYUP_CALLBACKS) {
		delete _KEYUP_CALLBACKS;
		_KEYUP_CALLBACKS = NULL;
	}
	if(_ASCII_CALLBACKS) {
		delete _ASCII_CALLBACKS;
		_ASCII_CALLBACKS = NULL;
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
		for(std::map<int_t, void (*) ()>::iterator i = _STARTUP_CALLBACKS->begin(); i != _STARTUP_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunShutdownCallbacks () {
	if(_SHUTDOWN_CALLBACKS)
		for(std::map<int_t, void (*) ()>::iterator i = _SHUTDOWN_CALLBACKS->begin(); i != _SHUTDOWN_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunDrawCallbacks () {
	if(_DRAW_CALLBACKS)
		for(std::map<int_t, void (*) ()>::iterator i = _DRAW_CALLBACKS->begin(); i != _DRAW_CALLBACKS->end(); i++)
			i->second();
}

void GSystem::RunMouseCallbacks (int_t x, int_t y, int_t button) {
	if(_MOUSE_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t, int_t)>::iterator i = _MOUSE_CALLBACKS->begin(); i != _MOUSE_CALLBACKS->end(); i++)
			i->second(x, y, button);
}

void GSystem::RunMouseUpCallbacks (int_t x, int_t y, int_t button) {
	if(_MOUSEUP_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t, int_t)>::iterator i = _MOUSEUP_CALLBACKS->begin(); i != _MOUSEUP_CALLBACKS->end(); i++)
			i->second(x, y, button);
}

void GSystem::RunMouseMoveCallbacks (int_t x, int_t y) {
	if(_MOUSEMOVE_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t)>::iterator i = _MOUSEMOVE_CALLBACKS->begin(); i != _MOUSEMOVE_CALLBACKS->end(); i++)
			i->second(x, y);
}

void GSystem::RunMouseDragCallbacks (int_t x, int_t y, int_t button) {
	if(_MOUSEDRAG_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t, int_t)>::iterator i = _MOUSEDRAG_CALLBACKS->begin(); i != _MOUSEDRAG_CALLBACKS->end(); i++)
			i->second(x, y, button);
}

void GSystem::RunMouseWheelCallbacks (float xdelta, float ydelta) {
	if(_MOUSEWHEEL_CALLBACKS)
		for(std::map<int_t, void (*) (float, float)>::iterator i = _MOUSEWHEEL_CALLBACKS->begin(); i != _MOUSEWHEEL_CALLBACKS->end(); i++)
			i->second(xdelta, ydelta);
}

void GSystem::RunKeyCallbacks (vkey_t key) {
	if(_KEY_CALLBACKS)
		for(std::map<int_t, void (*) (vkey_t)>::iterator i = _KEY_CALLBACKS->begin(); i != _KEY_CALLBACKS->end(); i++)
			i->second(key);
}

void GSystem::RunKeyUpCallbacks (vkey_t key) {
	if(_KEYUP_CALLBACKS)
		for(std::map<int_t, void (*) (vkey_t)>::iterator i = _KEYUP_CALLBACKS->begin(); i != _KEYUP_CALLBACKS->end(); i++)
			i->second(key);
}

void GSystem::RunASCIICallbacks (char key) {
	if(_ASCII_CALLBACKS)
		for(std::map<int_t, void (*) (char)>::iterator i = _ASCII_CALLBACKS->begin(); i != _ASCII_CALLBACKS->end(); i++)
			i->second(key);
}

void GSystem::RunTouchCallbacks (int_t x, int_t y) {
	if(_TOUCH_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t)>::iterator i = _TOUCH_CALLBACKS->begin(); i != _TOUCH_CALLBACKS->end(); i++)
			i->second(x, y);
}

void GSystem::RunTouchUpCallbacks (int_t x, int_t y) {
	if(_TOUCHUP_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t)>::iterator i = _TOUCHUP_CALLBACKS->begin(); i != _TOUCHUP_CALLBACKS->end(); i++)
			i->second(x, y);
}

void GSystem::RunTouchMoveCallbacks (int_t x, int_t y) {
	if(_TOUCHMOVE_CALLBACKS)
		for(std::map<int_t, void (*) (int_t, int_t)>::iterator i = _TOUCHMOVE_CALLBACKS->begin(); i != _TOUCHMOVE_CALLBACKS->end(); i++)
			i->second(x, y);
}
