#include "zgdb.h"

bool checkName(document d) {
    return strcmp(d.header.name, "test2") == 0;
}

bool checkName2(document d) {
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

    resultList list = findIfFromRoot(pFile, isRootDocument);
    document rootDoc = list.head->document;
    destroyResultList(&list);
    documentSchema schema = initSchema(7);
    addBooleanToSchema(&schema, "bool1", 0);
    addDoubleToSchema(&schema, "double1", 1.0);
    addIntToSchema(&schema, "int1", 4);
    addBooleanToSchema(&schema, "bool2", 0);
    addDoubleToSchema(&schema, "double2", 1.0);
    addIntToSchema(&schema, "int2", 4);
    addTextToSchema(&schema, "string1", "lol");
    createDocument(pFile, "test2", schema, rootDoc);
    destroySchema(&schema);

    list = findIfFromRoot(pFile, checkName);
    document doc = list.head->document;
    destroyResultList(&list);
    printDocumentElements(pFile, doc);

    deleteDocument(pFile, doc);
    finish(pFile);

    return 0;
}

