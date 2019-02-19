#ifndef _P_FILE_H_
#define _P_FILE_H_

#include "PPlatform.h"
#include "PString.h"

/// A PFile is a wrapper class around a C-File function calls.  This class uses the
/// native width versions of the file functions allowing for very large files.
class PFile {
public:
	
	PFile ();
	~PFile ();
	
	/// This constructor calls OpenForRead
	PFile (const PString& path);
	
	/// Opens a file for reading only
	bool OpenForRead (const PString& path);
	
	/// Opens a file for reading and writing
	bool OpenForWrite (const PString& path);
	
	/// Appends a file for reading and writing
	bool OpenForAppend (const PString& path);
	
	/// Closes this file
	void Close ();
	
	/// Read data from this file
	bool Read (void* data, uint_t size);
	
	/// Write data to this file
	bool Write (const void* data, uint_t size);
	
	/// Is this file currently open
	bool IsOpen () const;
	
	/// Returns the current position for reading and writing in the file
	uint_t GetPosition () const;
	
	/// Returns the size of the file
	uint_t GetSize () const;
	
	/// Sets the reading and writing position in the file
	bool SetPosition (uint_t position);
	
private:
	FILE* _file;
};

#endif // _P_FILE_H_
