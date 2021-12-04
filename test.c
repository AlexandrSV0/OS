// LR 3 Semin

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

pthread_mutex_t mutex;              


typedef struct {
    int p;
    int* arr;
    int size;
}data;

void resh_era(data* st) {
    int p = st->p;
    if (p == 0 || p == 1) {
        st->p++;
        return;
    }

    for (int i = 0; i < st->size; i++) {
        if (st->arr[i] % p == 0 && st->arr[i] != p) {
            st->arr[i] = 0;
        }
    }
    st->p++;
}

void* pthr_resh(void* arg) {
    
    pthread_mutex_lock(&mutex);
    data* st_ = (data*)arg;

    resh_era(st_);
    pthread_mutex_unlock(&mutex);
}

bool check_prostoe(data* st, int a) {
    for (int i = 0; i < st->size;i++) {
        if (st->arr[i] == a) 
            return true;
    }
    return false;
}

void scan_int(int* user, char c_user) {
    int res = 0;
    res = c_user - '0';
    int i = 10;
    char c = getchar();
    while(c != ' ' && c != '\n' && c != '\0' && c != EOF) {
        if (c >= '0' && c <= '9') {
            res = res*i + (c -'0'); 
            i *= 10;
        } else {
            printf("user oshibsya\n");
            break;
        }
        c = getchar();
    }
    putchar(c);
    *user = res;
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        printf("Указан неверный ключ\n");
    } else if(argc < 2) {
        printf("Не указан ключ\n");
        return -1;
    }
    int thrs = atoi(argv[1]);

    int N;
    printf("Введите размер массива:\n");
    scanf("%d", &N);

    data st;
    st.p = 0;
    st.size = N;
    int thr_size = thrs;
    if (thrs < N) {
        printf("Недостаточное количество потоков\n");
        return -1;
    }
    int* new_ = malloc(sizeof(int)*N);

    if (new_ == NULL) {
        return -1;
    }
    
    st.arr = new_;
    bool arr_bool[N];

    for (int i = 0; i < N; i++) {
        arr_bool[i] = false;
        st.arr[i] = i;
    }

/////////////////////////////////////////////////

    pthread_t thread[thr_size];
    pthread_mutex_init(&mutex, NULL);


//////////// выполнение алгоритма решето эратосфена
   

    int status;
    for (int i = 0; i < N; i++) {
        if (arr_bool[i] == true) {
            continue;
        }

        status = pthread_create(&thread[i], NULL, pthr_resh, (void*)&st);
        printf("Поток %ld создан\n", thread[i]);
        if (status != 0) {
            return -1;
        }
        arr_bool[i] = true;
    }
    int ret;


////////// ожидание выполнения всех потоков
    

    for (int i = 0; i <N; i++) {
        ret = pthread_join(thread[i], NULL);
        printf("Ожидание потока %ld\n", thread[i]);
        if (ret != 0) {
            return -1;
        }
    }

    printf("Преобразование массива с помощью ''решета'' Эратосфена выполнено\n");

    for (int i = 0; i < N; i++) {
        if (st.arr[i] == 0) {
            continue;
        } else {
            printf("%d \n", st.arr[i]);
        }
    }
//////////////// пользовательское взаимодействие
    bool first = true;
    while(true) {
        if (!first) {
            printf("Введите число от 1 до %d, которое хотите проверить на простоту\n", N-1);
            printf("Для завершения программы введите q\n");
        }
        char c = getchar();
        int user = 0;
        if (c == 'q') {
            break;
        } else if (!first && (c >= '0' && c <= '9')) {
            scan_int(&user, c);
        } else if (!first && (user >= N || user == 0 || user < 0)) {
            printf("Wrong value\n");
            continue;
        }
        bool res = false;
        if (!first) {
            res = check_prostoe(&st,user);
            if (res) 
                printf("Число %d простое\n", user);
            else 
                printf("Число %d непростое\n", user);
        }
            
        first = false;
    }

    free(st.arr);
    pthread_mutex_destroy(&mutex);
}