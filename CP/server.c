#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> //mknod
#include <semaphore.h>
#include <pthread.h>
#include "game.h"
#include "Message.h"
#define MAX_PATH_NAME_SIZE 128
#define GAME_NAME_SIZE 32
#define MAX_GAMES_COUNT    10
#define MAX_PLAYERS_COUNT  32

static sem_t phore;
static game_st *games[MAX_GAMES_COUNT];
static pl_st *players[MAX_PLAYERS_COUNT];
static int pl_number = 0;
static int games_count = 0;

void serv_sem_init(){
	sem_init(&phore, 1, 1);
}

void lock(){
	sem_wait(&phore);
}

void unlock(){
	sem_post(&phore);
}

void add_to_str(char* buf, int x){
	char num[24];
	int i = 0;
	if (x < 0){
		num[i++] = '-';
		x = -x;
	}
	do{
		num[i++] = x % 10 + '0';
		x /= 10;
	} while (x > 0);
	while(--i >= 0)
		*buf++ = num[i];
	*buf = 0;
}

typedef struct{
	char path_sr[MAX_PATH_NAME_SIZE];
	char path_sw[MAX_PATH_NAME_SIZE];
} pipes_st;

void pipes_st_init(pipes_st *pl, int num){
	strcpy(pl->path_sr, "/tmp/bulls_and_cows_sr");
	add_to_str(pl->path_sr + 22, num);
	strcpy(pl->path_sw, "/tmp/bulls_and_cows_sw");
	add_to_str(pl->path_sw + 22, num);
}


static int list_reply(char *rep){
	int len = 0;
	int i = 0;
	game_st **g = games;
	int l;
	lock();
	int gc = games_count;
	printf("Games count %d\n", gc);
	if(gc == 0){
		len = sprintf(rep, "No games running\n");
		unlock();
		return len;
	}
	do{
		if(i++ >= MAX_GAMES_COUNT)
			break;
		if(*g == NULL){
			g++;
			continue;
		}
		l = sprintf(rep, "%s[%d\\%d]\t", (*g)->name, (*g)->max_players, (*g)->pl_number);
		g++;
		rep += l;
		len += l;
	} while(--gc > 0);
	unlock();
	*(--rep) = '\n';
	return len;
}

static int print_reply(char *rep){
	game_st **g = games;
	lock();
	int gc = 0;
	int i = 0;
	printf("--------Reply-------\n");
	printf("Total games: %d\n", games_count);
	do{
		if(i++ >= MAX_GAMES_COUNT)
			break;
		if(*g == NULL){
			g++;
			continue;
		}
		printf("%s[%d\\%d] | '%s' %s Active player's ID: %d\n", (*g)->name, (*g)->max_players, (*g)->pl_number, 
			(*g)->hidden_word, (*g)->win_id < 0 ? "" : "| completed | ", (*g)->active_pl_id);
//			printf("\n");
		g++;
		gc++;
	} while(true);

	printf("=====End of reply====\n\n");
	unlock();
	*rep = 0;
	return 0;
}

int new_game_serv(char *name, int max_players, pl_st *first_player){
	game_st *g = NULL;
	static int ind = 0;
	int rvl = -1;
	int i;
	g = new_game(name, max_players, first_player);
//	printf("HW: %s\n", g->hidden_word);
	if(g == NULL)
		return -1;
	lock();
	for(i = 0; i < MAX_GAMES_COUNT; i++){
		if(games[(i + ind) % MAX_GAMES_COUNT] == NULL){
			rvl = (i + ind) % MAX_GAMES_COUNT;
			games[rvl] = g;
			games_count++;
			ind = (i + ind + 1) % MAX_GAMES_COUNT;
			break;
		}
	}
	unlock();
	return rvl;
}

void remove_game(int ind){
	if(ind >= MAX_GAMES_COUNT || ind < 0)
		return;
	if(games[ind] != NULL){
		games[ind] = NULL;
		games_count--;
	}
}

int add_player(pl_st *p){
	static int ind = 0;
	int rvl = -1;
	int i;
	if(p == NULL)
		return -1;

	lock();
	for(i = 0; i < MAX_PLAYERS_COUNT; i++){
		if(players[(i + ind) % MAX_PLAYERS_COUNT] == NULL){
			rvl = (i + ind) % MAX_PLAYERS_COUNT;
			players[rvl] = p;
			pl_number++;
			ind = (i + ind + 1) % MAX_PLAYERS_COUNT;
			break;
		}
	}
	printf("Player was added successfully!\n");
	unlock();
	p->user_id = rvl;
	return rvl;
}

void remove_player(int ind){
	if(ind >= MAX_PLAYERS_COUNT || ind < 0)
		return;
	lock();
	if(players[ind] != NULL){
		players[ind] = NULL;
		pl_number--;
	}
	unlock();
}

static void* client_thread(void *arg){
	int game_ind = -1;
	game_st* game = NULL;
	pl_st* player = (pl_st*)arg;
	int my_ind = player->user_id;
	char req[MAX_REQUEST_SIZE];
	char rep[MAX_REPLY_SIZE];
	int fd_r = player->fd_r;
	int fd_w = player->fd_w;
	printf("## Thread: %d\n\n", my_ind);
	for (;;) {
		read_str(fd_r, req, MAX_REQUEST_SIZE);
		if (*req == 'q') {
			printf("!GAME OVER!\n");
			remove_player(my_ind);
			free(player);
			return NULL;
		}
		if (strcmp(req, "ping") == 0) {
			printf("$REQUEST: ping the server\n");
			printf("-------------------------\n");
			write_msg(fd_w, "pong\n", 5);
			continue;
		}
		if (*req == 'c') {
			printf("$REQUEST: create the game\n");
			printf("-------------------------\n");
			if (game != NULL) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			char name[GAME_NAME_SIZE];
			name[0] = 0;
			int max_players = -1;
			sscanf(req + 1, "%d*%s", &max_players, name);
			if(max_players <= 0) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			game_ind = new_game_serv(name, max_players, player);
			if(game_ind == -1) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			game = games[game_ind];
//			printf("HW2: %s\n", game->hidden_word);
			write_msg(fd_w, game->name, strlen(game->name));
			write_msg(fd_w, "\n", 1);
			continue;
		}
		if (*req == 'a') {
			printf("$REQUEST: check user's answer\n");
			printf("-----------------------------\n");
			int bulls = 0;
			int cows = 0;
			char* word = malloc(sizeof(char)*4);
			for (int i = 0; i < 4; i++) {
				word[i] = req[i+1];
			}
			printf("GAME = %s [%d\\%d].\nWORD: ''%s''\n", game->name, game->max_players, game->pl_number, word);
			if (game == NULL || !active_game(game)) {
				printf("# NOT ACTIVE!\n");
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			if (game->players[game->active_pl_id] != player) {
				printf("NOT ME %s %p %p\n", game->name, game->players[game->active_pl_id], player);
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			bulls_and_cows(game, word, &bulls, &cows);
			int len = sprintf(rep, "%d%d\n", bulls, cows);
			if (game->max_players > 1) {
				do {
						game->active_pl_id = (game->active_pl_id + 1) % game->max_players;	
					} while (game->players[game->active_pl_id] == NULL);
			}
			printf("Active player's ID %d\n\n", game->active_pl_id);
			write_msg(fd_w, rep, len);
			if (bulls >= WIN_BULLS){
				game->win_id = my_ind;
			}
			continue;
		}
		if (*req == 'l'){
			printf("$REQUEST: list of games\n");
			printf("-----------------------\n");
			int list_len = list_reply(rep);
			printf("# Active games: %s\n", rep);
			write_msg(fd_w, rep, list_len);
			continue;
		}
		if (*req == 'p'){
			printf("$REQUEST: print reply\n");
			printf("---------------------\n\n");
			int print_len = print_reply(rep);
			print_len += sprintf(rep + print_len, "\rPlayer ID %d. Game ID: %d. Game: %s\n", my_ind, game_ind, game ? game->name : "NULL");
			write_msg(fd_w, rep, print_len);
			continue;
		}
		if (*req == 'j'){
			printf("$REQUEST: join to the game\n");
			printf("----------------------\n");
			char *p = req + 1;
			if (game != NULL) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			lock();
			for (int i = 0; i < MAX_GAMES_COUNT; i++){
				if (games[i] == NULL)
					continue;
				if (active_game(games[i]))
					continue;
				if (*p == 0 || strcmp(p, games[i]->name) == 0) {
					games[i]->players[games[i]->pl_number++] = player;
					game = games[i];
					game_ind = i;
//					printf("HW3: %s\n", game->hidden_word);
					if (game->pl_number >= game->max_players) game->active_pl_id = 0;
					break;
				}
			}
			unlock();
			if(game == NULL) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			int len = sprintf(rep, "%s %d %d\n", game->name, game->max_players, game->pl_number);
			write_msg(fd_w, rep, len);
			continue;
		}
		if(*req == 'w'){
			int len;
			if(game->active_pl_id >= 0) {
				if(game->win_id >= 0){
					if(game->win_id == my_ind)
						len = sprintf(rep, "w%d\n", game->max_players);
					else 
						len = sprintf(rep, "l%d\n", game->max_players);
				}
				else
					if(game->players[game->active_pl_id] == player){
						len = sprintf(rep, "r%d\n", game->max_players);
					}
					else 
						len = sprintf(rep, "t%d\n", game->max_players);
			}
			else 
				len = sprintf(rep, "p%d\n", game->pl_number);
			write_msg(fd_w, rep, len);
			continue;
		}
		if(*req == 'e'){
			printf("$REQUEST: leave the game\n");
			printf("------------------------\n");
			if(game == NULL) {
				write_msg(fd_w, "!\n", 2);
				continue;
			}
			lock();

			if (game->pl_number == 1) {
				for(int i = 0; i < game->max_players; i++){
					game->players[i] = NULL;
				}
				game->max_players = 0;
				game->pl_number--;
			} else {
				for(int i = 0; i < game->max_players; i++){
					if(game->players[i] == player){
						game->players[i] = NULL;
						game->pl_number--;
					}
				}
			}

			if (game->max_players >= 1) {
				do {
					game->active_pl_id = (game->active_pl_id + 1) % game->max_players;	

				} while (game->players[game->active_pl_id] == NULL);
			}
			if(game->pl_number <= 0){
				if (game_ind < 0 || game_ind >= MAX_GAMES_COUNT)
					printf("Game ID = %d\n", game_ind);
				else if(games[game_ind] == NULL)
					printf("Null ID\n");
				else
					printf("%d.Game-name: %s(%s) PC: %d\n", game_ind, games[game_ind]->name, game->name, game->pl_number);
				unlock(); print_reply(rep); lock();
				remove_game(game_ind);
				unlock(); print_reply(rep); lock();
				free(game);
			}
			game_ind = -1;
			game = NULL;
			unlock();
			write_msg(fd_w, "ok\n", 3);
			continue;
		}
	}
}

void pthr_player_begin(int fd_r, int fd_w){
	pl_st *player = malloc(sizeof(pl_st));
	player->fd_r = fd_r;
	player->fd_w = fd_w;
	int idx = add_player(player);
	printf("Player-ID: %d\n", idx);
	pthread_create(&player->t_id, NULL, &client_thread, player);
	printf("## Thread started successfully!\n");
}

void server_thread_start(pipes_st pl){
	int fd_r = -1;
	int fd_w = -1;

	if(mknod(pl.path_sw, S_IFIFO|S_IWUSR|S_IWOTH|S_IRUSR|S_IROTH, 0) < 0){
		perror("Error: MKNOD path_sw\n!");
		printf("PIPES: %s %s\n", pl.path_sw, pl.path_sr);
	}

	if(mknod(pl.path_sr, S_IFIFO|S_IWUSR|S_IWOTH|S_IRUSR|S_IROTH, 0) < 0){
		perror("Error: MKNOD path_sr\n!");
		printf("PIPES: %s %s\n", pl.path_sw, pl.path_sr);
	}
	printf("## Pipes created\n");
	if((fd_r = open(pl.path_sr, O_RDONLY)) < 0){
		perror("Can't open pipe for reading!\n");
		printf("PIPES: %s %s\n", pl.path_sw, pl.path_sr);
	}
	if((fd_w = open(pl.path_sw, O_WRONLY)) < 0){
		perror("Can't open pipe for writing!\n");
		printf("PIPES: %s %s\n", pl.path_sw, pl.path_sr);
	}
	printf("CLIENT-PIPES:\n \t%s\n \t%s\n", pl.path_sw, pl.path_sr);
	pthr_player_begin(fd_r, fd_w);
}

int main(){
	serv_sem_init();
	int pipe_id = 1;
	pipes_st connection_pl;
	pipes_st_init(&connection_pl, 0);
	if(mknod(connection_pl.path_sw, S_IFIFO|S_IWUSR|S_IWOTH|S_IRUSR|S_IROTH, 0) < 0)
			perror("Error: MKNOD!\n");
	for(;;) {
		int fd_w;
		if((fd_w = open(connection_pl.path_sw, O_WRONLY)) < 0)
			perror("Can't open pipe for writing\n");
		pipes_st client_pl;
		pipes_st_init(&client_pl, pipe_id);
		write(fd_w, client_pl.path_sr, strlen(client_pl.path_sr));
		write(fd_w, "\n", 1);
		write(fd_w, client_pl.path_sw, strlen(client_pl.path_sw));
		write(fd_w, "\n", 1);

		server_thread_start(client_pl);
		sleep(1);
		close(fd_w);

		pipe_id++;
	}
	remove(connection_pl.path_sw);
	return 0;
}
