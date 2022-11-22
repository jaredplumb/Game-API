#include "UITransition.h"

static const int64_t _FADE_TIME = 250;

UITransition::UITransition ()
:	_transition(NONE)
,	_timer(0)
{
}

UITransition::UITransition (eTransition transition, GNode* parent)
:	_transition(NONE)
,	_timer(0)
{
	New(transition, parent);
}

UITransition::~UITransition () {
	Delete();
}

bool UITransition::New (eTransition transition, GNode* parent) {
	_transition = transition;
	switch(_transition) {
		case NONE:
			break;
		case FADE_BLACK:
			_image.New(GColor::BLACK);
			break;
		case FADE_IN_BLACK:
			_image.New(GColor::BLACK);
			break;
		case FADE_OUT_BLACK:
			_image.New(GColor::BLACK);
			break;
	}
	if(parent)
		parent->AddNode(*this);
	return true;
}

void UITransition::Delete () {
	_transition = NONE;
	_timer = 0;
	_image.Delete();
}

void UITransition::OnDraw () {
	switch(_transition) {
		case NONE:
			return;
		case FADE_BLACK:
			if(_timer == 0)
				_timer = GetMilliseconds();
			if(_timer > 0) {
				float alpha = 1.0f - (float)(GetMilliseconds() - _timer) / (float)_FADE_TIME;
				if(alpha < 0.0f) {
					_timer = 0;
					_transition = FADE_OUT_BLACK;
					return;
				}
				_image.Draw(GetScreenRect(), alpha);
			}
			break;
		case FADE_IN_BLACK:
			if(_timer == 0)
				_timer = GetMilliseconds();
			if(_timer > 0) {
				float alpha = 1.0f - (float)(GetMilliseconds() - _timer) / (float)_FADE_TIME;
				if(alpha < 0.0f) {
					_transition = NONE;
					return;
				}
				_image.Draw(GetScreenRect(), alpha);
			}
			break;
		case FADE_OUT_BLACK:
			if(_timer > 0) {
				float alpha = (float)(GetMilliseconds() - _timer) / (float)_FADE_TIME;
				if(alpha > 1.0f)
					alpha = 1.0f;
				_image.Draw(GetScreenRect(), alpha);
			}
			break;
	}
}

bool UITransition::OnExit () {
	switch(_transition) {
		case NONE:
			return true;
		case FADE_BLACK:
			// This transition is handled by FADE_OUT_BLACK
			break;
		case FADE_IN_BLACK:
			return true;
		case FADE_OUT_BLACK:
			if(_timer == 0)
				_timer = GetMilliseconds();
			return (GetMilliseconds() - _timer >= _FADE_TIME) ? true : false;
	}
	return true;
}
