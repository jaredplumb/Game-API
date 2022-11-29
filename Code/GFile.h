#ifndef GFILE_H_
#define GFILE_H_

#include "GTypes.h"
#include <cstdint>
#include <vector>

/// A GFile is a wrapper class around C-File function calls.  This class uses the
/// native width versions of the file functions allowing for very large files.  The 
/// implemnation is flatform dependent attempting to use a C FILE object 
/// whenever possible.
class GFile {
public:
	GFile ();
	
	/// This constructor calls OpenForRead
	GFile (const GString& path);
	
	~GFile ();
	
	/// Opens a file for reading only
	bool OpenForRead (const GString& path);
	
	/// Opens a file for reading and writing
	bool OpenForWrite (const GString& path);
	
	/// Appends a file for reading and writing
	bool OpenForAppend (const GString& path);
	
	/// Closes this file
	void Close ();
	
	/// Read data from this file
	bool Read (void* data, int64_t size);
	
	/// Write data to this file
	bool Write (const void* data, int64_t size);
	
	/// Is this file currently open
	bool IsOpen () const;
	
	/// Returns the current position for reading and writing in the file
	int64_t GetPosition () const;
	
	/// Returns the size of the file
	int64_t GetSize () const;
	
	/// Sets the reading and writing position in the file
	bool SetPosition (int64_t position);
	
	/// Returns a list of file names in the directory relative to the path sent into the function and all sub directories
	static std::vector<GString> GetFileNamesInDirectory (const GString& path);
	
private:
	FILE* _file;
};

#endif // GFILE_H_
