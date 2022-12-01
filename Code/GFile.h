#ifndef G_FILE_H_
#define G_FILE_H_

#include "GTypes.h"
#include "GSystem.h"
#include <cstdint>

class GFile {
public:
	inline GFile ()											: _file(nullptr) {}
	inline ~GFile ()										{ Close(); }
	inline void Close ()									{ if(_file) { fclose(_file); _file = nullptr; } }
	
	inline bool OpenForRead (const GString& path)			{ Close(); _file = fopen(path, "rb"); return _file != nullptr; }
	inline bool OpenForWrite (const GString& path)			{ Close(); _file = fopen(path, "wb+"); return _file != nullptr; }
	inline bool OpenForAppend (const GString& path)			{ Close(); _file = fopen(path, "ab+"); return _file != nullptr; }
	inline bool OpenResourceForRead (const GString& path)	{ Close(); _file = GSystem::OpenResourceFileForRead(path); return _file != nullptr; }
	inline bool OpenSaveForRead (const GString& name)		{ Close(); _file = GSystem::OpenSaveFileForRead(name); return _file != nullptr; }
	inline bool OpenSaveForWrite (const GString& name)		{ Close(); _file = GSystem::OpenSaveFileForWrite(name); return _file != nullptr; }
	
	inline bool Read (void* data, int64_t size)				{ return _file != nullptr && fread(data, size, 1, _file) == 1; }
	inline bool Write (const void* data, int64_t size)		{ return _file != nullptr && fwrite(data, size, 1, _file) == 1; }
	
	inline int64_t GetPosition () const						{ return _file ? (int64_t)ftello(_file) : 0; }
	inline int64_t GetSize () const							{ if(_file == nullptr) return 0; off_t current = ftello(_file); fseeko(_file, 0, SEEK_END); off_t size = ftello(_file); fseeko(_file, current, SEEK_SET); return (int64_t)size; }
	inline bool SetPosition (int64_t position)				{ return _file != nullptr && fseeko(_file, (off_t)position, SEEK_SET) == 0; }
	inline bool IsOpen () const								{ return _file != nullptr; }
	
private:
	FILE* _file;
};

#endif // G_FILE_H_
