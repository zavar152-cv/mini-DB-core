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
    printf("Nodes: %lu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %lu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %ld\n\n", pFile->zgdbHeader.fileSize);


    resultList list = findIfFromRoot(pFile, isRootDocument);
    document rootDoc = list.head->document;
    destroyResultList(&list);
    if(pFile->zgdbHeader.nodes == 1) {
        documentSchema schema = initSchema(8);
        //addTextToSchema(&schema, "string2", "hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world");
        addBooleanToSchema(&schema, "bool1", 0);
        addDoubleToSchema(&schema, "double1", 1.0);
        addIntToSchema(&schema, "int1", 4);
        addTextToSchema(&schema, "string1", "lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol lol");
        addTextToSchema(&schema, "string3", "qwerty1 qwerty2 qwerty3 qwerty4 qwerty5 qwerty6 qwerty7");
        addBooleanToSchema(&schema, "bool2", 0);
        addDoubleToSchema(&schema, "double2", 1.0);
        addIntToSchema(&schema, "int2", 4);
        createDocument(pFile, "test2", &schema, rootDoc);
        destroySchema(&schema);

        list = findIfFromRoot(pFile, isRootDocument);
        rootDoc = list.head->document;
        destroyResultList(&list);
        documentSchema schema2 = initSchema(3);
        addBooleanToSchema(&schema2, "bool1", 1);
        addDoubleToSchema(&schema2, "double6", 1.5);
        addIntToSchema(&schema2, "int9", -1);
        createDocument(pFile, "test3", &schema2, rootDoc);
        createDocument(pFile, "test4", &schema2, rootDoc);
        destroySchema(&schema2);
    }

    list = findIfFromRoot(pFile, isRootDocument);
    rootDoc = list.head->document;
    destroyResultList(&list);

    resultList list1 = join(pFile, rootDoc);
    printf("Join operation:\n");
    printResultList(&list1);


    finish(pFile);
    return 0;
}

