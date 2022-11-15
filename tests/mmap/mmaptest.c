#include "zgdb.h"

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

    printf("Checking using array...\n");
    zgdbIndex* pIndex = (zgdbIndex*) pFile->pIndexesMmap;
    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", pIndex[i].flag);
        printf("Offset: %llu\n\n", pIndex[i].offset);
    }

    printf("Checking using getIndex...\n");
    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }

    printf("Update index...\n");
    attachIndexToBlock(pFile, 2, 228);

    printf("Index %d\n", 2);
    printf("Flag: %d\n", getIndex(pFile, 2).flag);
    printf("Offset: %llu\n\n", getIndex(pFile, 2).offset);

    printf("Create indexes...\n");
    createIndexes(pFile, 3);

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }

    printf("Update index...\n");
    if(attachIndexToBlock(pFile, 3, 1337)) {
        printf("Index %d\n", 3);
        printf("Flag: %d\n", getIndex(pFile, 3).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, 3).offset);
    } else {
        printf("false\n");
    }

    printf("Kill index...\n");
    if(killIndex(pFile, pFile->zgdbHeader.indexCount-1)) {
        printf("Index %d\n", pFile->zgdbHeader.indexCount-1);
        printf("Flag: %d\n", getIndex(pFile, pFile->zgdbHeader.indexCount-1).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, pFile->zgdbHeader.indexCount-1).offset);
    } else {
        printf("false\n");
    }

    printList(&(pFile->freeList));

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }
    printf("Size: %ld\n\n", pFile->zgdbHeader.fileSize);

    finish(pFile);

    return 0;
}