#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

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



game_t *new_game(char *name, int max_players, player_t *first_player){
	static int game_number = 0;
	game_t *g = malloc(sizeof(game_t) + sizeof(player_t *) * (max_players - 1));
	if (g == NULL) {
		perror("Error: malloc\n");
		return NULL;
	}
	if(name == NULL || strcmp(name, "") == 0)
		sprintf(g->name, "game%d", game_number++);
	else
		strcpy(g->name, name);

	if (max_players == 1) 
		g->active_player = 0;
	else 
		g->active_player = -1;
	g->max_players = max_players;

	g->players[0] = first_player;
	g->pl_number = 1;
	g->winner_idx = -1;
	g->hidden_word = get_rand_word();
	return g;
}

bool add_player_to_game(game_t *g, player_t *p){
	if(g == NULL || p == NULL)
		return false;
	if(active_game(g))
		return false;
	g->players[g->pl_number++] = p;
	return true;
}

void bulls_and_cows(game_t* g, char* user_word, int *bulls, int *cows){
	char bukva;
	int bll = 0;
	int cw = 0;
	if(g == NULL)
		goto BALLOUT;
	for (int i = 0; i < WIN_BULLS; i++){
		bukva = user_word[i];
		if (bukva == g->hidden_word[i]) bll++;
		
		for(int j = 0; j < WIN_BULLS; j++){
		if(g->hidden_word[j] == bukva){
				cw++;
				break;
			}
		}
	}

BALLOUT:
	printf("\tHidden word: '%s'\n\tBULLS: %d\n\tCOWS: %d\n", g->hidden_word, bll, cw);
	if(bulls != NULL) *bulls = bll;
	if(cows != NULL) *cows = cw - bll;
}


bool ok_number_str(char *str){
	bool in_use[10];
	int i;
	for(i = 0; i < 10; i++)
		in_use[i] = false;
	i = 0;
	while(*str != 0){
		if(!isdigit(*str))
			return false;
		if (in_use[*str - '0'] == true)
			return false;
		in_use[*str - '0'] = true;
		i++;
		str++;
		if(i > 4)
			return false;
	}
	return true;
}

bool ok_number(int num){
	char buf[16];
	if (snprintf(buf, sizeof(buf), "%04d", num) != 4){
		printf("IN COUNT\n");
		return false;
	}
	return ok_number_str(buf);
}
