#define CURL_STATICLIB
#define _CRT_SECURE_NO_WARNINGS

#include "datafuncs.h"
#include <time.h>

//Global variables
#define PLAYERSONLINEINDEX 2
#define FIRSTWORLDINDEX 3
const char filename[] = "data.txt";

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
        *string = malloc(chunk.size+1);
        if (string == NULL)
        {
            printf("malloc failed in WriteDataInString()!\n");
            return 0;
        }
        strncpy(*string, chunk.memory, chunk.size+1);
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

int GetWorlds(jsmntok_t* tokens, int tokencount, char* string, worldstruct* worlds, int worldsortflag)
{
    char* tempString;
    char delimiter[] = "WC";
    int i = FIRSTWORLDINDEX, tempPlayercount, worldCount = 0;

    if (tokens[FIRSTWORLDINDEX].type != JSMN_STRING && tokens[FIRSTWORLDINDEX + 1].type != JSMN_OBJECT)
    {
        printf("ERROR! The downloaded data from the API doesnt match the signature of what should have been obtained.\n");
        printf("Either the API of Wynncraft/Wynntils changed or there was a error while trying retrieve data from the API.\n\n");
        printf("Downloaded message from API:\n%s\n\n", string);
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
        //Skip TEST- and YT-World
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

    if (worldsortflag == NUMBER)
        qsort(worlds, worldCount, sizeof(worldstruct), cmpNumber);
    else if (worldsortflag == PLAYERS)
        qsort(worlds, worldCount, sizeof(worldstruct), cmpPlayers);
    else qsort(worlds, worldCount, sizeof(worldstruct), cmpUptime);

    return worldCount;
}

int cmpNumber(const void* a, const void* b)
{
    char delimiter[] = "WC";
    worldstruct *left = (worldstruct*)a, *right = (worldstruct*)b;
    return atoi(strtok(left->name, delimiter)) - atoi(strtok(right->name, delimiter));
}

int cmpPlayers(const void* a, const void* b)
{
    worldstruct* left = (worldstruct*)a, * right = (worldstruct*)b;
    return left->playercount - right->playercount;
}

int cmpUptime(const void* a, const void* b)
{
    worldstruct* left = (worldstruct*)a, * right = (worldstruct*)b;
    return left->uptime - right->uptime;
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

void CheckPlayer(CURL* curl, worldstruct* worlds, int worldcount, const char* website, char* playername)
{
    char* apiURL, * apiString, tempplayername[25] = { 0 }, *temp, delimiter[4] = "[\"]";
    int tokenCount, n = 0;
    bool foundflag = false;
    FILE* file;
    jsmn_parser parser;
    jsmntok_t* tokens = NULL;
    jsmn_init(&parser);
    
    //Creating URL (combining first part of URL and playername)
    apiURL = malloc(strlen(website) + strlen(playername) + 1);
    if (apiURL == NULL)
    {
        printf("Error: malloc in CheckPlayer()\n");
        return;
    }
    strcpy(apiURL, website);
    strcat(apiURL, playername);

    //Getting data from wynncrafts search-API
    WriteDataInString(curl, apiURL, &apiString);
    tokenCount = strlen(apiString) / 5;
    tokens = malloc(sizeof(jsmntok_t) * tokenCount);
    if (!GetTokens(&parser, tokens, tokenCount, apiString))
    {
        Sleep(3000);
        exit(EXIT_FAILURE);
    }
    
    //Check if player has ever been on wynncraft before
    for (int n = 0; n < tokenCount; n++)
    {
        temp = GetCertainString(apiString, tokens, n);
        if (!strcmp("players", temp))
        {
            free(temp);
            break;
        }
        free(temp);
    }

    if (n == tokenCount)
    {
        printf("ERROR! Downloaded data from API doesnt match the signature of what should be downloaded (in CheckPlayer(), wynncraft API).\n");
        printf("API updated/changed or there was a error.\n");
        printf("Downloaded data:\n%s\n", apiString);
        Sleep(3000);
        exit(EXIT_FAILURE);
    }
    else
    {
        strcat(tempplayername, "[\"");
        strcat(tempplayername, playername);  
        strcat(tempplayername, "\"]");
        while (1)
        {
            temp = GetCertainString(apiString, tokens, n);
            if (!strcmp("search", temp))
            {
                free(temp);
                break;
            }
            else if (!_stricmp(tempplayername, temp))
            {
                strcpy(tempplayername, temp);
                free(temp);
                foundflag = true;
                break;
            }
            free(temp);
            n++;
        }
    }

    //If found write name in file
    if (foundflag)
    {
        char fileline[24];
        n = 0;
        //Check if already in file
        file = fopen(filename, "a+");
        while (!feof(file))
        {
            if (n > 20)
            {
                foundflag = false;                      //list is full (20 names)
                break;
            }
            n++;

            fgets(fileline, 24, file);
            if (strtok(fileline, "\n") == NULL)
            {
                printf("strtok error\n");
                Sleep(3000);
                exit(0);
            }
            if (!_stricmp(fileline, playername))
            {
                foundflag = false;                      //playername already in file, change foundflag to avoid if below
                break;
            }
        }
        fclose(file);

        if (foundflag)
        {
            //Cut ["
            temp = &tempplayername[2];
            //Cut "]
            if (strtok(&tempplayername[2], "\"") == NULL)
            {
                printf("strtok error\n");
                Sleep(3000);
                exit(0);
            }

            file = fopen(filename, "a");
            fprintf(file, "%s\n", temp);
            temp = NULL;
            fclose(file);
        }
    }

    playername[0] = 0;

    return;
}

playerstruct* GetPlayers(worldstruct* worlds, int worldCount, int *n)
{
    char buffer[18];
    playerstruct* players;
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return NULL;
    while (fgets(buffer, sizeof(buffer), file) != NULL)
        *n+=1;

    if (!*n)
        return NULL;
    else
        fseek(file, 0, SEEK_SET);

    players = malloc(*n * sizeof(playerstruct));
    if(players == NULL)
    {
        printf("Error: malloc in GetPlayers()\n");
        Sleep(3000);
        exit(0);
    }

    for (int i = 0; i < *n; i++)
    {
        fgets(buffer, sizeof(buffer), file);
        if (strtok(buffer, "\n") == NULL)
        {
            printf("strtok error\n");
            Sleep(3000);
            exit(0);
        }
        strcpy(players[i].name, buffer);
        players[i].online = CheckPlayerOnline(players[i].name, &players[i].world, worlds, worldCount);
    }
    fclose(file);
    return players;
}

bool CheckPlayerOnline(char* name, worldstruct* playerworld, worldstruct* worlds, int worldCount)
{
    for (int i = 0; i < worldCount; i++)
        for (int j = 0; j < MAXWORLDPLAYERS; j++)
            if (!strcmp(name, worlds[i].players[j]))
            {
                *playerworld = worlds[i];
                return true;
            }
    return false;
}

char* GetCertainString(char* buffer, jsmntok_t* tokens, int placeInFile)
{
    jsmntok_t key = tokens[placeInFile];

    if (key.type == JSMN_ARRAY || key.type == JSMN_OBJECT || key.type == JSMN_PRIMITIVE || key.type == JSMN_STRING)
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


