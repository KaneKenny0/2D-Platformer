#pragma once
#include <SDL.h>
#include <iostream>

class sound
{
public:

	const char* filepath;

	sound(const char* _filePath);
	void play();
};

