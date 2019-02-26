#ifndef _UIBUTTON_H_
#define _UIBUTTON_H_

#include "UINode.h"
#include "GImage.h"
#include "GSound.h"

class UIButton : public UINode {
public:
	UIButton ();
	UIButton (int_t x, int_t y, const GString& button, const GString& down, const GString& click);
	UIButton (const GPoint& loc, const GString& button, const GString& down, const GString& click);
	virtual ~UIButton ();
	
	bool New (int_t x, int_t y, const GString& button, const GString& down, const GString& click);
	bool New (const GPoint& loc, const GString& button, const GString& down, const GString& click);
	void Delete ();
	
	bool IsDown () const;
	
	virtual void OnDraw () override;
	virtual void OnTouch (int_t x, int_t y) override;
	virtual void OnTouchUp (int_t x, int_t y) override;
	virtual void OnTouchMove (int_t x, int_t y) override;
	
private:
	GPoint		_touch;
	bool		_down;
	GImage		_imageButton;
	GImage		_imageDown;
	GSound		_soundClick;
};

#endif // _UIBUTTON_H_
