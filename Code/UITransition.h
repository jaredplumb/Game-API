#ifndef _UITRANSITION_H_
#define _UITRANSITION_H_

#include "UINode.h"
#include "GImage.h"

class UITransition : public UINode {
public:
	enum eTransition {
		TRANSITION_NODE,
		TRANSITION_FADE_BLACK,
		TRANSITION_FADE_IN_BLACK,
		TRANSITION_FADE_OUT_BLACK,
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
