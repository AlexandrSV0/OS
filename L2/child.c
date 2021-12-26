#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include "iout.h"

typedef enum{
	read_suc,
	read_eol,
	read_wrong_value,
	read_eof,
} read_num_stat;


read_num_stat read_float(int fd, float* cur){
	bool dot_fnd = false;
	char c;
	*cur = 0;
	double i = 0.1;
	int res = read(fd, &c, sizeof(char));
	while(res > 0){
		if (c == '-') {
			i *= -1;
			res = read(fd, &c, sizeof(char));
			continue;
		}
		if(c == '\n') return read_eol;
		if(c == ' ') 
			break;
		if(((c < '0') || (c > '9')) && c != '.'){
			return read_wrong_value;
		}
		if (!dot_fnd) {
			if(c == '.') 
				dot_fnd = true;
			else {
				*cur = *cur * 10 + c - '0';
			}
		} else {
			if(c == '.')
				return read_wrong_value;

			*cur = *cur + i * (c - '0');
			i /= 10;
		}
		res = read(fd, &c, sizeof(char));
	}
	if(res == 0) 
		return read_eof;
	
	return read_suc;
}


int main() {
	float cur = 0, sec = 0.0, third = 0.0;
	int line_in_file = 0;
	read_num_stat status = read_float(STDIN_FILENO, &cur);
	while (status == read_eol || status == read_suc) {
		line_in_file++;
		status = read_float(STDIN_FILENO, &sec);
		if (status == read_wrong_value) 
			return -1;
		if (status == read_eof) {
			my_print("Wrong commands! Line should looks like <number number number<endline>>\n");
			return -2;
		}
		if (status == read_eol){
			my_print("Incorrect type of commands in file\n");	
			return -3;
		}

		status = read_float(STDIN_FILENO, &third);

		if (status == read_wrong_value)
			return -1;
		if (sec == 0 || third == 0) {
			my_print("Error: division by 0 is forbidden!\n");
			return -4;
		}
		if (status == read_suc) {
			my_print("Wrong commadns! Line should looks like <number number number<endline>>\n");
			return -5;
		}
		float res1 = cur / sec; 
		float res2 = cur / third;

		write(STDOUT_FILENO, &line_in_file, sizeof(int));
		write(STDOUT_FILENO, &res1, sizeof(float));
		write(STDOUT_FILENO, &res2, sizeof(float));	
		status = read_float(STDIN_FILENO, &cur);
	}
	if (status == read_wrong_value || status == read_eol) {
		return -1;
	}
	return 0;

}