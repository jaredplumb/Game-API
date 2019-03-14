#include "UIImage.h"

UIImage::UIImage ()
:	_image(NULL)
{
}

UIImage::UIImage (GImage* image, int_t x, int_t y, UINode* parent)
:	_image(NULL)
{
	New(image, x, y, parent);
}

UIImage::UIImage (GImage* image, const GRect& dst, UINode* parent)
:	_image(NULL)
{
	New(image, dst, parent);
}

UIImage::~UIImage () {
	Delete();
}

bool UIImage::New (GImage* image, int_t x, int_t y, UINode* parent) {
	_image = image;
	if(_image)
		SetRect(_image->GetRect());
	if(parent)
		parent->Add(*this);
	return true;
}

bool UIImage::New (GImage* image, const GRect& dst, UINode* parent) {
	_image = image;
	if(_image)
		SetRect(dst);
	if(parent)
		parent->Add(*this);
	return true;
}

void UIImage::Delete () {
	_image = NULL;
}

void UIImage::OnDraw () {
	if(_image)
		_image->Draw();
}
