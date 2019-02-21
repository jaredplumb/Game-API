#ifndef _GFONT_H_
#define _GFONT_H_

// NOTE: Add a feature where you can flatten a string into a single image using the font

#include "GTypes.h"

#include "GFontResource.h"
#include "GImage.h"


class GFont {
public:
	
	GFont ();
	~GFont ();
	GFont (const GFontResource& resource, const GString& name = NULL);
	GFont (const GString& resource);
	
	bool New (const GFontResource& resource, const GString& name = NULL);
	bool New (const GString& resource);
	void Delete ();
	
	int_t GetLineHeight () const; // Returns the full line used when rendering a font line
	int_t GetBaseHeight () const; // Returns the height from the top to the base, where the font characters sit, some characters will go below the base
	GRect GetRect (const GString& text) const; // Returns a rect containing the actual rendered pixels, x and y are offsets to the top left
	bool IsEmpty () const;
	
	void Draw (const GString& text, int_t x, int_t y, float alpha = 1.0f);
	
private:
	struct _PrivateData;
	_PrivateData* _data;
	static std::map<GString, GFont::_PrivateData*>* _FONTS;
	static _PrivateData* _FindData (const GString& key);
	static void _AddData (const GString& key, _PrivateData* data);
	static void _RemoveData (const GString& key);
};

#endif // _GFONT_H_
