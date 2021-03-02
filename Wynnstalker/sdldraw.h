#ifndef _SDLDRAW_H_
#define _SDLDRAW_H_

#include <SDL.h>
#include <SDL_ttf.h>
#include "datafuncs.h"

void SetupRenderer(SDL_Renderer* gRenderer);
void FillCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius);
void DrawMenu(SDL_Renderer* gRenderer, TTF_Font *font, SDL_Rect *menuboxes, int width, int height, int playerCount);

void DrawWorlds(SDL_Renderer* gRenderer, TTF_Font* font, SDL_Rect backbox, SDL_Rect* sortboxes, int worldsortflag, worldstruct* worlds, int worldCount, int width, int height);
void DrawSingleWorld(SDL_Renderer* gRenderer, TTF_Font* font, worldstruct world, SDL_Rect worldRect);
void DrawInfo(SDL_Renderer* gRenderer, TTF_Font* font, SDL_Rect backbox, int width, int height);

#endif

