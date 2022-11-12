#ifndef _UILABEL_H_
#define _UILABEL_H_

#include "UINode.h"
#include "GFont.h"

class UILabel : public UINode {
public:
	UILabel ();
	UILabel (const GString& text, int x, int y, GFont* font, UINode* parent = NULL);
	virtual ~UILabel ();
	
	bool New (const GString& text, int x, int y, GFont* font, UINode* parent = NULL);
	void Delete ();
	
	void SetText (const GString& text);
	GString GetText () const;
	
	virtual void OnDraw () override;
	
private:
	GString		_text;
	GFont*		_font;
};

#endif // _UILABEL_H_
