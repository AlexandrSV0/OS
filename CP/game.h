#ifndef __GAME_H_
#define __GAME_H_

#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>

#define GAME_NAME_SIZE 32
#define WIN_BULLS 4

typedef struct{
	int fd_r;
	int fd_w;
	int user_id;
	pthread_t tid;
} player_t;

typedef struct{
	int winner_idx;
	char name[GAME_NAME_SIZE];
	char* hidden_word; // CHANGE TO MYSTERY_WORD
	int max_players;
	int pl_number;
	int active_player;

	player_t *players[1];
} game_t;

static inline bool active_game(game_t *g){
	return g->active_player >= 0;
}

game_t* new_game(char *name, int max_players, player_t *first_player);
bool add_player_to_game(game_t *g, player_t *p);
bool ok_number_str(char *str);
bool ok_number(int num);

void bulls_and_cows(game_t* g, char* user_word, int *bulls, int *cows);

#endif