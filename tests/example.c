#include "zgdb.h"

int main() {
    //open database
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
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Nodes: %lu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %lu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %ld\n\n", pFile->zgdbHeader.fileSize);

    //get root document
    document rootDoc;
    rootDoc.header = getDocumentHeader(pFile, 0);
    rootDoc.isRoot = true;
    rootDoc.indexParent = 0;

    //if we don't have any documents then we should create them
    if(pFile->zgdbHeader.nodes == 1) {
        //build schema
        documentSchema schema2 = initSchema(4);
        addBooleanToSchema(&schema2, "bool1", 1);
        addDoubleToSchema(&schema2, "double6", 1.5);
        addIntToSchema(&schema2, "int9", -1);
        addTextToSchema(&schema2, "text1", "Hello world!");

        //create 3 docs with schema2 and attach them to root one
        createDocument(pFile, "test1", &schema2, rootDoc);
        createDocument(pFile, "test1", &schema2, rootDoc);
        createDocument(pFile, "test3", &schema2, rootDoc);

        //create request path
        path p = createPath(1);
        step s;
        strcpy(s.stepName, "test1");//find doc with this name
        s.pType = ABSOLUTE_PATH;
        s.sType = DOCUMENT_STEP;
        s.pred = (predicate*) malloc(sizeof(predicate));
        s.pred->isInverted = false;
        s.pred->type = BY_ELEMENT_VALUE;
        s.pred->logOp = NONE;
        s.pred->nextPredicate = NULL;
        checkType byValue = {.input = "-1", .operator = EQUALS, .key = "int9"};//find doc with this element
        s.pred->byValue = byValue;
        addStep(&p, s);//adding new step to path
        findIfResult ifResult = findIfFromRoot(pFile, p);//find
        destroyPath(&p);
        resultList list = ifResult.documentList;//get result

        //add another doc and attach it to first document from response
        createDocument(pFile, "test5", &schema2, list.head->document);
        documentSchema schema3 = initSchema(3);
        addBooleanToSchema(&schema3, "bool1", 1);
        addDoubleToSchema(&schema3, "double6", 4.5);
        addIntToSchema(&schema3, "int9", -1);
        createDocument(pFile, "test1", &schema3, list.head->document);
        destroySchema(&schema3);
        createDocument(pFile, "test5", &schema2, list.head->document);
        destroyResultList(&list);
        //create request with 2 steps
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

    path p0 = createPath(1);
    step s0;
    strcpy(s0.stepName, "test1");
    s0.pType = ABSOLUTE_PATH;
    s0.sType = DOCUMENT_STEP;
    s0.pred = (predicate*) malloc(sizeof(predicate));
    s0.pred->isInverted = false;
    s0.pred->type = BY_ELEMENT_VALUE;
    s0.pred->logOp = NONE;
    s0.pred->nextPredicate = NULL;
    checkType byValue1 = {.input = "4502471671294391", .operator = EQUALS, .key = "id"};//check by id
    s0.pred->byValue = byValue1;
    addStep(&p0, s0);
    findIfResult ifResult1 = findIfFromRoot(pFile, p0);
    destroyPath(&p0);
    resultList list1 = ifResult1.documentList;
    destroyResultList(&list1);

    //relative step in path
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

    documentSchema schema3 = initSchema(3);
    addBooleanToSchema(&schema3, "bool4", 1);
    addDoubleToSchema(&schema3, "double1", 4.5);
    addIntToSchema(&schema3, "int6", -1);
    createDocument(pFile, "test54", &schema3, list.head->document);
    destroySchema(&schema3);

    path p3 = createPath(1);
    step s3;
    strcpy(s3.stepName, "test54");
    s3.pType = ABSOLUTE_PATH;
    s3.sType = DOCUMENT_STEP;
    s3.pred = NULL;
    addStep(&p3, s3);
    findIfResult ifResult3 = findIfFromRoot(pFile, p3);
    resultList list3 = ifResult3.documentList;

    //update element
    printf("Before:\n");
    printDocumentElements(pFile, list3.head->document);
    updateElement(pFile, list3.head->document, "int6", "45");
    printf("After:\n");
    printDocumentElements(pFile, list3.head->document);
    destroyResultList(&list);

    //get element as result
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

