#define _CRT_SECURE_NO_WARNINGS

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

bool GetInput(int* flag, int* worldsortFlag, bool* searchactive, bool* enterflag, SDL_Rect* menuboxes, SDL_Rect backbox, SDL_Rect* sortboxes, SDL_Rect textRect, char* playername)
{
	SDL_Event e;
	SDL_Point mouse;
	bool change = false;
	SDL_StartTextInput();
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
		//Search Players searchboxflag
		if (*flag == SEARCH_PLAYERS && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && mouse.x > textRect.x && mouse.y > textRect.y && mouse.x < textRect.x + textRect.w && mouse.y < textRect.y + textRect.h)
			*searchactive = true;
		else if (*flag != SEARCH_PLAYERS || (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && (mouse.x < textRect.x || mouse.y < textRect.y || mouse.x > textRect.x + textRect.w || mouse.y > textRect.y + textRect.h)))
			*searchactive = false;
		//Menuboxes
		if (((*flag == MENU && (mouse.x > menuboxes[3].x && mouse.y > menuboxes[3].y && mouse.x < menuboxes[3].x + menuboxes[3].w && mouse.y < menuboxes[3].y + menuboxes[3].h)) && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT))
		{
			*flag = QUIT;
			change = true;
		}
		if (e.button.button == SDL_BUTTON_LEFT && e.type == SDL_MOUSEBUTTONUP && *flag == MENU && (mouse.x > menuboxes[0].x && mouse.y > menuboxes[0].y && mouse.x < menuboxes[0].x + menuboxes[0].w && mouse.y < menuboxes[0].y + menuboxes[0].h))
		{
			*flag = SEARCH_PLAYERS;
			change = true;
		}
		if (e.button.button == SDL_BUTTON_LEFT && e.type == SDL_MOUSEBUTTONUP && *flag == MENU && (mouse.x > menuboxes[1].x && mouse.y > menuboxes[1].y && mouse.x < menuboxes[1].x + menuboxes[1].w && mouse.y < menuboxes[1].y + menuboxes[1].h))
		{
			*flag = SHOW_WORLDS;
			change = true;
		}
		//Show Worlds Sortboxes
		if (*flag == SHOW_WORLDS && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
		{
			if (*worldsortFlag != NUMBER && mouse.x > sortboxes[0].x && mouse.y > sortboxes[0].y && mouse.x < sortboxes[0].x + sortboxes[0].w && mouse.y < sortboxes[0].y + sortboxes[0].h)
				*worldsortFlag = NUMBER;
			if (*worldsortFlag != PLAYERS && mouse.x > sortboxes[1].x && mouse.y > sortboxes[1].y && mouse.x < sortboxes[1].x + sortboxes[1].w && mouse.y < sortboxes[1].y + sortboxes[1].h)
				*worldsortFlag = PLAYERS;
			if (*worldsortFlag != UPTIME && mouse.x > sortboxes[2].x && mouse.y > sortboxes[2].y && mouse.x < sortboxes[2].x + sortboxes[2].w && mouse.y < sortboxes[2].y + sortboxes[2].h)
				*worldsortFlag = UPTIME;
		}
		//Show Players textinput
		if (*flag == SEARCH_PLAYERS && *searchactive == true)
		{
			
			if (e.type == SDL_TEXTINPUT || e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL && (strlen(playername) + strlen(SDL_GetClipboardText())) <= 16)
					strcat(playername, SDL_GetClipboardText());
				else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE && strlen(playername) > 0)
					playername[strlen(playername) - 1] = 0;
				else if (e.type == SDL_TEXTINPUT && strlen(playername) < 16)
					strcat(playername, e.text.text);
			}
		}
		//Show Players textinput finished (hitting enter)
		if (*flag == SEARCH_PLAYERS && *searchactive == true && strlen(playername) > 2 && e.key.keysym.sym == SDLK_RETURN)
		{
			*searchactive = false;
			*enterflag = true;
		}
		else *enterflag = false;
	}
	SDL_StopTextInput();
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

