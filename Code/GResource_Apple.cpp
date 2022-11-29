#include "GResource.h"
#ifdef __APPLE__
#include <dirent.h>

bool GResource::OpenForRead (const GString& path) {
	Close();
	_resource = (void*)fopen(path, "rb");
	return _resource != nullptr;
}

bool GResource::OpenForWrite (const GString& path) {
	Close();
	_resource = (void*)fopen(path, "wb+");
	return _resource != nullptr;
}

bool GResource::OpenForAppend (const GString& path) {
	Close();
	_resource = (void*)fopen(path, "ab+");
	return _resource != nullptr;
}

void GResource::Close () {
	if(_resource) {
		fclose((FILE*)_resource);
		_resource = nullptr;
	}
}

bool GResource::Read (void* data, int64_t size) {
	return _resource != nullptr && fread(data, size, 1, (FILE*)_resource) == 1;
}

bool GResource::Write (const void* data, int64_t size) {
	return _resource != nullptr && fwrite(data, size, 1, (FILE*)_resource) == 1;
}

int64_t GResource::GetPosition () const {
	return _resource ? (int64_t)ftello((FILE*)_resource) : 0;
}

int64_t GResource::GetSize () const {
	if(_resource == nullptr)
		return 0;
	off_t current = ftello((FILE*)_resource);
	fseeko((FILE*)_resource, 0, SEEK_END);
	off_t size = ftello((FILE*)_resource);
	fseeko((FILE*)_resource, current, SEEK_SET);
	return (int64_t)size;
}

bool GResource::SetPosition (int64_t position) {
	return _resource != nullptr && fseeko((FILE*)_resource, (off_t)position, SEEK_SET) == 0;
}

std::vector<GString> GResource::GetFileNamesInDirectory (const GString& path) {
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
