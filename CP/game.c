#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "game.h"

static char* get_rand_word() {
	srand(time(NULL));	
	int i = rand()%21;
	char* str = malloc(sizeof(char)*4);
	if (i == 0) str = "bear";
	if (i == 1) str = "vibe";
	if (i == 2) str = "neck";
	if (i == 3) str = "rose";
	if (i == 4) str = "bike";
	if (i == 5) str = "road";
	if (i == 6) str = "year";
	if (i == 7) str = "wine";
	if (i == 8) str = "fork";
	if (i == 9) str = "page";	
	if (i == 10) str = "sign";
	if (i == 11) str = "leaf";
	if (i == 12) str = "wind";	
	if (i == 13) str = "home";
	if (i == 14) str = "head";
	if (i == 15) str = "hole";
	if (i == 16) str = "camp";
	if (i == 17) str = "lamp";
	if (i == 18) str = "plan";
	if (i == 19) str = "face";
	if (i == 20) str = "cave";
	return str;
}

game_st* new_game(char* name, int max_players, pl_st* first_player){
	static int game_number = 0;
	game_st *g = malloc(sizeof(game_st) + sizeof(pl_st *) * (max_players - 1));
	
	if (g == NULL) {
		perror("Error: malloc\n");
		return NULL;
	}
	if(name == NULL || strcmp(name, "") == 0)
		sprintf(g->name, "game%d", game_number++);
	else
		strcpy(g->name, name);

	if (max_players == 1) 
		g->active_pl_id = 0;
	else 
		g->active_pl_id = -1;

	g->max_players = max_players;
	g->players[0] = first_player;
	g->pl_number = 1;
	g->win_id = -1;
	g->hidden_word = get_rand_word();
	return g;
}

void bulls_and_cows(game_st* g, char* user_word, int* bulls, int* cows) {
	char bukva;
	int bll = 0;
	int cw = 0;
	
	if (g == NULL) {
		printf("\tHidden word: '%s'\n\tBULLS: %d\n\tCOWS: %d\n", g->hidden_word, bll, cw);
		if(bulls != NULL) *bulls = bll; 
		if(cows != NULL) *cows = cw - bll;
	}
	
	for (int i = 0; i < WIN_BULLS; i++) {
		bukva = user_word[i];
		if (bukva == g->hidden_word[i]) bll++;
		
		for (int j = 0; j < WIN_BULLS; j++) {
		if (g->hidden_word[j] == bukva) {
				cw++;
				break;
			}
		}
	}
	printf("\tHidden word: '%s'\n\tBULLS: %d\n\tCOWS: %d\n", g->hidden_word, bll, cw);
	if(bulls != NULL) *bulls = bll;
	if(cows != NULL) *cows = cw - bll;
}

