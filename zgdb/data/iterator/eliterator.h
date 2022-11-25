#ifndef ZGDBPROJECT_ELITERATOR_H
#define ZGDBPROJECT_ELITERATOR_H

#include <data/zgdbdata.h>

#define READ_BUFFER_SIZE 32 //TODO change

typedef struct elementIterator {
    off_t offsetInFile;
    uint32_t allAttributesCount;
    uint64_t allAttributesSize;
    uint64_t currentPosition;
    uint32_t passedAttributesCount;
} elementIterator;

typedef struct elementsChunk {
    uint32_t size;
    element* elements;
} elementsChunk;

elementIterator createIterator(off_t offsetInFile, uint32_t attrCount, uint64_t attrSize);

bool hasNext(elementIterator* iterator);

elementsChunk next(zgdbFile* file, elementIterator* iterator);

#endif
