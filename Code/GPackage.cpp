#include "GPackage.h"
#include <list>

static const uint8_t			_VERSION = 5;
static const char				_IDENTIFIER[] = "PACKAGE";
static std::list<GPackage*>*	_packages = NULL;
static GString					_lastResource;
static GPackage*				_lastPackage = NULL;
static int64_t					_lastSize = 0;

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

GPackage::GPackage (const GString& path, bool setDefaultWD)
:	_footer(0)
{
	if(setDefaultWD)
		GSystem::SetDefaultWD();
	OpenForRead(path);
}

bool GPackage::OpenForRead (const GString& path) {
	
	Close();
	
	if(_file.OpenForRead(path) == false) {
		GSystem::Debug("ERROR: Failed to open package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t version;
	if(_file.Read(&version, sizeof(version)) == false) {
		GSystem::Debug("ERROR: Failed to read version from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8_t identifier[sizeof(_IDENTIFIER)];
	if(_file.Read(identifier, sizeof(_IDENTIFIER)) == false) {
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

	if(_packages == NULL)
		_packages = new std::list<GPackage*>;
	_packages->push_back(this);
	
    GSystem::Debug("%s ready for reading...\n", (const char*)path);
    
	return true;
}

bool GPackage::OpenForWrite (const GString& path) {
	Close();
	
	if(_file.OpenForWrite(path) == false) {
		GSystem::Debug("ERROR: Failed to open the package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(&_VERSION, sizeof(_VERSION)) == false) {
		GSystem::Debug("ERROR: Failed to write version for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(_IDENTIFIER, sizeof(_IDENTIFIER)) == false) {
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
	
	if(_packages)
		for(std::list<GPackage*>::iterator i = _packages->begin(); i != _packages->end(); i++)
			if(*i == this) {
				_packages->erase(i);
				break;
			}
	
	if(_packages && _packages->empty()) {
		delete _packages;
		_packages = NULL;
	}
	
	_file.Close();
	_footer = 0;
	_resources.clear();
	return true;
}

int64_t GPackage::GetSize (const GString& resource) {
	_lastResource = resource;
	_lastPackage = NULL;
	_lastSize = 0;
	
	if(_packages == NULL)
		return 0;
	
	for(std::list<GPackage*>::iterator p = _packages->begin(); p != _packages->end(); p++) {
		std::map<GString, int64_t>::iterator r = (*p)->_resources.find(resource);
		if(r != (*p)->_resources.end()) {
			
			if((*p)->_file.SetPosition(r->second) == false) {
				GSystem::Debug("ERROR: Failed to set package position for resource \"%s\"!\n", (const char*)resource);
				return 0;
			}
			
			int64_t size;
			if((*p)->_file.Read(&size, sizeof(size)) == false) {
				GSystem::Debug("ERROR: Failed to read resource size for resource \"%s\"!\n", (const char*)resource);
				return 0;
			}
			
			_lastPackage = (*p);
			_lastSize = size;
			return size;
		}
	}
	
	//GSystem::Debug("ERROR: Failed to find resource \"%s\"!\n", (const char*)resource);
	return 0;
}

bool GPackage::Read (const GString& resource, void* data, int64_t size) {
	if(resource != _lastResource || _lastPackage == NULL)
		GetSize(resource);
	
	if(_lastSize == 0)
		return false;
	
	if(_lastPackage->_file.Read(data, (int)size) == false) {
		GSystem::Debug("ERROR: Failed to read from package resource \"%s\"!\n", (const char*)resource);
		_lastPackage = NULL;
		return false;
	}
	
	_lastPackage = NULL;
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
	
	if(_file.Write(&_VERSION, sizeof(_VERSION)) == false) {
		GSystem::Debug("ERROR: Failed to write version to package!\n");
		return false;
	}
	
	if(_file.Write(_IDENTIFIER, sizeof(_IDENTIFIER)) == false) {
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
