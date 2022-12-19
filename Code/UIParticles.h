#ifndef UI_PARTICLES_H_
#define UI_PARTICLES_H_

#include "Game.h"
#include <string>
#include <list>

class UIParticles {
public:
	static constexpr int64_t FADE_TIME = 200;
	
	void Add (GImage* picture, int frames, int rate, int x1, int y1, int x2, int y2, int speed);
	void Add (const GString& text, GFont* font, int x1, int y1, int x2, int y2, int speed);
	void Draw ();
	
private:
	enum eType {
		TYPE_UNKNOWN = 0,
		TYPE_IMAGE,
		TYPE_FONT,
	};
	
	struct Particle {
		eType type;
		
		// TYPE_IMAGE
		GImage* image;
		int frames;
		int rate;
		
		// TYPE_FONT
		std::string text;
		GFont* font;
		
		// Common
		int x1, y1;
		int x2, y2;
		int speed;
		int64_t timer;
	};
	
	std::list<Particle> _list;
};

#endif // UI_PARTICLES_H_
