#define DEBUG_OUTPUT
#include "zgdb.h"

bool checkName(document d) {
    return strcmp(d.header.name, "test2") == 0;
}

bool checkName2(document d) {
    return strcmp(d.header.name, "test3") == 0;
}

bool checkName3(document d) {
    return strcmp(d.header.name, "test5") == 0;
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

    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
        printf("Index %d\n", i);
        printf("Flag: %d\n", getIndex(pFile, i).flag);
        printf("Offset: %ld\n\n", getIndex(pFile, i).offset);
    }
    document rootDoc;
    rootDoc.header = getDocumentHeader(pFile, 0);
    rootDoc.isRoot = true;
    rootDoc.indexParent = 0;

    if(pFile->zgdbHeader.nodes == 1) {
        documentSchema schema2 = initSchema(3);
        addBooleanToSchema(&schema2, "bool1", 1);
        addDoubleToSchema(&schema2, "double6", 1.5);
        addIntToSchema(&schema2, "int9", -1);
        createDocument(pFile, "test1", &schema2, rootDoc);
        createDocument(pFile, "test1", &schema2, rootDoc);
        createDocument(pFile, "test3", &schema2, rootDoc);

        path p;
        p.size = 1;
        p.steps = (step*) malloc(p.size * sizeof(step));
        step s;
        strcpy(s.stepName, "test1");
        s.pType = ABSOLUTE_PATH;
        p.steps[0] = s;
        resultList list = findIfFromRoot(pFile, p);

        createDocument(pFile, "test5", &schema2, list.head->document);
        createDocument(pFile, "test6", &schema2, list.head->document);
        createDocument(pFile, "test5", &schema2, list.head->document);
        destroyResultList(&list);
        path p1;
        p1.size = 2;
        p1.steps = (step*) malloc(p1.size * sizeof(step));
        step s1;
        strcpy(s1.stepName, "test1");
        step s2;
        strcpy(s2.stepName, "test5");
        s1.pType = ABSOLUTE_PATH;
        s2.pType = ABSOLUTE_PATH;
        p1.steps[0] = s1;
        p1.steps[1] = s2;
        list = findIfFromRoot(pFile, p1);
        destroyResultList(&list);

//        list = findIfFromRoot(pFile, checkName);
//        document doc = list.head->document;
//        destroyResultList(&list);
//
//        createDocument(pFile, "test6", &schema2, doc);
//        createDocument(pFile, "test7", &schema2, doc);
//
//        list = findIfFromRoot(pFile, checkName2);
//        doc = list.head->document;
//        destroyResultList(&list);
//
//        createDocument(pFile, "test4", &schema2, doc);
//        createDocument(pFile, "test5", &schema2, doc);
//
//        list = findIfFromRoot(pFile, checkName3);
//        doc = list.head->document;
//        destroyResultList(&list);
//
//        createDocument(pFile, "test101", &schema2, doc);

        destroySchema(&schema2);
    }

//    printf("After creation:\n");
//    list = findIfFromRoot(pFile, isRootDocument);
//    rootDoc = list.head->document;
//    destroyResultList(&list);

    finish(pFile);
    return 0;
}

