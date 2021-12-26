#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "iout.h"

int main(int argc, char *argv[]) {
	int lines;
	my_print("Enter number of lines in your text file:\n");
	int stat = my_read_int(0, &lines);
	if (stat < 0) {
		return -1;
	}

	if (argc != 2) {
		my_print("USAGE: ./a.out <filename>\n");
		return -1;
	}
	int N = lines;
	
	//открытие входного файла

	int input = open(argv[1], O_RDWR);
	if (input == -1) {
		my_print("Can't open file\n");
		return -2;
	}


	//временный файл
	char tmp_name[] = "tmpXXXXXX";
	if (mkstemp(tmp_name) < 0) {
		my_print("Can't create temporary file\n");
		return -3;
	}
	
	int fd_tmp = open(tmp_name, O_RDWR);
	if (fd_tmp < 0) {
		my_print("Can't open temporary file\n");
		return -2;
	}

	//разделение процессов
	pid_t pid = fork();
	if (pid == -1) {
		perror("Fork error");
		return -1;
	}
	if (pid != 0) {
		my_print("Child's process was created. Id is ");
		print_int(pid);
		my_print("\n");
	} 

	//ожидание выполнения дочернего процесса
	int status = 0;
	wait(&status);
	if (WEXITSTATUS(status)) {
		return -5;
	}

	if (pid == 0) { 				// child process
		my_print("In child\n");
		dup2(input, STDIN_FILENO);
		dup2(fd_tmp, STDOUT_FILENO);
		execl("child", "", NULL);
	} else {						 // parent process
		int line_in_file = 1;
		my_print("In parent\n");

		char* data = mmap(0, sizeof(char)*100*N,PROT_READ, MAP_SHARED, fd_tmp, 0);
		int DATA_COUNT = 0;
		while (lines > 0) {
			my_print("line "); print_int(line_in_file);
			//write int numb of line
			my_print(": res1 = ");
			if (data[DATA_COUNT] == '\n') DATA_COUNT++;
			for (DATA_COUNT; data[DATA_COUNT] != ' '; DATA_COUNT++) {
				write(1, &data[DATA_COUNT], sizeof(char));
			}

			//write float res1
			my_print(", res2 = ");
			for (DATA_COUNT; data[DATA_COUNT] != '\n' && data[DATA_COUNT] != '\0'; DATA_COUNT++) {
				write(1, &data[DATA_COUNT], sizeof(char));
			}
			//write float res2
			my_print("\n");
			line_in_file++;
			lines--;
		} 
		int err = munmap(data, sizeof(char)*100*N);
		if (err < 0) {
			my_print("MUNMAP ERROR\n");
		}
	}
	close(input);
	unlink(tmp_name);
	return 0;
}