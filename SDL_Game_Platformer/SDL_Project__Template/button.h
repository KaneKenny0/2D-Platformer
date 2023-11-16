#pragma once
#include <SDL.h>
//#include <SDL_ttf.h>
#include <iostream>
class button {
public:

    SDL_Renderer* renderer;
    const char* text;
    int x;
    int y;
    int w;
    int h;
    SDL_Rect rect{ x, y, w, h};
   // TTF_Font* font;
    SDL_Texture* texture;
    SDL_Rect textRect{ x + 10, y + 10, w - 20, h - 20 };

    button(SDL_Renderer* _renderer, const char* text, int _x, int _y, int _w, int _h);

    ~button();

    bool isClicked(int _x, int _y);

    void render(); 

};