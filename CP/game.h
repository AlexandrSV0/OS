#ifndef GAME_H_
#define GAME_H_

#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>

#define GAME_NAME 32
#define WIN_BULLS 4

typedef struct{
	int fd_r;
	int fd_w;
	int user_id;
	pthread_t t_id;
} pl_st;

typedef struct{
	int win_id;
	char name[GAME_NAME];
	char* hidden_word;
	int max_players;
	int pl_number;
	int active_pl_id;
	pl_st *players[1];
} game_st;


game_st* new_game(char *name, int max_players, pl_st *first_player);
void bulls_and_cows(game_st* g, char* user_word, int *bulls, int *cows);

#endif