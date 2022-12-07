#include "eliterator.h"
#include "index/zgdbindex.h"

elementIterator createElIterator(zgdbFile* file, document* doc) {
    elementIterator iterator;
    iterator.doc = doc;
    iterator.offsetInFile = (off_t) (getIndex(file, doc->header.indexAttached).offset + sizeof(documentHeader));
    iterator.allAttributesCount = doc->header.attrCount;
    iterator.allAttributesSize = doc->header.size - sizeof(documentHeader);
    iterator.passedAttributesCount = 0;
    iterator.buffer = NULL;
    return iterator;
}

void destroyElIterator(elementIterator* iterator) {
    if(iterator->buffer != NULL)
        fclose(iterator->buffer);
}

bool hasNextEl(elementIterator* iterator) {
    return iterator->allAttributesCount != iterator->passedAttributesCount;
}

elementEntry nextEl(zgdbFile* file, elementIterator* iterator, bool reqData) {
    elementEntry entry;
    char bufTemp[READ_BUFFER_SIZE];
    if (iterator->buffer == NULL) {
        fseeko(file->file, iterator->offsetInFile, SEEK_SET);
        fread(&bufTemp, sizeof(char), READ_BUFFER_SIZE, file->file);
        iterator->buffer = fmemopen(&bufTemp, sizeof(char) * READ_BUFFER_SIZE, "r");
    }
    off_t curPos = 0;
    off_t prevPos = 0;
    bool reqUpdateBuffer = false;
    uint8_t type;

    updateBuffer:
    if(reqUpdateBuffer) {
        off_t offset = iterator->offsetInFile;
        fclose(iterator->buffer);
        fseeko(file->file, offset, SEEK_SET);
        fread(&bufTemp, sizeof(char), READ_BUFFER_SIZE, file->file);
        iterator->buffer = fmemopen(&bufTemp, sizeof(char) * READ_BUFFER_SIZE, "r");
        reqUpdateBuffer = false;
    }


    curPos = ftello(iterator->buffer);
    prevPos = curPos;
    if(READ_BUFFER_SIZE - curPos >= sizeof(uint8_t)) {
        fread(&type, sizeof(uint8_t), 1, iterator->buffer);
        entry.element.type = type;
    } else {
        reqUpdateBuffer = true;
        goto updateBuffer;
    }

    curPos = ftello(iterator->buffer);
    if(READ_BUFFER_SIZE - curPos >= 13 * sizeof(char)) {
        fread(&entry.element.key, 13 * sizeof(char), 1, iterator->buffer);
    } else {
        reqUpdateBuffer = true;
        goto updateBuffer;
    }

    curPos = ftello(iterator->buffer);
    switch (type) {
        case TYPE_BOOLEAN: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(uint8_t)) {
                if(reqData)
                    fread(&entry.element.booleanValue, sizeof(uint8_t), 1, iterator->buffer);
                else
                    fseeko(iterator->buffer, sizeof(uint8_t), SEEK_CUR);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_INT: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(int32_t)) {
                if(reqData)
                    fread(&entry.element.integerValue, sizeof(int32_t), 1, iterator->buffer);
                else
                    fseeko(iterator->buffer, sizeof(int32_t), SEEK_CUR);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_DOUBLE: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(double)) {
                if(reqData)
                    fread(&entry.element.doubleValue, sizeof(double), 1, iterator->buffer);
                else
                    fseeko(iterator->buffer, sizeof(double), SEEK_CUR);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_TEXT: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(firstTextChunk)) {
                if(reqData) {
                    firstTextChunk firstChunk;
                    fread(&firstChunk, sizeof(firstTextChunk), 1, iterator->buffer);

                    div_t divRes = div((int) firstChunk.size, CHUNK_SIZE);
                    int chunks = divRes.quot;
                    if(divRes.rem != 0)
                        chunks++;
                    printf("Chunks: %d\n", chunks);
                    zgdbIndex indexToast = getIndex(file, iterator->doc->header.firstToastIndex);
                    toast tempToast;
                    fseeko(file->file, indexToast.offset, SEEK_SET);
                    fread(&tempToast, sizeof(toast), 1, file->file);
                    char buf[firstChunk.size];
                    memset(buf, 0, firstChunk.size);
                    textChunk tempChunk;
                    uint8_t chunkType;
                    off_t nextOffset = firstChunk.offsetInToast;

                    for (size_t count = 0; count < chunks; ++count) {
                        fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                        memset(tempChunk.data, 0, CHUNK_SIZE);

                        fread(&chunkType, sizeof(uint8_t), 1, file->file);
                        fread(&tempChunk, sizeof(textChunk), 1, file->file);
                        if(count == chunks - 1) {
                            for (int i = 0; i < divRes.rem; ++i) {
                                buf[CHUNK_SIZE * count + i] = tempChunk.data[i];
                            }
                        } else {
                            for (int i = 0; i < CHUNK_SIZE; ++i) {
                                buf[CHUNK_SIZE * count + i] = tempChunk.data[i];
                            }
                        }
                        if(tempChunk.toastIndex == tempToast.indexAttached) {
                            nextOffset = tempChunk.nextOffset;
                        } else {
                            indexToast = getIndex(file, tempChunk.toastIndex);
                            fseeko(file->file, indexToast.offset, SEEK_SET);
                            fread(&tempToast, sizeof(toast), 1, file->file);
                            nextOffset = tempChunk.nextOffset;
                        }
                    }

                    entry.element.textValue.size = firstChunk.size;
                    entry.element.textValue.data = malloc(entry.element.textValue.size);
                    strcpy(entry.element.textValue.data, buf);
                } else {
                    fseeko(iterator->buffer, sizeof(firstTextChunk), SEEK_CUR);
                }
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        default:
            break;
    }

    if(reqUpdateBuffer) {
        goto updateBuffer;
    }

    entry.offsetInDocument = iterator->offsetInFile;
    off_t i = ftello(iterator->buffer);
    iterator->offsetInFile += (i - prevPos);
    iterator->passedAttributesCount++;
    return entry;
}