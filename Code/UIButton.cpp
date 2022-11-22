#include "UIButton.h"

UIButton::UIButton ()
:	_isDown(false)
,	_font(NULL)
,	_button(NULL)
,	_down(NULL)
,	_click(NULL)
,	_alloc(NULL)
{
}

UIButton::UIButton (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click, GNode* parent)
:	_isDown(false)
,	_font(NULL)
,	_button(NULL)
,	_down(NULL)
,	_click(NULL)
,	_alloc(NULL)
{
	New(text, x, y, font, button, down, click, parent);
}

UIButton::UIButton (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click, GNode* parent)
:	UIButton(text, loc.x, loc.y, font, button, down, click, parent)
{
}

UIButton::UIButton (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click, GNode* parent)
:	_isDown(false)
,	_font(NULL)
,	_button(NULL)
,	_down(NULL)
,	_click(NULL)
,	_alloc(NULL)
{
	New(text, x, y, font, button, down, click, parent);
}

UIButton::UIButton (int x, int y, const GString& button, const GString& down, const GString& click, GNode* parent)
:	UIButton(NULL, x, y, NULL, button, down, click, parent)
{
}

UIButton::~UIButton () {
	Delete();
}

bool UIButton::New (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click, GNode* parent) {
	_text = text;
	_loc.x = 0;
	_loc.y = 0;
	_isDown = false;
	_font = font;
	_button = button;
	_down = down;
	_click = click;
	_alloc = nullptr;
	if(_button)
		SetRect(GRect(x, y, _button->GetWidth(), _button->GetHeight()));
	if(parent)
		parent->AddNode(*this);
	return true;
}

bool UIButton::New (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click, GNode* parent) {
	return New(text, loc.x, loc.y, font, button, down, click, parent);
}

bool UIButton::New (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click, GNode* parent) {
	if(_alloc)
		Delete();
	_alloc = new _Alloc;
	_alloc->_font = font.IsEmpty() ? nullptr : new GFont(font);
	_alloc->_button = button.IsEmpty() ? nullptr : new GImage(button);
	_alloc->_down = down.IsEmpty() ? nullptr : new GImage(down);
	_alloc->_click = click.IsEmpty() ? nullptr : new GSound(click);
	return New(text, x, y, _alloc->_font, _alloc->_button, _alloc->_down, _alloc->_click, parent);
}

bool UIButton::New (int x, int y, const GString& button, const GString& down, const GString& click, GNode* parent) {
	return New(NULL, x, y, NULL, button, down, click, parent);
}

void UIButton::Delete () {
	_text.Delete();
	_loc.x = 0;
	_loc.y = 0;
	_isDown = false;
	_font = nullptr;
	_button = nullptr;
	_down = nullptr;
	_click = nullptr;
	if(_alloc) {
		if(_alloc->_font)
			delete _alloc->_font;
		if(_alloc->_button)
			delete _alloc->_button;
		if(_alloc->_down)
			delete _alloc->_down;
		if(_alloc->_click)
			delete _alloc->_click;
		delete _alloc;
		_alloc = nullptr;
	}
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

void UIButton::OnTouch (int x, int y) {
	if(x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_loc.x = x;
		_loc.y = y;
		_isDown = true;
		if(_click)
			_click->Play();
	}
}

void UIButton::OnTouchUp (int x, int y) {
	if(_isDown == true && x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_loc.x = x;
		_loc.y = y;
		GNode* root = this;
		while(root->GetParent() != NULL)
			root = root->GetParent();
		root->SendEvent(this);
	}
	_isDown = false;
}

void UIButton::OnTouchMove (int x, int y) {
	_loc.x = x;
	_loc.y = y;
}
