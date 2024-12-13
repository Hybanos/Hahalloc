#ifndef HAHALLOC_C
#define HAHALLOC_C

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>

// just a few colors
#define GRAY_GRAY printf("\033[30;100m");
#define GRAY printf("\033[90m");
#define RED printf("\033[91m");
#define GREEN printf("\033[92m");
#define RESET printf("\033[0m");
    
// Un-comment this line to print linked lists info
// #define DEBUG

#define bool char
#define true 1
#define false 0

#define PAGESIZE 4096

// metadata byte description :
// 00000000
//     │││└─ range is the root of a list
//     ││└── range is allocated
//     │└─── range is the start of a mapping
//     └──── oversize mapping

#define IS_ROOT 0x01
#define IS_ALLOCATED 0x02
#define IS_MAPPING_START 0x04
#define IS_OVERSIZE 0x08

// from 512B to 1 GB
#define SMALLEST_SEGMENT 1l << 8
#define RANGE_SEGMENTS 22

typedef struct range {
    struct range *prev;
    struct range *next;
    size_t size;
    unsigned short meta;
} range;

void *hahalloc(size_t);
void *rehehalloc(void *, size_t);
void *chahalloc(size_t);
void  frhehe(void *);
void  mem_lhihiks();

#endif