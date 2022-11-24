#ifndef ZGDBPROJECT_FREELIST_H
#define ZGDBPROJECT_FREELIST_H
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdint.h>

typedef struct node node;

typedef struct node {
    uint64_t indexOrder: 40;
    off_t blockSize;
    node* next;
    node* prev;
} node;

typedef struct freeIndexesList {
    struct node* head;
    struct node* tail;
    uint64_t indexesCount: 40;
    uint64_t newIndexesCount: 40;
} freeIndexesList;

typedef struct relevantIndexMeta {
    uint64_t indexOrder: 40;
    off_t blockSize;
} relevantIndexMeta;

freeIndexesList createIndexesList();

void destroyIndexesList(freeIndexesList* list);

void insertNewIndex(freeIndexesList* list, uint64_t indexOrder);

void insertDeadIndex(freeIndexesList* list, uint64_t indexOrder, off_t blockSize);

relevantIndexMeta* findRelevantIndex(freeIndexesList* list, off_t reqSize);

void printFreeIndexesList(freeIndexesList* list);

#endif
