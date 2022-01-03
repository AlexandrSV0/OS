#include "Message.h"
#include <unistd.h>
#include <stdio.h>

void read_str(int fd, char* str, int max_size){
	char symb;
	int len;
	int i = 0;
	while((len = read(fd, &symb, 1)) >= 0 && i < (max_size - 1)){
		if(len == 0)
			continue;
		if(symb == '\n')
			break;
		str[i++] = symb;
	}
	str[i] = 0;
}

int write_msg(int fd, char* buf, int size){
	int write_rvl;
	int written = 0;
	do{
		write_rvl = write(fd, buf + written, size - written);
		if(write_rvl < 0){
			perror("Write ERROR!");
			return 0;
		}
		written += write_rvl;
	} while(written < size);
	return 1;
}