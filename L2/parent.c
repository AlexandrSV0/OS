#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include "iout.h"

int main(int argc, char *argv[]) {
	int lines;
	my_print("Enter number of lines in your text file:\n");
	my_read_int(&lines);

	int file = open(argv[1], 0);
	if (file == -1) {
		my_print("Can't open file\n");
		return 2;
	}
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if (pid == -1) {
		perror("Fork error");
		return -1;
	}
	if (pid != 0) {
		my_print("Child's process was created. Id is ");
		print_int(pid);
		// write id of child
		my_print("\n");
	} 
	int status = 0;
	wait(&status);
	if (WEXITSTATUS(status)) {
		return -5;
	}

	if (pid == 0) { 				// child process
		close(fd[0]);
		dup2(file, STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		execl("child", "", NULL);

	} else {						 // parent process
		int line_in_file = 0;
		float res1 = 0, res2 = 0;
		close(fd[1]);
		while (lines > 0) {
			read(fd[0], &line_in_file, sizeof(int));
			read(fd[0], &res1, sizeof(float));
			read(fd[0], &res2, sizeof(float));
			my_print("line "); print_int(line_in_file);
			//write int numb of line
			my_print(": res1 = "); print_float(res1);
			//write float res1
			my_print(", res2 = "); print_float(res2);
			//write float res2
			my_print("\n");
			
			lines--;
		} 
	}
	close(file);
	return 0;
}