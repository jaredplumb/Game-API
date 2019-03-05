#include "UIButton.h"

UIButton::UIButton ()
:	_isDown(false)
,	_font(NULL)
,	_button(NULL)
,	_down(NULL)
,	_click(NULL)
{
}

UIButton::UIButton (const GString& text, int_t x, int_t y, GFont* font, GImage* button, GImage* down, GSound* click, UINode* parent)
:	_isDown(false)
,	_font(NULL)
,	_button(NULL)
,	_down(NULL)
,	_click(NULL)
{
	New(text, x, y, font, button, down, click, parent);
}

UIButton::UIButton (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click, UINode* parent)
:	UIButton(text, loc.x, loc.y, font, button, down, click, parent)
{
}

UIButton::UIButton (const GString& text, int_t x, int_t y, ButtonFactory& factory, UINode* parent)
:	UIButton(text, x, y, &factory.font, &factory.button, &factory.down, &factory.click, parent)
{
}

UIButton::UIButton (const GString& text, const GPoint& loc, ButtonFactory& factory, UINode* parent)
:	UIButton(text, loc.x, loc.y, &factory.font, &factory.button, &factory.down, &factory.click, parent)
{
}

UIButton::~UIButton () {
	Delete();
}

bool UIButton::New (const GString& text, int_t x, int_t y, GFont* font, GImage* button, GImage* down, GSound* click, UINode* parent) {
	_text = text;
	_loc.x = 0;
	_loc.y = 0;
	_isDown = false;
	_font = font;
	_button = button;
	_down = down;
	_click = click;
	if(_button)
		SetRect(GRect(x, y, _button->GetWidth(), _button->GetHeight()));
	if(parent)
		parent->Add(*this);
	return true;
}

bool UIButton::New (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click, UINode* parent) {
	return New(text, loc.x, loc.y, font, button, down, click, parent);
}

bool UIButton::New (const GString& text, int_t x, int_t y, ButtonFactory& factory, UINode* parent) {
	return New(text, x, y, &factory.font, &factory.button, &factory.down, &factory.click, parent);
}

bool UIButton::New (const GString& text, const GPoint& loc, ButtonFactory& factory, UINode* parent) {
	return New(text, loc.x, loc.y, &factory.font, &factory.button, &factory.down, &factory.click, parent);
}

void UIButton::Delete () {
	_text.Delete();
	_loc.x = 0;
	_loc.y = 0;
	_isDown = false;
	_font = NULL;
	_button = NULL;
	_down = NULL;
	_click = NULL;
}

bool UIButton::IsDown () const {
	return _isDown;
}

void UIButton::OnDraw () {
	if(_isDown == true && _down != NULL && _loc.x >= 0 && _loc.x < GetWidth() && _loc.y >= 0 && _loc.y < GetHeight())
		_down->Draw((GetWidth() - _down->GetWidth()) / 2, (GetHeight() - _down->GetHeight()) / 2);
	else if(_button != NULL)
		_button->Draw((GetWidth() - _button->GetWidth()) / 2, (GetHeight() - _button->GetHeight()) / 2);
	if(_font != NULL) {
		GRect rect = _font->GetRect(_text);
		rect.x = (GetWidth() - rect.width) / 2 - rect.x;
		rect.y = (GetHeight() - rect.height) / 2 - rect.y;
		_font->Draw(_text, rect.x, rect.y);
	}
}

void UIButton::OnTouch (int_t x, int_t y) {
	if(x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_loc.x = x;
		_loc.y = y;
		_isDown = true;
		if(_click)
			_click->Play();
	}
}

void UIButton::OnTouchUp (int_t x, int_t y) {
	if(_isDown == true && x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_loc.x = x;
		_loc.y = y;
		UINode* root = this;
		while(root->GetParent() != NULL)
			root = root->GetParent();
		root->SendEvent(this);
	}
	_isDown = false;
}

void UIButton::OnTouchMove (int_t x, int_t y) {
	_loc.x = x;
	_loc.y = y;
}

UIButton::ButtonFactory::ButtonFactory ()
{
}

UIButton::ButtonFactory::ButtonFactory (const GString& font_, const GString& button_, const GString& down_, const GString& click_)
:	font(font_)
,	button(button_)
,	down(down_)
,	click(click_)
{
}
