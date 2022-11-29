#include "GFile.h"
#ifdef __APPLE__
#include <dirent.h>

GFile::GFile ()
:	_file(nullptr)
{
}

GFile::~GFile () {
	Close();
}

GFile::GFile (const GString& path)
:	_file(nullptr)
{
	OpenForRead(path);
}

bool GFile::OpenForRead (const GString& path) {
	Close();
	_file = fopen(path, "rb");
	return _file != nullptr;
}

bool GFile::OpenForWrite (const GString& path) {
	Close();
	_file = fopen(path, "wb+");
	return _file != nullptr;
}

bool GFile::OpenForAppend (const GString& path) {
	Close();
	_file = fopen(path, "ab+");
	return _file != nullptr;
}

void GFile::Close () {
	if(_file) {
		fclose(_file);
		_file = nullptr;
	}
}

bool GFile::Read (void* data, int64_t size) {
	return _file != nullptr && fread(data, size, 1, _file) == 1;
}

bool GFile::Write (const void* data, int64_t size) {
	return _file != nullptr && fwrite(data, size, 1, _file) == 1;
}

bool GFile::IsOpen () const {
	return _file != nullptr;
}

int64_t GFile::GetPosition () const {
	return _file ? (int64_t)ftello(_file) : 0;
}

int64_t GFile::GetSize () const {
	if(_file == nullptr)
		return 0;
	off_t current = ftello(_file);
	fseeko(_file, 0, SEEK_END);
	off_t size = ftello(_file);
	fseeko(_file, current, SEEK_SET);
	return (int64_t)size;
}

bool GFile::SetPosition (int64_t position) {
	return _file != nullptr && fseeko(_file, (off_t) position, SEEK_SET) == 0;
}

std::vector<GString> GFile::GetFileNamesInDirectory (const GString& path) {
	std::vector<GString> files;
	
	GString directory = path;
	if (directory.IsEmpty())
		directory = "./";
	
	if (directory[directory.GetLength() - 1] != '/')
		directory += "/";
	
	DIR* dir = opendir(directory);
	if (dir == nullptr) {
		FILE* file = fopen(path, "rb");
		if (file != nullptr) {
			files.push_back(path);
			fclose(file);
			return files;
		}
		return files;
	}
	
	for (dirent* info = readdir(dir); info != nullptr; info = readdir(dir)) {
		if (info->d_type == DT_DIR) {
			if (GString::strcmp(info->d_name, ".") != 0 && GString::strcmp(info->d_name, "..") != 0) {
				std::vector<GString> sub = GetFileNamesInDirectory(directory + info->d_name);
				files.reserve(files.size() + sub.size());
				files.insert(files.end(), sub.begin(), sub.end());
			}
		} else {
			files.push_back(directory + info->d_name);
		}
	}
	
	closedir(dir);
	return files;
}

#endif // __APPLE__
