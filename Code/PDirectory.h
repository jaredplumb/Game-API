#ifndef _P_DIRECTORY_H_
#define _P_DIRECTORY_H_

#include "PPlatform.h"
#include "PString.h"

class PDirectory {
public:
	
	PDirectory ();
	~PDirectory ();
	
	PDirectory (const PString& path);
	
	bool Open (const PString& path);
	
	void Close ();
	
	uint_t GetSize () const;
	
	PString GetFile (uint_t index) const;
	
private:
	std::vector<PString> _files; // Files are relative to the path used with Open
};

#endif // _P_DIRECTORY_H_
