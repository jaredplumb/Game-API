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
