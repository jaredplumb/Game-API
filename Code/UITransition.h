#ifndef _UITRANSITION_H_
#define _UITRANSITION_H_

#include "UINode.h"
#include "GImage.h"

// NOTE: To allow the fade to completely cover all UI elements, the UITransition object must
// added last after all other UI objects such as UIButton and UILabel.  This is done on purpose
// to allow custom UITransition that might only fade parts of the screen.

class UITransition : public UINode {
public:
	enum eTransition {
		NONE,
		FADE_BLACK,			// 250 ms fade
		FADE_IN_BLACK,		// 250 ms fade
		FADE_OUT_BLACK,		// 250 ms fade
	};
	
	UITransition ();
	UITransition (eTransition transition, UINode* parent = NULL);
	virtual ~UITransition ();
	
	bool New (eTransition transition, UINode* parent = NULL);
	void Delete ();
	
	virtual void OnDraw () override;
	virtual bool OnExit () override;
	
private:
	eTransition		_transition;
	uint64			_timer;
	GImage			_image;
};

#endif // _UITRANSITION_H_
