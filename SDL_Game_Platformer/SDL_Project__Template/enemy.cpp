#include "enemy.h"

enemy::enemy(SDL_Rect pos_, SDL_Rect source_, int speed_, bool patrolling_)
{
	pos = pos_;
	source = source_;
	speed = speed_;
	patrolling = patrolling_;
}

void enemy::patrol() {
	if (patrolling)
	{
		if (dirSwitch)
		{
			pos.x += speed;
		}
		else
		{
			pos.x -= speed;
		}
	}
}