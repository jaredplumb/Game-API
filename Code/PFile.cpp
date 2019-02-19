#include "PFile.h"

PFile::PFile ()
:	_file(NULL)
{
}

PFile::~PFile () {
	Close();
}

PFile::PFile (const PString& path)
:	_file(NULL)
{
	OpenForRead(path);
}

bool PFile::OpenForRead (const PString& path) {
	Close();
	_file = fopen(path, "rb");
	return _file != NULL;
}

bool PFile::OpenForWrite (const PString& path) {
	Close();
	_file = fopen(path, "wb+");
	return _file != NULL;
}

bool PFile::OpenForAppend (const PString& path) {
	Close();
	_file = fopen(path, "ab+");
	return _file != NULL;
}

void PFile::Close () {
	if(_file) {
		fclose(_file);
		_file = NULL;
	}
}

bool PFile::Read (void* data, uint_t size) {
	return _file ? (fread(data, size, 1, _file) == 1) : false;
}

bool PFile::Write (const void* data, uint_t size) {
	return _file ? (fwrite(data, size, 1, _file) == 1) : false;
}

bool PFile::IsOpen () const {
	return _file != NULL;
}

uint_t PFile::GetPosition () const {
#if PLATFORM_WINDOWS
	return _file ? (uint_t)_ftelli64(_file) : 0;
#else
	return _file ? (uint_t)ftello(_file) : 0;
#endif
}

uint_t PFile::GetSize () const {
	if(_file == NULL)
		return 0;
#if PLATFORM_WINDOWS
	uint_t current = (uint_t)_ftelli64(_file);
	_fseeki64(_file, 0, SEEK_END);
	uint_t size = (uint_t)_ftelli64(_file);
	_fseeki64(_file, (long long)current, SEEK_SET);
#else
	uint_t current = (uint_t)ftello(_file);
	fseeko(_file, 0, SEEK_END);
	uint_t size = (uint_t)ftello(_file);
	fseeko(_file, current, SEEK_SET);
#endif
	return size;
}

bool PFile::SetPosition (uint_t position) {
#if PLATFORM_WINDOWS
	return _file ? (_fseeki64(_file, (long long)position, SEEK_SET) == 0) : false;
#else
	return _file ? (fseeko(_file, (off_t)position, SEEK_SET) == 0) : false;
#endif
}
