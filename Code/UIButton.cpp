#include "UIButton.h"
#include "GSystem.h"

UIButton::UIButton ()
:	_touch(-1, -1)
,	_down(false)
{
}

UIButton::UIButton (int_t x, int_t y, const GString& button, const GString& down, const GString& click)
:	_touch(-1, -1)
,	_down(false)
,	_imageButton(button)
,	_imageDown(down)
,	_soundClick(click)
{
	SetRect(GRect(x, y, _imageButton.GetWidth(), _imageButton.GetHeight()));
}

UIButton::UIButton (const GPoint& loc, const GString& button, const GString& down, const GString& click)
:	_touch(-1, -1)
,	_down(false)
,	_imageButton(button)
,	_imageDown(down)
,	_soundClick(click)
{
	SetRect(GRect(loc.x, loc.y, _imageButton.GetWidth(), _imageButton.GetHeight()));
}

UIButton::~UIButton () {
	Delete();
}

bool UIButton::New (int_t x, int_t y, const GString& button, const GString& down, const GString& click) {
	_touch.x = -1;
	_touch.y = -1;
	_down = false;
	_imageButton.New(button);
	_imageDown.New(down);
	_soundClick.New(click);
	SetRect(GRect(x, y, _imageButton.GetWidth(), _imageButton.GetHeight()));
	return !_imageButton.IsEmpty();
}

bool UIButton::New (const GPoint& loc, const GString& button, const GString& down, const GString& click) {
	return New(loc.x, loc.y, button, down, click);
}

void UIButton::Delete () {
	_touch.x = -1;
	_touch.y = -1;
	_down = false;
	_imageButton.Delete();
	_imageDown.Delete();
	_soundClick.Delete();
}

bool UIButton::IsDown () const {
	return _down;
}

void UIButton::OnDraw () {
	if(_down && _touch.x >= 0 && _touch.x < GetWidth() && _touch.y >= 0 && _touch.y < GetHeight())
		_imageDown.Draw((GetWidth() - _imageDown.GetWidth()) / 2, (GetHeight() - _imageDown.GetHeight()) / 2);
	else
		_imageButton.Draw((GetWidth() - _imageButton.GetWidth()) / 2, (GetHeight() - _imageButton.GetHeight()) / 2);
}

void UIButton::OnTouch (int_t x, int_t y) {
	_touch.x = x;
	_touch.y = y;
	if(x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_soundClick.Play();
		_down = true;
		//_startingScreenLoc = GetScreenLoc();
	}
}

void UIButton::OnTouchUp (int_t x, int_t y) {
	_touch.x = x;
	_touch.y = y;
	if(_down && x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight())
		GSystem::RunEventCallbacks(GetUniqueRef(), this);
	_down = false;
}

void UIButton::OnTouchMove (int_t x, int_t y) {
	_touch.x = x;
	_touch.y = y;
	//if(_down && _startingScreenLoc != GetScreenLoc())
	//	_down = false;
}
