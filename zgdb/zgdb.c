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

documentId generateId(off_t offset) {
    uint32_t now = (uint32_t) time(NULL);
    documentId id = {.timestamp = now, .offset = offset};
    return id;
}

off_t writeElement(zgdbFile* file, element cur) {
    off_t t = ftello(file->file);
    fwrite(&cur.type, sizeof(uint8_t), 1, file->file);
    fwrite(&cur.key, 13 * sizeof(char), 1, file->file);
    switch (cur.type) {
        case TYPE_BOOLEAN:
            fwrite(&cur.booleanValue, sizeof(uint8_t), 1, file->file);
            break;
        case TYPE_INT:
            fwrite(&cur.integerValue, sizeof(int32_t), 1, file->file);
            break;
        case TYPE_DOUBLE:
            fwrite(&cur.doubleValue, sizeof(double), 1, file->file);
            break;
        case TYPE_TEXT:
            fwrite(&cur.textValue.size, sizeof(uint32_t), 1, file->file);
            fwrite(&cur.textValue.data, cur.textValue.size * sizeof(char), 1, file->file);
            break;
    }
    return ftello(file->file) - t;
}

element* readElement(zgdbFile* file) {
    element* cur = malloc(sizeof(element));
    fread(&cur->type, sizeof(uint8_t), 1, file->file);
    fread(&cur->key, 13 * sizeof(char), 1, file->file);
    switch (cur->type) {
        case TYPE_BOOLEAN:
            fread(&cur->booleanValue, sizeof(uint8_t), 1, file->file);
            break;
        case TYPE_INT:
            fread(&cur->integerValue, sizeof(int32_t), 1, file->file);
            break;
        case TYPE_DOUBLE:
            fread(&cur->doubleValue, sizeof(double), 1, file->file);
            break;
        case TYPE_TEXT:
            fread(&cur->textValue.size, sizeof(uint32_t), 1, file->file);
            fread(&cur->textValue.data, cur->textValue.size * sizeof(char), 1, file->file);
            break;
    }
    return cur;
}

void printDocumentElements(zgdbFile* file, document document) {
    zgdbIndex index = getIndex(file, document.header.indexAttached);
    fseeko(file->file, index.offset, SEEK_SET);
    fseeko(file->file, sizeof(documentHeader), SEEK_CUR);
    for (int i = 0; i < document.header.attrCount; ++i) {
        element* pElement = readElement(file);
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
        printf("\n");
    }
}

void createDocument(zgdbFile* file, const char* name, documentSchema schema, document parent) {
    documentHeader parentHeader = parent.header;
    off_t docSize = sizeof(documentHeader);
    if(schema.capacity != schema.size) {
        printf("Invalid schema\n");
        return;
    }

    if(file->freeList.indexesCount - file->freeList.newIndexesCount <= INDEX_INITIAL_CAPACITY/2) {
        printf("Needed to expand indexes");
    }

    relevantIndexMeta* pRelevantIndexMeta = findRelevantIndex(&file->freeList, docSize);
    printf("%lu\n", pRelevantIndexMeta->indexOrder);
    zgdbIndex indexToAttach = getIndex(file, pRelevantIndexMeta->indexOrder);

    off_t offset = file->zgdbHeader.fileSize;
    if(indexToAttach.flag == INDEX_DEAD)
        offset = indexToAttach.offset;
    else if(indexToAttach.flag == INDEX_NEW)
        offset = file->zgdbHeader.fileSize;

    fseeko(file->file, offset, SEEK_SET);
    fseeko(file->file, sizeof(documentHeader), SEEK_CUR);
    for (int i = 0; i < schema.capacity; ++i) {
        element cur = schema.elements[i];
        docSize += writeElement(file, cur);
    }
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    documentHeader header = {.id = id, .size = docSize, .capacity = pRelevantIndexMeta->blockSize,
            .attrCount = schema.capacity, .indexAttached = pRelevantIndexMeta->indexOrder, .indexBrother = 0,
            .indexSon = 0};
    strcpy(header.name, name);
    attachIndexToBlock(file, pRelevantIndexMeta->indexOrder, offset);
    fwrite(&header, sizeof(documentHeader), 1, file->file);
    file->zgdbHeader.fileSize += docSize;
    saveHeader(file);

    parentHeader.indexSon = pRelevantIndexMeta->indexOrder;
    fseeko(file->file, getIndex(file, parent.header.indexAttached).offset, SEEK_SET);
    fwrite(&parentHeader, sizeof(documentHeader), 1, file->file);
}

void findIf0(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
    treeStack* pStack = createStack();
    nodeEntry* next = malloc(sizeof(nodeEntry));
    next->orderParent = orderParent;
    next->order = order;
#if defined(__MINGW32__) && defined(DEBUG_INFO)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    SIZE_T physMemUsedByMe;
#endif
    while (next != NULL) {
        documentHeader header = getDocumentHeader(file, next->order);
        printf("Visited document: %s, ", header.name);
        printf("parent: %llu\n", next->orderParent);
        document document;
        document.header = header;
        document.isRoot = isRootDocument0(header);
        document.indexParent = next->orderParent;
        //TODO reading elements

        if ((*predicate)(document)) {
            printf("Found document: %s, ", document.header.name);
            printf("parent: %llu\n", document.indexParent);
            insertResult(list, document);
        }

        if (document.header.indexBrother == 0 && document.header.indexSon == 0) {
            if (peek(pStack) == NULL) {
                next = NULL;
            } else {
                next = pop(pStack);
            }
        } else if (document.header.indexBrother != 0 && document.header.indexSon != 0) {
            next->order = document.header.indexBrother;
            next->orderParent = document.indexParent;
            nodeEntry temp = {.order = document.header.indexSon, .orderParent = document.header.indexAttached};
            push(pStack, temp);
        } else if (document.header.indexBrother != 0 && document.header.indexSon == 0) {
            next->order = document.header.indexBrother;
            next->orderParent = document.indexParent;
        } else if (document.header.indexBrother == 0 && document.header.indexSon != 0) {
            next->order = document.header.indexSon;
            next->orderParent = document.header.indexAttached;
        }
#if defined(__MINGW32__) && defined(DEBUG_INFO)
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        physMemUsedByMe = pmc.WorkingSetSize;
        printf("Usage memory by iteration findIf: %llu\n", physMemUsedByMe);
#endif
    }
    deleteStack(&pStack);
}

void
findIf0Recursive(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
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

resultList* findIfFromRoot(zgdbFile* file, bool (* predicate)(document)) {
    resultList* pList = createResultList();
    findIf0(file, 0, 0, predicate, pList);
    return pList;
}

resultList* findIfFromDocument(zgdbFile* file, bool (* predicate)(document), document document) {
    resultList* pList = createResultList();
    findIf0(file, document.header.indexAttached, document.indexParent, predicate, pList);
    return pList;
}

void createRootDocument(zgdbFile* file, off_t offset) {
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    documentHeader header = {.id = id, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 0, .indexBrother = 0,
            .indexSon = 0, .name = "root"};//TODO REMOVE SON !!!! ONLY FOR TEMP TEST

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

