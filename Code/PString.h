#ifndef _P_STRING_H_
#define _P_STRING_H_

#include "PPlatform.h"

class PString {
public:
	PString ();
	PString (const PString& string);
	PString (const char* string);
	~PString ();
	PString& New (const PString& string);
	PString& New (const char* string);
	void Delete ();
	PString& Format (const char* string, ...);
	int_t GetLength () const;
	bool IsEmpty () const; // Returns true if the string is NULL or ""
	PString& Add (const PString& string);
	PString& Add (const char* string);
	PString& ToLower ();
	PString& ToUpper ();
	PString& TrimSpaces ();
	PString& TrimExtension (); // Removes the extension off of a file path
	PString& TrimToDirectory (); // Removes the file off of a file path
	
	// Standard C string overrides with modifications
	static bool isalnum (char c);
	static bool isalpha (char c);
	static bool isdigit (char c);
	static bool isgraph (char c);
	static bool islower (char c);
	static bool isprint (char c);
	static bool ispunct (char c);
	static bool isspace (char c);
	static bool isupper (char c);
	static bool isxdigit (char c);
	static char* strcat (char* dst, const char* src);
	static int_t strcmp (const char* s1, const char* s2);
	static char* strcpy (char* dst, const char* src);
	static int_t stricmp (const char* s1, const char* s2);
	static char* stristr (const char* s, const char* find);
	static int_t strlen (const char* s);
	static char* strncat (char* dst, const char* src, int_t len);
	static int_t strncmp (const char* s1, const char* s2, int_t len);
	static char* strncpy (char* dst, const char* src, int_t len);
	static int_t strnicmp (const char* s1, const char* s2, int_t len);
	static char* strnistr (const char* s, const char* find, int_t len);
	static char* strnstr (const char* s, const char* find, int_t len);
	static char* strstr (const char* s, const char* find);
	
	// Finds the first accurance of find in s, and returns a pointer to the first character after, or NULL if find failed
	static char* strnnext (const char* s, const char* find, int_t len);
	static char* strnext (const char* s, const char* find);
	static char* strninext (const char* s, const char* find, int_t len);
	static char* strinext (const char* s, const char* find);
	
	static int_t strtoi (const char* s, char** end, int_t base); // Returns an int_t of the string, with an optional end pointer and base
	// strtod https://opensource.apple.com/source/tcl/tcl-10/tcl/compat/strtod.c
	static char tolower (char c);
	static char* tolower (char* s);
	static char toupper (char c);
	static char* toupper (char* s);
	
	// Operator overloads
	inline operator char* (void)									{ _length = 0; return _string; }
	inline operator const char* (void) const						{ *(const_cast<int_t*>(&_length)) = 0; return _string; }
	inline char& operator[] (int_t index)							{ _length = 0; return _string[index]; }
	inline const char& operator[] (int_t index) const				{ return _string[index]; }
	inline PString& operator= (const PString& string)				{ return New(string); }
	inline PString& operator= (const char* string)					{ return New(string); }
	inline const PString operator+ (const PString& string) const	{ return PString(*this).Add(string); }
	inline const PString operator+ (const char* string) const		{ return PString(*this).Add(string); }
	inline PString& operator+= (const PString& string)				{ return Add(string); }
	inline PString& operator+= (const char* string)					{ return Add(string); }
	inline bool operator== (const PString& string) const			{ return strcmp(_string, string._string) == 0; }
	inline bool operator== (const char* string) const				{ return strcmp(_string, string) == 0; }
	inline bool operator!= (const PString& string) const			{ return strcmp(_string, string._string) != 0; }
	inline bool operator!= (const char* string) const				{ return strcmp(_string, string) != 0; }
	inline bool operator< (const PString& string) const				{ return strcmp(_string, string._string) < 0; }
	inline bool operator< (const char* string) const				{ return strcmp(_string, string) < 0; }
	inline bool operator<= (const PString& string) const			{ return strcmp(_string, string._string) <= 0; }
	inline bool operator<= (const char* string) const				{ return strcmp(_string, string) <= 0; }
	inline bool operator> (const PString& string) const				{ return strcmp(_string, string._string) > 0; }
	inline bool operator> (const char* string) const				{ return strcmp(_string, string) > 0; }
	inline bool operator>= (const PString& string) const			{ return strcmp(_string, string._string) >= 0; }
	inline bool operator>= (const char* string) const				{ return strcmp(_string, string) >= 0; }
	
private:
	char* _string;
	int_t _length;
};

#endif // _P_STRING_H_
