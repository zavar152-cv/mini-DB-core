#include "zgdb.h"

bool checkName(document d) {
    return strcmp(d.header.name, "test2") == 0;
}

bool checkName2(document d) {
    return strcmp(d.header.name, "test4") == 0;
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
    //addTextToSchema(&schema, "string2", "hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world");
    addBooleanToSchema(&schema, "bool1", 0);
    addDoubleToSchema(&schema, "double1", 1.0);
    addIntToSchema(&schema, "int1", 4);
    addTextToSchema(&schema, "string1", "lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol");
    addBooleanToSchema(&schema, "bool2", 0);
    addDoubleToSchema(&schema, "double2", 1.0);
    addIntToSchema(&schema, "int2", 4);
    createDocument(pFile, "test2", &schema, rootDoc);
    printFreeIndexesList(&(pFile->freeList));
    destroySchema(&schema);
    list = findIfFromRoot(pFile, checkName);
    document doc = list.head->document;
    destroyResultList(&list);

    documentSchema schema2 = initSchema(3);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);
    createDocument(pFile, "test3", &schema2, doc);
    createDocument(pFile, "test4", &schema2, doc);
    destroySchema(&schema2);

    printDocumentElements(pFile, doc);

    updateElement(pFile, doc, "bool2", "true");
    updateElement(pFile, doc, "double2", "3.14");
    updateElement(pFile, doc, "string1", "lol a1234567890 b1234567890 c1234567890 d1234567890 e1234567890 f1234567890 g1234567890");
    //updateElement(pFile, doc, "string2", "olleh hello world hello world hello world hello world hello world hello world hello 12345");

    printf("After update:\n");
    printDocumentElements(pFile, doc);

    elementIterator iterator = createIterator(pFile, &doc);

    elementEntry entry;
    while(hasNext(&iterator)) {
        entry = next(pFile, &iterator, true);
        printf("nop\n");
    }

    destroyIterator(&iterator);

    list = findIfFromRoot(pFile, checkName);
    doc = list.head->document;
    destroyResultList(&list);
    deleteDocument(pFile, doc);
    printf("After delete:\n");
    list = findIfFromRoot(pFile, checkName);
    destroyResultList(&list);
    finish(pFile);

    return 0;
}

