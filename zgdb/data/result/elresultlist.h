#ifndef ZGDBPROJECT_ELRESULTLIST_H
#define ZGDBPROJECT_ELRESULTLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "data/zgdbdata.h"

typedef struct eLresult eLresult;

typedef struct eLresult {
    document document;
    element el;
    eLresult* next;
    eLresult* prev;
} eLresult;

typedef struct eLresultList {
    struct eLresult* head;
    struct eLresult* tail;
    uint64_t size: 40;
} eLresultList;

eLresultList createElResultList();

eLresult* getNextElOf(eLresult* of);

eLresult* getPrevElOf(eLresult* of);

void destroyElResultList(eLresultList* list);

void insertElResult(eLresultList* list, document document, element el);

void printElResultList(eLresultList* list);

#endif
