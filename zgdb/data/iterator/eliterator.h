#ifndef ZGDBPROJECT_ELITERATOR_H
#define ZGDBPROJECT_ELITERATOR_H

#include <data/zgdbdata.h>
#ifdef __linux__
#include <stdio.h>
#endif
#ifdef __MINGW32__
#include "fmemopen_windows/libfmemopen.h"
#endif

#define READ_BUFFER_SIZE 64 //TODO change

typedef struct elementIterator {
    off_t offsetInFile;
    document* doc;
    uint32_t allAttributesCount;
    uint64_t allAttributesSize;
    uint32_t passedAttributesCount;
    FILE* buffer;
} elementIterator;

typedef struct elementEntry {
    element element;
    off_t offsetInDocument;
} elementEntry;

elementIterator createIterator(zgdbFile* file, document* doc);

void destroyIterator(elementIterator* iterator);

bool hasNext(elementIterator* iterator);

elementEntry next(zgdbFile* file, elementIterator* iterator, bool reqData);

#endif
