
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

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %llu\n\n", getIndex(pFile, i).offset);
    }

    documentHeader header = getDocumentHeader(pFile, 0);
    
    closeZgdbFile(pFile);

    return 0;
}