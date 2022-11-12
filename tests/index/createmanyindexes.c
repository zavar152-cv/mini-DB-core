#include "../../zgdb/index/zgdbindex.h"

int main() {
    zgdbFile* pFile = loadOrCreateZgdbFile("/tmp/test.zgdb");
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

    createIndexes(pFile, 20);
    for (int i = 0; i < 20; ++i) {
        attachIndexToBlock(pFile, i, i+34);
    }

    printf("After:\n");
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Offset: %lu\n", pFile->zgdbHeader.freeListOffset);
    printf("Index count: %lu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %ld\n", pFile->zgdbHeader.fileSize);


    return 0;
}