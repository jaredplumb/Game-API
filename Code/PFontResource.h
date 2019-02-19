#ifndef _P_FONT_RESOURCE_H_
#define _P_FONT_RESOURCE_H_

#include "PPlatform.h"
#include "PString.h"
#include "PPackage.h"
#include "PRect.h"


#include "PXML.h"
//#include "PArchive.h"

#include "PSystem.h"

//#include "PImageResource.h"
#include "PImage.h"

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
	PImage::Resource	image;			// The image data for this font
	
	PFontResource ();
	~PFontResource ();
	
	PFontResource (const PString& resource);
	
	bool New (const PString& resource);
	bool NewFromPackage (const PString& resource);
	bool NewFromFile (const PString& resource);
	
	
	void Delete ();
	
	bool WriteToPackage (PPackage& package, const PString& name);
	
	
};

#endif // _P_FONT_RESOURCE_H_
