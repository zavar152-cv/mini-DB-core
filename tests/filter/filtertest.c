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
        s.sType = DOCUMENT_STEP;
        s.pred = (predicate*) malloc(sizeof(predicate));
        s.pred->isInverted = false;
        s.pred->type = BY_ELEMENT_VALUE;
        s.pred->logOp = NONE;
        s.pred->nextPredicate = NULL;
        checkType byValue = {.input = "-1", .operator = EQUALS, .key = "int9"};
        s.pred->byValue = byValue;
        p.steps[0] = s;
        findIfResult ifResult = findIfFromRoot(pFile, p);

        resultList list = ifResult.documentList;

        createDocument(pFile, "test5", &schema2, list.head->document);
        documentSchema schema3 = initSchema(3);
        addBooleanToSchema(&schema3, "bool1", 1);
        addDoubleToSchema(&schema3, "double6", 4.5);
        addIntToSchema(&schema3, "int9", -1);
        destroySchema(&schema3);
        createDocument(pFile, "test1", &schema3, list.head->document);
        createDocument(pFile, "test5", &schema2, list.head->document);
        destroyResultList(&list);
        path p1;
        p1.size = 2;
        p1.steps = (step*) malloc(p1.size * sizeof(step));
        step s1;
        strcpy(s1.stepName, "test1");
        step s2;
        strcpy(s2.stepName, "int9");
        s1.pType = ABSOLUTE_PATH;
        s1.sType = DOCUMENT_STEP;
        s1.pred = NULL;
        s2.pType = ABSOLUTE_PATH;
        s2.sType = ELEMENT_STEP;
        s2.pred = NULL;
        p1.steps[0] = s1;
        p1.steps[1] = s2;
        ifResult = findIfFromRoot(pFile, p1);

        eLresultList elist = ifResult.elementList;
        destroyElResultList(&elist);



        destroySchema(&schema2);
    }

    path p;
    p.size = 1;
    p.steps = (step*) malloc(p.size * sizeof(step));
    step s;
    strcpy(s.stepName, "test1");
    s.pType = RELATIVE_PATH;
    s.sType = DOCUMENT_STEP;

    predicate* second = (predicate*) malloc(sizeof(predicate));
    second->nextPredicate = NULL;
    second->logOp = AND;
    second->type = BY_ELEMENT_VALUE;
    second->isInverted = false;
    checkType byValue0 = {.input = "1.5", .operator = EQUALS, .key = "double6"};
    second->byValue = byValue0;

    s.pred = (predicate*) malloc(sizeof(predicate));
    s.pred->isInverted = false;
    s.pred->type = BY_ELEMENT_VALUE;
    s.pred->logOp = NONE;
    s.pred->nextPredicate = second;
    checkType byValue = {.input = "-2", .operator = NOT_EQUALS, .key = "int9"};
    s.pred->byValue = byValue;
    p.steps[0] = s;
    findIfResult ifResult = findIfFromRoot(pFile, p);

    resultList list = ifResult.documentList;

    destroyResultList(&list);


    path p1;
    p1.size = 1;
    p1.steps = (step*) malloc(p1.size * sizeof(step));
    step s1;
    strcpy(s1.stepName, "int9");
    s1.pType = RELATIVE_PATH;
    s1.sType = ELEMENT_STEP;
    s1.pred = NULL;
    p1.steps[0] = s1;
    ifResult = findIfFromRoot(pFile, p1);

    eLresultList elist = ifResult.elementList;
    destroyElResultList(&elist);

    finish(pFile);
    return 0;
}

