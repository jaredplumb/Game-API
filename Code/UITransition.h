#ifndef UI_TRANSITION_H_
#define UI_TRANSITION_H_

#include "GNode.h"
#include "GImage.h"

class UITransition : public GNode {
public:
	static constexpr int64_t FADE_TIME = 250;
	
	enum eTransition {
		NONE,
		FADE_BLACK,			// 250 ms fade
		FADE_IN_BLACK,		// 250 ms fade
		FADE_OUT_BLACK,		// 250 ms fade
	};
	
	inline UITransition (): _transition(NONE), _timer(0) {}
	inline UITransition (eTransition transition, GNode* parent = nullptr): _transition(NONE), _timer(0) { New(transition, parent); }
	void New (eTransition transition, GNode* parent = nullptr);
	virtual void OnDraw () override;
	virtual bool OnExit () override;
	
private:
	eTransition _transition;
	int64_t _timer;
	GImage _image;
};

#endif // UI_TRANSITION_H_
