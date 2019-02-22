#include "GFont.h"



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
 value represented by their 2-4 byte UTF-8 characters sequences similar to an RGBA value.
 */




struct GFont::_PrivateData {
	int_t						height;
	int_t						base;
	int_t						charCount;
	int_t						hashCount;
	GRect*						rects;
	GPoint*						offsets;
	int_t*						advances;
	std::map<int_t, int_t>**	kernings;
	uint32*						hash;
	GImage						image;
};




static int_t _FindIndexFromHash (uint32* hash, int_t hashCount, uint32 character) {
	if(character < (1 << 7)) {
		return character;
	} else {
		// Binary search since the hash is pre-sorted
		int_t min = 0;
		int_t max = hashCount - 1;
		while(max >= min) {
			int_t mid = min + (max - min) / 2;
			if(character > hash[mid])
				min = mid + 1;
			else if(character < hash[mid])
				max = mid - 1;
			else
				return 128 + mid; // The index from the hash starts at 128 because they are non-ASCII
		}
	}
	GConsole::Debug("ERROR: Could not find font character (%u) in hash!\n", character);
	return 0;
}





GFont::GFont ()
:	_data(NULL)
{
}

GFont::GFont (const Resource& resource)
:	_data(NULL)
{
	New(resource);
}

GFont::GFont (const GString& resource)
:	_data(NULL)
{
	New(resource);
}

GFont::~GFont () {
	Delete();
}

bool GFont::New (const Resource& resource) {
	
	_data = new _PrivateData;
	_data->height = resource.height;
	_data->base = resource.base;
	_data->charCount = resource.charCount;
	_data->hashCount = resource.hashCount;
	_data->rects = NULL;
	_data->offsets = NULL;
	_data->advances = NULL;
	_data->kernings = NULL;
	if(_data->charCount > 0) {
		_data->rects = new GRect[_data->charCount];
		_data->offsets = new GPoint[_data->charCount];
		_data->advances = new int_t[_data->charCount];
		_data->kernings = new std::map<int_t, int_t>*[_data->charCount];
		for(int_t i = 0; i < _data->charCount; i++) {
			_data->rects[i].x = resource.chars[i].x;
			_data->rects[i].y = resource.chars[i].y;
			_data->rects[i].width = resource.chars[i].width;
			_data->rects[i].height = resource.chars[i].height;
			_data->offsets[i].x = resource.chars[i].xoffset;
			_data->offsets[i].y = resource.chars[i].yoffset;
			_data->advances[i] = resource.chars[i].xadvance;
			_data->kernings[i] = NULL;
		}
	}
	
	_data->hash = NULL;
	if(_data->hashCount > 0) {
		_data->hash = new uint32[_data->hashCount];
		for(int_t i = 0; i < _data->hashCount; i++)
			_data->hash[i] = resource.hash[i];
	}
	
	
	if(_data->kernings != NULL) {
		for(uint32 i = 0; i < resource.kernCount; i++) {
			uint32 firstIndex = (uint32)(resource.kernings[i] & 0x0000000000ffffff);
			uint32 secondIndex = (uint32)((resource.kernings[i] & 0x0000ffffff000000) >> 24);
			int16 amount = (int16)((resource.kernings[i] & 0xffff000000000000) >> 48);
			if(firstIndex != 0 && secondIndex != 0 && amount != 0) {
				if(_data->kernings[firstIndex] == NULL)
					_data->kernings[firstIndex] = new std::map<int_t, int_t>();
				_data->kernings[firstIndex]->insert(std::pair<int_t, int_t>((int_t)secondIndex, (int_t)amount));
			}
		}
	}
	
	
	
	
	if(_data->image.New(resource.image) == false) {
		GConsole::Debug("ERROR: Could not create image with name font resource data!\n");
		return false;
	}
	
	return true;
}

bool GFont::New (const GString& resource) {
	return New(Resource(resource));
}

void GFont::Delete () {
	if(_data == NULL)
		return;
	
	if(_data->kernings) {
		for(int_t i = 0; i < _data->charCount; i++)
			if(_data->kernings[i])
				delete _data->kernings[i];
		delete [] _data->kernings;
		_data->kernings = NULL;
	}
	
	if(_data->rects) {
		delete [] _data->rects;
		delete [] _data->offsets;
		delete [] _data->advances;
		_data->rects = NULL;
		_data->offsets = NULL;
		_data->advances = NULL;
		_data->charCount = 0;
	}
	
	if(_data->hash) {
		delete [] _data->hash;
		_data->hash = NULL;
		_data->hashCount = 0;
	}
	
	delete _data;
	_data = NULL;
}



int_t GFont::GetLineHeight () const {
	return (_data ? _data->height : 0);
}

int_t GFont::GetBaseHeight () const {
	return (_data ? _data->base : 0);
}

GRect GFont::GetRect (const GString& text) const {
	
	if(_data == NULL || _data->charCount == 0 || text.IsEmpty())
		return GRect();
	
	int_t x1 = 1073741824;
	int_t y1 = 1073741824;
	int_t x2 = -1073741824;
	int_t y2 = -1073741824;
	
	int_t length = text.GetLength();
	int_t x = 0;
	int_t y = 0;
	int_t index = 0;
	int_t last = 0; // Last character rendered for kerning
	
	for(int_t i = 0; i < length; i++) {
		if(text[i] == '\n') {
			x = 0;
			y += _data->height;
			last = 0;
		} else {
			// Find the index
			if((uint8)text[i] <= 0x7f) {
				// ASCII characters are always indexed exactly
				index = text[i];
			} else if((uint8)text[i] >= 0xc2 && (uint8)text[i] <= 0xdf) {
				// Find the index of two-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, ((0) | (0 << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 1;
			} else if((uint8)text[i] >= 0xe0 && (uint8)text[i] <= 0xef) {
				// Find the index of three-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, ((0) | ((uint8)text[i + 2] << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 2;
			} else if((uint8)text[i] >= 0xf0 && (uint8)text[i] <= 0xf4) {
				// Find the index of four-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, (((uint8)text[i + 3]) | ((uint8)text[i + 2] << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 3;
			} else {
				GConsole::Debug("ERROR: Unknown character (%x) found while drawing font!\n", (uint8)text[i]);
			}
			
			// Draw the character
			if(index != 0 && index < _data->charCount) {
				
				// Find the kerning value if one exists
				int_t kerning = 0;
				if(last != 0 && _data->kernings[index] != NULL) {
					std::map<int_t, int_t>::iterator find = _data->kernings[index]->find(last);
					if(find != _data->kernings[index]->end())
						kerning = find->second;
				}
				
				
				if(x1 > x + _data->offsets[index].x + kerning)
					x1 = x + _data->offsets[index].x + kerning;
				if(y1 > y + _data->offsets[index].y)
					y1 = y + _data->offsets[index].y;
				
				if(x2 < x + _data->offsets[index].x + kerning + _data->rects[index].width)
					x2 = x + _data->offsets[index].x + kerning + _data->rects[index].width;
				if(y2 < y + _data->offsets[index].y + _data->rects[index].height)
					y2 = y + _data->offsets[index].y + _data->rects[index].height;
				
				
				x += _data->advances[index];
				
				
				
			} else {
				GConsole::Debug("ERROR: Index not found while drawing font!\n");
			}
			
			last = index;
		}
	}
	
	return GRect(x1, y1, x2 - x1, y2 - y1);
}


bool GFont::IsEmpty () const {
	return _data == NULL || _data->image.IsEmpty();
}


void GFont::Draw (const GString& text, int_t x, int_t y, float alpha) {
	if(_data == NULL || _data->charCount == 0 || text.IsEmpty())
		return;
	
	int_t length = text.GetLength();
	int_t xreset = x;
	int_t index = 0;
	int_t last = 0; // Last character rendered for kerning
	
	for(int_t i = 0; i < length; i++) {
		if(text[i] == '\n') {
			x = xreset;
			y += _data->height;
			last = 0;
		} else {
			// Find the index
			if((uint8)text[i] <= 0x7f) {
				// ASCII characters are always indexed exactly
				index = text[i];
			} else if((uint8)text[i] >= 0xc2 && (uint8)text[i] <= 0xdf) {
				// Find the index of two-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, ((0) | (0 << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 1;
			} else if((uint8)text[i] >= 0xe0 && (uint8)text[i] <= 0xef) {
				// Find the index of three-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, ((0) | ((uint8)text[i + 2] << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 2;
			} else if((uint8)text[i] >= 0xf0 && (uint8)text[i] <= 0xf4) {
				// Find the index of four-byte non-ASCII characters
				index = _FindIndexFromHash(_data->hash, _data->hashCount, (((uint8)text[i + 3]) | ((uint8)text[i + 2] << 8) | ((uint8)text[i + 1] << 16) | ((uint8)text[i] << 24)));
				i += 3;
			} else {
				GConsole::Debug("ERROR: Unknown character (%x) found while drawing font!\n", (uint8)text[i]);
			}
			
			// Draw the character
			if(index != 0 && index < _data->charCount) {
				
				// Find the kerning value if one exists
				int_t kerning = 0;
				if(last != 0 && _data->kernings[index] != NULL) {
					std::map<int_t, int_t>::iterator find = _data->kernings[index]->find(last);
					if(find != _data->kernings[index]->end())
						kerning = find->second;
				}
				
				_data->image.Draw(_data->rects[index], x + _data->offsets[index].x + kerning, y + _data->offsets[index].y, alpha);
				
				x += _data->advances[index];
				
				
				
			} else {
				GConsole::Debug("ERROR: Index not found while drawing font!\n");
			}
			
			last = index;
		}
	}
}


















GFont::Resource::Resource ()
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

GFont::Resource::Resource (const GString& resource)
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

GFont::Resource::~Resource () {
	Delete();
}







bool GFont::Resource::New (const GString& resource) {
	if(NewFromPackage(resource))
		return true;
	return NewFromFile(resource);
}



bool GFont::Resource::NewFromPackage (const GString& resource) {
	
	uint64 archiveSize = GPackage::GetSize(resource + ".font");
	uint8* archiveBuffer = new uint8[archiveSize];
	if(GPackage::Read(resource + ".font", archiveBuffer, archiveSize) == false) {
		GConsole::Debug("ERROR: Failed to read \"%s\" from packages!\n", (const char*)resource);
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
		GConsole::Debug("ERROR: Failed to read \"%s\"'s image file from packages!\n", (const char*)resource);
		return false;
	}
	
	return true;
}



// This function converts a UTF-32 (or Unicode) character to UTF-8, then converts it into a hash value combining up to 4 bytes into a
// single 32-bit value.  This is used becuase the Font engine accepts UTF-8 strings, and to convert the UTF-8 strings to a normal unicode
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



bool GFont::Resource::NewFromFile (const GString& resource) {
	
	GFile file;
	if(file.OpenForRead(resource) == false) {
		GConsole::Debug("ERROR: Failed to open font file \"%s\"!\n", (const char*)resource);
		return false;
	}
	
	const uint_t size = file.GetSize();
	uint8 buffer[size + 1];
	buffer[size] = 0;
	if(file.Read(buffer, sizeof(uint8) * size) == false) {
		GConsole::Debug("ERROR: Failed to read entire contents of font file \"%s\"!\n", (const char*)resource);
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
						GConsole::Debug("ERROR: Failed to get src image for font \"%s\"!\n", (const char*)resource);
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
						GConsole::Debug("ERROR: Too many characters found in font file \"%s\"!\n", (const char*)resource);
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
						GConsole::Debug("ERROR: Kernings \"first\" or \"second\" not found in font file \"%s\"!\n", (const char*)resource);
					
				} else {
					GConsole::Debug("ERROR: Too many kerning values in font file \"%s\"!\n", (const char*)resource);
				}
			}
			
		} else if(GString::isprint(*line)) {
			GConsole::Debug("ERROR: Unknown line found in font file \"%s\"!\n", (const char*)resource);
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



void GFont::Resource::Delete () {
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



bool GFont::Resource::WriteToPackage (GPackage& package, const GString& name) {
	
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
		GConsole::Debug("Failed to write to package resource \"%s\"!\n", (const char*)name);
		delete [] archiveBuffer;
		return false;
	}
	
	delete [] archiveBuffer;
	
	if(image.WriteToPackage(package, name + ".font") == false) {
		GConsole::Debug("Failed to write to package resource \"%s\"'s image!\n", (const char*)name);
		return false;
	}
	
	return true;
}





