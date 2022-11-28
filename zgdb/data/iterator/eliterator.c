#include "eliterator.h"
#include "index/zgdbindex.h"

elementIterator createIterator(zgdbFile* file, document* doc) {
    elementIterator iterator;
    iterator.offsetInFile = (off_t) (getIndex(file, doc->header.indexAttached).offset + sizeof(documentHeader));
    iterator.allAttributesCount = doc->header.attrCount;
    iterator.allAttributesSize = doc->header.size - sizeof(documentHeader);
    iterator.passedAttributesCount = 0;
    iterator.buffer = NULL;
    return iterator;
}

void destroyIterator(elementIterator* iterator) {
    if(iterator->buffer != NULL)
        fclose(iterator->buffer);
}

bool hasNext(elementIterator* iterator) {
    return iterator->allAttributesCount != iterator->passedAttributesCount;
}

elementEntry next(zgdbFile* file, elementIterator* iterator, bool onlyOffset) {
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
                fread(&entry.element.booleanValue, sizeof(uint8_t), 1, iterator->buffer);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_INT: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(int32_t)) {
                fread(&entry.element.integerValue, sizeof(int32_t), 1, iterator->buffer);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_DOUBLE: {
            if (READ_BUFFER_SIZE - curPos >= sizeof(double)) {
                fread(&entry.element.doubleValue, sizeof(double), 1, iterator->buffer);
            } else {
                reqUpdateBuffer = true;
            }
            break;
        }
        case TYPE_TEXT: {
//            if (READ_BUFFER_SIZE - curPos >= sizeof(firstTextChunk)) {
//                fseeko(file->file, iterator->offsetInFile + ftello(iterator->buffer), SEEK_SET);
//                firstTextChunk firstChunk;
//                fread(&firstChunk, sizeof(firstTextChunk), 1, file->file);
//                fseeko(iterator->buffer, sizeof(firstTextChunk), SEEK_CUR);
//                off_t x = ftello(iterator->buffer);
//                div_t divRes = div((int) firstChunk.size, CHUNK_SIZE);
//                int chunks = divRes.quot;
//                if (divRes.rem != 0)
//                    chunks++;
//                printf("Chunks: %d\n", chunks);
//                int count = 0;
//                textChunk temp;
//                uint8_t chunkType;
//                uint32_t nextOffset = ftello(file->file) - sizeof(firstTextChunk) + firstChunk.nextOffset;
//                char buf[firstChunk.size];
//                off_t rec = 0;
//                uint64_t currentToast = 0;
//                toast toastTemp;
//                memset(buf, 0, firstChunk.size);
//                for (count = 0; count < chunks; ++count) {
//                    fseeko(file->file, nextOffset, SEEK_SET);
//                    fread(&chunkType, sizeof(uint8_t), 1, file->file);
//                    fread(&temp, sizeof(textChunk), 1, file->file);
//                    if (currentToast == 0) {
//                        if (READ_BUFFER_SIZE - curPos >= (sizeof(uint8_t) + sizeof(textChunk))) {
//                            fseeko(iterator->buffer,
//                                   (off_t) (ftello(iterator->buffer) + (sizeof(uint8_t) + sizeof(textChunk))),
//                                   SEEK_SET);
//                            x = ftello(iterator->buffer);
//                        } else {
//                            fseeko(iterator->buffer, (off_t) (ftello(iterator->buffer) -
//                                                              (sizeof(uint8_t) + 13 * sizeof(char) +
//                                                               sizeof(firstTextChunk))), SEEK_SET);
//                            reqUpdateBuffer = true;
//                            break;
//                        }
//                    }
//
//                    if (temp.toastIndex == currentToast) {
//                        nextOffset = ftello(file->file) - (sizeof(uint8_t) + sizeof(textChunk)) + temp.nextOffset;
//                        for (int i = 0; i < CHUNK_SIZE; ++i) {
//                            buf[CHUNK_SIZE * count + i] = temp.data[i];
//                        }
//                    } else {
//                        //TODO move to toast
//                        rec = ftello(file->file);
//                        zgdbIndex index;
//                        if (currentToast == 0) {
//                            index = getIndex(file, iterator->doc->header.firstToastIndex);
//                            currentToast = iterator->doc->header.firstToastIndex;
//                        } else {
//                            index = getIndex(file, toastTemp.nextToastIndex);
//                            currentToast = toastTemp.nextToastIndex;
//                        }
//                        fseeko(file->file, index.offset, SEEK_SET);
//                        fread(&toastTemp, sizeof(toast), 1, file->file);
//                        nextOffset = ftello(file->file) + temp.nextOffset;//TODO check
//                    }
//                }
//                entry.element.textValue.size = firstChunk.size + 1;
//                entry.element.textValue.data = malloc(entry.element.textValue.size);
//                strcpy(entry.element.textValue.data, buf);
//                if (rec != 0)
//                    fseeko(file->file, rec, SEEK_SET);
//                break;
//            } else {
//                fseeko(iterator->buffer, (off_t) (ftello(iterator->buffer) - (sizeof(uint8_t) + 13 * sizeof(char))),
//                       SEEK_SET);
//                reqUpdateBuffer = true;
//            }
            break;
        }
        default: {
            //TODO for chunks
            break;
        }
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