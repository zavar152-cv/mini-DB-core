#ifndef ZGDBPROJECT_RESULTLIST_H
#define ZGDBPROJECT_RESULTLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "data/zgdbdata.h"

typedef struct result result;

typedef struct result {
    document document;
    result* next;
    result* prev;
} result;

typedef struct resultList {
    struct result* head;
    struct result* tail;
    uint64_t size: 40;
} resultList;

resultList createResultList();

result* getNextDocOf(result* of);

result* getPrevDocOf(result* of);

void destroyResultList(resultList* list);

void insertResult(resultList* list, document document);

void printResultList(resultList* list);

#endif
