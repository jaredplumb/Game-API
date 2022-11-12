#ifndef _UIBUTTON_H_
#define _UIBUTTON_H_

#include "UINode.h"
#include "GImage.h"
#include "GSound.h"
#include "GFont.h"

class UIButton : public UINode {
public:
	UIButton ();
	UIButton (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	UIButton (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	UIButton (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click = NULL, UINode* parent = NULL);
	UIButton (int x, int y, const GString& button, const GString& down, const GString& click = NULL, UINode* parent = NULL);
	virtual ~UIButton ();
	
	bool New (const GString& text, int x, int y, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	bool New (const GString& text, const GPoint& loc, GFont* font, GImage* button, GImage* down, GSound* click = NULL, UINode* parent = NULL);
	bool New (const GString& text, int x, int y, const GString& font, const GString& button, const GString& down, const GString& click = NULL, UINode* parent = NULL);
	bool New (int x, int y, const GString& button, const GString& down, const GString& click = NULL, UINode* parent = NULL);
	void Delete ();
	
	bool IsDown () const;
	
	virtual void OnDraw () override;
	virtual void OnTouch (int x, int y) override;
	virtual void OnTouchUp (int x, int y) override;
	virtual void OnTouchMove (int x, int y) override;
	
private:
	GString		_text;
	GPoint		_loc;
	bool		_isDown;
	GFont*		_font;
	GImage*		_button;
	GImage*		_down;
	GSound*		_click;
	
	// This portion of data is only used when an object is created using string references and is auto deleted when the object is destroyed
	struct _Alloc {
		GFont*		_font;
		GImage*		_button;
		GImage*		_down;
		GSound*		_click;
	};
	_Alloc*		_alloc;
};

#endif // _UIBUTTON_H_
