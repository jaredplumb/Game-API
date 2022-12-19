#include "UITextbox.h"

void UITextbox::Update () {
	if(_text.empty() || _font == nullptr)
		return;
	_lines.clear();
	
	for(int left = 0; left < _text.size(); left++) {
		int right = left;
		
		if(GetWidth() == 0)
			right = (int)_text.size();
		
		for(int scan = right; scan <= _text.size(); scan++) {
			if(_text[scan] == '\0' || std::isspace(_text[scan])) {
				if(right == left) {
					right = scan;
				} else {
					std::string line = _text.substr(left, scan - left);
					if(_font->GetWidth(line.c_str()) <= GetWidth()) {
						right = scan;
					} else {
						break;
					}
				}
				if(_text[scan] == '\0' || _text[scan] == '\n')
					break;
			}
		}
		
		std::string line = _text.substr(left, right - left);
		left = right;
		
		int x = 0; // JUSTIFY_LEFT
		if(_justify == JUSTIFY_RIGHT)
			x = GetWidth() - _font->GetWidth(line.c_str());
		if(_justify == JUSTIFY_CENTER)
			x = (GetWidth() - _font->GetWidth(line.c_str())) / 2;
		_lines.push_back(std::make_pair(line, x));
	}
}

void UITextbox::OnDraw () {
	if(_font && _lines.size() > 0) {
		int y = 0; // ALIGNMENT_TOP
		if(_alignment == ALIGNMENT_BOTTOM)
			y = GetHeight() - _font->GetBaseHeight() - (int)(_lines.size() - 1) * _font->GetLineHeight();
		if(_alignment == ALIGNMENT_CENTER)
			y = (GetHeight() - _font->GetBaseHeight() - (int)(_lines.size() - 1) * _font->GetLineHeight()) / 2;
		for(int i = 0; i < _lines.size(); i++)
			_font->Draw(_lines[i].first.c_str(), _lines[i].second, y + i * _font->GetLineHeight());
	}
}
