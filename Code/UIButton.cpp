#include "UIButton.h"



void UIButton::New (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click, GNode* parent) {
	_text = text;
	_isDown = false;
	_font = font;
	_button = button;
	_down = down;
	_click = click;
	if(_button)
		SetRect(GRect(x, y, _button->GetWidth(), _button->GetHeight()));
	if(parent)
		parent->AddNode(*this);
}



void UIButton::New (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click, GNode* parent) {
	_alloc = std::make_unique<Alloc>();
	if(!font.IsEmpty())
		_alloc->_font = std::make_unique<GFont>(font);
	if(!button.IsEmpty())
		_alloc->_button = std::make_unique<GImage>(button);
	if(!down.IsEmpty())
		_alloc->_down = std::make_unique<GImage>(down);
	if(!click.IsEmpty())
		_alloc->_click = std::make_unique<GSound>(click);
	return New(text, x, y, _alloc->_font.get(), _alloc->_button.get(), _alloc->_down.get(), _alloc->_click.get(), parent);
}



void UIButton::OnDraw () {
	if(_isDown && _down && _touch.x >= 0 && _touch.x < GetWidth() && _touch.y >= 0 && _touch.y < GetHeight())
		_down->Draw((GetWidth() - _down->GetWidth()) / 2, (GetHeight() - _down->GetHeight()) / 2);
	else if(_button)
		_button->Draw((GetWidth() - _button->GetWidth()) / 2, (GetHeight() - _button->GetHeight()) / 2);
	if(_font) {
		GRect rect = _font->GetRect(_text);
		rect.x = (GetWidth() - rect.width) / 2 - rect.x;
		rect.y = (GetHeight() - rect.height) / 2 - rect.y;
		_font->Draw(_text, rect.x, rect.y);
	}
}



void UIButton::OnTouch (int x, int y) {
	if(x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_isDown = true;
		_touch.x = x;
		_touch.y = y;
		if(_click)
			_click->Play();
	}
}



void UIButton::OnTouchUp (int x, int y) {
	if(_isDown && x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight()) {
		_touch.x = x;
		_touch.y = y;
		GNode* root = this;
		while(root->GetParent())
			root = root->GetParent();
		root->SendEvent(this);
	}
	_isDown = false;
}



void UIButton::OnTouchMove (int x, int y) {
	_touch.x = x;
	_touch.y = y;
}
