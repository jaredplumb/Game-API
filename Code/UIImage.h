#ifndef _UIIMAGE_H_
#define _UIIMAGE_H_

#include "UINode.h"
#include "GImage.h"

class UIImage : public UINode {
public:
	UIImage ();
	UIImage (GImage* image, int x, int y, UINode* parent = NULL);
	UIImage (GImage* image, const GRect& dst, UINode* parent = NULL);
	virtual ~UIImage ();
	
	bool New (GImage* image, int x, int y, UINode* parent = NULL);
	bool New (GImage* image, const GRect& dst, UINode* parent = NULL);
	void Delete ();
	
	virtual void OnDraw () override;
	
private:
	GImage*		_image;
};

#endif // _UIIMAGE_H_
