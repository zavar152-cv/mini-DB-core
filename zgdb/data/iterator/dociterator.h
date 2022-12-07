#ifndef ZGDBPROJECT_DOCITERATOR_H
#define ZGDBPROJECT_DOCITERATOR_H

#include <data/zgdbdata.h>
#include "data/treestack/treestack.h"

typedef struct documentIterator {
    bool stop;
    nodeEntry next;
    treeStack* pStack;

    documentHeader header;
    document document;
    nodeEntry temp;
} documentIterator;

documentIterator createDocIterator(zgdbFile* file, uint64_t order, uint64_t orderParent);

void destroyDocIterator(documentIterator* iterator);

bool hasNextDoc(documentIterator* iterator);

document nextDoc(zgdbFile* file, documentIterator* iterator);

#endif
