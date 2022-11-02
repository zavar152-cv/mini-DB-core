#include "../../zgdb/index/zgdbindex.h"

int main() {
    zgdbFile* pFile = loadOrCreateZgdbFile("/tmp/test.zgdb");
    if(pFile == NULL) {
        printf("Invalid format");
        return -1;
    }
    printf("Before:\n");
    printf("%d\n", pFile->zgdbHeader.zgdbType);
    printf("%lu\n", pFile->zgdbHeader.freeListOffset);
    printf("%lu\n", pFile->zgdbHeader.indexCount);

    createIndex(pFile);

    printf("After:\n");
    printf("%d\n", pFile->zgdbHeader.zgdbType);
    printf("%lu\n", pFile->zgdbHeader.freeListOffset);
    printf("%lu\n", pFile->zgdbHeader.indexCount);

    zgdbIndex index = *getIndex(pFile, 0);
    printf("%d\n", index.flag);
    printf("%lu\n", index.offset);

    return 0;
}