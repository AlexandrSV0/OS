#include <math.h>
#include <stdbool.h>
#include <stdio.h>

//Рассчет производной функции cos(x) в точке A с приращением deltaX
float Derivative(float A, float deltaX) {
	float res = (cos(A+deltaX) - cos(A)) / deltaX;
	return res;
}

//Подсчёт наибольшего общего делителя для двух натуральных чисел A B
//Алгоритм Евклидa
int GCF(int A, int B) {
	while (A != B) {
		if (A > B) 
			A -= B;
		else B -= A;
	}
	return A;
}
