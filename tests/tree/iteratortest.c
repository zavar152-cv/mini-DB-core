
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
    printFreeIndexesList(&(pFile->freeList));
    documentHeader tempHeader = getDocumentHeader(pFile, 7);

    printf("%s\n", tempHeader.name);

    resultList* pList = findIfFromRoot(pFile, hasChild);
    printResultList(pList);
    
    finish(pFile);

    return 0;
}
