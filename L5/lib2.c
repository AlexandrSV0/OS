#include <math.h>


//Рассчет производной функции cos(x) в точке A с приращением deltaX
float Derivative(float A, float deltaX) {
	float res = (cos(A+deltaX) - cos(A-deltaX)) / (2*deltaX);
	return res;
}

//Подсчёт наибольшего общего делителя для двух натуральных чисел A B
//Наивный алгоритм.
int GCF(int A, int B) {
	int prev_nod = 0;
	int nod = 1;
	while (nod < A && nod < B) {
		if (A % nod == 0 && B % nod == 0) {
			prev_nod = nod;
		}
		nod++;
	}
	return prev_nod;
}