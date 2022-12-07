#ifndef HERMES_H_
#define HERMES_H_

#include "GTypes.h"

static const char* VERSION_STRING = "1.1";

class Hermes {
public:
	static GString GetVersionString () { return VERSION_STRING; };
	static bool Build (const GString& path);
};

#endif // HERMES_H_
