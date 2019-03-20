#include "UITransition.h"

UITransition::UITransition ()
:	_transition(TRANSITION_NODE)
,	_timer(0)
{
}

UITransition::UITransition (eTransition transition, UINode* parent)
:	_transition(TRANSITION_NODE)
,	_timer(0)
{
	New(transition, parent);
}

UITransition::~UITransition () {
	Delete();
}

bool UITransition::New (eTransition transition, UINode* parent) {
	_transition = transition;
	switch(_transition) {
		case TRANSITION_NODE:
			break;
		case TRANSITION_FADE_IN_BLACK:
			_image.New(GColor::BLACK);
			break;
		case TRANSITION_FADE_OUT_BLACK:
			_image.New(GColor::BLACK);
			break;
	}
	if(parent)
		parent->Add(*this);
	return true;
}

void UITransition::Delete () {
	_transition = TRANSITION_NODE;
	_timer = 0;
	_image.Delete();
}

void UITransition::OnDraw () {
	switch(_transition) {
		case TRANSITION_NODE:
			return;
		case TRANSITION_FADE_IN_BLACK:
			if(_timer == 0)
				_timer = GetMilliseconds();
			if(_timer > 0) {
				float alpha = 1.0f - (float)(GetMilliseconds() - _timer) / 250.0f;
				if(alpha < 0.0f) {
					_transition = TRANSITION_NODE;
					return;
				}
				_image.Draw(GetScreenRect(), alpha);
			}
			break;
		case TRANSITION_FADE_OUT_BLACK:
			if(_timer > 0) {
				float alpha = (float)(GetMilliseconds() - _timer) / 250.0f;
				if(alpha > 1.0f)
					alpha = 1.0f;
				_image.Draw(GetScreenRect(), alpha);
			}
			break;
	}
}

bool UITransition::OnExit () {
	switch(_transition) {
		case TRANSITION_NODE:
			return true;
		case TRANSITION_FADE_IN_BLACK:
			return true;
		case TRANSITION_FADE_OUT_BLACK:
			if(_timer == 0)
				_timer = GetMilliseconds();
			return (GetMilliseconds() - _timer >= 250) ? true : false;
	}
	return true;
}
