#include "GPackage.h"

GPackage::GPackage ()
:	_footer(0)
{
}

GPackage::~GPackage () {
	Close();
}

GPackage::GPackage (const GString& path)
:	_footer(0)
{
	OpenForRead(path);
}

bool GPackage::OpenForRead (const GString& path) {
	
	Close();
	
	if(_file.OpenResourceForRead(path) == false) {
		GSystem::Debug("ERROR: Failed to open package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t version;
	if(_file.Read(&version, sizeof(version)) == false) {
		GSystem::Debug("ERROR: Failed to read version from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t identifier[sizeof(IDENTIFIER)];
	if(_file.Read(identifier, sizeof(IDENTIFIER)) == false) {
		GSystem::Debug("ERROR: Failed to read identifier from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Read(&_footer, sizeof(_footer)) == false) {
		GSystem::Debug("ERROR: Failed to read footer offset from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.SetPosition((int)_footer) == false) {
		GSystem::Debug("ERROR: Failed to set offset for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	int64_t count;
	if(_file.Read(&count, sizeof(count)) == false) {
		GSystem::Debug("ERROR: Failed to read number of resources from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(count == 0)
		return true;
	
	int64_t bufferSize;
	if(_file.Read(&bufferSize, sizeof(bufferSize)) == false) {
		GSystem::Debug("ERROR: Failed to read buffer size from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	int64_t archiveSize;
	if(_file.Read(&archiveSize, sizeof(archiveSize)) == false) {
		GSystem::Debug("ERROR: Failed to read archive size from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t* archive = new uint8_t[archiveSize];
	if(_file.Read(archive, sizeof(uint8_t) * archiveSize) == false) {
		GSystem::Debug("ERROR: Failed to read archive from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t* buffer = new uint8_t[bufferSize];
	archiveSize = GArchive::Decompress(archive, archiveSize, buffer, bufferSize);
	if(archiveSize != bufferSize) {
		GSystem::Debug("ERROR: Failed to decompress resource table for package \"%s\"!\n", (const char*)path);
		delete[] archive;
		return false;
	}
	
	delete[] archive;

	int64_t offset = 0;
	for(int64_t i = 0; i < count; i++) {
		char* resourceName = (char*)(buffer + offset);
		offset += GString::strlen(resourceName) + 1;
		int64_t resourceOffset = *((int64_t*)(buffer + offset));
		offset += sizeof(int64_t);
		_resources.insert(std::make_pair(resourceName, resourceOffset));
	}
	
	delete[] buffer;
	
	PACKAGES.push_back(this);
	
    GSystem::Debug("%s ready for reading...\n", (const char*)path);
    
	return true;
}

bool GPackage::OpenFileForWrite (const GString& path) {
	Close();
	
	if(_file.OpenForWrite(path) == false) {
		GSystem::Debug("ERROR: Failed to open the package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(&VERSION, sizeof(VERSION)) == false) {
		GSystem::Debug("ERROR: Failed to write version for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(IDENTIFIER, sizeof(IDENTIFIER)) == false) {
		GSystem::Debug("ERROR: Failed to write identifier for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(&_footer, sizeof(_footer)) == false) {
		GSystem::Debug("ERROR: Failed to write footer offset for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	_footer = _file.GetPosition();
	return true;
}

bool GPackage::Close () {
	PACKAGES.remove(this);
	_file.Close();
	_footer = 0;
	_resources.clear();
	return true;
}

int64_t GPackage::GetSize (const GString& resource) {
	CACHED_RESOURCE = resource;
	CACHED_PACKAGE = nullptr;
	CACHED_SIZE = 0;
	
	for(std::list<GPackage*>::iterator p = PACKAGES.begin(); p != PACKAGES.end(); p++) {
		std::map<GString, int64_t>::iterator i = (*p)->_resources.find(resource);
		if(i != (*p)->_resources.end()) {
			
			if((*p)->_file.SetPosition(i->second) == false) {
				GSystem::Debug("ERROR: Failed to set package position for resource \"%s\"!\n", (const char*)resource);
				return 0;
			}
			
			int64_t size;
			if((*p)->_file.Read(&size, sizeof(size)) == false) {
				GSystem::Debug("ERROR: Failed to read resource size for resource \"%s\"!\n", (const char*)resource);
				return 0;
			}
			
			CACHED_PACKAGE = (*p);
			CACHED_SIZE = size;
			return size;
		}
	}
	
	return 0;
}

bool GPackage::Read (const GString& resource, void* data, int64_t size) {
	if(resource != CACHED_RESOURCE || CACHED_PACKAGE == nullptr)
		GetSize(resource);
	
	if(CACHED_SIZE == 0)
		return false;
	
	if(CACHED_PACKAGE->_file.Read(data, (int)size) == false) {
		GSystem::Debug("ERROR: Failed to read from package resource \"%s\"!\n", (const char*)resource);
		CACHED_PACKAGE = nullptr;
		return false;
	}
	
	CACHED_PACKAGE = nullptr;
	return true;
}

bool GPackage::Write (const GString& resource, const void* data, int64_t size) {
	if(_file.SetPosition(_footer) == false) {
		GSystem::Debug("ERROR: Failed to set package position for resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	if(_file.Write(&size, sizeof(size)) == false) {
		GSystem::Debug("ERROR: Failed to write resource size for resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	if(_file.Write(data, size) == false) {
		GSystem::Debug("ERROR: Failed to write resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	_resources.insert(std::make_pair(resource, _footer));
	_footer = _file.GetPosition();
	
	if(_file.SetPosition(0) == false) {
		GSystem::Debug("ERROR: Failed to set package position to 0!\n");
		return false;
	}
	
	if(_file.Write(&VERSION, sizeof(VERSION)) == false) {
		GSystem::Debug("ERROR: Failed to write version to package!\n");
		return false;
	}
	
	if(_file.Write(IDENTIFIER, sizeof(IDENTIFIER)) == false) {
		GSystem::Debug("ERROR: Failed to write identifier to package!\n");
		return false;
	}
	
	if(_file.Write(&_footer, sizeof(_footer)) == false) {
		GSystem::Debug("ERROR: Failed to write footer offset to package!\n");
		return false;
	}
	
	if(_file.SetPosition(_footer) == false) {
		GSystem::Debug("ERROR: Failed to set package position to footer!\n");
		return false;
	}
	
	int64_t count = _resources.size();
	if(_file.Write(&count, sizeof(count)) == false) {
		GSystem::Debug("ERROR: Failed to write number of resources to package!\n");
		return false;
	}
	
	if(count == 0)
		return true;
	
	int64_t bufferSize = 0;
	for(std::map<GString, int64_t>::iterator i = _resources.begin(); i != _resources.end(); i++)
		bufferSize += i->first.GetLength() + 1 + sizeof(int64_t);
	
	if(_file.Write(&bufferSize, sizeof(bufferSize)) == false) {
		GSystem::Debug("ERROR: Failed to write footer buffer size to package!\n");
		return false;
	}
	
	uint8_t* buffer = new uint8_t[bufferSize];
	int64_t offset = 0;
	for(std::map<GString, int64_t>::iterator i = _resources.begin(); i != _resources.end(); i++) {
		
		// Copy the resource name
		GString::strcpy((char*)(buffer + offset), i->first);
		offset += i->first.GetLength() + 1;
		
		// Copy the resource offset
		*((int64_t*)(buffer + offset)) = i->second;
		offset += sizeof(int64_t);
	}
	
	int64_t archiveSize = GArchive::GetBufferBounds(bufferSize);
	uint8_t* archive = new uint8_t[archiveSize];
	archiveSize = GArchive::Compress(buffer, bufferSize, archive, archiveSize);
	if(archive == 0) {
		GSystem::Debug("ERROR: Failed to compress footer archive to package!\n");
		delete [] buffer;
		return false;
	}
	
	if(_file.Write(&archiveSize, sizeof(archiveSize)) == false) {
		GSystem::Debug("ERROR: Failed to write footer archive size to package!\n");
		delete [] buffer;
		delete [] archive;
		return false;
	}
	
	if(_file.Write(archive, sizeof(uint8_t) * archiveSize) == false) {
		GSystem::Debug("ERROR: Failed to write footer archive to package!\n");
		delete [] buffer;
		delete [] archive;
		return false;
	}
	
	delete [] buffer;
	delete [] archive;

	return true;
}
