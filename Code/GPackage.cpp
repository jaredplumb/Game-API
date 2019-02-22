#include "GPackage.h"

static const uint8				_VERSION = 5;
static const uint8				_IDENTIFIER[] = "PACKAGE";
static std::list<GPackage*>*	_packages = NULL;
static GString					_lastResource;
static GPackage*				_lastPackage = NULL;
static uint64					_lastSize = 0;

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
		GConsole::Debug("ERROR: Failed to open package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8 version;
	if(_file.Read(&version, sizeof(version)) == false) {
		GConsole::Debug("ERROR: Failed to read version from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8 identifier[sizeof(_IDENTIFIER)];
	if(_file.Read(identifier, sizeof(_IDENTIFIER)) == false) {
		GConsole::Debug("ERROR: Failed to read identifier from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Read(&_footer, sizeof(_footer)) == false) {
		GConsole::Debug("ERROR: Failed to read footer offset from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.SetPosition((uint_t)_footer) == false) {
		GConsole::Debug("ERROR: Failed to set offset for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint64 count;
	if(_file.Read(&count, sizeof(count)) == false) {
		GConsole::Debug("ERROR: Failed to read number of resources from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(count == 0)
		return true;
	
	uint64 bufferSize;
	if(_file.Read(&bufferSize, sizeof(bufferSize)) == false) {
		GConsole::Debug("ERROR: Failed to read buffer size from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint64 archiveSize;
	if(_file.Read(&archiveSize, sizeof(archiveSize)) == false) {
		GConsole::Debug("ERROR: Failed to read archive size from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8* archive = new uint8[archiveSize];
	if(_file.Read(archive, sizeof(uint8) * archiveSize) == false) {
		GConsole::Debug("ERROR: Failed to read archive from package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	uint8* buffer = new uint8[bufferSize];
	archiveSize = GArchive::Decompress(archive, archiveSize, buffer, bufferSize);
	if(archiveSize != bufferSize) {
		GConsole::Debug("ERROR: Failed to decompress resource table for package \"%s\"!\n", (const char*)path);
		delete[] archive;
		return false;
	}
	
	delete[] archive;

	uint64 offset = 0;
	for(uint64 i = 0; i < count; i++) {
		char* resourceName = (char*)(buffer + offset);
		offset += GString::strlen(resourceName) + 1;
		uint64 resourceOffset = *((uint64*)(buffer + offset));
		offset += sizeof(uint64);
		_resources.insert(std::make_pair(resourceName, resourceOffset));
	}
	
	delete[] buffer;

	if(_packages == NULL)
		_packages = new std::list<GPackage*>;
	_packages->push_back(this);
	
    GConsole::Debug("%s ready for reading...\n", (const char*)path);
    
	return true;
}

bool GPackage::OpenForWrite (const GString& path) {
	Close();
	
	if(_file.OpenForWrite(path) == false) {
		GConsole::Debug("ERROR: Failed to open the package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(&_VERSION, sizeof(_VERSION)) == false) {
		GConsole::Debug("ERROR: Failed to write version for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(_IDENTIFIER, sizeof(_IDENTIFIER)) == false) {
		GConsole::Debug("ERROR: Failed to write identifier for package \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(_file.Write(&_footer, sizeof(_footer)) == false) {
		GConsole::Debug("ERROR: Failed to write footer offset for package \"%s\"!\n", (const char*)path);
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

uint64 GPackage::GetSize (const GString& resource) {
	_lastResource = resource;
	_lastPackage = NULL;
	_lastSize = 0;
	
	if(_packages == NULL)
		return 0;
	
	for(std::list<GPackage*>::iterator p = _packages->begin(); p != _packages->end(); p++) {
		std::map<GString, uint64>::iterator r = (*p)->_resources.find(resource);
		if(r != (*p)->_resources.end()) {
			
			if((*p)->_file.SetPosition(r->second) == false) {
				GConsole::Debug("ERROR: Failed to set package position for resource \"%s\"!\n", (const char*)resource);
				return 0;
			}
			
			uint64 size;
			if((*p)->_file.Read(&size, sizeof(size)) == false) {
				GConsole::Debug("ERROR: Failed to read resource size for resource \"%s\"!\n", (const char*)resource);
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

bool GPackage::Read (const GString& resource, void* data, uint64 size) {
	if(resource != _lastResource || _lastPackage == NULL)
		GetSize(resource);
	
	if(_lastSize == 0)
		return false;
	
	if(_lastPackage->_file.Read(data, (uint_t)size) == false) {
		GConsole::Debug("ERROR: Failed to read from package resource \"%s\"!\n", (const char*)resource);
		_lastPackage = NULL;
		return false;
	}
	
	_lastPackage = NULL;
	return true;
}

bool GPackage::Write (const GString& resource, const void* data, uint64 size) {
	if(_file.SetPosition(_footer) == false) {
		GConsole::Debug("ERROR: Failed to set package position for resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	if(_file.Write(&size, sizeof(size)) == false) {
		GConsole::Debug("ERROR: Failed to write resource size for resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	if(_file.Write(data, size) == false) {
		GConsole::Debug("ERROR: Failed to write resource \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	_resources.insert(std::make_pair(resource, _footer));
	_footer = _file.GetPosition();
	
	if(_file.SetPosition(0) == false) {
		GConsole::Debug("ERROR: Failed to set package position to 0!\n");
		return false;
	}
	
	if(_file.Write(&_VERSION, sizeof(_VERSION)) == false) {
		GConsole::Debug("ERROR: Failed to write version to package!\n");
		return false;
	}
	
	if(_file.Write(_IDENTIFIER, sizeof(_IDENTIFIER)) == false) {
		GConsole::Debug("ERROR: Failed to write identifier to package!\n");
		return false;
	}
	
	if(_file.Write(&_footer, sizeof(_footer)) == false) {
		GConsole::Debug("ERROR: Failed to write footer offset to package!\n");
		return false;
	}
	
	if(_file.SetPosition(_footer) == false) {
		GConsole::Debug("ERROR: Failed to set package position to footer!\n");
		return false;
	}
	
	uint64 count = _resources.size();
	if(_file.Write(&count, sizeof(count)) == false) {
		GConsole::Debug("ERROR: Failed to write number of resources to package!\n");
		return false;
	}
	
	if(count == 0)
		return true;
	
	uint64 bufferSize = 0;
	for(std::map<GString, uint64>::iterator i = _resources.begin(); i != _resources.end(); i++)
		bufferSize += i->first.GetLength() + 1 + sizeof(uint64);
	
	if(_file.Write(&bufferSize, sizeof(bufferSize)) == false) {
		GConsole::Debug("ERROR: Failed to write footer buffer size to package!\n");
		return false;
	}
	
	uint8* buffer = new uint8[bufferSize];
	uint64 offset = 0;
	for(std::map<GString, uint64>::iterator i = _resources.begin(); i != _resources.end(); i++) {
		
		// Copy the resource name
		GString::strcpy((char*)(buffer + offset), i->first);
		offset += i->first.GetLength() + 1;
		
		// Copy the resource offset
		*((uint64*)(buffer + offset)) = i->second;
		offset += sizeof(uint64);
	}
	
	uint64 archiveSize = GArchive::GetBufferBounds(bufferSize);
	uint8* archive = new uint8[archiveSize];
	archiveSize = GArchive::Compress(buffer, bufferSize, archive, archiveSize);
	if(archive == 0) {
		GConsole::Debug("ERROR: Failed to compress footer archive to package!\n");
		delete [] buffer;
		return false;
	}
	
	if(_file.Write(&archiveSize, sizeof(archiveSize)) == false) {
		GConsole::Debug("ERROR: Failed to write footer archive size to package!\n");
		delete [] buffer;
		delete [] archive;
		return false;
	}
	
	if(_file.Write(archive, sizeof(uint8) * archiveSize) == false) {
		GConsole::Debug("ERROR: Failed to write footer archive to package!\n");
		delete [] buffer;
		delete [] archive;
		return false;
	}
	
	delete [] buffer;
	delete [] archive;

	return true;
}
