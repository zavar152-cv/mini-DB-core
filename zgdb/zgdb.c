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

bool isRootDocument0(documentHeader header) {
    return strcmp(header.name, "root") == 0 && header.indexAttached == 0 && header.attrCount == 0 &&
           header.indexBrother == 0;
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
            case TYPE_TEXT:
                printf("%s\n", pElement->textValue.data);
                break;
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
    }
}

//TODO FIX ROOT
void deleteDocument(zgdbFile* file, document doc) {
    documentHeader parentHeader = getDocumentHeader(file, doc.indexParent);

    if(parentHeader.indexSon == doc.header.indexAttached) {
        parentHeader.indexSon = doc.header.indexBrother;
        off_t i = getIndex(file, parentHeader.indexAttached).offset;
        fseeko(file->file, i, SEEK_SET);
        fwrite(&parentHeader, sizeof(documentHeader), 1, file->file);
    } else {
        documentHeader temp = getDocumentHeader(file, parentHeader.indexSon);
        documentHeader prev = temp;
        while(temp.indexBrother != 0) {
            if(temp.indexAttached == doc.header.indexAttached) {
                break;
            }
            temp = getDocumentHeader(file, temp.indexBrother);
            prev = temp;
        }
        prev.indexBrother = temp.indexBrother;
        off_t i = getIndex(file, prev.indexAttached).offset;
        fseeko(file->file, i, SEEK_SET);
        fwrite(&prev, sizeof(documentHeader), 1, file->file);
    }

    if(doc.header.indexSon != 0) {
        document child;
        documentHeader childHeader = getDocumentHeader(file, doc.header.indexSon);
        child.header = childHeader;
        child.isRoot = isRootDocument0(childHeader);
        child.indexParent = doc.indexParent;
        forEachDocument(file, del, child);
    }

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

    del(doc, file);
}

void forEachDocument(zgdbFile* file, void (* consumer)(document, zgdbFile*), document start) {
    treeStack* pStack = createStack();
    nodeEntry next;
    next.orderParent = start.indexParent;
    next.order = start.header.indexAttached;
    documentHeader header;
    document document;
    nodeEntry temp;
    bool stop = false;

    while (!stop) {
        header = getDocumentHeader(file, next.order);
        printf("Visited document: %s, size: %llu, ", header.name, header.size);
        printf("parent: %llu (index: %llu)\n", next.orderParent, next.order);
        document.header = header;
        document.isRoot = isRootDocument0(header);
        document.indexParent = next.orderParent;
        //TODO reading elements

        (*consumer) (document, file);

        if (document.header.indexBrother == 0 && document.header.indexSon == 0) {
            if (peek(pStack) == NULL) {
                stop = true;
            } else {
                nodeEntry* pEntry = pop(pStack);
                next.order = pEntry->order;
                next.orderParent = pEntry->orderParent;
            }
        } else if (document.header.indexBrother != 0 && document.header.indexSon != 0) {
            next.order = document.header.indexBrother;
            next.orderParent = document.indexParent;
            temp.order = document.header.indexSon;
            temp.orderParent = document.header.indexAttached;
            push(pStack, temp);
        } else if (document.header.indexBrother != 0 && document.header.indexSon == 0) {
            next.order = document.header.indexBrother;
            next.orderParent = document.indexParent;
        } else if (document.header.indexBrother == 0 && document.header.indexSon != 0) {
            next.order = document.header.indexSon;
            next.orderParent = document.header.indexAttached;
        }
    }
    deleteStack(&pStack);
}

void findIf0(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
    treeStack* pStack = createStack();
    nodeEntry next;
    next.orderParent = orderParent;
    next.order = order;
    documentHeader header;
    document document;
    nodeEntry temp;
    bool stop = false;
#if defined(__MINGW32__) && defined(DEBUG_INFO)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    SIZE_T physMemUsedByMe;
#endif
    while (!stop) {
        header = getDocumentHeader(file, next.order);
        printf("Visited document: %s, ", header.name);
        printf("parent: %llu (index: %llu)\n", next.orderParent, next.order);
        document.header = header;
        document.isRoot = isRootDocument0(header);
        document.indexParent = next.orderParent;
        //TODO reading elements

        if ((*predicate)(document)) {
            printf("Found document: %s, ", document.header.name);
            printf("parent: %llu\n", document.indexParent);
            insertResult(list, document);
        }

        if (document.header.indexBrother == 0 && document.header.indexSon == 0) {
            if (peek(pStack) == NULL) {
                stop = true;
            } else {
                nodeEntry* pEntry = pop(pStack);
                next.order = pEntry->order;
                next.orderParent = pEntry->orderParent;
            }
        } else if (document.header.indexBrother != 0 && document.header.indexSon != 0) {
            next.order = document.header.indexBrother;
            next.orderParent = document.indexParent;
            temp.order = document.header.indexSon;
            temp.orderParent = document.header.indexAttached;
            push(pStack, temp);
        } else if (document.header.indexBrother != 0 && document.header.indexSon == 0) {
            next.order = document.header.indexBrother;
            next.orderParent = document.indexParent;
        } else if (document.header.indexBrother == 0 && document.header.indexSon != 0) {
            next.order = document.header.indexSon;
            next.orderParent = document.header.indexAttached;
        }
#if defined(__MINGW32__) && defined(DEBUG_INFO)
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        physMemUsedByMe = pmc.WorkingSetSize;
        printf("Usage memory by iteration findIf: %llu\n", physMemUsedByMe);
#endif
    }
    deleteStack(&pStack);
}

void findIf0Recursive(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
#ifdef __MINGW32__
    PROCESS_MEMORY_COUNTERS_EX pmc;
    SIZE_T physMemUsedByMe;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    physMemUsedByMe = pmc.WorkingSetSize;
    printf("Usage memory by findIf: %llu\n", physMemUsedByMe);
#endif
    documentHeader header = getDocumentHeader(file, order);
    printf("Visited document: %s, ", header.name);
    printf("parent: %llu\n", orderParent);
    document document;
    document.header = header;
    document.isRoot = isRootDocument0(header);
    document.indexParent = orderParent;
    //TODO reading elements

    if ((*predicate)(document)) {
        printf("Found document: %s, ", document.header.name);
        printf("parent: %llu\n", document.indexParent);
        insertResult(list, document);
    }

    if (document.header.indexBrother == 0 && document.header.indexSon == 0) {
        return;
    } else if (document.header.indexBrother != 0 && document.header.indexSon != 0) {
        findIf0(file, document.header.indexBrother, document.indexParent, predicate, list);
        findIf0(file, document.header.indexSon, document.header.indexAttached, predicate, list);
    } else if (document.header.indexBrother != 0 && document.header.indexSon == 0) {
        findIf0(file, document.header.indexBrother, document.indexParent, predicate, list);
    } else if (document.header.indexBrother == 0 && document.header.indexSon != 0) {
        findIf0(file, document.header.indexSon, document.header.indexAttached, predicate, list);
    }
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
    saveHeader(file);
}

documentHeader getDocumentHeader(zgdbFile* file, uint64_t order) {
    zgdbIndex index = getIndex(file, order);
    fseeko(file->file, index.offset, SEEK_SET);
    documentHeader header;
    fread(&header, sizeof(documentHeader), 1, file->file);
    return header;
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

//TODO not found
updateElementStatus updateElement(zgdbFile* file, document doc, char* key, char* input) {
    if(strlen(key) > 13)
        return INVALID_NAME;
    
    elementIterator iterator = createIterator(file, &doc);
    elementEntry entry;
    while(hasNext(&iterator)) {
        entry = next(file, &iterator, false);
        if(strcmp(key, entry.element.key) == 0) {
            switch (entry.element.type) {
                case TYPE_BOOLEAN: {
                    uint8_t booleanValue;
                    str2boolean(&booleanValue, input);
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&booleanValue, sizeof(uint8_t), 1, file->file);
                    break;
                }
                case TYPE_INT: {
                    int32_t intValue;
                    str2intStatus status = str2int(&intValue, input);
                    if(status != STR2INT_SUCCESS)
                        return TYPE_PARSE_ERROR;
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&intValue, sizeof(int32_t), 1, file->file);
                    break;
                }
                case TYPE_DOUBLE: {
                    double doubleValue;
                    str2doubleStatus status = str2double(&doubleValue, input);
                    if(status != STR2DOUBLE_SUCCESS)
                        return TYPE_PARSE_ERROR;
                    fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                    fwrite(&doubleValue, sizeof(double), 1, file->file);
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

                        div_t divResNew = div((int) newLength, CHUNK_SIZE);
                        int chunksNew = divResNew.quot;
                        if(divResNew.rem != 0)
                            chunksNew++;

                        printf("Chunks old: %d\n", chunks);
                        printf("Chunks new: %d\n", chunksNew);
                        zgdbIndex indexToast = getIndex(file, doc.header.firstToastIndex);
                        toast tempToast;
                        fseeko(file->file, indexToast.offset, SEEK_SET);
                        fread(&tempToast, sizeof(toast), 1, file->file);
                        char buf[newLength];
                        memset(buf, 0, newLength);
                        textChunk tempChunk;
                        uint8_t chunkType;
                        off_t nextOffset = firstChunk.offsetInToast;

                        //get last chunk
                        for (size_t count = 0; count < chunks; ++count) {
                            fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                            memset(tempChunk.data, 0, CHUNK_SIZE);
                            fread(&chunkType, sizeof(uint8_t), 1, file->file);
                            fread(&tempChunk, sizeof(textChunk), 1, file->file);
                            nextOffset = tempChunk.nextOffset;
                            printf("Current chunk (toast: %lu, offset: %ld)\n", tempChunk.toastIndex, tempChunk.nextOffset);
                        }

                        if(chunksNew <= chunks) {
                            nextOffset = firstChunk.offsetInToast;
                            for (size_t count = 0; count < chunksNew; ++count) {
                                fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
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
                                if(tempChunk.toastIndex == tempToast.indexAttached) {
                                    nextOffset = tempChunk.nextOffset;
                                } else {
                                    indexToast = getIndex(file, tempChunk.toastIndex);
                                    fseeko(file->file, indexToast.offset, SEEK_SET);
                                    fread(&tempToast, sizeof(toast), 1, file->file);
                                    nextOffset = tempChunk.nextOffset;
                                }
                            }
                        } else {
                            uint64_t left = (tempToast.capacity - sizeof(toast))/(sizeof(uint8_t) + sizeof(textChunk)) - tempToast.used;
                            int req = chunksNew - chunks;
                            printf("Left in current toast: %lu, req: %d\n", left, req);
                            if(left >= req) {

                                nextOffset = firstChunk.offsetInToast;
                                size_t lastSymbol = 0;
                                for (size_t count = 0; count < chunks; ++count) {
                                    fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                                    memset(tempChunk.data, 0, CHUNK_SIZE);

                                    fread(&chunkType, sizeof(uint8_t), 1, file->file);
                                    off_t rec = ftello(file->file);
                                    fread(&tempChunk, sizeof(textChunk), 1, file->file);
                                    if(count == chunks - 1) {
//                                        for (int i = 0; i < divRes.rem; ++i) {
//                                            tempChunk.data[i] = input[CHUNK_SIZE * count + i];
//                                        }
                                        lastSymbol = CHUNK_SIZE * count;
                                        //lastSymbol++;
                                        tempChunk.nextOffset = (off_t) (tempToast.used * (sizeof(uint8_t) + sizeof(textChunk)));
                                    }
                                        for (int i = 0; i < CHUNK_SIZE; ++i) {
                                            tempChunk.data[i] = input[CHUNK_SIZE * count + i];
                                        }

                                    fseeko(file->file, rec, SEEK_SET);
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

                                textChunk tempTextChunk;
                                chunkType = TYPE_TEXT_CHUNK;
                                off_t tempOffset = tempChunk.nextOffset;
                                fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + (tempToast.used * (sizeof(uint8_t) + sizeof(textChunk)))), SEEK_SET);
                                for (int count = 0; count < req; ++count) {
                                    memset(tempTextChunk.data, 0, CHUNK_SIZE);
                                    if(count == req - 1) {
                                        tempTextChunk.nextOffset = 0;
                                        tempTextChunk.toastIndex = tempToast.indexAttached;
                                        for (int i = 0; i < divResNew.rem; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    } else {
                                        tempTextChunk.nextOffset = (off_t) ((tempToast.used + count + 1) * (sizeof(uint8_t) + sizeof(textChunk)));//offset in toast after header
                                        tempTextChunk.toastIndex = tempToast.indexAttached;
                                        for (int i = 0; i < CHUNK_SIZE; ++i) {
                                            tempTextChunk.data[i] = input[lastSymbol + (CHUNK_SIZE * (count + 1)) + i];
                                        }
                                    }
                                    fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                                    fwrite(&tempTextChunk, sizeof(textChunk), 1, file->file);
                                }
                                tempToast.used += req;
                                fseeko(file->file, indexToast.offset, SEEK_SET);
                                fwrite(&tempToast, sizeof(toast), 1, file->file);
                            } else if(left < req) {
                                //check if left == 0
                            }
                        }

//                            if(count == chunks - 1) {
//                                for (int i = 0; i < divRes.rem; ++i) {
//                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
//                                }
//                            } else {
//                                for (int i = 0; i < CHUNK_SIZE; ++i) {
//                                    tempChunk.data[i] = input[CHUNK_SIZE * count + i];
//                                }
//                            }
//                            fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
//                            if(tempChunk.toastIndex == tempToast.indexAttached && tempChunk.nextOffset != 0) {
//                                nextOffset = tempChunk.nextOffset;
//                            } else if(tempChunk.toastIndex == tempToast.indexAttached && tempChunk.nextOffset == 0) {
//                                uint64_t left = (tempToast.capacity - sizeof(toast))/(sizeof(uint8_t) + sizeof(textChunk)) - tempToast.used;
//
//                                if(left >= (chunks - count)) {
//                                    textChunk tempLeftChunk = {.nextOffset = 0, .toastIndex = tempChunk.toastIndex};
//                                    memset(tempLeftChunk.data, 0, CHUNK_SIZE);
//                                    off_t endOfToast = (off_t) (tempToast.used * (sizeof(uint8_t) + sizeof(textChunk)));
//                                    fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + endOfToast), SEEK_SET);
//                                    fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
//                                    fwrite(&tempLeftChunk, sizeof(textChunk), 1, file->file);
//                                    tempToast.used++;
//                                    fseeko(file->file, indexToast.offset, SEEK_SET);
//                                    fwrite(&tempToast, sizeof(toast), 1, file->file);
//                                    nextOffset = endOfToast;
//                                    tempChunk.nextOffset = endOfToast;
//                                } else if(left == 0 || left < (chunks - count)) {
//
//                                }
//
//                                fseeko(file->file, prev, SEEK_SET);
//                                fwrite(&tempChunk, sizeof(textChunk), 1, file->file);
//                            } else {
//                                indexToast = getIndex(file, tempChunk.toastIndex);
//                                fseeko(file->file, indexToast.offset, SEEK_SET);
//                                fread(&tempToast, sizeof(toast), 1, file->file);
//                                nextOffset = tempChunk.nextOffset;
//                            }
//                        }
                        firstChunk.size = newLength;
                        fseeko(file->file, (off_t) (entry.offsetInDocument + sizeof(uint8_t) + 13 * sizeof(char)), SEEK_SET);
                        fwrite(&firstChunk, sizeof(firstTextChunk), 1, file->file);
                    }

                    break;
                }
            }
        }
    }
    
    destroyIterator(&iterator);
    return ELEMENT_NOT_FOUND;
}