#ifndef ZGDBPROJECT_FREELIST_H
#define ZGDBPROJECT_FREELIST_H

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((packed)) node node;

typedef struct __attribute__((packed)) node {
    uint64_t indexOrder: 40;
    uint64_t blockSize: 40;
    node* next;
    node* prev;
} node;

typedef struct __attribute__((packed)) freeIndexesList {
    struct node* head;
    struct node* tail;
    uint64_t indexesCount: 40;
    uint64_t newIndexesCount: 40;
} freeIndexesList;

freeIndexesList* createIndexesList();

void insertNewIndex(freeIndexesList* list, uint64_t indexOrder);

void insertDeadIndex(freeIndexesList* list, uint64_t indexOrder, uint64_t blockSize);

node* findRelevantIndex(freeIndexesList* list, uint64_t reqSize);

void printList(freeIndexesList* list);

#endif
