#define _CRT_SECURE_NO_WARNINGS

#include "sdldraw.h"

void SetupRenderer(SDL_Renderer* gRenderer)
{
	SDL_Color black = { 0, 0, 0, 255 };

	//Clear screen into a blackscreen
	SDL_SetRenderDrawColor(gRenderer, black.r, black.g, black.b, black.a);
	SDL_RenderClear(gRenderer);

	//Acticate the alphachannel (used in FillButton)
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

	return;
}

void FillCircle(SDL_Renderer* gRenderer, int32_t centreX, int32_t centreY, int32_t radius)
{
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		SDL_RenderDrawLine(gRenderer, centreX + x, centreY - y, centreX + x, centreY + y);
		SDL_RenderDrawLine(gRenderer, centreX - x, centreY - y, centreX - x, centreY + y);
		SDL_RenderDrawLine(gRenderer, centreX + y, centreY - x, centreX + y, centreY + x);
		SDL_RenderDrawLine(gRenderer, centreX - y, centreY - x, centreX - y, centreY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

void DrawMenu(SDL_Renderer* gRenderer, TTF_Font* font, SDL_Rect* menuboxes, int width, int height, int playerCount)
{
	//Draw Online-counter top left
	SDL_Surface* messageSurface = NULL;
	SDL_Texture* message = NULL;
	SDL_Point mouse;
	SDL_Rect programName = { width / 100, 0, width / 8, height / 20 };
	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color red = { 255, 0, 0, 255 };
	char buffer[15];
	SDL_GetMouseState(&mouse.x, &mouse.y);
	sprintf(buffer, "Online: %d", playerCount);
	messageSurface = TTF_RenderText_Blended(font, buffer, white);
	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &programName);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);


	//Draw Buttons
	//Button "Search players"
	if(mouse.x > menuboxes[0].x && mouse.y > menuboxes[0].y && mouse.x < menuboxes[0].x + menuboxes[0].w && mouse.y < menuboxes[0].y + menuboxes[0].h)
	{
		SDL_SetRenderDrawColor(gRenderer, white.r, white.g, white.b, white.a - 150);
		SDL_RenderFillRect(gRenderer, &menuboxes[0]);
	}

	sprintf(buffer, "Search players");
	messageSurface = TTF_RenderText_Blended(font, buffer, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &menuboxes[0]);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	//Button "Show worlds"
	if (mouse.x > menuboxes[1].x && mouse.y > menuboxes[1].y && mouse.x < menuboxes[1].x + menuboxes[1].w && mouse.y < menuboxes[1].y + menuboxes[1].h)
	{
		SDL_SetRenderDrawColor(gRenderer, white.r, white.g, white.b, white.a - 150);
		SDL_RenderFillRect(gRenderer, &menuboxes[1]);
	}

	sprintf(buffer, "Show worlds");
	messageSurface = TTF_RenderText_Blended(font, buffer, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &menuboxes[1]);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	//Button "Info"
	if (mouse.x > menuboxes[2].x && mouse.y > menuboxes[2].y && mouse.x < menuboxes[2].x + menuboxes[2].w && mouse.y < menuboxes[2].y + menuboxes[2].h)
	{
		SDL_SetRenderDrawColor(gRenderer, white.r, white.g, white.b, white.a - 150);
		SDL_RenderFillRect(gRenderer, &menuboxes[2]);
	}

	sprintf(buffer, "Info");
	messageSurface = TTF_RenderText_Blended(font, buffer, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &menuboxes[2]);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	//Button "Quit"
	if (mouse.x > menuboxes[3].x && mouse.y > menuboxes[3].y && mouse.x < menuboxes[3].x + menuboxes[3].w && mouse.y < menuboxes[3].y + menuboxes[3].h)
	{
		SDL_SetRenderDrawColor(gRenderer, red.r, red.g, red.b, red.a - 150);
		SDL_RenderFillRect(gRenderer, &menuboxes[3]);
	}

	sprintf(buffer, "Quit");
	messageSurface = TTF_RenderText_Blended(font, buffer, red);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &menuboxes[3]);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	return;
}

void DrawWorlds(SDL_Renderer* gRenderer, TTF_Font* font, SDL_Rect backbox, int width, int height, worldstruct* worlds, int worldCount)
{
	int rows, columns, widthUnit, heightUnit, counter = 0;
	SDL_Rect tempWorld;

	if (worldCount % (worldCount/10+3) != 0)
		rows = worldCount / (worldCount / 10 + 3) + 1;
	else rows = worldCount / (worldCount / 10 + 3);

	if (worldCount % rows != 0)
		columns = worldCount / rows + 1;
	else columns = worldCount / rows;

	widthUnit = width / columns;
	heightUnit = height / rows;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			tempWorld.x = j * (widthUnit/2 + widthUnit/20);
			tempWorld.y = i * (heightUnit-heightUnit/3) - heightUnit/50;
			tempWorld.w = widthUnit;
			tempWorld.h = heightUnit;
			if (worlds[counter].playercount < MAXWORLDPLAYERS && worlds[counter].playercount >= 0)
				DrawSingleWorld(gRenderer, font, worlds[counter], tempWorld);
			counter++;
		}
	}
}

void DrawSingleWorld(SDL_Renderer* gRenderer, TTF_Font* font, worldstruct world, SDL_Rect worldRect)
{
	char buffer[30];
	int radius = worldRect.w < worldRect.h ? worldRect.w / 3: worldRect.h / 3, hours, minutes;
	double rest;
	SDL_Surface* messageSurface = NULL;
	SDL_Texture* message = NULL;
	SDL_Rect playercountRect = { (worldRect.x + (worldRect.x + worldRect.w) / 2) - radius/2, (worldRect.y + (worldRect.y + worldRect.h) / 2), radius, radius };
	SDL_Rect uptimeRect = { (worldRect.x + (worldRect.x + worldRect.w) / 2) - 2*radius, \
							((worldRect.y + (worldRect.y + worldRect.h) / 2) - radius / 2) + 2 * radius - radius / 2, 4 * radius, radius - radius/4 };
	SDL_Rect worldnameRect = { playercountRect.x, playercountRect.y - radius/2, radius, radius - radius / 4 };
	SDL_Color green = { 46, 204, 113, 255 }, orange = { 248, 148, 6, 255 }, red = { 207, 0, 15, 255 }, white = { 255, 255, 255, 255 };

	//Worlds playercount
	if (world.playercount > MAXWORLDPLAYERS - 6)
		SDL_SetRenderDrawColor(gRenderer, red.r, red.g, red.b, red.a);
	else if (world.playercount > MAXWORLDPLAYERS - 11)
		SDL_SetRenderDrawColor(gRenderer, orange.r, orange.g, orange.b, orange.a);
	else SDL_SetRenderDrawColor(gRenderer, green.r, green.g, green.b, green.a);

	FillCircle(gRenderer, worldRect.x + (worldRect.x + worldRect.w) / 2, worldRect.y + (worldRect.y + worldRect.h) / 2, radius);

	sprintf(buffer, "%d/%d", world.playercount, MAXWORLDPLAYERS - 5);
	messageSurface = TTF_RenderText_Blended(font, buffer, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &playercountRect);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	//Worlds uptime
	hours = world.uptime / 3600;
	rest = (double) world.uptime / 3600 - hours;
	minutes = rest * 60;

	if(hours == 0)
		sprintf(buffer, "Uptime: %d mins", minutes);
	else
		sprintf(buffer, "Uptime: %d hours %d mins", hours, minutes);
	messageSurface = TTF_RenderText_Blended(font, buffer, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &uptimeRect);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);

	//Worlds name
	messageSurface = TTF_RenderText_Blended(font, world.name, white);

	message = SDL_CreateTextureFromSurface(gRenderer, messageSurface);
	SDL_RenderCopy(gRenderer, message, NULL, &worldnameRect);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(message);
}

void DrawInfo(SDL_Renderer* gRenderer, TTF_Font* font, SDL_Rect backbox, int width, int height)
{

}