#ifndef _P_FONT_RESOURCE_H_
#define _P_FONT_RESOURCE_H_

#include "GTypes.h"
#include "PPackage.h"


#include "PXML.h"
//#include "PArchive.h"

#include "GSystem.h"

//#include "PImageResource.h"
#include "GImage.h"

#include "PFile.h"


class PFontResource {
public:
	struct Char {
		uint16 x;
		uint16 y;
		uint16 width;
		uint16 height;
		int16 xoffset;
		int16 yoffset;
		int16 xadvance;
	};
	
	uint16			height;
	uint16			base;
	uint32			charCount;
	uint32			hashCount;
	uint32			kernCount;
	Char*			chars;
	uint32*			hash;
	uint64*			kernings;
	GImage::Resource	image;			// The image data for this font
	
	PFontResource ();
	~PFontResource ();
	
	PFontResource (const GString& resource);
	
	bool New (const GString& resource);
	bool NewFromPackage (const GString& resource);
	bool NewFromFile (const GString& resource);
	
	
	void Delete ();
	
	bool WriteToPackage (PPackage& package, const GString& name);
	
	
};

#endif // _P_FONT_RESOURCE_H_
