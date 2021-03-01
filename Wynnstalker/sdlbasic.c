#include "sdlbasic.h"
#include <stdio.h>

bool init(SDL_Window** gWindow, SDL_Renderer** gRenderer)
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 || TTF_Init() < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		*gWindow = SDL_CreateWindow("Wynnstalker", 500, 200, 0, 0, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (*gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			*gRenderer = SDL_CreateRenderer(*gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (*gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

bool GetInput(int *flag, SDL_Rect* menuboxes, SDL_Rect backbox)
{
	SDL_Event e;
	SDL_Point mouse;
	bool change = false;

	while (SDL_PollEvent(&e) != 0)
	{
		SDL_GetMouseState(&mouse.x, &mouse.y);

		if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
		{
			*flag = QUIT;
			change = true;
		}
		//Backbox
		if ((*flag == SEARCH_PLAYERS || *flag == SHOW_WORLDS || *flag == INFO) && (mouse.x > backbox.x && mouse.y > backbox.y && mouse.x < backbox.x + backbox.w && mouse.y < backbox.y + backbox.h) && (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT))
		{
			*flag = MENU;
			change = true;
		}
		if (((*flag == MENU && (mouse.x > menuboxes[3].x && mouse.y > menuboxes[3].y && mouse.x < menuboxes[3].x + menuboxes[3].w && mouse.y < menuboxes[3].y + menuboxes[3].h)) && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT))
		{
			*flag = QUIT;
			change = true;
		}
		if (e.button.button == SDL_BUTTON_LEFT && e.type == SDL_MOUSEBUTTONUP && *flag == MENU && (mouse.x > menuboxes[1].x && mouse.y > menuboxes[1].y && mouse.x < menuboxes[1].x + menuboxes[1].w && mouse.y < menuboxes[1].y + menuboxes[1].h))
		{
			*flag = SHOW_WORLDS;
			change = true;
		}
	}
	return change;
}

void closeSDL(SDL_Window** gWindow, SDL_Renderer** gRenderer, SDL_Surface** gSurface)
{
	//Destroy window	
	SDL_FreeSurface(*gSurface);
	SDL_DestroyRenderer(*gRenderer);
	SDL_DestroyWindow(*gWindow);
	*gWindow = NULL;
	*gRenderer = NULL;
	*gSurface = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	SDL_Quit();
}

