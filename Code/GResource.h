#ifndef G_RESOURCE_H_
#define G_RESOURCE_H_

#include "GTypes.h"
#include <cstdint>
#include <vector>

// A resource is a platform dependent object that allows for reading and sometimes writing of files.
// On MacOS and iOS this is a simple wrapper around a standard C object.
// On Android this reads from the asset manager.
class GResource {
public:
	inline GResource ()							: _resource(nullptr) {}
	inline GResource (const GString& path)		: _resource(nullptr) { OpenForRead(path); }
	inline ~GResource ()						{ Close(); }
	
	bool OpenForRead (const GString& path);
	bool OpenForWrite (const GString& path); // Not all platforms support this function
	bool OpenForAppend (const GString& path); // Not all platforms support this function
	void Close ();
	
	bool Read (void* data, int64_t size);
	bool Write (const void* data, int64_t size);  // Not all platforms support this function
	
	int64_t GetPosition () const;
	int64_t GetSize () const;
	bool SetPosition (int64_t position);
	inline bool IsOpen () const					{ return _resource != nullptr; }
	
	// Returns a list of file names in the directory provided by the path including all sub directories
	static std::vector<GString> GetFileNamesInDirectory (const GString& path);
	
private:
	void* _resource;
};

#endif // G_RESOURCE_H_
