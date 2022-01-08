
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "Message.h"
#include "game.h"

#define MAX_PATH_NAME_SIZE 128
#define MAX_CMD_SIZE 	   64 
#define GAME_NAME_SIZE 32

char *skip_separator(char *str){
	if(str == NULL)
		return NULL;
	while(*str != 0){
		if(*str == ' ' || *str == '\t'){
			str++;
		}
		else 
			break;
	}
	return str;
}

char *search_separator(char *str){
	if(str == NULL)
		return NULL;
	while(*str != 0){
		if(*str != ' ' && *str != '\t'){
			str++;
		}
		else 
			break;
	}
	return str;
}

char wait_request(pl_st *p, int *n){
	int num = -1;
	char rep[32];

	write_msg(p->fd_w, "w\n", 2);
	read_str(p->fd_r, rep, 32);
	switch(rep[0]){
		case 'p':
		case 't':
		case 'r':
		case 'w':
		case 'l':
			num = atoi(rep + 1);
			break; 
		default:
			rep[0] = 'r';
			break;
	}
	if (rep[0] != 'l') {
		num = atoi(rep+1);
	}
	if (n != NULL)
		*n = num;
	return rep[0];
}

void erase(int n){
	while(n-- > 0){
		printf("\b \b");
	}
	fflush(stdout);
}

int wait_for_turn(game_st * game){
	pl_st * p = game->players[0];
	int num = -1;
	int len = 0;
	char w = ' ';
	do{
		sleep(1);
		w = wait_request(p, &num);
		erase(len);
		if(w == 'p')
			len = printf("%s", "Waiting for players...");
		else if(w == 't')
			len = printf("%s", "Waiting for your turn...");
		fflush(stdout);
	} while(w != 'r' && w != 'l' && w != 'w');
	erase(len);
	if(w == 'l' || w == 'w'){
		w == 'l' ? printf("You loosed! Good luck next time\n") : printf("You win!\n");
		game->win_id = w == 'w' ? 0 : 1;
	}
	return num;
}

static void put_begin(game_st *g, pl_st* player){
	bool pl_exist = false;
	if (g != NULL) {
		for (int i = 0; i < g->max_players; i++) {
			if (g->players[i] == player) {
				pl_exist = true;
				break;
			}
		}
	}
	if(!pl_exist || g == NULL){
		printf("> ");
		fflush(stdout);
		return;
	}
	if(g->win_id < 0){
		int n = wait_for_turn(g);
		g->max_players = n;
		g->pl_number = n;
	}
	printf("%s%s%c ", g->name, g->win_id < 0 ? "" : g->win_id == 0 ? " winner" : " loser", g->active_pl_id >= 0 ? '#' : '>');
	fflush(stdout);
}

static game_st* CreateGame(pl_st *player, char *name, int max_players){
	int num;
	char msg[MAX_REQUEST_SIZE];
	char rep[MAX_REPLY_SIZE];
	int len = snprintf(msg, MAX_REQUEST_SIZE, "c%d*%s\n", max_players, name == NULL ? "" : name);
	write_msg(player->fd_w, msg, len);
	read_str(player->fd_r, rep, MAX_REPLY_SIZE);
	if(*rep == '!' || *rep == 0)
		return NULL;
	len = 0;
	game_st *game = new_game(rep, max_players, player);
//	printf("HW: %s\n", game->hidden_word);
	num = wait_for_turn(game);
	game->active_pl_id = 0;
	game->pl_number = num;
	game->max_players = max_players;

	return game;
}

static game_st* JoinGame(pl_st *player, char *name){
	char server_game_name[GAME_NAME_SIZE];
	char msg[MAX_REQUEST_SIZE];
	char rep[MAX_REPLY_SIZE];
	int server_max_players = -1;
	int server_active_pl_ids = -1;
	int len = snprintf(msg, MAX_REQUEST_SIZE, "j%s\n", name == NULL ? "" : name);
	write_msg(player->fd_w, msg, len);
	read_str(player->fd_r, rep, MAX_REPLY_SIZE);
	printf("Joining to the %s\n", rep);
	if(*rep == '!')
		return NULL;
	sscanf(rep, "%s %d %d", server_game_name, &server_max_players, &server_active_pl_ids);
	if(*server_game_name == 0)
		return NULL;
	printf("%s %d %d\n", server_game_name, server_max_players, server_active_pl_ids);
	if(server_game_name[0] == 0 || server_max_players < 1 || server_active_pl_ids < 1)
		return NULL;
	game_st* game = new_game(server_game_name, server_max_players, player);
	int num = wait_for_turn(game);
	game->pl_number = num;
	game->max_players = num;
	
	return game;
}

void process_cmd(int fd_r, int fd_w){
	pl_st player;
	player.fd_r = fd_r;
	player.fd_w = fd_w;
	game_st *game = NULL;
	char cmd[MAX_CMD_SIZE];
	char rep[MAX_REPLY_SIZE];
	char req[MAX_REQUEST_SIZE];
	for(;;){
		put_begin(game, &player);
		read_str(0, cmd, MAX_CMD_SIZE);
		if (strcmp(cmd,"exit") == 0 ) {
			write_msg(fd_w, "e\n", 2);
			read_str(fd_r, rep, MAX_REPLY_SIZE);
			if(*rep == '!') {
				printf("Wrong command\n");
				continue;
			} 
			game = NULL;				
			continue;
		} else
		if(strcmp(cmd, "ping") == 0){
			write_msg(fd_w, "ping\n", 5);
			read_str(fd_r, rep, MAX_REPLY_SIZE);
			printf("%s\n", rep);
			continue;
		} else
		if(strncmp(cmd, "create ", 7) == 0){
			char *name = NULL;
			int max_players = 1;
			if(game != NULL) {
				printf("You can't create new game now!\n");
				continue;
			}
			char *p = cmd + 7;
			p = skip_separator(p);
			if(*p != 0){
				char *new_p = search_separator(p);
				max_players = atoi(p);
				if(*new_p != 0){
					p = skip_separator(new_p);
					if(*p != 0){
						name = p;
						p = search_separator(p);
						*p = 0;
					}
				}
			}
			if(max_players <= 0)
				max_players = 1;
			if(name != NULL && *name == 'g'){
				printf("Wrong command\n");
				continue;
			}
			game = CreateGame(&player, name, max_players);
			continue;
		} else
		if(strncmp(cmd, "join", 4) == 0){
			if(game != NULL) {
				printf("You can't join other game while you are in active game\n");
				continue;
			}
			char *name = NULL;
			char *p = cmd + 4;
			p = skip_separator(p);
			if(*p != 0){
				char *new_p = search_separator(p);
				*new_p = 0;
				name = p;
			}
			game = JoinGame(&player, name);
			if (game->pl_number >= game->max_players) game->active_pl_id = 0;
			continue;
		} else
		if(cmd[0] == 'a') {
			char* w = malloc(sizeof(char)*5);
			read_str(0, w, sizeof(char)*5);
			write_msg(fd_w, "a", 1);
			write_msg(fd_w, w, strlen(w));
			write_msg(fd_w, "\n", 1);
			read_str(fd_r, rep, MAX_REPLY_SIZE);
			if(*rep == '!'){
				printf("Wrong command\n");
				continue;
			}
			printf("Bulls: %c, Cows: %c%s\n", rep[0], rep[1], rep[0] == (WIN_BULLS + '0') ? "\nCongratulations!\n" : "");
			free(w);
			continue;
		}else
		if(strcmp(cmd, "list") == 0) {
			write_msg(fd_w, "l\n", 2);
			read_str(fd_r, rep, MAX_REPLY_SIZE);
			printf("%s\n", rep);
			continue;
		} else
		if(strcmp(cmd, "player") == 0) {
			write_msg(fd_w, "p\n", 2);
			read_str(fd_r, rep, MAX_REPLY_SIZE);
			printf("%s\n", rep);
			continue;
		} else
		if (cmd[0] == 'q') {
			if(game != NULL){
				printf("To leave your current game write the command <exit>\n");
				continue;
			}
			write_msg(fd_w, "q\n", 5);
			printf("Leaving the client...\n");
			return;
		} else {
			printf("Wrong command\n");
		}
	}
}

int main () {
	int fd_r = -1;
	int fd_w = -1;
	printf("Client started!\n");
	if((fd_r = open("/tmp/bulls_and_cows_sw0", O_RDONLY)) < 0){
		perror("Can't open pipe for reading");
		if(fd_r >= 0)
			close(fd_r);
		if(fd_w >= 0)
			close(fd_w);
		return 0;
	}
	char pl_r[MAX_PATH_NAME_SIZE];
	char pl_w[MAX_PATH_NAME_SIZE];
	read_str(fd_r, pl_w, MAX_PATH_NAME_SIZE);
	read_str(fd_r, pl_r, MAX_PATH_NAME_SIZE);
	close(fd_r);
//	printf("%s %s\n", pl_r, pl_w);
	if((fd_w = open(pl_w, O_WRONLY)) < 0){
		perror("Can't open pipe for writing");
		if(fd_r >= 0)
			close(fd_r);
		if(fd_w >= 0)
			close(fd_w);
		return 0;
	}
	if((fd_r = open(pl_r, O_RDONLY)) < 0){
		perror("Can't open pipe for reading");
		if(fd_r >= 0)
			close(fd_r);
		if(fd_w >= 0)
			close(fd_w);
		return 0;
	}
	process_cmd(fd_r, fd_w);

	if(fd_r >= 0)
		close(fd_r);
	if(fd_w >= 0)
		close(fd_w);
	return 0;
}