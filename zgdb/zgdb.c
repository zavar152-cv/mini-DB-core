#include "zgdb.h"

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
    return strcmp(header.name, "root") == 0 && header.indexAttached == 0 && header.attrCount == 0 && header.indexBrother == 0;
}

void findIf0(zgdbFile* file, uint64_t order, uint64_t orderParent, bool (* predicate)(document), resultList* list) {
    documentHeader header = getDocumentHeader(file, order);
    printf("Visited document: %s, ", header.name);
    printf("parent: %llu\n", orderParent);
    document document;
    document.header = header;
    document.isRoot = isRootDocument0(header);
    document.indexParent = orderParent;
    document.elements = NULL;
    //TODO reading elements
//    if(header.attrCount == 0) {
//        document.elements = NULL;
//    } else {
//        document.elements = NULL;
//    }

    if((* predicate)(document)) {
        printf("Found document: %s, ", document.header.name);
        printf("parent: %llu\n", document.indexParent);
        insertResult(list, document);
    }

    if(document.header.indexBrother == 0 && document.header.indexSon == 0) {
        return;
    } else if(document.header.indexBrother != 0 && document.header.indexSon != 0) {
        findIf0(file, document.header.indexBrother, document.indexParent, predicate, list);
        findIf0(file, document.header.indexSon, document.header.indexAttached, predicate, list);
    } else if(document.header.indexBrother != 0 && document.header.indexSon == 0) {
        findIf0(file, document.header.indexBrother, document.indexParent, predicate, list);
    } else if(document.header.indexBrother == 0 && document.header.indexSon != 0) {
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

documentId generateId(off_t offset) {
    uint32_t now = (uint32_t) time(NULL);
    documentId id = {.timestamp = now, .offset = offset};
    return id;
}

void createRootDocument(zgdbFile* file, off_t offset) {
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    documentHeader header = {.id = id, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 0, .indexBrother = 0,
            .indexSon = 1, .name = "root"};//TODO REMOVE SON !!!! ONLY FOR TEMP TEST

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

