#include "GTypes.h"
#include <dirent.h>
#include <zlib.h>




static char* _CHAR_ALLOC (int size) {
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

int GString::GetLength () const {
	if(_length <= 0)
		*(const_cast<int*>(&_length)) = strlen(_string);
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
	for(int i = GetLength() - 1; i >= 0 && isspace(_string[i]); i--)
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
	for(int i = GetLength() - 1; i >= 0; i--)
		if(_string[i] == '.') {
			_string[i] = '\0';
			_length = 0;
			return *this;
		}
	return *this;
}

GString& GString::TrimToDirectory () {
	for(int i = GetLength(); i > 0; i--)
		if(_string[i - 1] == '/' || _string[i - 1] == '\\') {
			_string[i] = '\0';
			_length = 0;
			return *this;
		}
	return *this;
}

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

int GString::strcmp (const char* s1, const char* s2) {
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

int GString::stricmp (const char* s1, const char* s2) {
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
	for(int len = strlen(find); *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return const_cast<char*>(s);
	return NULL;
}

int GString::strlen (const char* s) {
	if(s == NULL)
		return 0;
	const char* i = s;
	while(*i)
		i++;
	return static_cast<int>(reinterpret_cast<intptr_t>(i) - reinterpret_cast<intptr_t>(s));
}

char* GString::strncat (char* dst, const char* src, int len) {
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

int GString::strncmp (const char* s1, const char* s2, int len) {
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

char* GString::strncpy (char* dst, const char* src, int len) {
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

int GString::strnicmp (const char* s1, const char* s2, int len) {
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

char* GString::strnistr (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

char* GString::strnstr (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

char* GString::strstr (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return NULL;
}

char* GString::strnnext (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strnext (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strninext (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

char* GString::strinext (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return NULL;
}

int GString::strtoi (const char* s, char** end, int base) {
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
	int i = 0;
	for(int c = *s; *s != 0; c = *++s) {
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

bool GFile::Read (void* data, int64_t size) {
	return _file ? (fread(data, size, 1, _file) == 1) : false;
}

bool GFile::Write (const void* data, int64_t size) {
	return _file ? (fwrite(data, size, 1, _file) == 1) : false;
}

bool GFile::IsOpen () const {
	return _file != NULL;
}

int64_t GFile::GetPosition () const {
	return _file ? (int)ftello(_file) : 0;
}

int64_t GFile::GetSize () const {
	if(_file == NULL)
		return 0;
	int64_t current = (int)ftello(_file);
	fseeko(_file, 0, SEEK_END);
	int64_t size = (int)ftello(_file);
	fseeko(_file, current, SEEK_SET);
	return size;
}

bool GFile::SetPosition (int64_t position) {
	return _file ? (fseeko(_file, (off_t)position, SEEK_SET) == 0) : false;
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













enum _eCompressType {
	_COMPRESS_TYPE_ZLIB = 0,
};

static int64_t _ZLIBCompress (uint8_t* srcBuffer, int64_t srcSize, uint8_t* dstBuffer, int64_t dstSize, int64_t level) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = static_cast<uInt>(srcSize);
	stream.next_out = dstBuffer;
	stream.avail_out = static_cast<uInt>(dstSize);
	
	if(deflateInit(&stream, level) != Z_OK)
		return 0;
	
	if(deflate(&stream, Z_FINISH) != Z_STREAM_END)
		return 0;
	
	if(deflateEnd(&stream) != Z_OK)
		return 0;
	
	return stream.total_out;
}

static int64_t _ZLIBDecompress (uint8_t* srcBuffer, int64_t srcSize, uint8_t* dstBuffer, int64_t dstSize) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = static_cast<uInt>(srcSize);
	stream.next_out = dstBuffer;
	stream.avail_out = static_cast<uInt>(dstSize);
	
	if(inflateInit(&stream) != Z_OK)
		return 0;
	
	if(inflate(&stream, Z_FINISH) != Z_STREAM_END)
		return 0;
	
	
	if(inflateEnd(&stream) != Z_OK)
		return 0;
	
	return stream.total_out;
}

int64_t GArchive::Compress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize) {
	if(dstSize < 2)
		return 0;
	
	// Set the header data
	((uint8_t*)dstBuffer)[0] = VERSION;
	((uint8_t*)dstBuffer)[1] = static_cast<uint8_t>(_COMPRESS_TYPE_ZLIB);
	dstBuffer = ((uint8_t*)dstBuffer) + 2;
	dstSize -= 2;
	
	// Max zlib compression
	dstSize = _ZLIBCompress((uint8_t*)srcBuffer, srcSize, (uint8_t*)dstBuffer, dstSize, 9);
	
	// Return the actual size of the compressed data
	return dstSize != 0 ? dstSize + 2 : 0;
}

int64_t GArchive::Decompress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize) {
	if(srcSize < 2)
		return 0;
	
	// Check the version
	if(((uint8_t*)srcBuffer)[0] != VERSION)
		return 0;
	
	// Get the remainder of the header data
	_eCompressType compressType = static_cast<_eCompressType>(((uint8_t*)srcBuffer)[1]);
	srcBuffer = ((uint8_t*)srcBuffer) + 2;
	srcSize -= 2;
	
	// Decompress depending on the type
	switch(compressType) {
		case _COMPRESS_TYPE_ZLIB:
			return _ZLIBDecompress((uint8_t*)srcBuffer, srcSize, (uint8_t*)dstBuffer, dstSize);
		default:
			return 0;
	}
	
	return 0;
}

int64_t GArchive::GetBufferBounds (int64_t srcSize) {
	// Returns the worst case buffer size for compression, plus the size of the needed
	// header information for this library (version number and compression type)
	return compressBound(srcSize) + 2;
}







void GConsole::Print (const char* message, ...) {
	if (message) {
		va_list args;
		va_start(args, message);
		vprintf(message, args);
		va_end(args);
	}
}

void GConsole::Debug (const char* message, ...) {
#if DEBUG
	if (message) {
		va_list args;
		va_start(args, message);
		vprintf(message, args);
		va_end(args);
	}
#endif
}
