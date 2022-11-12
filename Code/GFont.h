#ifndef _GFONT_H_
#define _GFONT_H_

// NOTE: Add a feature where you can flatten a string into a single image using the font

#include "GTypes.h"
#include "GImage.h"


class GFont {
public:
	class Resource;
	
	GFont ();
	GFont (const Resource& resource);
	GFont (const GString& resource);
	~GFont ();
	
	bool New (const Resource& resource);
	bool New (const GString& resource);
	void Delete ();
	
	/// Returns the full line used when rendering a font line
	int GetLineHeight () const;
	
	/// Returns the height from the top to the base, where the font characters sit, some characters will go below the base
	int GetBaseHeight () const;
	
	/// Returns a rect containing the actual rendered pixels, x and y are offsets to the top left
	GRect GetRect (const GString& text) const;
	
	/// Returns the width of the string
	inline int GetWidth (const GString& text) const { return GetRect(text).width; }
	
	bool IsEmpty () const;
	
	void Draw (const GString& text, int x, int y, float alpha = 1.0f);
	
	class Resource {
	public:
		struct Char {
			uint16_t	x;
			uint16_t	y;
			uint16_t	width;
			uint16_t	height;
			int16_t		xoffset;
			int16_t		yoffset;
			int16_t		xadvance;
		};
		
		uint16_t			height;
		uint16_t			base;
		uint32_t			charCount;
		uint32_t			hashCount;
		uint32_t			kernCount;
		Char*				chars;
		uint32_t*			hash;
		uint64_t*			kernings;
		GImage::Resource	image;			// The image data for this font
		
		Resource ();
		Resource (const GString& resource);
		~Resource ();
		bool New (const GString& resource);
		bool NewFromPackage (const GString& resource);
		bool NewFromFile (const GString& resource);
		void Delete ();
		bool WriteToPackage (GPackage& package, const GString& name);
	};
	
private:
	struct _PrivateData;
	_PrivateData* _data;
};

#endif // _GFONT_H_
