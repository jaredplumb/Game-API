#ifndef _P_THREAD_H_
#define _P_THREAD_H_

#include "PPlatform.h"

#if PLATFORM_IOS || PLATFORM_MACOSX

class PThread {
public:
	inline PThread ()														: _thread(NULL) {}
	virtual ~PThread ()														{ Finish(); }
	inline void Start ()													{ Finish(); pthread_create(&_thread, NULL, _ThreadProc, this); }
	inline void Finish ()													{ if(_thread) { pthread_join(_thread, NULL); _thread = NULL; } }
	inline void Sleep (uint64 milliseconds)									{ timespec time = { (long)(milliseconds / 1000), (long)(milliseconds % 1000 * 1000000) }; nanosleep(&time, NULL); }
	virtual void Run () = 0;
private:
	pthread_t _thread;
	static void* _ThreadProc (void* data)									{ ((PThread *)data)->Run(); return NULL; }
};

#elif PLATFORM_WINDOWS

class PThread {
public:
	inline PThread ()														: _thread(NULL) {}
	virtual ~PThread ()														{ Finish(); }
	inline void Start ()													{ Finish(); _thread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL); }
	inline void Finish ()													{ if(_thread) { WaitForSingleObject(_thread, INFINITE); CloseHandle(_thread); _thread = NULL; } }
	inline void Sleep (uint64 milliseconds)									{ Sleep(milliseconds); }
	virtual void Run () = 0;
private:
	HANDLE _thread;
	static DWORD WINAPI _ThreadProc (LPVOID lpParam)						{ ((PThread *)lpParam)->Run(); return 0; }
};

#endif

#endif // _P_THREAD_H_
