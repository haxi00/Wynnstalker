#ifndef _SDLBASIC_H_
#define _SDLBASIC_H_

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>

enum option {
	MENU,
	SEARCH_PLAYERS,
	SHOW_WORLDS,
	INFO,
	QUIT,
};

bool init(SDL_Window** gWindow, SDL_Renderer** gRenderer);
bool GetInput(int* flag, SDL_Rect* menuboxes, SDL_Rect backbox);
void closeSDL(SDL_Window** gWindow, SDL_Renderer** gRenderer, SDL_Surface** gSurface);

#endif

