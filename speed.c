#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "hahalloc.h"

void multiple_alloc_fixed_size();
void multiple_alloc_random_size();
void single_alloc();
void c_alloc();
void re_alloc();

int main() {

    multiple_alloc_random_size();
    multiple_alloc_fixed_size();
    single_alloc();
    c_alloc();
    re_alloc();

    return 0;
}

void single_alloc() {
    struct timeval tic, tac;
    FILE *f;
    f = fopen("py/single_alloc.txt", "w");

    for (double i = 1; i < (1l << 31); i *= 1.01) {
        printf("single alloc test: %d%% done (don't worry it speeds up)\r", (int)(i / (1l << 31) * 100));
        gettimeofday(&tic, NULL);
        for (int j = 0; j < 1024 * 32; j++) {
            int *ptr = hahalloc((size_t) i);
            frhehe(ptr);
        }
        gettimeofday(&tac, NULL);
        size_t haha_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        gettimeofday(&tic, NULL);
        for (int j = 0; j < 1024 * 32; j++) {
            int *ptr = malloc((size_t) i);
            free(ptr);
        }
        gettimeofday(&tac, NULL);
        size_t std_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);
        
        fprintf(f, "%lu %lu %lu\n",(size_t) i, haha_dt, std_dt);
    }

    printf("\n");
    fclose(f);
}

// One million 1024KiB allocs or frees, while keeping an average of buff_size / 2 pointers active.
void multiple_alloc_fixed_size() {
    struct timeval tic, tac;
    FILE *f;
    f = fopen("py/multiple_alloc_fixed_size.txt", "w");

    int MAX_BUFF = 200;
    int *buff[MAX_BUFF];

    for (int i = 0; i < MAX_BUFF; i++) {
        buff[i] = NULL;
    }

    for (int buff_size = 1; buff_size <= MAX_BUFF; buff_size++) {
        printf("multiple alloc fixed size, run %d\r", buff_size);

        size_t size = 1 << 20;
        
        // haha
        srand(4949492);
        gettimeofday(&tic, NULL);
        for (int i = 0; i < 1024 * 1024; i++) {
            int index = rand() % buff_size;

            if (buff[index] == NULL) {
                buff[index] = hahalloc(size);
            }
            else {
                frhehe(buff[index]);
                buff[index] = NULL;
            }
        }
        gettimeofday(&tac, NULL);
        size_t haha_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        for (int i = 0; i < buff_size; i++) {
            if (buff[i] != NULL) {
                frhehe(buff[i]);
                buff[i] = NULL;
            }
        }

        // std
        srand(4949492);
        gettimeofday(&tic, NULL);
        for (int i = 0; i < 1024 * 1024; i++) {
            int index = rand() % buff_size;

            if (buff[index] == NULL) {
                buff[index] = malloc(size);
            }
            else {
                free(buff[index]);
                buff[index] = NULL;
            }
        }
        gettimeofday(&tac, NULL);
        size_t std_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        fprintf(f, "%d %lu %lu\n", buff_size, haha_dt, std_dt);

        for (int i = 0; i < buff_size; i++) {
            if (buff[i] != NULL) {
                free(buff[i]);
                buff[i] = NULL;
            }
        }
    }
    printf("\n");
    fclose(f);
}

// One million random size allocs or frees, while keeping an average of buff_size / 2 pointers active.
void multiple_alloc_random_size() {
    struct timeval tic, tac;
    FILE *f;
    f = fopen("py/multiple_alloc_random_size.txt", "w");

    int MAX_BUFF = 200;
    int *buff[MAX_BUFF];

    for (int i = 0; i < MAX_BUFF; i++) {
        buff[i] = NULL;
    }

    for (int buff_size = 1; buff_size <= MAX_BUFF; buff_size++) {
        printf("multiple alloc random size, run %d\r", buff_size);

        srand(4949492);
        
        // haha
        gettimeofday(&tic, NULL);
        for (int i = 0; i < 1024 * 1024; i++) {
            int index = rand() % buff_size;

            if (buff[index] == NULL) {
                size_t size = (rand() % 128)  << (rand() % 10);
                buff[index] = hahalloc(size);
            }
            else {
                frhehe(buff[index]);
                buff[index] = NULL;
            }
        }
        gettimeofday(&tac, NULL);
        size_t haha_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        for (int i = 0; i < buff_size; i++) {
            if (buff[i] != NULL) {
                frhehe(buff[i]);
                buff[i] = NULL;
            }
        }

        // std
        srand(4949492);
        gettimeofday(&tic, NULL);
        for (int i = 0; i < 1024 * 1024; i++) {
            int index = rand() % buff_size;

            if (buff[index] == NULL) {
                size_t size = (rand() % 128)  << (rand() % 10);
                buff[index] = malloc(size);
            }
            else {
                free(buff[index]);
                buff[index] = NULL;
            }
        }
        gettimeofday(&tac, NULL);
        size_t std_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        fprintf(f, "%d %lu %lu\n", buff_size, haha_dt, std_dt);

        for (int i = 0; i < buff_size; i++) {
            if (buff[i] != NULL) {
                free(buff[i]);
                buff[i] = NULL;
            }
        }
    }
    printf("\n");
    fclose(f);
}

void re_alloc() {
    struct timeval tic, tac;
    FILE *f;
    f = fopen("py/re_alloc.txt", "w");

    for (size_t i = 1; i < 2000; i++) {
        printf("realloc test %lu\r", i);
        void *ptr = hahalloc(1);

        gettimeofday(&tic, NULL);
        for (int j = 1; j < 20000; j++) {
            ptr = rehehalloc(ptr, i*j);
        }
        gettimeofday(&tac, NULL);
        size_t haha_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);


        frhehe(ptr);
        ptr = malloc(1);

        gettimeofday(&tic, NULL);
        for (int j = 1; j < 20000; j++) {
            ptr = realloc(ptr, i*j);
        }
        gettimeofday(&tac, NULL);
        size_t std_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        free(ptr);
        fprintf(f, "%lu %lu %lu\n",(size_t) i, haha_dt, std_dt);
    }
    printf("\n");
    fclose(f);
}

void c_alloc() {
    struct timeval tic, tac;
    FILE *f;
    f = fopen("py/single_calloc.txt", "w");

    for (double i = 1; i < (1l << 30); i *= 1.2) {
        printf("single calloc test: %d%% done (don't worry it doesn't speed up)\r", (int)(i / (1l << 30) * 100));
        gettimeofday(&tic, NULL);
        for (int j = 0; j < 256; j++) {
            int *ptr = chahalloc((size_t) i);
            frhehe(ptr);
        }
        gettimeofday(&tac, NULL);
        size_t haha_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);

        gettimeofday(&tic, NULL);
        for (int j = 0; j < 256; j++) {
            int *ptr = malloc((size_t) i);
            free(ptr);
        }
        gettimeofday(&tac, NULL);
        size_t std_dt = (tac.tv_sec - tic.tv_sec) * 1000000 + (tac.tv_usec - tic.tv_usec);
        
        fprintf(f, "%lu %lu %lu\n",(size_t) i, haha_dt, std_dt);
    }

    printf("\n");
    fclose(f);
}