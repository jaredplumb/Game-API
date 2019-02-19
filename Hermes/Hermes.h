#ifndef _HERMES_H_
#define _HERMES_H_

#include "Plumb2D.h"

static const char* VERSION_STRING = "1.0.2";

class Hermes {
public:
	static PString GetVersionString () { return VERSION_STRING; };
	static bool Build (const PString& path);
};

#endif // _HERMES_H_
