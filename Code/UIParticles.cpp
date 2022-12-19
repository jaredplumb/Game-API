#include "UIParticles.h"

void UIParticles::Add (GImage* image, int frames, int rate, int x1, int y1, int x2, int y2, int speed) {
	if(image == nullptr)
		return;
	Particle p;
	p.type = TYPE_IMAGE;
	p.image = image;
	p.frames = frames;
	p.rate = rate;
	p.x1 = x1;
	p.y1 = y1;
	p.x2 = x2;
	p.y2 = y2;
	p.speed = speed;
	p.timer = GNode::GetMilliseconds();
	_list.push_back(p);
}

void UIParticles::Add (const GString& text, GFont* font, int x1, int y1, int x2, int y2, int speed) {
	if(font == nullptr)
		return;
	Particle p;
	p.type = TYPE_FONT;
	p.text = text;
	p.font = font;
	p.x1 = x1;
	p.y1 = y1;
	p.x2 = x2;
	p.y2 = y2;
	p.speed = speed;
	p.timer = GNode::GetMilliseconds();
	_list.push_back(p);
}

void UIParticles::Draw () {
	std::list<Particle>::iterator i = _list.begin();
	while(i != _list.end()) {
		const int time = (int)(GNode::GetMilliseconds() - i->timer);
		if(i->speed > time) {
			const int x = i->x1 + (i->x2 - i->x1) * time / i->speed;
			const int y = i->y1 + (i->y2 - i->y1) * time / i->speed;
			const float alpha = (time <= (i->speed - FADE_TIME)) ? 1.0f : (1.0f - (float)(FADE_TIME - (i->speed - time)) / (float)FADE_TIME);
			switch(i->type) {
				case TYPE_UNKNOWN:
					break;
				case TYPE_IMAGE:
					if(i->frames <= 1) {
						i->image->Draw(x, y, alpha);
					} else {
						GRect srcRect = i->image->GetRect();
						srcRect.width /= i->frames;
						if(i->rate > 0)
							srcRect.x += srcRect.width * (time / i->rate % i->frames);
						i->image->Draw(srcRect, x, y, alpha);
					}
					break;
				case TYPE_FONT:
					i->font->Draw(i->text.c_str(), x, y, alpha);
					break;
			}
			i++;
		} else {
			i = _list.erase(i);
		}
	}
}
