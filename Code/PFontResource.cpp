#include "PFontResource.h"


/* NOTES FOR BITMAP FONT CREATION
 I've been using the program bmGlyph to create fonts, and exporting for Unity, although any export
 should work as long as in creates the normal text file output for the glyph information.
 
 Good source of information can be found at http://www.angelcode.com/products/bmfont/documentation.html
 */


/* NOTES FOR FUTURE UTF-8 CONVERSION
https://en.wikipedia.org/wiki/UTF-8
https://tools.ietf.org/html/rfc3629

For the most part, UTF-8 works exactly the same as US-ASCII.  You can include special characters
right into the string, as in the example below.  However, when the value is 0x80 or higher, special
conditions must be used.  Any non-US-ASCII character is at least two bytes.

Example:
const char* string = "Hell™o•World!";
printf("%s\n", string);
for(int i = 0; string[i] != '\0'; i++)
	printf("%c %d %x\n", string[i], (uchar)string[i], (uchar)string[i]);

hex 00-7f is US-ASCII			(127 values)
hex c2-df indicates 2 bytes		(29 values)
hex e0-ef indicates 3 bytes		(15 values)
hex f0-f4 indicates 4 bytes		(4 values)
hex 80-bf is a sequencial byte	(63 values)
others are invalid

*/

/* NOTES ABOUT THE FORMAT USED HERE
 All printable ASCII characters are automatiaclly added to make the UTF8 work better, this means that 127
 characters are always present in the font file, then additional are added as a hash.  The hash is a 32-bit
 value represented by their 2-4 byte UTF-8 characters sequences similar to an ARGB value.
 */


PFontResource::PFontResource ()
:	height(0)
,	base(0)
,	charCount(0)
,	hashCount(0)
,	kernCount(0)
,	chars(NULL)
,	hash(NULL)
,	kernings(NULL)
{
}



PFontResource::~PFontResource () {
	Delete();
}



PFontResource::PFontResource (const GString& resource)
:	height(0)
,	base(0)
,	charCount(0)
,	hashCount(0)
,	kernCount(0)
,	chars(NULL)
,	hash(NULL)
,	kernings(NULL)
{
	New(resource);
}



bool PFontResource::New (const GString& resource) {
	if(NewFromPackage(resource))
		return true;
	return NewFromFile(resource);
}



bool PFontResource::NewFromPackage (const GString& resource) {
	
	uint64 archiveSize = PPackage::GetSize(resource + ".font");
	uint8* archiveBuffer = new uint8[archiveSize];
	if(PPackage::Read(resource + ".font", archiveBuffer, archiveSize) == false) {
		PSystem::Debug("ERROR: Failed to read \"%s\" from packages!\n", (const char*)resource);
		delete [] archiveBuffer;
		return false;
	}
	
	uint64 headerSize = sizeof(height) + sizeof(base) + sizeof(charCount) + sizeof(hashCount) + sizeof(kernCount);
	memcpy(this, archiveBuffer, headerSize);
	
	if(charCount > 0) {
		chars = new Char[charCount];
		memcpy(chars, archiveBuffer + headerSize, sizeof(Char) * charCount);
	}
	
	if(hashCount > 0) {
		hash = new uint32[hashCount];
		memcpy(hash, archiveBuffer + headerSize + sizeof(Char) * charCount, sizeof(uint32) * hashCount);
	}
	
	if(kernCount > 0) {
		kernings = new uint64[kernCount];
		memcpy(kernings, archiveBuffer + headerSize + sizeof(Char) * charCount + sizeof(uint32) * hashCount, sizeof(uint64) * kernCount);
	}
	
	delete [] archiveBuffer;
	
	if(image.NewFromPackage(resource + ".font") == false) {
		PSystem::Debug("ERROR: Failed to read \"%s\"'s image file from packages!\n", (const char*)resource);
		return false;
	}
	
	return true;
}



// This function converts a UTF-32 (or Unicode) character to UTF-8, then converts it into a hash value combining up to 4 bytes into a 
// single 32-bit value.  This is used becuase the Font engine accepts UTF-8 strings, and the convert the UTF-8 strings to a normal unicode
// lookup would take additional math, so this removes one layer of calculations needed when non-ASCII characters are encountered.
static uint32 _ConvertUnicodeToHash (uint32 c) {
	if(c < (1 << 7)) { // 1-byte ASCII characters
		return c;										// 0xxxxxxx
	} else if(c < (1 << 11)) { // 2-byte characters
		uint8 utf8[2];
		utf8[0] = (uint8)((c >> 6) | 0xc0);				// 110xxxxx
		utf8[1] = (uint8)((c & 0x3f) | 0x80);			// 10xxxxxx
		return ((0) | (0 << 8) | ((uint8)utf8[1] << 16) | ((uint8)utf8[0] << 24));
	} else if(c < (1 << 16)) { // 3-byte characters
		uint8 utf8[3];
		utf8[0] = (uint8)((c >> 12) | 0xe0);			// 1110xxxx
		utf8[1] = (uint8)(((c >> 6) & 0x3f) | 0x80);	// 10xxxxxx
		utf8[2] = (uint8)((c & 0x3f) | 0x80);			// 10xxxxxx
		return ((0) | ((uint8)utf8[2] << 8) | ((uint8)utf8[1] << 16) | ((uint8)utf8[0] << 24));
	} else if(c < (1 << 21)) { // 4-byte characters
		uint8 utf8[4];
		utf8[0] = (uint8)(((c >> 18)) | 0xF0);			// 11110xxx
		utf8[1] = (uint8)(((c >> 12) & 0x3F) | 0x80);	// 10xxxxxx
		utf8[2] = (uint8)(((c >> 6) & 0x3F) | 0x80);	// 10xxxxxx
		utf8[3] = (uint8)((c & 0x3F) | 0x80);			// 10xxxxxx
		return (((uint8)utf8[3]) | ((uint8)utf8[2] << 8) | ((uint8)utf8[1] << 16) | ((uint8)utf8[0] << 24));
	}
	return 0;
}



bool PFontResource::NewFromFile (const GString& resource) {
	
	PFile file;
	if(file.OpenForRead(resource) == false) {
		PSystem::Debug("ERROR: Failed to open font file \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	const uint_t size = file.GetSize();
	uint8 buffer[size + 1];
	buffer[size] = 0;
	if(file.Read(buffer, sizeof(uint8) * size) == false) {
		PSystem::Debug("ERROR: Failed to read entire contents of font file \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	uint32 charMax = 0;	// Max number of character values allowed (for in case the file is corrupted)
	uint32 hashMax = 0;	// Max number of hash values allowed (for in case the file is corrupted)
	uint32 kernMax = 0;	// Max number of kerning values allowed (for in case the file is corrupted)
	
	uint8* line = buffer;
	while(line != NULL && *line != 0) {
		
		
		if(GString::strnicmp("info", (const char*)line, 4) == 0) {
			
		} else if(GString::strnicmp("common", (const char*)line, 6) == 0) {
			
			line = (uint8*)GString::strinext((const char*)line, "lineHeight=");
			if(line)
				height = GString::strtoi((const char*)line, (char**)&line, 10);
			
			line = (uint8*)GString::strinext((const char*)line, "base=");
			if(line)
				base = GString::strtoi((const char*)line, (char**)&line, 10);
			
		} else if(GString::strnicmp("page", (const char*)line, 4) == 0) {
			
			line = (uint8*)GString::strinext((const char*)line, "file=\"");
			if(line) {
				
				uint8* end = (uint8*)GString::strstr((const char*)line, "\"");
				if(end) {
					*end = 0;
					
					
					if(image.NewFromFile(GString(resource).TrimToDirectory() + (const char*)line) == false) {
						PSystem::Debug("ERROR: Failed to get src image for font \"%s\"!\n", (const char*)resource);
						return false;
					}
					
					
					line = ++end;
				}
				
			}
			
		} else if(GString::strnicmp("chars", (const char*)line, 5) == 0) {
			
			line = (uint8*)GString::strinext((const char*)line, "count=");
			if(line) {
				charMax = (uint32)GString::strtoi((const char*)line, (char**)&line, 10);
				chars = new Char[charMax + 127];
				memset(chars, 0, sizeof(Char) * 127); // Only need to zero the first 127, since more might not be used
				hashMax = charMax;
				if(hashMax > 0)
					hash = new uint32[hashMax];
			}
			
		} else if(GString::strnicmp("char ", (const char*)line, 5) == 0) {
			
			uint32 id = 0;
			Char glyph;
			memset(&glyph, 0, sizeof(Char));
			line = (uint8*)GString::strinext((const char*)line, "id=");
			if(line) id = (uint32)GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "x=");
			if(line) glyph.x = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "y=");
			if(line) glyph.y = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "width=");
			if(line) glyph.width = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "height=");
			if(line) glyph.height = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "xoffset=");
			if(line) glyph.xoffset = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "yoffset=");
			if(line) glyph.yoffset = GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "xadvance=");
			if(line) glyph.xadvance = GString::strtoi((const char*)line, (char**)&line, 10);
			
			if((glyph.width != 0 && glyph.height != 0) || glyph.xadvance != 0) {
				if(id < (1 << 7)) { // ASCII characters
					chars[id] = glyph;
				} else if(id < (1 << 21)) { // Multi-Byte UTF-8 character
					if(hashCount < hashMax) {
						chars[128 + hashCount] = glyph;
						hash[hashCount++] =  _ConvertUnicodeToHash(id);
					} else {
						PSystem::Debug("ERROR: Too many characters found in font file \"%s\"!\n", (const char*)resource);
					}
				}
			}
			
		} else if(GString::strnicmp("kernings", (const char*)line, 8) == 0) {
			
			line = (uint8*)GString::strinext((const char*)line, "count=");
			if(line) {
				kernMax = (uint32)GString::strtoi((const char*)line, (char**)&line, 10);
				if(kernMax > 0) {
					kernings = new uint64[kernMax];
					memset(kernings, 0, sizeof(uint64) * kernMax);
				}
			}
			
		} else if(GString::strnicmp("kerning ", (const char*)line, 8) == 0) {
			
			uint32 first = 0;
			uint32 second = 0;
			int16 amount = 0;
			line = (uint8*)GString::strinext((const char*)line, "first=");
			if(line) first = (uint32)GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "second=");
			if(line) second = (uint32)GString::strtoi((const char*)line, (char**)&line, 10);
			line = (uint8*)GString::strinext((const char*)line, "amount=");
			if(line) amount = (uint16)GString::strtoi((const char*)line, (char**)&line, 10);
			
			if(first != 0 && second != 0 && amount != 0) {
				// "first" and "second" are converted into an index values, indexing into the "chars" themselves
				// since Unicode characters only use 21 bits of information, first and second are converted to 24-bit values, 
				// allowing amount to remain in the last 16-bits
				if(kernCount < kernMax) {
					
					uint32 firstIndex = first;
					if(firstIndex >= (1 << 7)) {
						firstIndex = 0;
						first = _ConvertUnicodeToHash(first);
						for(uint32 i = 0; i < hashCount && firstIndex == 0; i++)
							if(first == hash[i])
								firstIndex = 128 + i;
					}
					
					uint32 secondIndex = second;
					if(secondIndex >= (1 << 7)) {
						secondIndex = 0;
						second = _ConvertUnicodeToHash(second);
						for(uint32 i = 0; i < hashCount && secondIndex == 0; i++)
							if(second == hash[i])
								secondIndex = 128 + i;
					}
					
					if(firstIndex != 0 && secondIndex != 0)
						kernings[kernCount++] = ((uint64)firstIndex | ((uint64)secondIndex << 24) | ((uint64)amount << 48));
					else
						PSystem::Debug("ERROR: Kernings \"first\" or \"second\" not found in font file \"%s\"!\n", (const char*)resource);
					
				} else {
					PSystem::Debug("ERROR: Too many kerning values in font file \"%s\"!\n", (const char*)resource);
				}
			}
			
		} else if(GString::isprint(*line)) {
			PSystem::Debug("ERROR: Unknown line found in font file \"%s\"!\n", (const char*)resource);
		}
		
		
		if(line != NULL) {
			while(GString::isprint(*line))
				line++;
			line++;
		}
		
	}
	
	// Adjust charCount to be the actual number of characters found
	charCount = 127 + hashCount;
	
	// Sort the hash values
	if(hashCount > 0) {
		for(uint32 i = 0; i < hashCount; i++)
			for(uint32 j = i + 1; j < hashCount; j++)
				if(hash[i] > hash[j]) {
					uint32 tempHash = hash[i];
					hash[i] = hash[j];
					hash[j] = tempHash;
					Char tempChar = chars[128 + i];
					chars[128 + i] = chars[128 + j];
					chars[128 + j] = tempChar;
				}
	}
	
	return true;
}



void PFontResource::Delete () {
	height = 0;
	base = 0;
	charCount = 0;
	hashCount = 0;
	kernCount = 0;
	if(chars) {
		delete [] chars;
		chars = NULL;
	}
	if(hash) {
		delete [] hash;
		hash = NULL;
	}
	if(kernings) {
		delete [] kernings;
		kernings = NULL;
	}
	image.Delete();
}



bool PFontResource::WriteToPackage (PPackage& package, const GString& name) {
	
	uint64 headerSize = sizeof(height) + sizeof(base) + sizeof(charCount) + sizeof(hashCount) + sizeof(kernCount);
	uint64 archiveSize = headerSize + sizeof(Char) * charCount + sizeof(uint32) * hashCount + sizeof(uint64) * kernCount;
	
	uint8* archiveBuffer = new uint8[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	if(charCount > 0)
		memcpy(archiveBuffer + headerSize, chars, sizeof(Char) * charCount);
	
	if(hashCount > 0)
		memcpy(archiveBuffer + headerSize + sizeof(Char) * charCount, hash, sizeof(uint32) * hashCount);
	
	if(kernCount > 0)
		memcpy(archiveBuffer + headerSize + sizeof(Char) * charCount + sizeof(uint32) * hashCount, kernings, sizeof(uint64) * kernCount);
	
	if(package.Write(name + ".font", archiveBuffer, archiveSize) == false) {
		PSystem::Debug("Failed to write to package resource \"%s\"!\n", (const char*)name);
		delete [] archiveBuffer;
		return false;
	}
	
	delete [] archiveBuffer;
	
	if(image.WriteToPackage(package, name + ".font") == false) {
		PSystem::Debug("Failed to write to package resource \"%s\"'s image!\n", (const char*)name);
		return false;
	}
	
	return true;
}


