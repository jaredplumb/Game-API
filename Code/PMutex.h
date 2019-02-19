#ifndef _P_MUTEX_H_
#define _P_MUTEX_H_

#include "PPlatform.h"

#if PLATFORM_IOS || PLATFORM_MACOSX

class PMutex {
public:
	inline PMutex ()														: _mutex(new pthread_mutex_t) { pthread_mutex_init(_mutex, NULL); }
	inline ~PMutex ()														{ if(_mutex) { pthread_mutex_destroy(_mutex); delete _mutex; _mutex = NULL; } }
	inline bool Lock ()														{ return pthread_mutex_lock(_mutex) == 0; }
	inline bool Unlock ()													{ return pthread_mutex_unlock(_mutex) == 0; }
private:
	pthread_mutex_t* _mutex;
};

#elif PLATFORM_WINDOWS

class PMutex {
public:
	inline PMutex ()														: _mutex(CreateMutex(NULL, FALSE, NULL)) {}
	inline ~PMutex ()														{ if (_mutex) { CloseHandle(_mutex); _mutex = NULL; } }
	inline bool Lock ()														{ return WaitForSingleObject(_mutex, INFINITE) == WAIT_OBJECT_0; }
	inline bool Unlock ()													{ return ReleaseMutex(_mutex) == TRUE; }
private:
	HANDLE _mutex;
};

#endif

#endif // _P_MUTEX_H_
