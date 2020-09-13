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
	int_t GetLineHeight () const;
	
	/// Returns the height from the top to the base, where the font characters sit, some characters will go below the base
	int_t GetBaseHeight () const;
	
	/// Returns a rect containing the actual rendered pixels, x and y are offsets to the top left
	GRect GetRect (const GString& text) const;
	
	bool IsEmpty () const;
	
	void Draw (const GString& text, int_t x, int_t y, float alpha = 1.0f);
	
	class Resource {
	public:
		struct Char {
			uint16		x;
			uint16		y;
			uint16		width;
			uint16		height;
			int16		xoffset;
			int16		yoffset;
			int16		xadvance;
		};
		
		uint16				height;
		uint16				base;
		uint32				charCount;
		uint32				hashCount;
		uint32				kernCount;
		Char*				chars;
		uint32*				hash;
		uint64*				kernings;
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
