#include "GSave.h"
#include "GFile.h"

static constexpr uint8_t _VERSION = 5;
static constexpr char _IDENTIFIER[] = "SAVE";

bool GSave::Read (const GString& name, void* data, int64_t size) {
	
#if DEBUG
	GSystem::Debug("Reading save file \"%s/%s.sav\" ... ", (const char*)GSystem::GetSaveDirectory(), (const char*)name);
	int64_t elapse = GSystem::GetMilliseconds();
#endif
	
	GFile file;
	if(file.OpenForRead(GString().Format("%s/%s.sav", (const char*)GSystem::GetSaveDirectory(), (const char*)name)) == false) {
		GSystem::Debug("Failed to open file for reading!\n");
		return false;
	}
	
	uint8_t version;
	if(file.Read(&version, sizeof(version)) == false) {
		GSystem::Debug("Failed to read version number!\n");
		return false;
	}
	
	if(version != _VERSION) {
		GSystem::Debug("Save file version %d does not match current version %d!\n", version, _VERSION);
		return false;
	}
	
	char identifier[sizeof(_IDENTIFIER) / sizeof(char)];
	if(file.Read(identifier, sizeof(_IDENTIFIER) / sizeof(char)) == false) {
		GSystem::Debug("Failed to read identifier!\n");
		return false;
	}
	
	if(GString::strncmp(identifier, _IDENTIFIER, sizeof(_IDENTIFIER) / sizeof(char)) != 0) {
		GSystem::Debug("Save file indintifier \"%s\" does not match the current identfier \"%s\"!", identifier, _IDENTIFIER);
		return false;
	}
	
	int64_t archiveSize;
	if(file.Read(&archiveSize, sizeof(archiveSize)) == false) {
		GSystem::Debug("Failed to read archive size!\n");
		return false;
	}
	
	uint8_t archive[archiveSize];
	if(file.Read(archive, archiveSize) == false) {
		GSystem::Debug("Failed to read archive!\n");
		return false;
	}
	
	if(GArchive::Decompress(archive, archiveSize, data, size) != size) {
		GSystem::Debug("Failed to decompress data!\n");
		return false;
	}
	
#if DEBUG
	elapse = GSystem::GetMilliseconds() - elapse;
	GSystem::Debug("Done (%ld ms)\n", elapse);
#endif
	
	return true;
}


bool GSave::Write (const GString& name, const void* data, int64_t size) {
	
#if DEBUG
	GSystem::Debug("Writing save file \"%s/%s.sav\" ... ", (const char*)GSystem::GetSaveDirectory(), (const char*)name);
	int64_t elapse = GSystem::GetMilliseconds();
#endif
	
	GFile file;
	if(file.OpenForWrite(GString().Format("%s/%s.sav", (const char*)GSystem::GetSaveDirectory(), (const char*)name)) == false) {
		GSystem::Debug("Failed to open for writing!\n");
		return false;
	}
	
	if(file.Write(&_VERSION, sizeof(_VERSION)) == false) {
		GSystem::Debug("Failed to write version number!\n");
		return false;
	}
	
	if(file.Write(_IDENTIFIER, sizeof(_IDENTIFIER) / sizeof(char)) == false) {
		GSystem::Debug("Failed to write identifier!\n");
		return false;
	}
	
	int64_t archiveSize = GArchive::GetBufferBounds(size);
	uint8_t archive[archiveSize];
	archiveSize = GArchive::Compress(data, size, archive, archiveSize);
	if(archiveSize == 0) {
		GSystem::Debug("Failed to compress data!\n");
		return false;
	}
	
	if(file.Write(&archiveSize, sizeof(archiveSize)) == false) {
		GSystem::Debug("Failed to write archive size!\n");
		return false;
	}
	
	if(file.Write(archive, archiveSize) == false) {
		GSystem::Debug("Failed to write archive!\n");
		return false;
	}
	
#if DEBUG
	elapse = GSystem::GetMilliseconds() - elapse;
	GSystem::Debug("Done (%ld bytes compressed to %ld bytes in %ld ms)\n", size, archiveSize, elapse);
#endif
	
	return true;
}
