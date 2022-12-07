#include "GTypes.h"
#include <zlib.h>




static char* CHAR_ALLOC (int size) {
	return new char[size];
}

static void FREE (char* string) {
	delete [] string;
}







GString::GString (const GString& string)
:	_string(nullptr)
,	_length(string.GetLength())
{
	if(string._string != nullptr) {
		_string = CHAR_ALLOC(_length + 1);
		strncpy(_string, string._string, _length);
		_string[_length] = '\0';
	}
}

GString::GString (const char* string)
:	_string(nullptr)
,	_length(strlen(string))
{
	if(string != nullptr) {
		_string = CHAR_ALLOC(_length + 1);
		strncpy(_string, string, _length);
		_string[_length] = '\0';
	}
}

GString::~GString () {
	if(_string) {
		FREE(_string);
		_string = nullptr;
	}
	_length = 0;
}

GString& GString::New (const GString& string) {
	if(_string) {
		FREE(_string);
		_string = nullptr;
	}
	_length = string.GetLength();
	if(string._string != nullptr) {
		_string = CHAR_ALLOC(_length + 1);
		strncpy(_string, string._string, _length);
		_string[_length] = '\0';
	}
	return *this;
}

GString& GString::New (const char* string) {
	if(_string) {
		FREE(_string);
		_string = nullptr;
	}
	_length = strlen(string);
	if(string != nullptr) {
		_string = CHAR_ALLOC(_length + 1);
		strncpy(_string, string, _length);
		_string[_length] = '\0';
	}
	return *this;
}

void GString::Delete () {
	if(_string) {
		FREE(_string);
		_string = nullptr;
	}
	_length = 0;
}

GString& GString::Format (const char* string, ...) {
	if(_string) {
		FREE(_string);
		_string = nullptr;
		_length = 0;
	}
	
	if(string == nullptr)
		return *this;
	
	// TODO: if I change to an allocator system, I could get
	//		the default allocator size, then do a vsnprintf,
	//		and if it does not return 0, then grab the actual
	//		allocated size instead
	
	va_list args;
	
	va_start(args, string);
	_length = vsnprintf(nullptr, 0, string, args);
	va_end(args);
	
	if(_length > 0) {
		_string = CHAR_ALLOC(_length + 1);
		va_start(args, string);
		vsnprintf(_string, _length + 1, string, args);
		va_end(args);
		_string[_length] = '\0';
	}
	
	return *this;
}

int GString::GetLength () const {
	if(_length <= 0)
		*(const_cast<int*>(&_length)) = strlen(_string);
	return _length;
}

bool GString::IsEmpty () const {
	return _string == nullptr || _string[0] == '\0';
}

GString& GString::Add (const GString& string) {
	if(string._string == nullptr || *string == '\0')
		return *this;
	
	if(_string == nullptr)
		return New(string);
	
	_length = GetLength() + string.GetLength();
	char* newstring = CHAR_ALLOC(_length + 1);
	strcpy(newstring, _string);
	strcat(newstring, string._string);
	FREE(_string);
	_string = newstring;
	return *this;
}

GString& GString::Add (const char* string) {
	if(string == nullptr || *string == '\0')
		return *this;
	
	if(_string == nullptr)
		return New(string);
	
	_length = GetLength() + strlen(string);
	char* newstring = CHAR_ALLOC(_length + 1);
	strcpy(newstring, _string);
	strcat(newstring, string);
	FREE(_string);
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
	if(_string == nullptr)
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
	char* n = CHAR_ALLOC(_length + 1);
	strncpy(n, s, _length);
	n[_length] = '\0';
	FREE(_string);
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
	if(dst == nullptr || src == nullptr) return dst;
	char* save = dst;
	while(*save != 0)
		save++;
	while((*save++ = *src++) != 0)
		;
	return dst;
}

int GString::strcmp (const char* s1, const char* s2) {
	if(s1 == nullptr)
		return (s2 ? -s2[0] : 0);
	if(s2 == nullptr)
		return s1[0];
	while(*s1 == *s2++)
		if(*s1++ == 0)
			return 0;
	return *s1 - *(s2 - 1);
}

char* GString::strcpy (char* dst, const char* src) {
	if(dst == nullptr)
		return nullptr;
	if(src == nullptr) {
		*dst = 0;
	} else {
		char* save = dst;
		while((*save++ = *src++) != 0)
			;
	}
	return dst;
}

int GString::stricmp (const char* s1, const char* s2) {
	if(s1 == nullptr)
		return (s2 ? -tolower(s2[0]) : 0);
	if(s2 == nullptr)
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
	return nullptr;
}

int GString::strlen (const char* s) {
	if(s == nullptr)
		return 0;
	const char* i = s;
	while(*i)
		i++;
	return static_cast<int>(reinterpret_cast<intptr_t>(i) - reinterpret_cast<intptr_t>(s));
}

char* GString::strncat (char* dst, const char* src, int len) {
	if(dst == nullptr)
		return nullptr;
	char* save = dst;
	while(*save != 0)
		save++;
	if(src != nullptr)
		while(--len >= 0 && (*save++ = *src++) != 0)
			;
	while(--len >= 0)
		*save++ = 0;
	return dst;
}

int GString::strncmp (const char* s1, const char* s2, int len) {
	if(len == 0)
		return 0;
	if(s1 == nullptr)
		return (s2 ? -s2[0] : 0);
	if(s2 == nullptr)
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
	if(dst == nullptr)
		return nullptr;
	char* save = dst;
	if(src != nullptr)
		while(--len >= 0 && (*save++ = *src++) != 0)
			;
	while(--len >= 0)
		*save++ = 0;
	return dst;
}

int GString::strnicmp (const char* s1, const char* s2, int len) {
	if(len == 0)
		return 0;
	if(s1 == nullptr)
		return (s2 ? -tolower(s2[0]) : 0);
	if(s2 == nullptr)
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
	return nullptr;
}

char* GString::strnstr (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return nullptr;
}

char* GString::strstr (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)s;
	return nullptr;
}

char* GString::strnnext (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return nullptr;
}

char* GString::strnext (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strncmp(s, find, len) == 0)
			return (char*)(s + len);
	return nullptr;
}

char* GString::strninext (const char* s, const char* find, int len) {
	for(; *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return nullptr;
}

char* GString::strinext (const char* s, const char* find) {
	for(int len = strlen(find); *s != 0; s++)
		if(strnicmp(s, find, len) == 0)
			return (char*)(s + len);
	return nullptr;
}

int GString::strtoi (const char* s, char** end, int base) {
	if(s == nullptr) {
		if(end != nullptr)
			*end = nullptr;
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
	if(end != nullptr)
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















enum eCompressType {
	COMPRESS_TYPE_ZLIB = 0,
};

static int64_t ZLIBCompress (uint8_t* srcBuffer, int64_t srcSize, uint8_t* dstBuffer, int64_t dstSize, int64_t level) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (uInt)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (uInt)dstSize;
	if(deflateInit(&stream, level) != Z_OK || deflate(&stream, Z_FINISH) != Z_STREAM_END || deflateEnd(&stream) != Z_OK)
		return 0;
	return stream.total_out;
}

static int64_t ZLIBDecompress (uint8_t* srcBuffer, int64_t srcSize, uint8_t* dstBuffer, int64_t dstSize) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (uInt)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (uInt)dstSize;
	if(inflateInit(&stream) != Z_OK || inflate(&stream, Z_FINISH) != Z_STREAM_END || inflateEnd(&stream) != Z_OK)
		return 0;
	return stream.total_out;
}

int64_t GArchive::Compress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize) {
	if(dstSize < 2 * sizeof(uint8_t))
		return 0;
	*((uint8_t*)dstBuffer + 0 * sizeof(uint8_t)) = VERSION;
	*((uint8_t*)dstBuffer + 1 * sizeof(uint8_t)) = (uint8_t)COMPRESS_TYPE_ZLIB;
	dstSize = ZLIBCompress((uint8_t*)srcBuffer, srcSize, (uint8_t*)dstBuffer + 2 * sizeof(uint8_t), dstSize - 2 * sizeof(uint8_t), 9);
	return dstSize != 0 ? dstSize + 2 * sizeof(uint8_t) : 0;
}

int64_t GArchive::Decompress (const void* srcBuffer, int64_t srcSize, void* dstBuffer, int64_t dstSize) {
	if(srcSize < 2 * sizeof(uint8_t))
		return 0;
	if(*((uint8_t*)srcBuffer) != VERSION)
		return 0;
	switch((eCompressType)*((uint8_t*)srcBuffer + sizeof(uint8_t))) {
		case COMPRESS_TYPE_ZLIB:
			return ZLIBDecompress((uint8_t*)srcBuffer + 2 * sizeof(uint8_t), srcSize - 2 * sizeof(uint8_t), (uint8_t*)dstBuffer, dstSize);
		default:
			return 0;
	}
	return 0;
}

int64_t GArchive::GetBufferBounds (int64_t srcSize) {
	// Returns the worst case buffer size for compression plus the size of the header information (version number and compression type)
	return compressBound(srcSize) + 2 * sizeof(uint8_t);
}
