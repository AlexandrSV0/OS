#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "iout.h"



void print_num_in_data(char* data, int a, int DATA_COUNT) {
	if (a > 0 && a < 10) {
		data[DATA_COUNT] = '0' + a;
	}
}

void write_in_data(char* data,int* DATA_COUNT, float val) {
	int counter = *DATA_COUNT;
	if (val < 0) {
		data[counter] = '-';
		counter++;
		val *= -1;
	}
	float left;
	float right = modff(val, &left);

	//вывод целой части
	int l = left;
	int arr[100] = {'\0'};

	int arr_size = reverse(l, arr);
	if (l == 0) {
		data[counter] = '0';
		counter++;
	} else {
		for (int i = arr_size -1; i >= 0; i--) {
			print_num_in_data(data, arr[i], counter);
			counter++;
		}
	}
	data[counter] = '.';
	counter++;

	//вывод дробной часи

	int i = 0;
	while (right > 0) { //0,456
		float new = right*10; //4,56
		right = right*10;
		while (right > 1) right--; // по итогу right = 0,56
		float for_print = new - right; // 4,56 - 0,56
		print_num_in_data(data,for_print,counter); // вывод цифры 4
		counter++;
		//повтор цикла для 0,56 и тд
		i++;
		if (i == 4) {
			break; 
		} 
	}
	*DATA_COUNT = counter;
}


int main() {
	int N;
	my_read_int(STDIN_FILENO, &N);
	int DATA_COUNT = 0;
//	my_read_int(STDIN_FILENO, &N);
	float cur = 0.0, sec = 0.0, third = 0.0;
	int status_trunc = ftruncate(STDOUT_FILENO, sizeof(char)*100*N);
	if (status_trunc < 0) {
	//	my_print("FTRUNCATE ERROR\n");
		return -1;
	}
	char* data = mmap(0, sizeof(char)*100*N, PROT_WRITE, MAP_SHARED, STDOUT_FILENO, 0);

	int status = read_float(STDIN_FILENO, &cur);

	while (status == 0 && N > 0) {
		status = read_float(STDIN_FILENO, &sec);
		if (status == -1) //неверное значение
			return -1;
		if (status == -3) { //перенос строки после второго числа
	//		my_print("Incorrect type of commands in file\n");	
			return -3;
		}

		status = read_float(STDIN_FILENO, &third);

		if (status == -1) // неверное значение
			return -1;
		if (sec == 0 || third == 0) { // проверка деления на 0
	//		my_print("Error: division by 0 is forbidden!\n");
			return -4;
		}
		if (status == 0) { //отсутствует перенос строки после 3-го числа
	//		my_print("Wrong commadns! Line should looks like <number number number<endline>>\n");
			return -5;
		}
		float res1 = cur / sec; 
		float res2 = cur / third;


		write_in_data(data, &DATA_COUNT, res1);
		data[DATA_COUNT] = ' ';
		DATA_COUNT++;
		write_in_data(data, &DATA_COUNT, res2);
		data[DATA_COUNT] = '\n';
		DATA_COUNT++;
		N--;
		status = read_float(STDIN_FILENO, &cur);
	}

	msync(data, sizeof(char)*100*N, MS_SYNC);
	int err = munmap(data, sizeof(char)*100*N);
/*	if (err < 0) {
		my_print("MUNMAP ERROR\n");
		return -1;
	}*/

	return 0;

}