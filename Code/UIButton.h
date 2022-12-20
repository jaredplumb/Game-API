#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include <memory>
#include "GNode.h"
#include "GImage.h"
#include "GSound.h"
#include "GFont.h"

class UIButton : public GNode {
public:
	inline UIButton () : _isDown(false), _font(nullptr), _button(nullptr), _down(nullptr), _click(nullptr), _alloc(nullptr) {}
	inline UIButton (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click = nullptr, GNode* parent = nullptr) : _isDown(false), _font(nullptr), _button(nullptr), _down(nullptr), _click(nullptr), _alloc(nullptr) { New(text, x, y, font, button, down, click, parent); }
	inline UIButton (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = nullptr, GNode* parent = nullptr) : UIButton(text, loc.x, loc.y, font, button, down, click, parent) {}
	inline UIButton (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click = nullptr, GNode* parent = nullptr) : _isDown(false), _font(nullptr), _button(nullptr), _down(nullptr), _click(nullptr), _alloc(nullptr) { New(text, x, y, font, button, down, click, parent); }
	inline UIButton (int x, int y, const GString& button, const GString& down, const GString& click, GNode* parent) : UIButton(nullptr, x, y, nullptr, button, down, click, parent) {}
	inline UIButton (const GPoint& loc, const GString& button, const GString& down, const GString& click, GNode* parent) : UIButton(nullptr, loc.x, loc.y, nullptr, button, down, click, parent) {}
	
	void New (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click = nullptr, GNode* parent = nullptr);
	inline void New (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = nullptr, GNode* parent = nullptr) { return New(text, loc.x, loc.y, font, button, down, click, parent); }
	void New (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click = nullptr, GNode* parent = nullptr);
	inline void New (int x, int y, const GString& button, const GString& down, const GString& click = nullptr, GNode* parent = nullptr) { return New(nullptr, x, y, nullptr, button, down, click, parent); }
	
	inline bool IsDown () const { return _isDown; }
	
	virtual void OnDraw () override;
	virtual void OnTouch (int x, int y) override;
	virtual void OnTouchUp (int x, int y) override;
	virtual void OnTouchMove (int x, int y) override;
	
private:
	GString		_text;
	bool		_isDown;
	GPoint		_touch;
	GFont*		_font;
	GImage*		_button;
	GImage*		_down;
	GSound*		_click;
	
	struct Alloc {
		std::unique_ptr<GFont> _font;
		std::unique_ptr<GImage> _button;
		std::unique_ptr<GImage> _down;
		std::unique_ptr<GSound> _click;
	};
	std::unique_ptr<Alloc> _alloc;
};

#endif // UI_BUTTON_H_
