#include "zgdb.h"

bool checkName(document d) {
    return strcmp(d.header.name, "test2") == 0;
}

bool checkName2(document d) {
    return strcmp(d.header.name, "test3") == 0;
}

void print(document d, zgdbFile* file) {

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
    documentSchema schema = initSchema(6);
    addBooleanToSchema(&schema, "bool1", 0);
    addDoubleToSchema(&schema, "double1", 1.0);
    addIntToSchema(&schema, "int1", 4);
    addBooleanToSchema(&schema, "bool2", 0);
    addDoubleToSchema(&schema, "double2", 1.0);
    addIntToSchema(&schema, "int2", 4);
    createDocument(pFile, "test2", schema, rootDoc);
    destroySchema(&schema);

    printf("After test2\n");
    printFreeIndexesList(&(pFile->freeList));

    list = findIfFromRoot(pFile, checkName);
    document doc = list.head->document;
    destroyResultList(&list);
    printDocumentElements(pFile, doc);

    documentSchema schema2 = initSchema(3);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);
    createDocument(pFile, "test3", schema2, doc);
    destroySchema(&schema2);

    documentSchema schema3 = initSchema(2);
    addBooleanToSchema(&schema3, "bool2", 0);
    addDoubleToSchema(&schema3, "double7", 3.5);
    createDocument(pFile, "test4", schema3, doc);
    destroySchema(&schema3);

    list = findIfFromRoot(pFile, checkName2);
    document doc2 = list.head->document;
    destroyResultList(&list);
    printDocumentElements(pFile, doc2);

    printFreeIndexesList(&(pFile->freeList));
    deleteDocument(pFile, doc);
    printf("Deleted\n");
    printFreeIndexesList(&(pFile->freeList));

    forEachDocument(pFile, print, rootDoc);

    finish(pFile);

#ifdef __linux__
    zgdbFile* pFile = init("/tmp/test.zgdb");
#endif
#ifdef __MINGW32__
    pFile = init("D:/test.zgdb");
#endif

    list = findIfFromRoot(pFile, isRootDocument);
    rootDoc = list.head->document;
    destroyResultList(&list);

    forEachDocument(pFile, print, rootDoc);

    finish(pFile);

    return 0;
}

