#include "UILabel.h"

UILabel::UILabel ()
:	_font(NULL)
{
}

UILabel::UILabel (const GString& text, int_t x, int_t y, GFont* font, UINode* parent)
:	_font(NULL)
{
	New(text, x, y, font, parent);
}

UILabel::~UILabel () {
	Delete();
}

bool UILabel::New (const GString& text, int_t x, int_t y, GFont* font, UINode* parent) {
	_text = text;
	_font = font;
	if(_font)
		SetRect(_font->GetRect(_text).Offset(x, y));
	if(parent)
		parent->Add(*this);
	return true;
}

void UILabel::Delete () {
	_text.Delete();
	_font = NULL;
}

void UILabel::SetText (const GString& text) {
	_text = text;
}

GString UILabel::GetText () const {
	return _text;
}

void UILabel::OnDraw () {
	if(_font)
		_font->Draw(_text, 0, 0);
}
