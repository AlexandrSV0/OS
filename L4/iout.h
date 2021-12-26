#ifndef IOUT_H
#define IOUT_H

#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

void my_print(char str[1024]) {
	write(1, str, strlen(str));
}

int my_read_int(int fd, int* a) {
	int res = 0;
	char c;
	read(fd, &c, sizeof(char));
	while(c != '\n' && c != EOF && c != ' ') {
		if (c < '0' || c > '9') {
//				my_print("Incorrect input\n");
				return -1;
			}
		res = res*10 + (c -'0');
		read(fd, &c, sizeof(char));
	}
	*a = res;
	return 0;
}

int read_2_floats(char* data,int* i, int data_size, float* fst, float* sec) {
	bool read_fst = true, read_sec = false;
	int count = *i;
	while(read_fst || read_sec) {
		bool dot_fnd = false;
		bool mines = false;
		char c;
		float cur = 0;
		double i = 0.1;
		c = data[count];
		while(data[count] != ' ' && data[count] != '\n' && data[count] != EOF) {
			if (c == '-') {
				mines = true;
				count++;
				c = data[count];
				continue;
			}

			if (((c < '0') || (c > '9')) && c != '.'){	
				return -1;
			}
			if (!dot_fnd) {
				if(c == '.') 
					dot_fnd = true;
				else {
					cur = cur * 10 + c - '0';
				}
			} else {
				if(c == '.')
					return -1;

				cur = cur + i * (c - '0');
				i /= 10;
			}
			if (count == data_size - 1) {
				printf("konec data\n");
			}
			count++;
			c = data[count];
		}
		count++;
		if (mines) {
			cur *= -1;
			mines = false;
		}
		if (read_fst) {
			*fst = cur;
			read_fst = false;
			read_sec = true;
			continue;
		}
		if (read_sec) {
			*sec = cur;
			read_sec = false;
			continue;
		}
	}

	*i = count;
	return 0;
}

int read_float(int fd, float* cur){
	bool dot_fnd = false;
	bool mines = false;
	char c;
	*cur = 0;
	double i = 0.1;
	int res = read(fd, &c, sizeof(char));
	while(res > 0){
		if (c == '-') {
			mines = true;
			res = read(fd, &c, sizeof(char));
			continue;
		}
		if(c == '\n') return -3;
		if(c == ' ') 
			break;
		if(((c < '0') || (c > '9')) && c != '.'){
			return -1;
		}
		if (!dot_fnd) {
			if(c == '.') 
				dot_fnd = true;
			else {
				*cur = *cur * 10 + c - '0';
			}
		} else {
			if(c == '.')
				return -1;

			*cur = *cur + i * (c - '0');
			i /= 10;
		}
		res = read(fd, &c, sizeof(char));
	}
	if (mines) {
		*cur *= -1;
	}
	if(res == 0) 
		return 0;
	return 0;
}

int reverse(int x, int* l) {
	int i = 0;
	while (x > 0) {
		int a = x % 10;
		l[i] = a;
		i++;
		x /= 10;
	}
	return i;
}

void print_num(int a) {
	char* num;
	if (a == 0) num = "0";
	if (a == 1) num = "1";
	if (a == 2) num = "2";
	if (a == 3) num = "3";
	if (a == 4) num = "4";
	if (a == 5) num = "5";
	if (a == 6) num = "6";
	if (a == 7) num = "7";
	if (a == 8) num = "8";
	if (a == 9) num = "9";
	write(1, num, sizeof(char));
}

void print_int(int x) {
	int data_int[100];
	int data_sz = reverse(x, data_int);
	if (x == 0) {
		print_num(x);
	}
	for (int i = 0; i < data_sz; i++) {
		print_num(data_int[i]);
	}
}

void print_float(float x) {
	if (x < 0) {
		write(1, "-", sizeof(char));
		x *= -1;
	}
	float left;
	float right = modff(x, &left);

	//вывод целой части
	int q = left;
	print_int(q);
	write(1, ".", sizeof(char));

	//вывод дробной часи
	int i = 0;
	while (right > 0) { //0,456
		float new = right*10; //4,56
		right = right*10;
		while (right > 1) right--; // по итогу right = 0,56
		float for_print = new - right; // 4,56 - 0,56
		print_num(for_print); // вывод цифры 4
		//повтор цикла для 0,56 и тд
		i++;
		if (i == 5) {
			break; 
		} 
	}
}

#endif