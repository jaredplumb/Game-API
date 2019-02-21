#include "GTypes.h"

// These are here to add static caching in the future
static char* _CHAR_ALLOC (int_t size) {
	return new char[size];
}

static void _FREE (char* string) {
	delete [] string;
}

GString::GString ()
:	_string(NULL)
,	_length(0)
{
}

GString::GString (const GString& string)
:	_string(NULL)
,	_length(string.GetLength())
{
	if(string._string != NULL) {
		_string = _CHAR_ALLOC(_length + 1);
		strncpy(_string, string._string, _length);
		_string[_length] = '\0';
	}
}

GString::GString (const char* string)
:	_string(NULL)
,	_length(strlen(string))
{
	if(string != NULL) {
		_string = _CHAR_ALLOC(_length + 1);
		strncpy(_string, string, _length);
		_string[_length] = '\0';
	}
}

GString::~GString () {
	if(_string) {
		_FREE(_string);
		_string = NULL;
	}
	_length = 0;
}

GString& GString::New (const GString& string) {
	if(_string) {
		_FREE(_string);
		_string = NULL;
	}
	_length = string.GetLength();
	if(string._string != NULL) {
		_string = _CHAR_ALLOC(_length + 1);
		strncpy(_string, string._string, _length);
		_string[_length] = '\0';
	}
	return *this;
}

GString& GString::New (const char* string) {
	if(_string) {
		_FREE(_string);
		_string = NULL;
	}
	_length = strlen(string);
	if(string != NULL) {
		_string = _CHAR_ALLOC(_length + 1);
		strncpy(_string, string, _length);
		_string[_length] = '\0';
	}
	return *this;
}

void GString::Delete () {
	if(_string) {
		_FREE(_string);
		_string = NULL;
	}
	_length = 0;
}

GString& GString::Format (const char* string, ...) {
	if(_string) {
		_FREE(_string);
		_string = NULL;
		_length = 0;
	}
	
	if(string == NULL)
		return *this;
	
	// TODO: if I change to an allocator system, I could get
	//		the default allocator size, then do a vsnprintf,
	//		and if it does not return 0, then grab the actual
	//		allocated size instead
	
	va_list args;
	
	va_start(args, string);
	_length = vsnprintf(NULL, 0, string, args);
	va_end(args);
	
	if(_length > 0) {
		va_start(args, string);
		_string = _CHAR_ALLOC(_length + 1);
		vsnprintf(_string, _length + 1, string, args);
		_string[_length] = '\0';
		va_end(args);
	}
	
	return *this;
}

int_t GString::GetLength () const {
	if(_length <= 0)
		*(const_cast<int_t*>(&_length)) = strlen(_string);
	return _length;
}

bool GString::IsEmpty () const {
	return _string == NULL || _string[0] == '\0';
}

GString& GString::Add (const GString& string) {
	if(string._string == NULL)
		return *this;
	
	if(_string == NULL)
		return New(string);
	
	_length = GetLength() + string.GetLength();
	char* newstring = _CHAR_ALLOC(_length + 1);
	strcpy(newstring, _string);
	strcat(newstring, string._string);
	_FREE(_string);
	_string = newstring;
	return *this;
}

GString& GString::Add (const char* string) {
	if(string == NULL)
		return *this;
	
	if(_string == NULL)
		return New(string);
	
	_length = GetLength() + strlen(string);
	char* newstring = _CHAR_ALLOC(_length + 1);
	strcpy(newstring, _string);
	strcat(newstring, string);
	_FREE(_string);
	_string = newstring;
	return *this;
}

GString& GString::ToLower () {
	tolower(_string);
	return *this;
}

GString& GString::ToUpper () {
	toupper(_string);
	return *this;
}

GString& GString::TrimSpaces () {
	if(_string == NULL)
		return *this;
	
	// Remove end spaces
	for(int_t i = GetLength() - 1; i >= 0 && isspace(_string[i]); i--)
		_string[i] = '\0';
	
	// Remove beginning spaces
	char* s = _string;
	while(*s != '\0' && isspace(*s))
		s++;
	
	// Create new string
	_length = strlen(s);
	char* n = _CHAR_ALLOC(_length + 1);
	strncpy(n, s, _length);
	n[_length] = '\0';
	_FREE(_string);
	_string = n;
	
	return *this;
}

GString& GString::TrimExtension () {
	for(int_t i = GetLength() - 1; i >= 0; i--)
		if(_string[i] == '.') {
			_string[i] = '\0';
			_length = 0;
			return *this;
		}
	return *this;
}

GString& GString::TrimToDirectory () {
	for(int_t i = GetLength(); i > 0; i--)
		if(_string[i - 1] == '/' || _string[i - 1] == '\\') {
			_string[i] = '\0';
			_length = 0;
			return *this;
		}
	return *this;
}

////////////////////////////////////////////////////
// Standard C string overrides with modifications //
////////////////////////////////////////////////////
bool GString::isalnum (char c) {
	return (isalpha(c) || isdigit(c));
}

bool GString::isalpha (char c) {
	return (isupper(c) || islower(c));
}

bool GString::isdigit (char c) {
	return (c >= '0' && c <= '9');
}

bool GString::isgraph (char c) {
	return (isprint(c) && !isspace(c));
}

bool GString::islower (char c) {
	return (c >= 'a' && c <= 'z');
}

bool GString::isprint (char c) {
	return (c >= ' ' && c <= '~');
}

bool GString::ispunct (char c) {
	return (isprint(c) && !isspace(c) && !isalnum(c));
}

bool GString::isspace (char c) {
	// \t \n \v \f \r and space
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}

bool GString::isupper (char c) {
	return (c >= 'A' && c <= 'Z');
}

bool GString::isxdigit (char c) {
	return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

char* GString::strcat (char* dst, const char* src) {
	if(dst == NULL || src == NULL) return dst;
	char* save = dst;
	while(*save != 0)
		save++;
	while((*save++ = *src++) != 0)
		;
	return dst;
}

int_t GString::strcmp (const char* s1, const char* s2) {
	if(s1 == NULL)
		return (s2 ? -s2[0] : 0);
	if(s2 == NULL)
		return s1[0];
	while(*s1 == *s2++)
		if(*s1++ == 0)
			return 0;
	return *s1 - *(s2 - 1);
}

char* GString::strcpy (char* dst, const char* src) {
	if(dst == NULL)
		return NULL;
	if(src == NULL) {
		*dst = 0;
	} else {
		char* save = dst;
		while((*save++ = *src++) != 0)
			;
	}
	return dst;
}

int_t GString::stricmp (const char* s1, const char* s2) {
	if(s1 == NULL)
		return (s2 ? -tolower(s2[0]) : 0);
	if(s2 == NULL)
		return tolower(s1[0]);
	while(tolower(*s1) == tolower(*s2++))
		if(*s1++ == 0)
			return 0;
	return tolower(*s1) - tolower(*(s2 - 1));
}

char* GString::stristr (const char* s, const char* find) {
	for(int_t len = strlen(find); *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

int_t GString::strlen (const char* s) {
	if(s == NULL)
		return 0;
	const char* i = s;
	while(*i)
		i++;
	return (int_t)i - (int_t)s;
}

char* GString::strncat (char* dst, const char* src, int_t len) {
	if(dst == NULL)
		return NULL;
	char* save = dst;
	while(*save != 0)
		save++;
	if(src != NULL)
		while(--len >= 0 && (*save++ = *src++) != 0)
			;
	while(--len >= 0)
		*save++ = 0;
	return dst;
}

int_t GString::strncmp (const char* s1, const char* s2, int_t len) {
	if(len == 0)
		return 0;
	if(s1 == NULL)
		return (s2 ? -s2[0] : 0);
	if(s2 == NULL)
		return s1[0];
	do {
		if(*s1 != *s2++)
			return *s1 - *(s2 - 1);
		if(*s1++ == 0)
			return 0;
	} while(--len != 0);
	return 0;
}

char* GString::strncpy (char* dst, const char* src, int_t len) {
	if(dst == NULL)
		return NULL;
	char* save = dst;
	if(src != NULL)
		while(--len >= 0 && (*save++ = *src++) != 0)
			;
	while(--len >= 0)
		*save++ = 0;
	return dst;
}

int_t GString::strnicmp (const char* s1, const char* s2, int_t len) {
	if(len == 0)
		return 0;
	if(s1 == NULL)
		return (s2 ? -tolower(s2[0]) : 0);
	if(s2 == NULL)
		return tolower(s1[0]);
	do {
		if(tolower(*s1) != tolower(*s2++))
			return tolower(*s1) - tolower(*(s2 - 1));
		if(*s1++ == 0)
			return 0;
	} while(--len != 0);
	return 0;
}

char* GString::strnistr (const char* s, const char* find, int_t len) {
	for(; *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

char* GString::strnstr (const char* s, const char* find, int_t len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

char* GString::strstr (const char* s, const char* find) {
	for(int_t len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}





char* GString::strnnext (const char* s, const char* find, int_t len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strnext (const char* s, const char* find) {
	for(int_t len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strninext (const char* s, const char* find, int_t len) {
	for(; *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strinext (const char* s, const char* find) {
	for(int_t len = strlen(find); *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}




int_t GString::strtoi (const char* s, char** end, int_t base) {
	if(s == NULL) {
		if(end != NULL)
			*end = NULL;
		return 0;
	}
	while(isspace(*s))
		s++;
	bool neg = (*s == '-');
	if(neg || *s == '+')
		s++;
	if(base != 10) {
		if((base == 0 || base == 16) && *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
			s += 2;
			base = 16;
		} else if((base == 0 || base == 2) && *s == '0' && (*(s + 1) == 'b' || *(s + 1) == 'B')) {
			s += 2;
			base = 2;
		} else if(base == 0) {
			base = 10;
		}
	}
	int_t i = 0;
	for(int_t c = *s; *s != 0; c = *++s) {
		if(isdigit(c))
			c -= '0';
		else if(isalpha(c))
			c -= (isupper(c) ? 'A' - 10 : 'a' - 10);
		else
			break;
		if(c >= base)
			break;
		i *= base;
		i += c;
	}
	if(end != NULL)
		*end = (char*)s;
	return neg ? -i : i;
}

char GString::tolower (char c) {
	return c + 0x20 * (c >= 'A' && c <= 'Z');
}

char * GString::tolower (char* s) {
	for(char* i = s; i && *i; i++) *i = tolower(*i);
	return s;
}

char GString::toupper (char c) {
	return c - 0x20 * (c >= 'a' && c <= 'z');
}

char* GString::toupper (char* s) {
	for(char* i = s; i && *i; i++) *i = toupper(*i);
	return s;
}











GFile::GFile ()
:	_file(NULL)
{
}

GFile::~GFile () {
	Close();
}

GFile::GFile (const GString& path)
:	_file(NULL)
{
	OpenForRead(path);
}

bool GFile::OpenForRead (const GString& path) {
	Close();
	_file = fopen(path, "rb");
	return _file != NULL;
}

bool GFile::OpenForWrite (const GString& path) {
	Close();
	_file = fopen(path, "wb+");
	return _file != NULL;
}

bool GFile::OpenForAppend (const GString& path) {
	Close();
	_file = fopen(path, "ab+");
	return _file != NULL;
}

void GFile::Close () {
	if(_file) {
		fclose(_file);
		_file = NULL;
	}
}

bool GFile::Read (void* data, uint_t size) {
	return _file ? (fread(data, size, 1, _file) == 1) : false;
}

bool GFile::Write (const void* data, uint_t size) {
	return _file ? (fwrite(data, size, 1, _file) == 1) : false;
}

bool GFile::IsOpen () const {
	return _file != NULL;
}

uint_t GFile::GetPosition () const {
#if PLATFORM_WINDOWS
	return _file ? (uint_t)_ftelli64(_file) : 0;
#else
	return _file ? (uint_t)ftello(_file) : 0;
#endif
}

uint_t GFile::GetSize () const {
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

bool GFile::SetPosition (uint_t position) {
#if PLATFORM_WINDOWS
	return _file ? (_fseeki64(_file, (long long)position, SEEK_SET) == 0) : false;
#else
	return _file ? (fseeko(_file, (off_t)position, SEEK_SET) == 0) : false;
#endif
}









GDirectory::GDirectory () {
}

GDirectory::~GDirectory () {
	Close();
}

GDirectory::GDirectory (const GString& path) {
	Open(path);
}

bool GDirectory::Open (const GString& path) {
	
#if PLATFORM_WINDOWS
	
	GString directory = path;
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
	GString directory = path;
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
			if(GString::strcmp(info->d_name, ".") != 0 && GString::strcmp(info->d_name, "..") != 0)
				Open(directory + info->d_name);
		} else {
			_files.push_back(directory + info->d_name);
		}
	}
	
	closedir(dir);
#endif
	
	return true;
}

void GDirectory::Close () {
	_files.clear();
}

uint_t GDirectory::GetSize () const {
	return _files.size();
}

GString GDirectory::GetFile (uint_t index) const {
	return index < _files.size() ? _files[index] : NULL;
}













#include <zlib.h>

enum _eCompressType {
	_COMPRESS_TYPE_ZLIB = 0,
};

static uint_t _ZLIBCompress (uint8* srcBuffer, uint_t srcSize, uint8* dstBuffer, uint_t dstSize, int level) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (unsigned int)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (unsigned int)dstSize;
	
	int error = deflateInit(&stream, level);
	if(error != Z_OK) return 0;
	
	error = deflate(&stream, Z_FINISH);
	if(error != Z_STREAM_END) return 0;
	
	dstSize = stream.total_out;
	
	error = deflateEnd(&stream);
	if(error != Z_OK) return 0;
	
	return dstSize;
}

static uint_t _ZLIBDecompress (uint8* srcBuffer, uint_t srcSize, uint8* dstBuffer, uint_t dstSize) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (unsigned int)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (unsigned int)dstSize;
	
	int error = inflateInit(&stream);
	if(error != Z_OK) return 0;
	
	error = inflate(&stream, Z_FINISH);
	if(error != Z_STREAM_END) return 0;
	
	dstSize = stream.total_out;
	
	error = inflateEnd(&stream);
	if(error != Z_OK) return 0;
	
	return dstSize;
}

uint_t GArchive::Compress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize) {
	if(dstSize < 2)
		return 0;
	
	// Set the header data
	((uint8*)dstBuffer)[0] = VERSION;
	((uint8*)dstBuffer)[1] = (uint8)_COMPRESS_TYPE_ZLIB;
	dstBuffer = ((uint8*)dstBuffer) + 2;
	dstSize -= 2;
	
	// Max zlib compression
	dstSize = _ZLIBCompress((uint8*)srcBuffer, srcSize, (uint8*)dstBuffer, dstSize, 9);
	
	// Return the actual size of the compressed data
	if(dstSize == 0)
		return 0;
	return dstSize + 2;
}

uint_t GArchive::Decompress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize) {
	if(srcSize < 2)
		return 0;
	
	// Check the version
	if(((uint8*)srcBuffer)[0] != VERSION) {
		//ERROR("Could not decompress archive of version %d!", ((uint8*)srcBuffer)[0]);
		return 0;
	}
	
	// Get the remainder of the header data
	_eCompressType compressType = (_eCompressType)((uint8*)srcBuffer)[1];
	srcBuffer = ((uint8*)srcBuffer) + 2;
	srcSize -= 2;
	
	// Decompress depending on the type
	switch(compressType) {
		case _COMPRESS_TYPE_ZLIB:
			return _ZLIBDecompress((uint8*)srcBuffer, srcSize, (uint8*)dstBuffer, dstSize);
		default:
			//ERROR("Could not decompress unknown compression type (%d)!", (int)compressType);
			return 0;
	}
	
	return 0;
}

uint_t GArchive::GetBufferBounds (uint_t srcSize) {
	// This is the size a dstBuffer needs to be to hold the headers when no compression happens.
	// This is taken from the function compressBound in the zlib library.
	// The last + number is what is needed for header values from GArchive.
	return srcSize + (srcSize >> 12) + (srcSize >> 14) + 11 + 2;
}



