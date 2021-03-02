#ifndef _DATAFUNCS_H_
#define _DATAFUNCS_H_

#include <stdio.h>
#include <stdlib.h>
#include <curl.h>
#include "jsmn.h"

#define MAXWORLDPLAYERS 45

enum worldsort {
	NUMBER,
    PLAYERS,
    UPTIME
};

typedef struct
{
    char name[5];
    char players[MAXWORLDPLAYERS][17];
    int uptime;
    int playercount;
}worldstruct;

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
int WriteDataInString(CURL* curl, const char* website, char** string);
int GetWorlds(jsmntok_t* tokens, int tokencount, char* string, worldstruct* worlds, int worldsortflag);
int cmpNumber(const void* a, const void* b);
int cmpPlayers(const void* a, const void* b);
int cmpUptime(const void* a, const void* b);
int GetPlayercount(jsmn_parser* parser, char* string);
void CheckPlayer(CURL* curl, worldstruct* worlds, int worldcount, const char* website);
char* GetCertainString(char* buffer, jsmntok_t* tokens, int placeInFile);
int GetTokens(jsmn_parser* parser, jsmntok_t* tokens, unsigned int tokencount, char* string);

#endif

