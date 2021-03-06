#ifndef _UIBUTTON_H_
#define _UIBUTTON_H_

#include "UINode.h"
#include "GImage.h"
#include "GSound.h"
#include "GFont.h"

class UIButton : public UINode {
public:
	UIButton ();
	UIButton (const GString& text, int_t x, int_t y, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	UIButton (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	virtual ~UIButton ();
	
	bool New (const GString& text, int_t x, int_t y, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	bool New (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	void Delete ();
	
	bool IsDown () const;
	
	virtual void OnDraw () override;
	virtual void OnTouch (int_t x, int_t y) override;
	virtual void OnTouchUp (int_t x, int_t y) override;
	virtual void OnTouchMove (int_t x, int_t y) override;
	
private:
	GString		_text;
	GPoint		_loc;
	bool		_isDown;
	GFont*		_font;
	GImage*		_button;
	GImage*		_down;
	GSound*		_click;
};

#endif // _UIBUTTON_H_
