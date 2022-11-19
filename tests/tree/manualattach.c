
#include "zgdb.h"

documentId _generateId(off_t offset) {
    uint32_t now = (uint32_t)time(NULL);
    documentId id = {.timestamp = now, .offset = offset};
    return id;
}

bool checkName(document d) {
    return strcmp(d.header.name, "test3") == 0;
}

int main() {
#ifdef __linux__
    zgdbFile* pFile = init("/tmp/test.zgdb");
#endif
#ifdef __MINGW32__
    zgdbFile* pFile = init("D:/test.zgdb");
#endif

    if(pFile == NULL) {
        printf("Invalid format");
        return -1;
    }
    printf("Before:\n");
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Offset: %lu\n", pFile->zgdbHeader.freeListOffset);
    printf("Index count: %lu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %ld\n\n", pFile->zgdbHeader.fileSize);

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header = {.id = id, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 1, .indexBrother = 2,
            .indexSon = 0, .name = "test1"};

    fwrite(&header, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 1, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);


    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id2 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header2 = {.id = id2, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 2, .indexBrother = 3,
            .indexSon = 6, .name = "test2"};

    fwrite(&header2, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 2, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id3 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header3 = {.id = id3, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 3, .indexBrother = 0,
            .indexSon = 4, .name = "test3"};

    fwrite(&header3, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 3, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id4 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header4 = {.id = id4, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 4, .indexBrother = 5,
            .indexSon = 0, .name = "test4"};

    fwrite(&header4, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 4, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id5 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header5 = {.id = id5, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 5, .indexBrother = 0,
            .indexSon = 0, .name = "test5"};

    fwrite(&header5, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 5, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id6 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header6 = {.id = id6, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 6, .indexBrother = 7,
            .indexSon = 0, .name = "test6"};

    fwrite(&header6, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 6, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    fseeko(pFile->file, pFile->zgdbHeader.fileSize, SEEK_SET);
    documentId id7 = _generateId(pFile->zgdbHeader.fileSize);
    documentHeader header7 = {.id = id7, .size = sizeof(documentHeader), .capacity = sizeof(documentHeader),
            .attrCount = 0, .indexAttached = 7, .indexBrother = 0,
            .indexSon = 0, .name = "test7"};

    fwrite(&header7, sizeof(documentHeader), 1, pFile->file);
    attachIndexToBlock(pFile, 7, pFile->zgdbHeader.fileSize);
    pFile->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(pFile);

    printf("After:\n");
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Offset: %lu\n", pFile->zgdbHeader.freeListOffset);
    printf("Index count: %lu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %ld\n\n", pFile->zgdbHeader.fileSize);

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }

    documentHeader tempHeader = getDocumentHeader(pFile, 7);

    printf("%s\n", tempHeader.name);

    findIfFromRoot(pFile, checkName);

    finish(pFile);

    return 0;
}
