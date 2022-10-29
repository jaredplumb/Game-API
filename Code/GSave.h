#ifndef _GSAVE_H_
#define _GSAVE_H_

#include "GSystem.h"

class GSave {
public:
	static bool Read (const GString& name, void* data, uint_t size);
	static bool Write (const GString& name, const void* data, uint_t size);
};

#endif // _GSAVE_H_
