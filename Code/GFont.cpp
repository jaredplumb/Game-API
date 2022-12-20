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
 hex c2-df indicates 2 bytes	(29 values)
 hex e0-ef indicates 3 bytes	(15 values)
 hex f0-f4 indicates 4 bytes	(4 values)
 hex 80-bf is a sequencial byte	(63 values)
 others are invalid
 
 */

/* NOTES ABOUT THE FORMAT USED HERE
 All printable ASCII characters are automatiaclly added to make the UTF8 work better, this means that 127
 characters are always present in the font file, then additional are added as a hash.  The hash is a 32-bit
 value represented by their 2-4 byte UTF-8 characters sequences similar to an RGBA value.
 */



bool GFont::New (const Resource& resource) {
	_height = 0;
	_base = 0;
	_rects.clear();
	_offsets.clear();
	_advances.clear();
	_has_kern.clear();
	_hash.clear();
	_kernings.clear();
	
	GImage::Resource imageResource;
	imageResource.width = resource.imageWidth;
	imageResource.height = resource.imageHeight;
	imageResource.bufferSize = resource.bufferSize;
	imageResource.buffer = resource.buffer;
	if(!_image.New(imageResource)) {
		GSystem::Debug("Could not create image with font resource data!\n");
		imageResource.buffer = nullptr;
		return false;
	}
	imageResource.buffer = nullptr;
	
	_height = resource.height;
	_base = resource.base;
	
	if(resource.charCount > 0) {
		_rects.resize(resource.charCount);
		_offsets.resize(resource.charCount);
		_advances.resize(resource.charCount);
		_has_kern.resize(resource.charCount);
		for(int i = 0; i < resource.charCount; i++) {
			_rects[i].x = resource.chars[i].srcX;
			_rects[i].y = resource.chars[i].srcY;
			_rects[i].width = resource.chars[i].srcWidth;
			_rects[i].height = resource.chars[i].srcHeight;
			_offsets[i].x = resource.chars[i].xOffset;
			_offsets[i].y = resource.chars[i].yOffset;
			_advances[i] = resource.chars[i].xAdvance;
			_has_kern[i] = false;
		}
	}
	
	if(resource.hashCount > 0 && resource.hashCount + (1 << 7) - 1 <= resource.charCount) {
		for(int i = 0; i < resource.hashCount; i++)
			_hash[resource.hash[i]] = i + (1 << 7);
	}
	
	if(resource.kernCount > 0) {
		for(int i = 0; i < resource.kernCount; i++) {
			int first = (int)(resource.kernings[i] & 0x0000000000ffffff);
			int second = (int)((resource.kernings[i] & 0x0000ffffff000000) >> 24);
			int amount = (int)((resource.kernings[i] & 0xffff000000000000) >> 48);
			_kernings[std::make_pair(first, second)] = amount;
		}
	}
	
	return true;
}



GRect GFont::GetRect (const GString& text) const {
	if(text.IsEmpty() || _rects.size() < ((1 << 7) - 1))
		return {};
	
	int left = std::numeric_limits<int>::max();
	int top = std::numeric_limits<int>::max();
	int right = std::numeric_limits<int>::min();
	int bottom = std::numeric_limits<int>::min();
	
	GPoint pos;
	int index = 0;
	int last = 0; // Last character rendered (used for kernings)
	
	for(int i = 0; i < text.GetLength(); i++) {
		if(text[i] != '\n') {
			// Find the index
			if((uint8_t)text[i] <= 0x7f) {
				// ASCII characters are always indexed exactly
				index = (int)(uint8_t)text[i];
			} else if((uint8_t)text[i] >= 0xc2 && (uint8_t)text[i] <= 0xdf && i + 1 < text.GetLength()) {
				// Find the index of two-byte non-ASCII characters
				index = GetIndexFromHash((0) | (0 << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 1;
			} else if((uint8_t)text[i] >= 0xe0 && (uint8_t)text[i] <= 0xef && i + 2 < text.GetLength()) {
				// Find the index of three-byte non-ASCII characters
				index = GetIndexFromHash((0) | ((uint8_t)text[i + 2] << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 2;
			} else if((uint8_t)text[i] >= 0xf0 && (uint8_t)text[i] <= 0xf4 && i + 3 < text.GetLength()) {
				// Find the index of four-byte non-ASCII characters
				index = GetIndexFromHash(((uint8_t)text[i + 3]) | ((uint8_t)text[i + 2] << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 3;
			} else {
				GSystem::Debug("Unknown character (0x%x) found while drawing font!\n", (uint8_t)text[i]);
			}
			
			// Adjust for a possible kerning
			int kerning = 0;
			if(_has_kern[last]) {
				auto k = _kernings.find(std::make_pair(last, index));
				if(k != _kernings.end())
					kerning = k->second;
			}
			
			// Find the visible rendered area
			const int renderLeft = pos.x + _offsets[index].x + kerning;
			const int renderTop = pos.y + _offsets[index].y;
			const int renderRight = renderLeft + _rects[index].width;
			const int renderBottom = renderTop + _rects[index].height;
			left = std::min(left, renderLeft);
			top = std::min(top, renderTop);
			right = std::max(right, renderRight);
			bottom = std::max(bottom, renderBottom);
			
			// Advance the x position
			pos.x += _advances[index];
			last = index;
		} else {
			// If a new line, reset the x position to 0 and move down the height
			pos.x = 0;
			pos.y += _height;
		}
	}
	
	return GRect(left, top, right - left, bottom - top);
}



void GFont::Draw (const GString& text, int x, int y, float alpha) {
	if(text.IsEmpty() || _rects.size() < ((1 << 7) - 1))
		return;
	
	GPoint pos(x, y);
	int index = 0;
	int last = 0; // Last character rendered (used for kernings)
	
	for(int i = 0; i < text.GetLength(); i++) {
		if(text[i] != '\n') {
			// Find the index
			if((uint8_t)text[i] <= 0x7f) {
				// ASCII characters are always indexed exactly
				index = (int)(uint8_t)text[i];
			} else if((uint8_t)text[i] >= 0xc2 && (uint8_t)text[i] <= 0xdf && i + 1 < text.GetLength()) {
				// Find the index of two-byte non-ASCII characters
				index = GetIndexFromHash((0) | (0 << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 1;
			} else if((uint8_t)text[i] >= 0xe0 && (uint8_t)text[i] <= 0xef && i + 2 < text.GetLength()) {
				// Find the index of three-byte non-ASCII characters
				index = GetIndexFromHash((0) | ((uint8_t)text[i + 2] << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 2;
			} else if((uint8_t)text[i] >= 0xf0 && (uint8_t)text[i] <= 0xf4 && i + 3 < text.GetLength()) {
				// Find the index of four-byte non-ASCII characters
				index = GetIndexFromHash(((uint8_t)text[i + 3]) | ((uint8_t)text[i + 2] << 8) | ((uint8_t)text[i + 1] << 16) | ((uint8_t)text[i] << 24));
				i += 3;
			} else {
				GSystem::Debug("Unknown character (0x%x) found while drawing font!\n", (uint8_t)text[i]);
			}
			
			// Draw the character adjusting for a possible kerning then advance the x position
			int kerning = 0;
			if(_has_kern[last]) {
				auto k = _kernings.find(std::make_pair(last, index));
				if(k != _kernings.end())
					kerning = k->second;
			}
			_image.Draw(_rects[index], pos.x + _offsets[index].x + kerning, pos.y + _offsets[index].y, alpha);
			pos.x += _advances[index];
			last = index;
		} else {
			// If a new line, reset the x position and move down the height
			pos.x = x;
			pos.y += _height;
		}
	}
}



bool GFont::Resource::New (const GString& name) {
	int64_t resourceSize = GSystem::ResourceSize(name + ".fnt");
	if(resourceSize <= sizeof(Resource))
		return false;
	
	std::unique_ptr<uint8_t[]> resourceBuffer(new uint8_t[resourceSize]);
	if(!GSystem::ResourceRead(name + ".fnt", resourceBuffer.get(), resourceSize))
		return false;
	
	int64_t offset = 0;
	height = *((int16_t*)(resourceBuffer.get() + offset));
	offset += sizeof(height);
	base = *((int16_t*)(resourceBuffer.get() + offset));
	offset += sizeof(base);
	charCount = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(charCount);
	hashCount = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(hashCount);
	kernCount = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(kernCount);
	imageWidth = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(imageWidth);
	imageHeight = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(imageHeight);
	bufferSize = *((int64_t*)(resourceBuffer.get() + offset));
	offset += sizeof(bufferSize);
	
	if(charCount * sizeof(Char) + hashCount * sizeof(int32_t) + kernCount * sizeof(int32_t) + bufferSize > resourceSize - offset)
		return false;
	
	if(charCount > 0) {
		chars = new Char[charCount];
		memcpy(chars, resourceBuffer.get() + offset, charCount * sizeof(Char));
		offset += charCount * sizeof(Char);
	}
	
	if(hashCount > 0) {
		hash = new uint32_t[hashCount];
		memcpy(hash, resourceBuffer.get() + offset, hashCount * sizeof(uint32_t));
		offset += hashCount * sizeof(uint32_t);
	}
	
	if(kernCount > 0) {
		kernings = new uint64_t[kernCount];
		memcpy(kernings, resourceBuffer.get() + offset, kernCount * sizeof(uint64_t));
		offset += kernCount * sizeof(uint64_t);
	}
	
	if(bufferSize > 0) {
		buffer = new uint8_t[bufferSize];
		memcpy(buffer, resourceBuffer.get() + offset, bufferSize);
		offset += bufferSize;
	}
	
	return true;
}



// This function converts a UTF-32 (or Unicode) character to UTF-8, then converts it into a hash value combining up to 4 bytes into a
// single 32-bit value.  This is used becuase the Font engine accepts UTF-8 strings, and to convert the UTF-8 strings to a normal unicode
// lookup would take additional math, so this removes one layer of calculations needed when non-ASCII characters are encountered.
static uint32_t ConvertUnicodeToHash (uint32_t c) {
	if(c < (1 << 7)) { // 1-byte ASCII characters
		return c;										// 0xxxxxxx
	} else if(c < (1 << 11)) { // 2-byte characters
		uint8_t utf8[2];
		utf8[0] = (uint8_t)((c >> 6) | 0xc0);			// 110xxxxx
		utf8[1] = (uint8_t)((c & 0x3f) | 0x80);			// 10xxxxxx
		return ((0) | (0 << 8) | ((uint8_t)utf8[1] << 16) | ((uint8_t)utf8[0] << 24));
	} else if(c < (1 << 16)) { // 3-byte characters
		uint8_t utf8[3];
		utf8[0] = (uint8_t)((c >> 12) | 0xe0);			// 1110xxxx
		utf8[1] = (uint8_t)(((c >> 6) & 0x3f) | 0x80);	// 10xxxxxx
		utf8[2] = (uint8_t)((c & 0x3f) | 0x80);			// 10xxxxxx
		return ((0) | ((uint8_t)utf8[2] << 8) | ((uint8_t)utf8[1] << 16) | ((uint8_t)utf8[0] << 24));
	} else if(c < (1 << 21)) { // 4-byte characters
		uint8_t utf8[4];
		utf8[0] = (uint8_t)(((c >> 18)) | 0xF0);		// 11110xxx
		utf8[1] = (uint8_t)(((c >> 12) & 0x3F) | 0x80);	// 10xxxxxx
		utf8[2] = (uint8_t)(((c >> 6) & 0x3F) | 0x80);	// 10xxxxxx
		utf8[3] = (uint8_t)((c & 0x3F) | 0x80);			// 10xxxxxx
		return (((uint8_t)utf8[3]) | ((uint8_t)utf8[2] << 8) | ((uint8_t)utf8[1] << 16) | ((uint8_t)utf8[0] << 24));
	}
	return 0;
}



bool GFont::Resource::NewFromFile (const GString& path) {
	int64_t fileSize = GSystem::ResourceSizeFromFile(path);
	if(fileSize <= 0)
		return false;
	
	std::unique_ptr<uint8_t[]> fileBuffer(new uint8_t[fileSize + 2]);
	if(!GSystem::ResourceReadFromFile(path, fileBuffer.get(), fileSize))
		return false;
	//for(int64_t i = 0; i < fileSize; i++)
	//	if(!GString::isprint((char)fileBuffer[i]))
	//		fileBuffer[i] = '\0';
	fileBuffer[fileSize + 0] = '\0';
	fileBuffer[fileSize + 1] = '\0';
	
	std::vector<Char> charsList((1 << 7) - 1);
	std::vector<uint32_t> hashList;
	std::vector<uint64_t> kerningsList;
	
	for(char* line = (char*)fileBuffer.get(); line != nullptr && *line != '\0'; line++) {
		
		// This tag holds information common to all characters.
		if(GString::strnicmp("common ", line, 7) == 0) {
			
			// This is the distance in pixels from the absolute top of the line to the next line.
			if((line = GString::strinext(line, "lineHeight=")) != nullptr)
				height = (int16_t)GString::strtoi(line, &line, 10);
			
			// The number of pixels from the absolute top of the line to the base of the characters (minus the spacing between lines).
			if((line = GString::strinext(line, "base=")) != nullptr)
				base = (int16_t)GString::strtoi(line, &line, 10);
			
		// This tag gives the name of a texture file. There is one for each page in the font.
		} else if(GString::strnicmp("page ", line, 5) == 0) {
			
			// The texture file name.
			if((line = GString::strinext(line, "file=\"")) != nullptr) {
				char* end = GString::strstr(line, "\"");
				if(end != nullptr) {
					*end = '\0';
					GImage::Resource imageResource;
					if(imageResource.NewFromFile(GString(path).TrimToDirectory() + line) == false) {
						GSystem::Debug("Failed to read src image for font \"%s\"!\n", (const char*)path);
						return false;
					}
					imageWidth = imageResource.width;
					imageHeight = imageResource.height;
					bufferSize = imageResource.bufferSize;
					buffer = imageResource.buffer;
					imageResource.buffer = nullptr; // This prevents the resource from being deleted
					*end = '\"';
				}
			}
			
		// This tag describes one character in the font. There is one for each included character in the font.
		} else if(GString::strnicmp("char ", line, 5) == 0) {
			
			// The character id.
			int index;
			if((line = GString::strinext(line, "id=")) == nullptr || (index = GString::strtoi(line, &line, 10)) <= 0)
				continue;
			
			Char glyph;
			
			// The left position of the character image in the texture.
			if((line = GString::strinext(line, "x=")) != nullptr)
				glyph.srcX = (int16_t)GString::strtoi(line, &line, 10);
			
			// The top position of the character image in the texture.
			if((line = GString::strinext(line, "y=")) != nullptr)
				glyph.srcY = (int16_t)GString::strtoi(line, &line, 10);
			
			// The width of the character image in the texture (note that some characters have a width of 0).
			if((line = GString::strinext(line, "width=")) != nullptr)
			   glyph.srcWidth = (int16_t)GString::strtoi(line, &line, 10);
			
			// The height of the character image in the texture (note that some characters have a height of 0).
			if((line = GString::strinext(line, "height=")) != nullptr)
			   glyph.srcHeight = (int16_t)GString::strtoi(line, &line, 10);
			
			// How much the current x position should be offset when copying the image from the texture to the screen.
			if((line = GString::strinext(line, "xoffset=")) != nullptr)
				glyph.xOffset = (int16_t)GString::strtoi(line, &line, 10);
			
			// How much the current y position should be offset when copying the image from the texture to the screen.
			if((line = GString::strinext(line, "yoffset=")) != nullptr)
				glyph.yOffset = (int16_t)GString::strtoi(line, &line, 10);
			
			// How much the current position should be advanced after drawing the character.
			if((line = GString::strinext(line, "xadvance=")) != nullptr)
				glyph.xAdvance = (int16_t)GString::strtoi(line, &line, 10);
			
			// Add the character to the lists, the first 127 characters (ASCII) are always present
			if(index < (1 << 7)) { // ASCII characters
				charsList[index] = glyph;
			} else if (index < (1 << 21)) { // Multi-Byte UTF-8 character
				charsList.push_back(glyph);
				hashList.push_back(ConvertUnicodeToHash((uint32_t)index));
			}
			
		// The kerning information is used to adjust the distance between certain characters, e.g. some characters should be placed closer to each other than others.
		} else if(GString::strnicmp("kerning ", line, 8) == 0) {
			
			// The first character id.
			int first;
			if((line = GString::strinext(line, "first=")) == nullptr || (first = GString::strtoi(line, &line, 10)) <= 0)
				continue;
			
			// The second character id.
			int second;
			if((line = GString::strinext(line, "second=")) == nullptr || (second = GString::strtoi(line, &line, 10)) <= 0)
				continue;
			
			// How much the x position should be adjusted when drawing the second character immediately following the first.
			int amount;
			if((line = GString::strinext(line, "amount=")) == nullptr || (amount = GString::strtoi(line, &line, 10)) == 0)
				continue;
			
			// "first" and "second are converted into hash index values (which remain the same value for ASCII characters).
			// Since Unicode characters only use 21 bits of information, "first" and "second" are converted to 24-bit values.
			// This leaves 16-bits remaining for the actual kerning value.
			kerningsList.push_back(((uint64_t)ConvertUnicodeToHash((uint32_t)first) | ((uint64_t)ConvertUnicodeToHash((uint32_t)second) << 24) | ((uint64_t)amount << 48)));
		}
		
		// Advance to the end of the line
		while(line != nullptr && *line != '\0' && *line != '\n')
			line++;
	}
	
	// Copy the chars list to the resrouce chars
	if(!charsList.empty()) {
		charCount = (int32_t)charsList.size();
		chars = new Char[charCount];
		for(int i = 0; i < charCount; i++)
			chars[i] = charsList[i];
	}
	
	// Copy the hash list to the resource hash
	if(!hashList.empty()) {
		hashCount = (int32_t)hashList.size();
		hash = new uint32_t[hashCount];
		for(int i = 0; i < hashCount; i++)
			hash[i] = hashList[i];
	}
	
	// Copy the kerning list to the resource kernings
	if(!kerningsList.empty()) {
		kernCount = (int32_t)kerningsList.size();
		kernings = new uint64_t[kernCount];
		for(int i = 0; i < kernCount; i++)
			kernings[i] = kerningsList[i];
	}
	
	return true;
}



bool GFont::Resource::Write (const GString& name) {
	const int64_t resourceSize = sizeof(height) + sizeof(base) + sizeof(charCount) + sizeof(hashCount) + sizeof(kernCount) + sizeof(imageWidth) + sizeof(imageHeight) + sizeof(bufferSize) + charCount * sizeof(Char) + hashCount * sizeof(uint32_t) + kernCount * sizeof(uint64_t) + bufferSize * sizeof(uint8_t);
	std::unique_ptr<uint8_t[]> resourceBuffer(new uint8_t[resourceSize]);
	int64_t offset = 0;
	*((int16_t*)(resourceBuffer.get() + offset)) = height;
	offset += sizeof(height);
	*((int16_t*)(resourceBuffer.get() + offset)) = base;
	offset += sizeof(base);
	*((int32_t*)(resourceBuffer.get() + offset)) = charCount;
	offset += sizeof(charCount);
	*((int32_t*)(resourceBuffer.get() + offset)) = hashCount;
	offset += sizeof(hashCount);
	*((int32_t*)(resourceBuffer.get() + offset)) = kernCount;
	offset += sizeof(kernCount);
	*((int32_t*)(resourceBuffer.get() + offset)) = imageWidth;
	offset += sizeof(imageWidth);
	*((int32_t*)(resourceBuffer.get() + offset)) = imageHeight;
	offset += sizeof(imageHeight);
	*((int64_t*)(resourceBuffer.get() + offset)) = bufferSize;
	offset += sizeof(bufferSize);
	memcpy(resourceBuffer.get() + offset, chars, sizeof(Char) * charCount);
	offset += sizeof(Char) * charCount;
	memcpy(resourceBuffer.get() + offset, hash, sizeof(uint32_t) * hashCount);
	offset += sizeof(uint32_t) * hashCount;
	memcpy(resourceBuffer.get() + offset, kernings, sizeof(uint64_t) * kernCount);
	offset += sizeof(uint64_t) * kernCount;
	memcpy(resourceBuffer.get() + offset, buffer, sizeof(uint8_t) * bufferSize);
	return GSystem::ResourceWrite(name + ".fnt", resourceBuffer.get(), resourceSize);
}
