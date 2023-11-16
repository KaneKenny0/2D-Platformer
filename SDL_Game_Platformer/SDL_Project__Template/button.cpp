#include "button.h"
#include <SDL_ttf.h>
button::button(SDL_Renderer* _renderer, const char* _text, int _x, int _y, int _w, int _h)
    : renderer(_renderer), text(_text), x(_x), y(_y), w(_w), h(_h)
{
    //font = TTF_OpenFont("arial.ttf", 16);
        //create font with ttf

    if (TTF_Init() < 0) {
       // texture = createTextureFromText(text, font, renderer);
        
        SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 250, 250, 250, 10);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
    }
    else
    {
       // std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        TTF_Font* ChrustyRock = TTF_OpenFont("ChrustyRock-ORLA.ttf", 24);
        SDL_Color black = { 0, 0, 0 };
      
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(ChrustyRock, text, black);
        texture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    }
}

button::~button() {
   // TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
}

bool button::isClicked(int _x, int _y) {
    if (_x >= x && _x < x + w && _y >= y && _y < y + h) {
        return true;
    }
    else
    {
        return false;
    }
}

void button::render() {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderCopy(renderer, texture, NULL, &textRect);
}

