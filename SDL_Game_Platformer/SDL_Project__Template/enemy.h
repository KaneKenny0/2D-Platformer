#pragma once
#include <SDL.h>
class enemy
{
private:


public:
	SDL_Rect pos;
	SDL_Rect source;
	int speed;
	bool patrolling;
	bool dirSwitch = true;



	enemy(SDL_Rect pos_, SDL_Rect source_, int speed_, bool patrolling_);

	void patrol(); 

};
