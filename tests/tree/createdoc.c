#include "zgdb.h"

bool checkName(document d) {
    return strcmp(d.header.name, "test1") == 0;
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

    document rootDoc = findIfFromRoot(pFile, isRootDocument)->head->document;
    documentSchema schema = initSchema(6);
    addBooleanToSchema(&schema, "bool1", 0);
    addDoubleToSchema(&schema, "double1", 1.0);
    addIntToSchema(&schema, "int1", 4);
    addBooleanToSchema(&schema, "bool2", 0);
    addDoubleToSchema(&schema, "double2", 1.0);
    addIntToSchema(&schema, "int2", 4);
    createDocument(pFile, "test1", schema, rootDoc);

    document doc = findIfFromRoot(pFile, checkName)->head->document;
    printDocumentElements(pFile, doc);
    finish(pFile);

    return 0;
}

