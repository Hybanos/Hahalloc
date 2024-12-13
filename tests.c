#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include "hahalloc.h"

void alloc_null();
void alloc_test();
void alloc_big();
void *alloc_lots();
void custom_test();
void realloc_test();
void thread_lock();

int main() {

    alloc_null();
    alloc_test();
    alloc_big();
    alloc_lots();
    custom_test();
    realloc_test();
    thread_lock();

    hahalloc(200000);

    return 0;
}

void alloc_null() {
    int *ptr = hahalloc(0);
    assert(ptr == NULL && "got a non-null pointer for size 0 requested");

    // fucking die
    frhehe(NULL);
}

void alloc_test() {
    int *ptr1, *ptr2, *ptr3;
    ptr1 = (int *) hahalloc(10 * sizeof(int));
    ptr2 = (int *) hahalloc(100 * sizeof(int));
    ptr3 = (int *) hahalloc(50 * sizeof(int));

    assert(ptr1 != NULL && "Got a null pointer from alloc");
    assert(ptr2 != NULL && "Got a null pointer from alloc");
    assert(ptr3 != NULL && "Got a null pointer from alloc");

    for (int i = 0; i < 10; i++) {
        ptr1[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        assert(ptr1[i] == i && "Allocated value changed");
    }

    frhehe(ptr2);
    frhehe(ptr3);
    frhehe(ptr1);
}

void alloc_big() {
    char *ptr = hahalloc(1000000000 * sizeof(char));
    assert(ptr != NULL && "Couldn't alloc a big range");
    frhehe(ptr);
}

void *alloc_lots() {
    // 2000 allocs of 1MB
    size_t *ptrs = hahalloc(2000 * sizeof(size_t));

    for (int i = 0; i < 2000; i++) {
        ptrs[i] = (size_t) hahalloc(1 << 20);
    }
    for (int i = 0; i < 2000; i++) {
        frhehe( (void *) ptrs[i]);
    }

    frhehe(ptrs);
}

void custom_test() {
    int *ptrs[10];
    for (int i = 0; i < 10; i+=2) {
        ptrs[i] = hahalloc(2000);
    }
    for (int i = 0; i < 10; i+=2) {
        frhehe(ptrs[i]);
    }
}

void realloc_test() {
    for (int n = 100; n < 10000; n*=2) {
        int* ptr = hahalloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) {
            ptr[i] = i;
        }

        ptr = rehehalloc(ptr, sizeof(int) * (n * n));
        for (int i = 0; i < n; i++) {
            assert(ptr[i] == i && "Realloc fail.");
        }
        frhehe(ptr);
    }

    int *ptr = hahalloc(sizeof(int) * 100);
    for (int i = 0; i < 100; i++) {
        ptr[i] = i;
    }

    ptr = rehehalloc(ptr, sizeof(int) * 50);

    for (int i = 0; i < 50; i++) {
        assert(ptr[i] == i && "Realloc fail.");
    }    

    frhehe(ptr);
}

void thread_lock() {

    // For some reason, if the thread count exceeds 4 (on my 4 core cpu)
    // everything locks and the world ends :(
    int count = sysconf(_SC_NPROCESSORS_ONLN) / 2;

    pthread_t threads[count];
    // for (int j = 0; j < 1000; j++) {
    //     printf("%d\n", j);
        for (int i = 0; i < count; i++) {
            pthread_create(&threads[i], NULL, alloc_lots, NULL);
        }

        for (int i = 0; i < count; i++) {
            pthread_join(threads[i], NULL);
        }
    // }
}