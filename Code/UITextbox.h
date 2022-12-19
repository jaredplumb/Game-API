#ifndef UI_TEXTBOX_H_
#define UI_TEXTBOX_H_

#include "GNode.h"
#include "GFont.h"

class UITextbox : public GNode {
public:
	
	enum eJustify { // Horizontal
		JUSTIFY_LEFT,
		JUSTIFY_RIGHT,
		JUSTIFY_CENTER,
	};
	
	enum eAlignment { // Vertical
		ALIGNMENT_TOP,
		ALIGNMENT_BOTTOM,
		ALIGNMENT_CENTER,
	};
	
	// A width of 0 will prevent wrapping except with new line characters
	inline UITextbox (): _font(nullptr), _justify(JUSTIFY_LEFT), _alignment(ALIGNMENT_TOP) {}
	inline UITextbox (const GRect& rect, GFont* font, GNode* parent, eJustify justify = JUSTIFY_LEFT, eAlignment alignment = ALIGNMENT_TOP): _font(font), _justify(justify), _alignment(alignment) { SetRect(rect); parent->AddNode(*this); }
	inline bool New (const GRect& rect, GFont* font, GNode* parent, eJustify justify = JUSTIFY_LEFT, eAlignment alignment = ALIGNMENT_TOP) { if(font && parent) { _font = font; _justify = justify; _alignment = alignment; _text.clear(); _lines.clear(); SetRect(rect); parent->AddNode(*this); return true; } return false; }
	inline void SetRect (const GRect& rect) { GNode::SetRect(rect); Update(); }
	inline void SetFont (GFont* font) { _font = font; Update(); }
	inline void SetJustify (eJustify justify) { _justify = justify; Update(); }
	inline void SetAlignment (eAlignment alignment) { _alignment = alignment; } // SetAlignment does not need an Update() because vertical alignment is calculated every frame
	inline void SetText (const GString& text) { _text = text; Update(); }
	void Update (); // If no text is set, then this function does nothing, causing SetRect, SetFont, and SetJustify to be very fast before SetText is called
	virtual void OnDraw () override;
	
private:
	GFont* _font;
	eJustify _justify;
	eAlignment _alignment;
	std::string _text;
	std::vector<std::pair<std::string, int>> _lines;
};

#endif // UI_TEXTBOX_H_
