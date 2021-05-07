#ifndef _SDLBASIC_H_
#define _SDLBASIC_H_

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "datafuncs.h"

enum option {
	MENU,
	SEARCH_PLAYERS,
	SHOW_WORLDS,
	INFO,
	QUIT,
};

bool init(SDL_Window** gWindow, SDL_Renderer** gRenderer);
bool GetInput(int* flag, int* worldsortFlag, bool* searchactive, bool* enterflag, SDL_Rect* menuboxes, SDL_Rect backbox, SDL_Rect* sortboxes, SDL_Rect textRect, char* playername);
void closeSDL(SDL_Window** gWindow, SDL_Renderer** gRenderer, SDL_Surface** gSurface);

#endif

