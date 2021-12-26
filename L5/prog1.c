#include <stdio.h>
#include "functions.h"

void menu() {
	printf("1. Рассчет производной функции cos(x) в точке A с приращением deltaX\n");
	printf("USAGE: float Derivative(float A, float deltaX)\n\n");
	printf("2. Подсчёт наибольшего общего делителя для двух натуральных чисел A B\n");
	printf("USAGE: int GCF(int A, int B)\n\n");
}


int main() {
	int cmd;
	menu();
	while(scanf("%d", &cmd) != EOF) {
		if (cmd == 1) {
			float x, y;
			if(scanf("%f %f", &x, &y) != 2) {
				printf("Invalid arguments!\n");
				continue;
			} 
			printf("%f\n", Derivative(x, y));
		} else if (cmd == 2) {
			int x, y;
			if(scanf("%d%d", &x, &y) != 2) {
				printf("Invalid arguments!\n");
				continue;
			} 
			printf("%d\n", GCF(x, y));
		} else {
			printf("Invalid command!\n");
			menu();
		}
	}
}