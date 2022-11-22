#ifndef _UILABEL_H_
#define _UILABEL_H_

#include "GNode.h"
#include "GFont.h"

class UILabel : public GNode {
public:
	UILabel ();
	UILabel (const GString& text, int x, int y, GFont* font, GNode* parent = NULL);
	virtual ~UILabel ();
	
	bool New (const GString& text, int x, int y, GFont* font, GNode* parent = NULL);
	void Delete ();
	
	void SetText (const GString& text);
	GString GetText () const;
	
	virtual void OnDraw () override;
	
private:
	GString		_text;
	GFont*		_font;
};

#endif // _UILABEL_H_
