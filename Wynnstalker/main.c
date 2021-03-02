/*Wynnstalker by haxi00*/

#define CURL_STATICLIB
#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <SDL_image.h>
#include "datafuncs.h"
#include "sdlbasic.h"
#include "sdldraw.h"

#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "libcurl_a_debug.lib")

//Global variables
const char playerSum[] = "https://api.wynncraft.com/public_api.php?action=onlinePlayersSum";
const char serverList[] = "https://athena.wynntils.com/cache/get/serverList";
const char playerSearch[] = "https://api.wynncraft.com/public_api.php?action=statsSearch&search=";


int main(int argc, char* args[])
{
    //Variables for measuring time
    clock_t timer = clock();
    double time = timer / CLOCKS_PER_SEC;

    //SDL init and required variables
    SDL_Window* gWindow = NULL;
    SDL_Renderer* gRenderer = NULL;
    SDL_Surface* gSurface = NULL;
    if (!init(&gWindow, &gRenderer))
    {
        printf("Failed to initialize SDL!\n");
        Sleep(3000);
        exit(EXIT_FAILURE);
    }
    gSurface = IMG_Load("icon.png");
    int width, height;
    //SDL_GL_GetDrawableSize(gWindow, &width, &height);
    width = 1280, height = 720;
    SDL_SetWindowSize(gWindow, width, height);
    SDL_SetWindowIcon(gWindow, gSurface);
    TTF_Font* font = TTF_OpenFont("OpenSans-Bold.ttf", 100);
    if (font == NULL)
    {
        printf("Font \"OpenSans - Bold.ttf\" probably missing in data files\n");
        Sleep(3000);
        exit(EXIT_FAILURE);
    }
    SDL_Rect backbox = { width - width / 8, height / 50,  width / 10, height / 10 };
    SDL_Rect menuboxes[4] = { { width / 2 - width / 4, height / 30, width / 2, height / 5 },
                               { width / 2 - width / 4, 2 * height / 30 + height / 5, width / 2, height / 5 },
                               { width / 2 - width / 12, 3 * height / 30 + 2 * height / 5, 2 * width / 12, height / 5 },
                               { width / 2 - width / 12, 4 * height / 30 + 3 * height / 5, 2 * width / 12, height / 5 } };
    SDL_Rect sortboxes[3] = { { width - width / 8, height - height / 3, width / 10, height / 10},
                               { width - width / 8, height - height / 3 + height / 10, width / 10, height / 10},
                               { width - width / 8, height - height / 3 + 2 * height / 10, width / 10, height / 10} };

    //Initializations of variables and Curl
    CURL* curl = curl_easy_init();
    jsmn_parser parser;
    jsmntok_t* tokens = NULL;
    int flag = MENU, worldsortflag = NUMBER, tokenCount, playerCount = 0, worldCount = 0;
    bool flagChanged = false;
    char* stringAPI = NULL;
    worldstruct* worlds = NULL;  //using malloc later because its too big for stack

    //Main loop
    while (flag != QUIT)
    {
        flagChanged = GetInput(&flag, &worldsortflag, menuboxes, backbox, sortboxes);
        if (flagChanged)
        {
            if (tokens != NULL) { free(tokens); tokens = NULL; }
            if (worlds != NULL) { free(worlds); worlds = NULL; }  
        }

        if (flag == MENU)
        {
            timer = clock();
            
            //Get data from API every ~3 seconds
            if (timer / CLOCKS_PER_SEC > time || flagChanged)
            {
                //Write Playersum in string
                if (!WriteDataInString(curl, playerSum, &stringAPI))
                    exit(EXIT_FAILURE);
                
                //Read Playersum from string and print it
                jsmn_init(&parser);
                playerCount = GetPlayercount(&parser, stringAPI);
                if (playerCount < 0)
                {
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }
                printf("Players online: %d\n", playerCount);

                //Force next update to take ~3 seconds
                if(!flagChanged)
                    time += 3;
            }
            
            //Draw
            SetupRenderer(gRenderer);
            DrawMenu(gRenderer, font, menuboxes, width, height, playerCount);
        }

        if (flag == SEARCH_PLAYERS)
        {
            timer = clock();

            //Get data from API every ~3 seconds
            if (timer / CLOCKS_PER_SEC > time || flagChanged)
            {
                //Cleaning up if used before
                if (tokens != NULL) { free(tokens); tokens = NULL; }
                if (worlds != NULL) { free(worlds); worlds = NULL; }

                //Allocate worlds
                worlds = malloc(sizeof(worldstruct) * 100);
                if (worlds == NULL)
                {
                    printf("malloc returned NULL for worldstruct (run out of RAM?)\n");
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Write Serverlist in string
                if (!WriteDataInString(curl, serverList, &stringAPI))
                {
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Generate parser tokens from string (new jsmn_init to reset used parser)
                tokenCount = strlen(stringAPI) / 10;
                tokens = malloc(sizeof(jsmntok_t) * tokenCount);
                jsmn_init(&parser);
                if (!GetTokens(&parser, tokens, tokenCount, stringAPI))
                {
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Fill worldstructs with data
                worldCount = GetWorlds(tokens, tokenCount, stringAPI, worlds, worldsortflag);

                //Playersearch and print result
                CheckPlayer(curl, worlds, worldCount, playerSearch);

                //Force next update to take ~3 seconds
                if (!flagChanged)
                    time += 3;
            }

            //Draw
            //SetupRenderer(gRenderer);
            //if (worlds != NULL)
                //DrawSearchPlayers(gRenderer, font, backbox, width, height,);
        }

        if (flag == SHOW_WORLDS)
        {
            timer = clock();

            //Get data from API every ~3 seconds
            if (timer / CLOCKS_PER_SEC > time || flagChanged)
            {
                //Cleaning up if used before
                if (tokens != NULL) { free(tokens); tokens = NULL; }
                if (worlds != NULL) { free(worlds); worlds = NULL; }

                //Allocate worlds
                worlds = malloc(sizeof(worldstruct) * 100);
                if (worlds == NULL)
                {
                    printf("malloc returned NULL for worldstruct (run out of RAM?)\n");
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Write Serverlist in string
                if (!WriteDataInString(curl, serverList, &stringAPI))
                {
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Generate parser tokens from string (new jsmn_init to reset used parser)
                tokenCount = strlen(stringAPI) / 10;
                tokens = malloc(sizeof(jsmntok_t) * tokenCount);
                jsmn_init(&parser);
                if (!GetTokens(&parser, tokens, tokenCount, stringAPI))
                {
                    Sleep(3000);
                    exit(EXIT_FAILURE);
                }

                //Fill worldstructs with data
                worlds = malloc(sizeof(worldstruct) * 100);
                worldCount = GetWorlds(tokens, tokenCount, stringAPI, worlds, worldsortflag);

                printf("Worlds: %d\n", worldCount);

                //Force next update to take ~3 seconds
                if (!flagChanged)
                    time += 3;
            }

            //Draw
            SetupRenderer(gRenderer);
            if(worlds != NULL && worldCount > 0)
                DrawWorlds(gRenderer, font, backbox, sortboxes, worldsortflag, worlds, worldCount, width, height);
        }

        if (flag == INFO)
        {

            //Draw
            //SetupRenderer(gRenderer);
            //DrawInfo(gRenderer, font, backbox, width, height);
        }

        SDL_RenderPresent(gRenderer);
        Sleep(17);
    }

    //Clear up before ending program
    if (tokens != NULL) free(tokens);
    if (worlds != NULL) free(worlds);
    curl_easy_cleanup(curl);
    closeSDL(&gWindow, &gRenderer, &gSurface);

    return 0;
}

/*worldsortFlag in input einbauen mit den sortboxes
  sorts in datafuncs.c einfügen:
  https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm*/