#include "zgdb.h"

#ifdef __linux__

#endif
#ifdef __MINGW32__
#include "windows.h"
#include "psapi.h"
#endif

void createRootDocument(zgdbFile* file, off_t offset);

zgdbFile* init(const char* path) {
    zgdbFile* pFile = loadOrCreateZgdbFile(path);
    if (pFile->zgdbHeader.indexCount == 0) {
        createIndexes(pFile, INDEX_INITIAL_CAPACITY);
        off_t offset = (off_t) (pFile->zgdbHeader.indexCount * sizeof(zgdbIndex) + sizeof(zgdbHeader));
        createRootDocument(pFile, offset);
        attachIndexToBlock(pFile, 0, offset);
        for (int i = 1; i < INDEX_INITIAL_CAPACITY; ++i) {
            insertNewIndex(&(pFile->freeList), i);
        }
    }
    pFile->pIndexesMmap = (char*) mmap(NULL, pFile->zgdbHeader.indexCount * sizeof(zgdbIndex),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED,
                                       fileno(pFile->file), 0) + sizeof(zgdbHeader);
    return pFile;
}

bool finish(zgdbFile* file) {
    munmap(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex));
    return closeZgdbFile(file);
}

void printDocumentElements(zgdbFile* file, document document) {
    zgdbIndex index = getIndex(file, document.header.indexAttached);
    fseeko(file->file, index.offset, SEEK_SET);
    fseeko(file->file, sizeof(documentHeader), SEEK_CUR);
    for (int i = 0; i < document.header.attrCount; ++i) {
        element* pElement = readElement(file, document);
        printf("EName: %s\n", pElement->key);
        printf("EType: %hhu\n", pElement->type);
        printf("EValue: ");
        switch (pElement->type) {
            case TYPE_BOOLEAN:
                printf("%hhu\n", pElement->booleanValue);
                break;
            case TYPE_INT:
                printf("%d\n", pElement->integerValue);
                break;
            case TYPE_DOUBLE:
                printf("%f\n", pElement->doubleValue);
                break;
            case TYPE_TEXT: {
                printf("%s\n", pElement->textValue.data);
                free(pElement->textValue.data);
                break;
            }
        }
        free(pElement);
        printf("\n");
    }
}

createStatus createDocument(zgdbFile* file, const char* name, documentSchema* schema, document parent) {
    documentHeader parentHeader = getDocumentHeader(file, parent.header.indexAttached);
    off_t docSize = sizeof(documentHeader);
    if(schema->capacity != schema->size) {
        printf("Invalid schema\n");
        return OUT_OF_INDEX;
    }

    if(file->freeList.newIndexesCount <= INDEX_INITIAL_CAPACITY/4) {
        printf("Needed to expand indexes\n");
    }

    docSize += schema->sizeOfElements;

    uint64_t toastIndex = 0;
    if(schema->reqToast) {
        relevantIndexMeta* pRelevantIndexMetaToast = findRelevantIndex(&file->freeList, 2 * schema->minToastCapacity * (sizeof(uint8_t) + sizeof(textChunk)));
        zgdbIndex indexToAttachToast = getIndex(file, pRelevantIndexMetaToast->indexOrder);
        off_t offsetToast = file->zgdbHeader.fileSize;
        if(indexToAttachToast.flag == INDEX_DEAD)
            offsetToast = indexToAttachToast.offset;
        else if(indexToAttachToast.flag == INDEX_NEW)
            offsetToast = file->zgdbHeader.fileSize;
        fseeko(file->file, offsetToast, SEEK_SET);
        uint64_t cap = pRelevantIndexMetaToast->blockSize == 0 ? 2*schema->minToastCapacity*(sizeof(uint8_t) + sizeof(textChunk)) + sizeof(toast) : pRelevantIndexMetaToast->blockSize;
        toast toastBlock = {.blockType = TOAST, .capacity = cap,
                .used = 0, .indexAttached = pRelevantIndexMetaToast->indexOrder, .nextToastIndex = 0};
        fwrite(&toastBlock, sizeof(toast), 1, file->file);
        char buf[cap];
        memset(&buf, 0, cap);
        fwrite(&buf, cap, 1, file->file);
        toastIndex = pRelevantIndexMetaToast->indexOrder;
        attachIndexToBlock(file, pRelevantIndexMetaToast->indexOrder, offsetToast);
        if(pRelevantIndexMetaToast->blockSize == 0)
            file->zgdbHeader.fileSize += (off_t) cap;
        saveHeader(file);
        free(pRelevantIndexMetaToast);
    }

    relevantIndexMeta* pRelevantIndexMeta = findRelevantIndex(&file->freeList, docSize);
    zgdbIndex indexToAttach = getIndex(file, pRelevantIndexMeta->indexOrder);

    off_t offset = file->zgdbHeader.fileSize;
    if(indexToAttach.flag == INDEX_DEAD)
        offset = indexToAttach.offset;
    else if(indexToAttach.flag == INDEX_NEW)
        offset = file->zgdbHeader.fileSize;

    fseeko(file->file, offset, SEEK_SET);
    fseeko(file->file, sizeof(documentHeader), SEEK_CUR);
    for (int i = 0; i < schema->capacity; ++i) {
        element cur = schema->elements[i];
        writeElement(file, cur, toastIndex);
    }
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    uint64_t cap = pRelevantIndexMeta->blockSize == 0 ? docSize : pRelevantIndexMeta->blockSize;
    documentHeader header = {.id = id, .size = docSize, .capacity = cap,
            .attrCount = schema->capacity, .indexAttached = pRelevantIndexMeta->indexOrder, .indexBrother = parentHeader.indexSon,
            .indexSon = 0, .firstToastIndex = toastIndex, .blockType = DOCUMENT};
    strcpy(header.name, name);
    attachIndexToBlock(file, pRelevantIndexMeta->indexOrder, offset);
    fwrite(&header, sizeof(documentHeader), 1, file->file);
    if(pRelevantIndexMeta->blockSize == 0)
        file->zgdbHeader.fileSize += docSize;
    file->zgdbHeader.nodes++;
    saveHeader(file);

    parentHeader.indexSon = pRelevantIndexMeta->indexOrder;
    fseeko(file->file, getIndex(file, parent.header.indexAttached).offset, SEEK_SET);
    fwrite(&parentHeader, sizeof(documentHeader), 1, file->file);
    free(pRelevantIndexMeta);
    return CREATE_OK;
}

void del(document document, zgdbFile* file) {
    if(!document.isRoot) {
        killIndex(file, document.header.indexAttached);
        insertDeadIndex(&(file->freeList), document.header.indexAttached, document.header.capacity);
        file->zgdbHeader.nodes--;
        saveHeader(file);
    }
}

void deleteDocument(zgdbFile* file, document doc) {
    documentHeader parentHeader = getDocumentHeader(file, doc.indexParent);
    document parent;
    parent.header = parentHeader;
    parent.isRoot = isRootDocumentHeader(parentHeader);
    parent.indexParent = 0;//it doesn't matter

    resultList pList = createResultList();
    insertResult(&pList, parent);

    document tempD;
    documentHeader tempHeader;
    uint64_t tempIndex = parentHeader.indexSon;
    while(tempIndex != 0) {
        tempHeader = getDocumentHeader(file, tempIndex);
        tempD.header = tempHeader;
        tempD.isRoot = isRootDocumentHeader(tempHeader);
        tempD.indexParent = doc.header.indexAttached;
        tempIndex = tempHeader.indexBrother;
        insertResult(&pList, tempD);
        if(doc.header.indexAttached == tempHeader.indexAttached)
            break;
    }

    result* forDelete = pList.tail;
    result* par = pList.head;

    if(par->document.header.indexSon == forDelete->document.header.indexAttached) {
        document p = par->document;
        p.header.indexSon = forDelete->document.header.indexBrother;
        off_t i = getIndex(file, p.header.indexAttached).offset;
        fseeko(file->file, i, SEEK_SET);
        fwrite(&p.header, sizeof(documentHeader), 1, file->file);
    } else {
        result* prev = forDelete->prev;
        document p = prev->document;
        p.header.indexBrother = forDelete->document.header.indexBrother;
        off_t i = getIndex(file, p.header.indexAttached).offset;
        fseeko(file->file, i, SEEK_SET);
        fwrite(&p.header, sizeof(documentHeader), 1, file->file);
    }
    del(doc, file);
    destroyResultList(&pList);
    //delete subtree
    if(doc.header.indexSon != 0) {
        document child;
        documentHeader childHeader = getDocumentHeader(file, doc.header.indexSon);
        child.header = childHeader;
        child.isRoot = isRootDocumentHeader(childHeader);
        child.indexParent = doc.header.indexAttached;
        forEachDocument(file, del, child);
    }

    //delete toast list
    if(doc.header.firstToastIndex != 0) {
        uint64_t order = doc.header.firstToastIndex;
        zgdbIndex indexToast;
        toast temp;
        while(order != 0) {
            indexToast = getIndex(file, order);
            fseeko(file->file, indexToast.offset, SEEK_SET);
            fread(&temp, sizeof(toast), 1, file->file);
            killIndex(file, order);
            insertDeadIndex(&(file->freeList), order, temp.capacity);
            order = temp.nextToastIndex;
        }
    }
}

void forEachDocument(zgdbFile* file, void (* consumer)(document, zgdbFile*), document start) {
    documentIterator iterator = createDocIterator(file, start.header.indexAttached, start.indexParent);
    while (hasNextDoc(&iterator)) {
        document doc = nextDoc(file, &iterator);
        (*consumer) (doc, file);
    }
    destroyDocIterator(&iterator);
}

void findIf0(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
    documentIterator iterator = createDocIterator(file, order, orderParent);
    while (hasNextDoc(&iterator)) {
        document doc = nextDoc(file, &iterator);
        if ((*predicate)(doc)) {
            printf("Found document: %s, ", doc.header.name);
            printf("parent: %llu\n", doc.indexParent);
            insertResult(list, doc);
        }
    }
    destroyDocIterator(&iterator);
}

resultList join(zgdbFile* file, document parent) {
    resultList pList = createResultList();

    document temp;
    documentHeader tempHeader;
    uint64_t tempIndex = parent.header.indexSon;
    while(tempIndex != 0) {
        tempHeader = getDocumentHeader(file, tempIndex);
        temp.header = tempHeader;
        temp.isRoot = isRootDocumentHeader(tempHeader);
        temp.indexParent = parent.header.indexAttached;
        tempIndex = tempHeader.indexBrother;
        insertResult(&pList, temp);
    }

    return pList;
}

resultList findIfFromRoot(zgdbFile* file, bool (* predicate)(document)) {
    resultList pList = createResultList();
    findIf0(file, 0, 0, predicate, &pList);
    return pList;
}

resultList findIfFromDocument(zgdbFile* file, bool (* predicate)(document), document document) {
    resultList pList = createResultList();
    findIf0(file, document.header.indexAttached, document.indexParent, predicate, &pList);
    return pList;
}

void createRootDocument(zgdbFile* file, off_t offset) {
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    documentHeader header = {.id = id, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 0, .indexBrother = 0,
            .indexSon = 0, .name = "root", .firstToastIndex = 0, .blockType = DOCUMENT};

    fwrite(&header, sizeof(documentHeader), 1, file->file);
    file->zgdbHeader.fileSize += sizeof(documentHeader);
    file->zgdbHeader.nodes++;
    saveHeader(file);
}

str2intStatus str2int(int32_t *out, char *s) {
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    int64_t l = strtol(s, &end, 10);
    if (l > INT32_MAX || errno == ERANGE)
        return STR2INT_OVERFLOW;
    if (l < INT32_MIN || errno == ERANGE)
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = (int32_t) l;
    return STR2INT_SUCCESS;
}

str2doubleStatus str2double(double *out, char *s) {
    char* end;
    double number;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2DOUBLE_INCONVERTIBLE;
    errno = 0;
    number = strtod(s, &end);
    if (errno == ERANGE)
        return STR2DOUBLE_INCONVERTIBLE;
    if(number == 0 && s == end)
        return STR2DOUBLE_INCONVERTIBLE;
    *out = (double) number;
    return STR2DOUBLE_SUCCESS;
}

void str2boolean(uint8_t *out, char *s) {
    if(strcmp(s, "true") == 0) {
        *out = 1;
    } else {
        *out = 0;
    }
}

uint64_t createNewToast(zgdbFile* file, uint64_t size) {
    uint64_t newToastIndex = 0;
    relevantIndexMeta* pRelevantIndexMetaToast = findRelevantIndex(&file->freeList, size);
    zgdbIndex indexToAttachToast = getIndex(file, pRelevantIndexMetaToast->indexOrder);
    off_t offsetToast = file->zgdbHeader.fileSize;
    if(indexToAttachToast.flag == INDEX_DEAD)
        offsetToast = indexToAttachToast.offset;
    else if(indexToAttachToast.flag == INDEX_NEW)
        offsetToast = file->zgdbHeader.fileSize;
    fseeko(file->file, offsetToast, SEEK_SET);
    uint64_t cap = pRelevantIndexMetaToast->blockSize == 0 ? size : pRelevantIndexMetaToast->blockSize;
    toast toastBlock = {.blockType = TOAST, .capacity = cap,
            .used = 0, .indexAttached = pRelevantIndexMetaToast->indexOrder, .nextToastIndex = 0};
    fwrite(&toastBlock, sizeof(toast), 1, file->file);
    char buf[cap];
    memset(&buf, 0, cap);
    fwrite(&buf, cap, 1, file->file);
    newToastIndex = pRelevantIndexMetaToast->indexOrder;
    attachIndexToBlock(file, pRelevantIndexMetaToast->indexOrder, offsetToast);
    if(pRelevantIndexMetaToast->blockSize == 0)
        file->zgdbHeader.fileSize += (off_t) cap;
    saveHeader(file);
    free(pRelevantIndexMetaToast);
    return newToastIndex;
}

//TODO return on update
updateElementStatus updateElement(zgdbFile* file, document doc, char* key, char* input) {
    if(strlen(key) > 13)
        return INVALID_NAME;

    updateElementStatus statusToReturn = ELEMENT_NOT_FOUND;
    elementIterator iterator = createElIterator(file, &doc);
    elementEntry entry;
    while(hasNextEl(&iterator)) {
        entry = nextEl(file, &iterator, false);
        if(strcmp(key, entry.element.key) == 0) {
            switch (entry.element.type) {
                case TYPE_BOOLEAN: {
                    uint8_t booleanValue;
                    str2boolean(&booleanValue, input);
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&booleanValue, sizeof(uint8_t), 1, file->file);
                    statusToReturn = UPDATE_OK;
                    break;
                }
                case TYPE_INT: {
                    int32_t intValue;
                    str2intStatus status = str2int(&intValue, input);
                    if(status != STR2INT_SUCCESS)
                        return TYPE_PARSE_ERROR;
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&intValue, sizeof(int32_t), 1, file->file);
                    statusToReturn = UPDATE_OK;
                    break;
                }
                case TYPE_DOUBLE: {
                    double doubleValue;
                    str2doubleStatus status = str2double(&doubleValue, input);
                    if(status != STR2DOUBLE_SUCCESS)
                        return TYPE_PARSE_ERROR;
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&doubleValue, sizeof(double), 1, file->file);
                    statusToReturn = UPDATE_OK;
                    break;
                }
                case TYPE_TEXT: {
                    firstTextChunk firstChunk;
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fread(&firstChunk, sizeof(firstTextChunk), 1, file->file);

                    size_t oldLength = firstChunk.size;
                    size_t newLength = strlen(input) + 1;

                    if(newLength == oldLength) {
                        div_t divRes = div((int) newLength, CHUNK_SIZE);
                        int chunks = divRes.quot;
                        if(divRes.rem != 0)
                            chunks++;
                        printf("Chunks: %d\n", chunks);
                        zgdbIndex indexToast = getIndex(file, doc.header.firstToastIndex);
                        toast tempToast;
                        fseeko(file->file, indexToast.offset, SEEK_SET);
                        fread(&tempToast, sizeof(toast), 1, file->file);
                        char buf[newLength];
                        memset(buf, 0, newLength);
                        textChunk tempChunk;
                        uint8_t chunkType;
                        off_t nextOffset = firstChunk.offsetInToast;

                        for (size_t count = 0; count < chunks; ++count) {
                            fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                            memset(tempChunk.data, 0, CHUNK_SIZE);

                            fread(&chunkType, sizeof(uint8_t), 1, file->file);
                            off_t prev = ftello(file->file);
                            fread(&tempChunk, sizeof(textChunk), 1, file->file);
                            fseeko(file->file, prev, SEEK_SET);
                            if(count == chunks - 1) {
                                for (int i = 0; i < divRes.rem; ++i) {
                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                }
                            } else {
                                for (int i = 0; i < CHUNK_SIZE; ++i) {
                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                }
                            }
                            fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
                            if(tempChunk.toastIndex == tempToast.indexAttached) {
                                nextOffset = tempChunk.nextOffset;
                            } else {
                                indexToast = getIndex(file, tempChunk.toastIndex);
                                fseeko(file->file, indexToast.offset, SEEK_SET);
                                fread(&tempToast, sizeof(toast), 1, file->file);
                                nextOffset = tempChunk.nextOffset;
                            }
                        }
                    } else if(newLength < oldLength) {
                        div_t divRes = div((int) newLength, CHUNK_SIZE);
                        int chunks = divRes.quot;
                        if(divRes.rem != 0)
                            chunks++;
                        if(divRes.rem == 0) {
                            newLength++;
                            chunks++;
                        }
                        printf("Chunks: %d\n", chunks);
                        zgdbIndex indexToast = getIndex(file, doc.header.firstToastIndex);
                        toast tempToast;
                        fseeko(file->file, indexToast.offset, SEEK_SET);
                        fread(&tempToast, sizeof(toast), 1, file->file);
                        char buf[newLength];
                        memset(buf, 0, newLength);
                        textChunk tempChunk;
                        uint8_t chunkType;
                        off_t nextOffset = firstChunk.offsetInToast;

                        for (size_t count = 0; count < chunks; ++count) {
                            fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                            memset(tempChunk.data, 0, CHUNK_SIZE);

                            fread(&chunkType, sizeof(uint8_t), 1, file->file);
                            off_t prev = ftello(file->file);
                            fread(&tempChunk, sizeof(textChunk), 1, file->file);
                            fseeko(file->file, prev, SEEK_SET);
                            if(count == chunks - 1) {
                                for (int i = 0; i < divRes.rem; ++i) {
                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                }
                            } else {
                                for (int i = 0; i < CHUNK_SIZE; ++i) {
                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                }
                            }
                            fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
                            if(tempChunk.toastIndex == tempToast.indexAttached) {
                                nextOffset = tempChunk.nextOffset;
                            } else {
                                indexToast = getIndex(file, tempChunk.toastIndex);
                                fseeko(file->file, indexToast.offset, SEEK_SET);
                                fread(&tempToast, sizeof(toast), 1, file->file);
                                nextOffset = tempChunk.nextOffset;
                            }
                        }
                        firstChunk.size = newLength;
                        fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                        fwrite(&firstChunk, sizeof(firstTextChunk), 1, file->file);
                    } else if(newLength > oldLength) {
                        div_t divRes = div((int) oldLength, CHUNK_SIZE);
                        int chunks = divRes.quot;
                        if(divRes.rem != 0)
                            chunks++;
                        if(divRes.rem == 0) {
                            newLength++;
                            chunks++;
                        }

                        div_t divResNew = div((int) newLength, CHUNK_SIZE);
                        int chunksNew = divResNew.quot;
                        if(divResNew.rem != 0)
                            chunksNew++;
                        if(divResNew.rem == 0) {
                            newLength++;
                            chunksNew++;
                        }

                        printf("Chunks old: %d\n", chunks);
                        printf("Chunks new: %d\n", chunksNew);
                        zgdbIndex indexCurrentToast = getIndex(file, doc.header.firstToastIndex);
                        toast tempCurrentToast;
                        fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                        fread(&tempCurrentToast, sizeof(toast), 1, file->file);

                        uint64_t currentLeft = (tempCurrentToast.capacity - sizeof(toast)) / (sizeof(uint8_t) + sizeof(textChunk)) - tempCurrentToast.used;

                        //TODO ???
//                        while(currentLeft == 0) {
//                            if(tempCurrentToast.nextToastIndex == 0) {
//                                uint64_t i = createNewToast(file, tempCurrentToast.capacity);
//                                tempCurrentToast.nextToastIndex = i;
//                                fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
//                                fwrite(&tempCurrentToast, sizeof(toast), 1, file->file);
//                            }
//                            indexCurrentToast = getIndex(file, tempCurrentToast.nextToastIndex);
//                            fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
//                            fread(&tempCurrentToast, sizeof(toast), 1, file->file);
//                            currentLeft = (tempCurrentToast.capacity - sizeof(toast)) / (sizeof(uint8_t) + sizeof(textChunk)) - tempCurrentToast.used;
//                        }

                        textChunk tempChunk;
                        uint8_t chunkType;
                        off_t nextOffset = firstChunk.offsetInToast;

                        //get last chunk
                        for (size_t count = 0; count < chunks; ++count) {
                            fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                            memset(tempChunk.data, 0, CHUNK_SIZE);
                            fread(&chunkType, sizeof(uint8_t), 1, file->file);
                            fread(&tempChunk, sizeof(textChunk), 1, file->file);
                            nextOffset = tempChunk.nextOffset;
                            printf("Current chunk (toast: %lu, offset: %ld)\n", tempChunk.toastIndex, tempChunk.nextOffset);
                        }

                        if(chunksNew <= chunks) {
                            nextOffset = firstChunk.offsetInToast;
                            for (size_t count = 0; count < chunksNew; ++count) {
                                fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                                memset(tempChunk.data, 0, CHUNK_SIZE);

                                fread(&chunkType, sizeof(uint8_t), 1, file->file);
                                off_t rec = ftello(file->file);
                                fread(&tempChunk, sizeof(textChunk), 1, file->file);
                                if(count == chunksNew - 1) {
                                    for (int i = 0; i < divResNew.rem; ++i) {
                                        tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                    }
                                } else {
                                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                                        tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                    }
                                }
                                fseeko(file->file, rec, SEEK_SET);
                                fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
                                if(tempChunk.toastIndex == tempCurrentToast.indexAttached) {
                                    nextOffset = tempChunk.nextOffset;
                                } else {
                                    indexCurrentToast = getIndex(file, tempChunk.toastIndex);
                                    fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                    fread(&tempCurrentToast, sizeof(toast), 1, file->file);
                                    nextOffset = tempChunk.nextOffset;
                                }
                            }
                        } else {
                            uint64_t left = (tempCurrentToast.capacity - sizeof(toast)) / (sizeof(uint8_t) + sizeof(textChunk)) - tempCurrentToast.used;
                            int req = chunksNew - chunks;
                            printf("Left in current toast: %lu, req: %d\n", left, req);
                            if(left >= req) {

                                nextOffset = firstChunk.offsetInToast;
                                size_t lastSymbol = 0;
                                for (size_t count = 0; count < chunks; ++count) {
                                    fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                                    memset(tempChunk.data, 0, CHUNK_SIZE);

                                    fread(&chunkType, sizeof(uint8_t), 1, file->file);
                                    off_t rec = ftello(file->file);
                                    fread(&tempChunk, sizeof(textChunk), 1, file->file);
                                    if(count == chunks - 1) {
                                        lastSymbol = CHUNK_SIZE * count;
                                        tempChunk.nextOffset = (off_t) (tempCurrentToast.used * (sizeof(uint8_t) + sizeof(textChunk)));
                                    }
                                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                                        tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                    }

                                    fseeko(file->file, rec, SEEK_SET);
                                    fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
                                    if(tempChunk.toastIndex == tempCurrentToast.indexAttached) {
                                        nextOffset = tempChunk.nextOffset;
                                    } else {
                                        indexCurrentToast = getIndex(file, tempChunk.toastIndex);
                                        fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                        fread(&tempCurrentToast, sizeof(toast), 1, file->file);
                                        nextOffset = tempChunk.nextOffset;
                                    }
                                }

                                textChunk tempTextChunk;
                                chunkType = TYPE_TEXT_CHUNK;

                                fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + (tempCurrentToast.used * (sizeof(uint8_t) + sizeof(textChunk)))), SEEK_SET);
                                for (int count = 0; count < req; ++count) {
                                    memset(tempTextChunk.data, 0, CHUNK_SIZE);
                                    if(count == req - 1) {
                                        tempTextChunk.nextOffset = 0;
                                        tempTextChunk.toastIndex = tempCurrentToast.indexAttached;
                                        for (int i = 0; i < divResNew.rem; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    } else {
                                        tempTextChunk.nextOffset = (off_t) ((tempCurrentToast.used + count + 1) * (sizeof(uint8_t) + sizeof(textChunk)));//offset in toast after header
                                        tempTextChunk.toastIndex = tempCurrentToast.indexAttached;
                                        for (int i = 0; i < CHUNK_SIZE; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    }
                                    fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                                    fwrite(&tempTextChunk, sizeof(textChunk), 1, file->file);
                                }
                                tempCurrentToast.used += req;
                                fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                fwrite(&tempCurrentToast, sizeof(toast), 1, file->file);
                            } else if(left < req) {
                                //check if left == 0

                                uint64_t newToastIndex = createNewToast(file, req*(sizeof(uint8_t) + sizeof(textChunk)) + tempCurrentToast.capacity);

                                //update prev toast
                                tempCurrentToast.nextToastIndex = newToastIndex;
                                fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                fwrite(&tempCurrentToast, sizeof(toast), 1, file->file);
                                size_t lastSymbol = 0;
                                nextOffset = firstChunk.offsetInToast;
                                for (size_t count = 0; count < chunks; ++count) {
                                    fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                                    memset(tempChunk.data, 0, CHUNK_SIZE);

                                    fread(&chunkType, sizeof(uint8_t), 1, file->file);
                                    off_t rec = ftello(file->file);
                                    fread(&tempChunk, sizeof(textChunk), 1, file->file);
                                    if(count == chunks - 1) {
                                        lastSymbol = CHUNK_SIZE * count;
                                        tempChunk.nextOffset = (off_t) ((tempCurrentToast.used) * (sizeof(uint8_t) + sizeof(textChunk)));
                                        if(left == 0) {
                                            tempChunk.nextOffset = 0;
                                            tempChunk.toastIndex = newToastIndex;
                                        }
                                    }
                                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                                        tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                    }

                                    fseeko(file->file, rec, SEEK_SET);
                                    fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
                                    if(tempChunk.toastIndex == tempCurrentToast.indexAttached) {
                                        nextOffset = tempChunk.nextOffset;
                                    } else {
                                        if(count != chunks - 1) {
                                            indexCurrentToast = getIndex(file, tempChunk.toastIndex);
                                            fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                            fread(&tempCurrentToast, sizeof(toast), 1, file->file);
                                            nextOffset = tempChunk.nextOffset;
                                        }
                                    }
                                }

                                size_t tempLastSymbol = 0;
                                if(left != 0) {
                                    textChunk tempTextChunk;
                                    chunkType = TYPE_TEXT_CHUNK;

                                    fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + (tempCurrentToast.used * (sizeof(uint8_t) + sizeof(textChunk)))), SEEK_SET);
                                    for (int count = 0; count < left; ++count) {
                                        memset(tempTextChunk.data, 0, CHUNK_SIZE);
                                        if(count == left - 1) {
                                            tempTextChunk.nextOffset = 0;//offset in toast after header
                                            tempTextChunk.toastIndex = newToastIndex;
                                            for (int i = 0; i < CHUNK_SIZE; ++i) {
                                                tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                            }
                                            tempLastSymbol += CHUNK_SIZE;
                                        } else {
                                            tempTextChunk.nextOffset = (off_t) ((tempCurrentToast.used + count + 1) * (sizeof(uint8_t) + sizeof(textChunk)));//offset in toast after header
                                            tempTextChunk.toastIndex = tempCurrentToast.indexAttached;
                                            for (int i = 0; i < CHUNK_SIZE; ++i) {
                                                tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                            }
                                            tempLastSymbol += CHUNK_SIZE;
                                        }
                                        fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                                        fwrite(&tempTextChunk, sizeof(textChunk), 1, file->file);
                                    }
                                    tempCurrentToast.used += left;
                                    fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                    fwrite(&tempCurrentToast, sizeof(toast), 1, file->file);
                                    lastSymbol += tempLastSymbol;
                                }
                                //move to new toast
                                indexCurrentToast = getIndex(file, tempCurrentToast.nextToastIndex);
                                fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                fread(&tempCurrentToast, sizeof(toast), 1, file->file);
                                textChunk tempTextChunk;
                                chunkType = TYPE_TEXT_CHUNK;
                                uint64_t leftTemp = chunksNew - (left + chunks);
                                fseeko(file->file, (off_t) (indexCurrentToast.offset + sizeof(toast) + (tempCurrentToast.used * (sizeof(uint8_t) + sizeof(textChunk)))), SEEK_SET);
                                for (int count = 0; count < leftTemp; ++count) {
                                    memset(tempTextChunk.data, 0, CHUNK_SIZE);
                                    if(count == leftTemp - 1) {
                                        tempTextChunk.nextOffset = 0;
                                        tempTextChunk.toastIndex = tempCurrentToast.indexAttached;
                                        for (int i = 0; i < divResNew.rem; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    } else {
                                        tempTextChunk.nextOffset = (off_t) ((tempCurrentToast.used + count + 1) * (sizeof(uint8_t) + sizeof(textChunk)));//offset in toast after header
                                        tempTextChunk.toastIndex = tempCurrentToast.indexAttached;
                                        for (int i = 0; i < CHUNK_SIZE; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    }
                                    fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                                    fwrite(&tempTextChunk, sizeof(textChunk), 1, file->file);
                                }
                                tempCurrentToast.used += leftTemp;
                                fseeko(file->file, indexCurrentToast.offset, SEEK_SET);
                                fwrite(&tempCurrentToast, sizeof(toast), 1, file->file);
                            }
                        }
                        firstChunk.size = newLength;
                        fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                        fwrite(&firstChunk, sizeof(firstTextChunk), 1, file->file);
                    }
                    statusToReturn = UPDATE_OK;
                    break;
                }
            }
        }
    }

    destroyElIterator(&iterator);
    return statusToReturn;
}