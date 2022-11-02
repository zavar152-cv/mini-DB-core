#include "../../zgdb/format/zgdbfile.h"

int main() {
    zgdbFile* pFile = loadOrCreateZgdbFile("/tmp/test.zgdb");
    if(pFile == NULL) {
        printf("Invalid format");
        return -1;
    }
    printf("%d\n", pFile->zgdbHeader.zgdbType);
    printf("%lu\n", pFile->zgdbHeader.freeListOffset);
    printf("%lu\n", pFile->zgdbHeader.indexCount);

    pFile->zgdbHeader.indexCount = 10;
    pFile->zgdbHeader.freeListOffset = 465;
    saveHeader(pFile);



    closeZgdbFile(pFile);
    return 0;
}
