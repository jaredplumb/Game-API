#include "PFont.h"


struct PFont::_PrivateData {
	int_t refCount; // Used for the smart pointer code
	GString refName; // Used for the smart pointer code
	
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


std::map<GString, PFont::_PrivateData*>* PFont::_FONTS = NULL;




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
	GSystem::Debug("ERROR: Could not find font character (%u) in hash!\n", character);
	return 0;
}





PFont::PFont ()
:	_data(NULL)
{
}

PFont::~PFont () {
	Delete();
}


PFont::PFont (const PFontResource& resource, const GString& name)
:	_data(NULL){
	New(resource, name);
}

PFont::PFont (const GString& resource)
:	_data(NULL)
{
	New(resource);
}

bool PFont::New (const PFontResource& resource, const GString& name) {
	
	_data = _FindData(name);
	if(_data != NULL) {
		_data->refCount++;
		return true;
	}
	
	_data = new _PrivateData;
	_data->refCount = 1;
	_data->refName = name;
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
	
	
	
	//if(_data->image.New(resource.image, name + ".font") == false) {
	if(_data->image.New(resource.image) == false) {
		GSystem::Debug("ERROR: Could not create image with name \"%s\"!\n", (const char*)(name + ".font"));
		return false;
	}
	
	_AddData(name, _data);
	
	return true;
}

bool PFont::New (const GString& resource) {
	return New(PFontResource(resource), resource);
}

void PFont::Delete () {
	if(_data == NULL)
		return;
	
	_data->refCount--;
	if(_data->refCount > 0) {
		_data = NULL;
		return;
	}
	
	_RemoveData(_data->refName);
	
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



int_t PFont::GetLineHeight () const {
	return (_data ? _data->height : 0);
}

int_t PFont::GetBaseHeight () const {
	return (_data ? _data->base : 0);
}

GRect PFont::GetRect (const GString& text) const {
	
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
				GSystem::Debug("ERROR: Unknown character (%x) found while drawing font!\n", (uint8)text[i]);
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
				GSystem::Debug("ERROR: Index not found while drawing font!\n");
			}
			
			last = index;
		}
	}
	
	return GRect(x1, y1, x2 - x1, y2 - y1);
}


bool PFont::IsEmpty () const {
	return _data == NULL || _data->image.IsEmpty();
}


void PFont::Draw (const GString& text, int_t x, int_t y, float alpha) {
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
				GSystem::Debug("ERROR: Unknown character (%x) found while drawing font!\n", (uint8)text[i]);
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
				GSystem::Debug("ERROR: Index not found while drawing font!\n");
			}
			
			last = index;
		}
	}
}

PFont::_PrivateData* PFont::_FindData (const GString& key) {
	if(_FONTS) {
		std::map<GString, PFont::_PrivateData*>::iterator i = _FONTS->find(key);
		if(i != _FONTS->end()) {
			return i->second;
		}
	}
	return NULL;
}

void PFont::_AddData (const GString& key, _PrivateData* data) {
	if(_FONTS == NULL)
		_FONTS = new std::map<GString, PFont::_PrivateData*>;
	_FONTS->insert(std::make_pair(key, data));
}

void PFont::_RemoveData (const GString& key) {
	if(_FONTS != NULL) {
		_FONTS->erase(_FONTS->find(key));
		if(_FONTS->empty()) {
			delete _FONTS;
			_FONTS = NULL;
		}
	}
}
