#include "eliterator.h"

elementIterator createIterator(off_t offsetInFile, uint32_t attrCount, uint64_t attrSize) {
    elementIterator iterator;
    iterator.offsetInFile = offsetInFile;
    iterator.allAttributesCount = attrCount;
    iterator.allAttributesSize = attrSize;
    iterator.currentPosition = 0;
    iterator.passedAttributesCount = 0;
    return iterator;
}

bool hasNext(elementIterator* iterator) {
    return iterator->allAttributesCount != iterator->passedAttributesCount;
}

elementsChunk next(zgdbFile* file, elementIterator* iterator) {
    char* buffer;
    size_t s;
    if(iterator->allAttributesSize < READ_BUFFER_SIZE) {
        s = iterator->allAttributesSize;
    } else {
        s = READ_BUFFER_SIZE;
    }
    buffer = malloc(s * sizeof(char));
    fseeko(file->file, (off_t)(iterator->offsetInFile + iterator->currentPosition), SEEK_SET);
    fread(buffer, sizeof(char), s * sizeof(char), file->file);

    size_t i = 0;

    while (true) {
        if(s - i < 1) {
            break;
        }
        uint8_t type = buffer[i];
        i++;
        if(s - i < 13) {
            break;
        }
        char key[13];
        for (int j = 0; j < 13; ++j) {
            key[j] = buffer[i + j];
        }
        i += 13;

        switch (type) {
            case TYPE_BOOLEAN:

                break;
            case TYPE_INT:

                break;
            case TYPE_DOUBLE:

                break;
            case TYPE_TEXT:

                break;
        }

    }
}