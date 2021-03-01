#define CURL_STATICLIB
#define _CRT_SECURE_NO_WARNINGS

#include "datafuncs.h"
#include <time.h>

//Global variables
#define PLAYERSONLINEINDEX 2
#define FIRSTWORLDINDEX 3

//Struct to store string from api temporary
struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int WriteDataInString(CURL* curl, const char* website, char** string)
{
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURLcode result;
    curl_easy_setopt(curl, CURLOPT_URL, website);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    result = curl_easy_perform(curl);

    if (chunk.memory != NULL)
    {
        *string = malloc(chunk.size);
        strncpy(*string, chunk.memory, chunk.size);
        *(*string+chunk.size) = '\0';
        free(chunk.memory);
    }
    else
        printf("Nullpointer while downloading data\n");

    if (result == CURLE_OK)
        return 1;
    else
    {
        printf("Erorr while downloading data: %s\n", curl_easy_strerror(result));
        return 0;
    }
}

int GetWorlds(jsmntok_t* tokens, int tokencount, char* string, worldstruct* worlds)
{
    worldstruct tempStruct;
    char* tempString, * tempWorld;
    char delimiter[] = "WC";
    int i = FIRSTWORLDINDEX, tempPlayercount, worldCount = 0, tempWorldnumberOne, tempWorldnumberTwo;

    if (tokens[FIRSTWORLDINDEX].type != JSMN_STRING && tokens[FIRSTWORLDINDEX + 1].type != JSMN_OBJECT)
    {
        printf("Wynntils/Wynncraft API changed! Program needs update! (Error in GetWorlds())\n");
        return 0;
    }

    while (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_STRING || tokens[i].type == JSMN_PRIMITIVE || tokens[i].type == JSMN_OBJECT)
    {
        start:

        //Get worldname (e.g. WC10)
        tempPlayercount = 0;
        tempString = GetCertainString(string, tokens, i);
        if (tempString == NULL)
        {
            printf("Error while getting string in GetWorlds() (tempString == NULL)\n");
            return 0;
        }
        if(!strcmp(tempString, "TEST") || !strcmp(tempString, "YT"))
        {
            free(tempString);
            i += 6;
            while (tokens[i+1].type != JSMN_OBJECT)
                i++;
            goto start;
        }
        strcpy(worlds[worldCount].name, tempString);
        free(tempString);
        i += 3;

        //Get worlds uptime
        tempString = GetCertainString(string, tokens, i);
        worlds[worldCount].uptime = _time64(NULL) - _atoi64(tempString) / CLOCKS_PER_SEC;
        free(tempString);
        i += 3;

        //Fill worlds with their players
        while (tokens[i + 1].type != JSMN_OBJECT || tokens[i].type != JSMN_STRING)
        {
            if (tokens[i].type > 3 || tokens[i].type < 0)
                break;

            tempString = GetCertainString(string, tokens, i);
            strcpy(worlds[worldCount].players[tempPlayercount], tempString);
            free(tempString);
            tempPlayercount++;
            i++;
        }
        worlds[worldCount].playercount = tempPlayercount;
        worldCount++;
    }

    for (int n = worldCount; n > 1; --n)
    {
        for (int k = 0; k < n - 1; ++k)
        {
            tempWorld = strtok(worlds[k].name, delimiter);
            tempWorldnumberOne = atoi(tempWorld);
            tempWorld = strtok(worlds[k + 1].name, delimiter);
            tempWorldnumberTwo = atoi(tempWorld);
            if (tempWorldnumberOne > tempWorldnumberTwo)
            {
                tempStruct = worlds[k];
                worlds[k] = worlds[k + 1];
                worlds[k + 1] = tempStruct;
            }
        }
    }

    return worldCount;
}

int GetPlayercount(jsmn_parser* parser, char* string)
{
    jsmntok_t* tokens = NULL;
    unsigned int tokencount = 10;
    tokens = malloc(sizeof(jsmntok_t) * tokencount);
    char* buffer = NULL;

    if (!GetTokens(parser, tokens, tokencount, string))
    {
        return -1;
    }
    buffer = GetCertainString(string, tokens, PLAYERSONLINEINDEX);

    if (buffer != NULL && string != NULL && atoi(buffer) > 1 && atoi(buffer) < 20000)
    {
        int var = atoi(buffer);
        free(tokens);
        free(buffer);
        return var;
    }
    else if (buffer != NULL && string != NULL)
    {
        printf("Wynncraft API changed or cant be reached!\n");
        free(tokens);
        free(buffer);
        return -1;
    }
    else
    {
        printf("Error while trying to get string from .JSON file\n");
        free(tokens);
        free(buffer);
        return -1;
    }
}

void CheckPlayer(CURL* curl, worldstruct* worlds, int worldcount, const char* website)
{
    char playername[17], *apiURL, *apiString;

    printf("Enter name: ");
    if (scanf(" %s", &playername) != 1)
    {
        printf("Error while entering a name for search! Code 1\n");
        return;
    }
    if (playername[strlen(playername)] != '\0')
    {
        printf("Error while entering a name for search! Code 2\n");
        return;
    }

    apiURL = malloc(strlen(website) + strlen(playername) + 1);
    if (apiURL == NULL)
    {
        printf("Error: malloc in CheckPlayer()\n");
        return;
    }
    strcpy(apiURL, website);
    strcat(apiURL, playername);

    WriteDataInString(curl, apiURL, &apiString);

    for (int i = 0; i <= worldcount; i++)
        for (int j = 0; j < worlds[i].playercount; j++)
            if (!strcmp(playername, worlds[i].players[j]))
            {
                printf("Found %s on %s\t%d players on %s\n", playername, worlds[i].name, worlds[i].playercount, worlds[i].name);
                return;
            }

    printf("%s was not found!\n", playername);

    return;
}

char* GetCertainString(char* buffer, jsmntok_t* tokens, int placeInFile)
{
    jsmntok_t key = tokens[placeInFile];

    if (key.type != JSMN_ARRAY && key.type != JSMN_OBJECT)
    {

        unsigned int laenge = key.end - key.start;
        char* keyString = malloc(laenge + 1);
        if (tokens == NULL)
        {
            printf("Error: tokens are NULL\n");
            return NULL;
        }
        else if (keyString != NULL)
        {
            strncpy(keyString, &buffer[key.start], laenge + 1);
            keyString[laenge] = '\0';
            return keyString;
        }
        else
        {
            printf("Error: keyString is NULL\n");
            return NULL;
        }

    }
    else
    {
        printf("\nToken is an array or object!\n");
        return NULL;
    }
}

int GetTokens(jsmn_parser* parser, jsmntok_t* tokens, unsigned int tokencount, char* string)
{
    if (string == NULL)
    {
        printf("JSON-String is NULL in GetTokens()\n");
        return 0;
    }

    int result = jsmn_parse(parser, string, strlen(string), tokens, tokencount);

    if (result == JSMN_ERROR_INVAL)
    {
        printf("Tokens error: INVAL\n");
        return 0;
    }
    else if (result == JSMN_ERROR_NOMEM)
    {
        printf("Tokens error: NOMEN\n");
        return 0;
    }
    else if (result == JSMN_ERROR_PART)
    {
        printf("Tokens error: PART\n");
        return 0;
    }
    else return 1;
}


