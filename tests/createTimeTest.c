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

//    for (int i = 0; i < pFile->zgdbHeader.indexCount; ++i) {
//        printf("Index %d\n", i);
//        printf("Flag: %d\n", getIndex(pFile, i).flag);
//        printf("Offset: %ld\n\n", getIndex(pFile, i).offset);
//    }
    document rootDoc;
    rootDoc.header = getDocumentHeader(pFile, 0);
    rootDoc.isRoot = true;
    rootDoc.indexParent = 0;

    if(pFile->zgdbHeader.nodes == 1) {
        documentSchema schema2 = initSchema(3);
        addBooleanToSchema(&schema2, "bool1", 1);
        addDoubleToSchema(&schema2, "double6", 1.5);
        addIntToSchema(&schema2, "int9", -1);

        long diff;
        for (size_t i = 0; i < 1000000; ++i) {
            struct timespec tv0;
            clock_gettime(CLOCK_REALTIME, &tv0);
            createDocument(pFile, "test1", &schema2, rootDoc);
            struct timespec tv1;
            clock_gettime(CLOCK_REALTIME, &tv1);
            diff = tv1.tv_nsec - tv0.tv_nsec;
        }
        printf("%llu, %llu, %ld\n", pFile->zgdbHeader.nodes, pFile->zgdbHeader.fileSize, diff);
        destroySchema(&schema2);
    }


    finish(pFile);
    return 0;
}

