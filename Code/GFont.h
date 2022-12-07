#ifndef G_FONT_H_
#define G_FONT_H_

// NOTE: Add a feature where you can flatten a string into a single image using the font

#include "GTypes.h"
#include "GImage.h"
#include <vector>
#include <unordered_map>

class GFont {
public:
	class Resource;
	
	inline GFont (): _height(0), _base(0) {}
	inline GFont (const Resource& resource): _height(0), _base(0) { New(resource); }
	inline GFont (const GString& resource): _height(0), _base(0) { New(Resource(resource)); }
	inline bool New (const GString& resource) { return New(Resource(resource)); }
	bool New (const Resource& resource);
	
	/// Returns a rect containing the actual rendered pixels, x and y may not be 0 depending on how the text renders with offsets and kernings.
	GRect GetRect (const GString& text) const;
	
	inline int GetWidth (const GString& text) const { return GetRect(text).width; }
	inline int GetHeight (const GString& text) const { return GetRect(text).height; }
	
	/// This is the distance in pixels from the absolute top of the line to the next line.
	inline int GetLineHeight () const { return _height; }
	
	/// This is the  number of pixels from the absolute top of the line to the base of the characters (minus the spacing between lines).
	inline int GetBaseHeight () const { return _base; }
	
	inline bool IsEmpty () const { return _image.IsEmpty(); }
	
	void Draw (const GString& text, int x, int y, float alpha = 1.0f);
	
	struct Resource {
		struct Char {
			int16_t srcX;
			int16_t srcY;
			int16_t srcWidth;
			int16_t srcHeight;
			int16_t xOffset;
			int16_t yOffset;
			int16_t xAdvance;
			inline Char (): srcX(0), srcY(0), srcWidth(0), srcHeight(0), xOffset(0), yOffset(0), xAdvance(0) {}
		};
		int16_t height; // This is the distance in pixels from the absolute top of the line to the next line.
		int16_t base; // The number of pixels from the absolute top of the line to the base of the characters (minus the spacing between lines).
		int32_t charCount;
		int32_t hashCount;
		int32_t kernCount;
		int32_t imageWidth;
		int32_t imageHeight;
		int64_t bufferSize;
		Char* chars;
		uint32_t* hash;
		uint64_t* kernings;
		uint8_t* buffer;
		inline Resource (): height(0), base(0), charCount(0), hashCount(0), kernCount(0), imageWidth(0), imageHeight(0), bufferSize(0), chars(nullptr), hash(nullptr), kernings(nullptr), buffer(nullptr) {}
		inline Resource (const GString& name): height(0), base(0), charCount(0), hashCount(0), kernCount(0), imageWidth(0), imageHeight(0), bufferSize(0), chars(nullptr), hash(nullptr), kernings(nullptr), buffer(nullptr) { New(name); }
		inline ~Resource () { height = 0; base = 0; charCount = 0; hashCount = 0; kernCount = 0; imageWidth = 0; imageHeight = 0; bufferSize = 0; if(chars) delete [] chars; if(hash) delete [] hash; if(kernings) delete [] kernings; if(buffer) delete [] buffer; chars = nullptr; hash = nullptr; kernings = nullptr; buffer = nullptr; }
		bool New (const GString& name);
		bool NewFromFile (const GString& path);
		bool Write (const GString& name);
	};
	
private:
	int _height;
	int _base;
	std::vector<GRect> _rects;
	std::vector<GPoint> _offsets;
	std::vector<int> _advances;
	std::vector<bool> _has_kern;
	std::unordered_map<uint32_t, int> _hash;
	std::map<std::pair<int, int>, int> _kernings;
	GImage _image;
	
	inline int GetIndexFromHash (uint32_t hash) const { std::unordered_map<uint32_t, int>::const_iterator i = _hash.find(hash); return i != _hash.end() ? i->second : 0; }
};

#endif // G_FONT_H_
