#include "GSave.h"
#if PLATFORM_MACOSX || PLATFORM_IOS


// TODO: I don't like this method
// THe encrypted portions should be brought into Read and Write
// GFile should be used instead of FILE
// Also, I think I am writing twice in the Write function



// TODO: Change to caps
static uint8_t _version = 3;
static const char _identifier[] = "save";


static const char* _GetDocumentDirectory () {
	static GString _DIRECTORY;
	if(_DIRECTORY.IsEmpty()) {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		if([paths count] > 0)
			_DIRECTORY.New([[paths objectAtIndex:0] fileSystemRepresentation]);
	}
	return (const char*)_DIRECTORY;
}

static bool _Read (void* data, uint_t size, FILE* file ) {
	return fread(data, size, 1, file) == 1;
}

static bool _ReadEncrypted (void* data, uint_t size, uint8 key[32], FILE* file) {
	bool result = _Read(data, size, file);
	for(uint_t i = 0; i < size; i++)
		((uint8*)data)[i] -= key[i % 32];
	return result;
}

//static bool _Write (const void* data, uint_t size, FILE* file) {
//	return fwrite(data, size, 1, file) == 1;
//}

//static bool _WriteEncrypted (void* data, uint_t size, uint8 key[32], FILE* file) {
//	for(uint_t i = 0; i < size; i++)
//		((uint8*)data)[i] += key[i % 32];
//	return _Write(data, size, file);
//}

//static bool _WriteEncryptedCopy (const void* data, uint_t size, uint8 key[32], FILE* file) {
//	uint8 copy[size];
//	memcpy(copy, data, size);
//	bool result = _WriteEncrypted(copy, size, key, file);
//	return result;
//}





bool GSave::Read (const GString& name, void* data, uint_t size) {
	if(name == NULL || data == NULL || size == 0) return false;
	
	FILE* file = fopen(GString().Format("%s/%s.sav", _GetDocumentDirectory(), (const char *)name), "rb");
	
	if(file) {
		
		uint8_t version;
		if(_Read(&version, sizeof(version), file) == false) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to read version)!", (const char*)name);
			fclose(file);
			return false;
		}
		
		if(version != _version) {
			GConsole::Debug("The save file \"%s\" does not match the current version!", (const char*)name);
			fclose(file);
			return false;
		}
		
		char identifier[sizeof(_identifier)];
		if(_Read(identifier, sizeof(_identifier) - 1, file) == false) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to read identifier)!", (const char*)name);
			fclose(file);
			return false;
		}
		identifier[sizeof(_identifier) - 1] = '\0';
		
		if(GString::strcmp(identifier, _identifier) != 0) {
			GConsole::Debug("The save file \"%s\" does not match the current identfier!", (const char*)name);
			fclose(file);
			return false;
		}
		
		uint8 key[32];
		if(_Read(key, sizeof(uint8) * 32, file) == false) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to read the key)!", (const char*)name);
			fclose(file);
			return false;
		}
		
		uint_t archiveSize;
		if(_ReadEncrypted(&archiveSize, sizeof(archiveSize), key, file) == false) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to read the archive size)!", (const char*)name);
			fclose(file);
			return false;
		}
		
		uint8 archive[archiveSize];
		if(_ReadEncrypted(archive, archiveSize, key, file) == false) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to read the archive)!", (const char*)name);
			fclose(file);
			return false;
		}
		
		fclose(file);
		
		uint_t dataSize = size;
		if((dataSize = GArchive::Decompress(archive, archiveSize, (uint8*)data, dataSize)) == 0) {
			GConsole::Debug("The save file \"%s\" is corrupted (failed to decompress the archive)!", (const char*)name);
			return false;
		}
		
		return dataSize == size;
	}
	
	return false;
}








// A save data is compressed and saved to a file.  The name is used and a ".sav" is 
// appended to the end of the name to create the file name.
bool GSave::Write (const GString& name, const void* data, uint_t size) {
	if(name == NULL || data == NULL || size == 0) return false;
	
#if _DEBUG
	uint64 time = GTime::GetMilliseconds();
#endif
	
	FILE * file = fopen(GString().Format("%s/%s.sav", _GetDocumentDirectory(), (const char*)name), "wb+");
	if(file) {
		uint_t archiveSize = GArchive::GetBufferBounds(size);
		uint8 archive[archiveSize];
		
		archiveSize = GArchive::Compress((uint8*)data, size, archive, archiveSize);
		if(archiveSize == 0) {
			GConsole::Debug("Could not compress data \"%s\"!", (const char *)name);
			fclose(file);
			return false;
		}
		
		//uint8 key[32];
		//for(int_t i = 0; i < 32; i++)
		//	key[i] = GTime::Random(256);
		
		//G_ASSERT(_Write(&_version, sizeof(_version), file) == true);
		//G_ASSERT(_Write(_identifier, sizeof(_identifier) - 1, file) == true);
		//G_ASSERT(_Write(key, sizeof(uchar) * 32, file) == true);
		//G_ASSERT(_WriteEncryptedCopy(&archiveSize, sizeof(archiveSize), key, file) == true);
		//G_ASSERT(_WriteEncrypted(archive, archiveSize, key, file) == true);
		
		fclose(file);
		
#if _DEBUG
		time = GTime::GetMilliseconds() - time;
		uint_t finalSize = sizeof(_version) + sizeof(_identifier) - 1 + sizeof(uint8) * 32 + sizeof(archiveSize) + archiveSize;
		GConsole::Debug("\"%s\"(%d->%d bytes) saved in %.2f seconds\n", (const char*)name, size, finalSize, (float)time / 1000.0f);
		if(time > 250) GConsole::Debug("Save time for \"%s\" is longer than 0.25 seconds!", (const char*)name);
#endif
		
		return true;
	}
	
	return false;
}


#endif // PLATFORM_MACOSX || PLATFORM_IOS
