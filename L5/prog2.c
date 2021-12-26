#include <stdio.h>
#include <dlfcn.h>

void menu() {
	printf("1. Рассчет производной функции cos(x) в точке A с приращением deltaX\n");
	printf("USAGE: float Derivative(float A, float deltaX)\n\n");
	printf("2. Подсчёт наибольшего общего делителя для двух натуральных чисел A B\n");
	printf("USAGE: int GCF(int A, int B)\n\n");
}



int main() {
	float (*Derivative)(float, float);
	int (*GCF)(int, int);

	void* l1_handler = dlopen("./lib1.so", RTLD_LAZY);
	void* l2_handler = dlopen("./lib2.so", RTLD_LAZY);
	if (!l1_handler || !l2_handler) {
	    fprintf(stderr,"DLOPEN error: %s\n", dlerror());
	    return -1;
	}
	Derivative = dlsym(l1_handler,"Derivative");
	GCF = dlsym(l1_handler,"GCF");

	int ver = 0;
	int cmd;
	menu();
	while (scanf("%d", &cmd) != EOF) {
		if (cmd == 0) {
			ver ^= 1;
			if (ver == 0) {
				Derivative = dlsym(l1_handler,"Derivative");
				GCF = dlsym(l1_handler,"GCF");
			} else {
				Derivative = dlsym(l2_handler,"Derivative");
				GCF = dlsym(l2_handler,"GCF");
			}
			printf("Switched to version %d\n", ver + 1);
		} else if (cmd == 1) {
			float x, y;
			if (scanf("%f %f", &x, &y) != 2) {
				printf("Invalid arguments!\n");
				continue;
			}
			printf("%f\n", Derivative(x, y));
		} else if (cmd == 2) {
			int x, y;
			if (scanf("%d %d", &x, &y) != 2) {
				printf("Invalid arguments!\n");
				continue;
			}
			printf("%d\n", GCF(x, y));
		} else {
			printf("Invalid command!\n");
			menu();
		}

	}
	dlclose(l1_handler);
	dlclose(l2_handler);
}