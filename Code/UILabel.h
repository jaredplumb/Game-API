#ifndef UI_LABEL_H_
#define UI_LABEL_H_

#include "GNode.h"
#include "GFont.h"
#include <string>

class UILabel : public GNode {
public:
	inline UILabel (): _font(nullptr) {}
	inline UILabel (const GString& text, int x, int y, GFont* font, GNode* parent): _text(text), _font(font) { if(font) SetRect(font->GetRect(text).Offset(x, y)); if(parent) parent->AddNode(*this); }
	inline void New (const GString& text, int x, int y, GFont* font, GNode* parent) { _text = text; _font = font; if(font) SetRect(font->GetRect(text).Offset(x, y)); if(parent) parent->AddNode(*this); }
	inline void SetText (const GString& text) { _text = text; }
	inline GString GetText () const { return _text.c_str(); }
	inline virtual void OnDraw () override { if(_font) _font->Draw(_text.c_str(), 0, 0); }
private:
	std::string _text;
	GFont* _font;
};

#endif // UI_LABEL_H_
