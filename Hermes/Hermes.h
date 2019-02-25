#ifndef _HERMES_H_
#define _HERMES_H_

#include "Game.h"

static const char* VERSION_STRING = "1.0.2";

class Hermes {
public:
	static GString GetVersionString () { return VERSION_STRING; };
	static bool Build (const GString& path);
};

#endif // _HERMES_H_
