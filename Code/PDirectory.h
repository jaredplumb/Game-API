#ifndef _P_DIRECTORY_H_
#define _P_DIRECTORY_H_

#include "GTypes.h"

class PDirectory {
public:
	
	PDirectory ();
	~PDirectory ();
	
	PDirectory (const GString& path);
	
	bool Open (const GString& path);
	
	void Close ();
	
	uint_t GetSize () const;
	
	GString GetFile (uint_t index) const;
	
private:
	std::vector<GString> _files; // Files are relative to the path used with Open
};

#endif // _P_DIRECTORY_H_
