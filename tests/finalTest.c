#include <unistd.h>
#include "zgdb.h"


void createTest(const char* fPath, const char* tPath) {
#ifdef __linux__
    zgdbFile* pFile = init("/tmp/createTest.zgdb");
#endif
#ifdef __MINGW32__
    zgdbFile* pFile = init(fPath);
#endif
    if(pFile == NULL) {
        printf("Invalid format");
        return;
    }
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Nodes: %llu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %llu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %llu\n\n", pFile->zgdbHeader.fileSize);

    document rootDoc = getRootDocument(pFile);

    documentSchema schema2 = initSchema(4);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);
    addTextToSchema(&schema2, "text0", "Low level programming");

    long diff;
    long prevDiff = 0;
    FILE* pIobuf = fopen(tPath, "a");
    for (size_t i = 0; i < 100000; ++i) {
        struct timespec tv0;
        clock_gettime(CLOCK_REALTIME, &tv0);
        createDocument(pFile, "test1", &schema2, rootDoc);
        struct timespec tv1;
        clock_gettime(CLOCK_REALTIME, &tv1);
        diff = tv1.tv_nsec - tv0.tv_nsec;
        if(diff <= 0 && prevDiff != 0)
            diff = prevDiff;
        //printf("%llu, %llu, %ld\n", pFile->zgdbHeader.nodes, pFile->zgdbHeader.fileSize, diff);
        fprintf(pIobuf, "%llu, %llu, %ld\n", pFile->zgdbHeader.nodes, pFile->zgdbHeader.fileSize, diff);
        prevDiff = diff;
    }
    printf("Finished\n");
    fclose(pIobuf);
    destroySchema(&schema2);
    finish(pFile);
}

void findIfTest(const char* fPath, const char* tPath) {
#ifdef __linux__
    zgdbFile* pFile = init("/tmp/findIfTest.zgdb");
#endif
#ifdef __MINGW32__
    zgdbFile* pFile = init(fPath);
#endif
    if(pFile == NULL) {
        printf("Invalid format");
        return;
    }
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Nodes: %llu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %llu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %llu\n\n", pFile->zgdbHeader.fileSize);

    document rootDoc = getRootDocument(pFile);

    documentSchema schema2 = initSchema(4);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);
    addTextToSchema(&schema2, "text0", "Low level programming");

    long diff;
    long prevDiff = 0;
    FILE* pIobuf = fopen(tPath, "a");
    for (size_t i = 0; i < 5000; ++i) {
        char numstr[21]; // enough to hold all numbers up to 64-bits
        sprintf(numstr, "%zu", i);
        createDocument(pFile, numstr, &schema2, rootDoc);
        struct timespec tv0;
        path p = createPath(1);
        step s;
        strcpy(s.stepName, "0");
        s.pType = ABSOLUTE_PATH;
        s.sType = DOCUMENT_STEP;
        s.pred = NULL;
        addStep(&p, s);
        clock_gettime(CLOCK_REALTIME, &tv0);
        findIfResult ifResult = findIfFromRoot(pFile, p);
        struct timespec tv1;
        clock_gettime(CLOCK_REALTIME, &tv1);
        resultList list = ifResult.documentList;
        destroyPath(&p);
        diff = tv1.tv_nsec - tv0.tv_nsec;
        if(diff <= 0 && prevDiff != 0)
            diff = prevDiff;
        fprintf(pIobuf, "%llu, %llu, %ld\n", pFile->zgdbHeader.nodes, pFile->zgdbHeader.fileSize, diff);
        prevDiff = diff;
    }
    fclose(pIobuf);
    printf("Finished\n");
    fclose(pIobuf);
    destroySchema(&schema2);
    finish(pFile);
}

void deleteTest(const char* fPath, const char* tPath) {
#ifdef __linux__
    zgdbFile* pFile = init("/tmp/deleteTest.zgdb");
#endif
#ifdef __MINGW32__
    zgdbFile* pFile = init(fPath);
#endif
    if(pFile == NULL) {
        printf("Invalid format");
        return;
    }
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Nodes: %llu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %llu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %llu\n\n", pFile->zgdbHeader.fileSize);

    document rootDoc = getRootDocument(pFile);

    documentSchema schema2 = initSchema(3);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);

    long diff;
    long prevDiff = 0;
    FILE* pIobuf = fopen(tPath, "a");
    size_t k = 0;
    for (size_t i = 0; i < 1000; ++i) {
        createDocument(pFile, "testNode", &schema2, rootDoc);
        path p = createPath(1);
        step s;
        strcpy(s.stepName, "testNode");
        s.pType = ABSOLUTE_PATH;
        s.sType = DOCUMENT_STEP;
        s.pred = NULL;
        addStep(&p, s);
        findIfResult ifResult = findIfFromRoot(pFile, p);
        resultList list = ifResult.documentList;
        document testNodeDoc = list.head->document;


        for (size_t j = 0; j < k; ++j) {
            char numstr[21]; // enough to hold all numbers up to 64-bits
            sprintf(numstr, "%zu", j);
            createDocument(pFile, numstr, &schema2, testNodeDoc);
        }

        k += 1;
        ifResult = findIfFromRoot(pFile, p);
        list = ifResult.documentList;
        testNodeDoc = list.head->document;
        destroyPath(&p);
        struct timespec tv0;
        clock_gettime(CLOCK_REALTIME, &tv0);
        deleteDocument(pFile, testNodeDoc);
        struct timespec tv1;
        clock_gettime(CLOCK_REALTIME, &tv1);

        diff = tv1.tv_nsec - tv0.tv_nsec;
        if(diff <= 0 && prevDiff != 0)
            diff = prevDiff;
        fprintf(pIobuf, "%llu, %ld\n", i, diff);
        prevDiff = diff;
    }
    fclose(pIobuf);
    printf("Finished\n");
    fclose(pIobuf);
    destroySchema(&schema2);
    finish(pFile);
}

void updateTest(const char* fPath, const char* tPath) {
#ifdef __linux__
    zgdbFile* pFile = init("/tmp/updateTest.zgdb");
#endif
#ifdef __MINGW32__
    zgdbFile* pFile = init(fPath);
#endif
    if(pFile == NULL) {
        printf("Invalid format");
        return;
    }
    printf("Type: %d\n", pFile->zgdbHeader.zgdbType);
    printf("Nodes: %llu\n", pFile->zgdbHeader.nodes);
    printf("Index count: %llu\n", pFile->zgdbHeader.indexCount);
    printf("Version: %d\n", pFile->zgdbHeader.version);
    printf("Size: %llu\n\n", pFile->zgdbHeader.fileSize);

    document rootDoc = getRootDocument(pFile);

    documentSchema schema2 = initSchema(4);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "int9", -1);
    addTextToSchema(&schema2, "text0", "Low level programming");

    long diff;
    long prevDiff = 0;
    FILE* pIobuf = fopen(tPath, "a");
    for (size_t i = 0; i < 5000; ++i) {
        char numstr[21]; // enough to hold all numbers up to 64-bits
        sprintf(numstr, "%zu", i);
        createDocument(pFile, numstr, &schema2, rootDoc);

        path p = createPath(1);
        step s;
        strcpy(s.stepName, "0");
        s.pType = ABSOLUTE_PATH;
        s.sType = DOCUMENT_STEP;
        s.pred = NULL;
        addStep(&p, s);
        findIfResult ifResult = findIfFromRoot(pFile, p);
        resultList list = ifResult.documentList;
        destroyPath(&p);
        struct timespec tv0;
        clock_gettime(CLOCK_REALTIME, &tv0);
        char numstrEl[21]; // enough to hold all numbers up to 64-bits
        sprintf(numstrEl, "%zu", i);
        updateElement(pFile, list.head->document, "int9", numstrEl);
        struct timespec tv1;
        clock_gettime(CLOCK_REALTIME, &tv1);
        diff = tv1.tv_nsec - tv0.tv_nsec;
        if(diff <= 0 && prevDiff != 0)
            diff = prevDiff;
        fprintf(pIobuf, "%llu, %ld\n", pFile->zgdbHeader.nodes, diff);
        prevDiff = diff;
    }
    fclose(pIobuf);
    printf("Finished\n");
    fclose(pIobuf);
    destroySchema(&schema2);
    finish(pFile);
}

int main() {
    printf("Launching creation and file size test...\n");
    createTest("D:/createTest.zgdb", "D:/createTest.csv");
    printf("Launching findIf test...\n");
    findIfTest("D:/findIfTest.zgdb", "D:/findIfTest.csv");
    printf("Launching delete test...\n");
    deleteTest("D:/deleteTest.zgdb", "D:/deleteTest.csv");
    printf("Launching update test...\n");
    updateTest("D:/updateTest.zgdb", "D:/updateTest.csv");
    return 0;
}

