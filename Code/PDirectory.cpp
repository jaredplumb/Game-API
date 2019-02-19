#include "PDirectory.h"

PDirectory::PDirectory () {
}

PDirectory::~PDirectory () {
	Close();
}

PDirectory::PDirectory (const PString& path) {
	Open(path);
}

bool PDirectory::Open (const PString& path) {
	
#if PLATFORM_WINDOWS

	PString directory = path;
	if (directory == NULL || directory[(int_t)0] == '\0')
		directory = "";

	if (directory[directory.GetLength() - 1] != '\\')
		directory += "\\";

	_finddata_t data;
	intptr_t handle = _findfirst(path + "*", &data);
	if(handle == -1)
		return false;

	do {
		if (data.attrib & _A_SUBDIR)
			Open(data.name);
		else
			_files.push_back(directory + data.name);
	} while(_findnext(handle, &data) != -1);

	_findclose(handle);


#else
	PString directory = path;
	if(directory == NULL || directory[(int_t)0] == '\0')
		directory = "./";
	
	if(directory[directory.GetLength() - 1] != '/')
		directory += "/";
	
	DIR* dir = opendir(directory);
	if(dir == NULL) {
		
		FILE* file = fopen(path, "rb");
		if(file != NULL) {
			_files.push_back(path);
			fclose(file);
			return true;
		}
		
		//ERROR("Failed to open file or directory \"%s\"!", directory.GetString());
		return false;
	}
	
	for(dirent* info = readdir(dir); info != NULL; info = readdir(dir)) {
		if(info->d_type == DT_DIR) {
			if(PString::strcmp(info->d_name, ".") != 0 && PString::strcmp(info->d_name, "..") != 0)
				Open(directory + info->d_name);
		} else {
			_files.push_back(directory + info->d_name);
		}
	}
	
	closedir(dir);
#endif

	return true;
}

void PDirectory::Close () {
	_files.clear();
}

uint_t PDirectory::GetSize () const {
	return _files.size();
}

PString PDirectory::GetFile (uint_t index) const {
	return index < _files.size() ? _files[index] : NULL;
}
