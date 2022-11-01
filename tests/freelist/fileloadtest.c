#include "../../zgdb/format/zgdbfile.h"

int main() {
    zgdbFile* pFile = loadOrCreateZgdbFile("/tmp/test.zgdb");
    printf("%d\n", pFile->zgdbHeader->zgdbType);
    printf("%lu\n", pFile->zgdbHeader->freeListOffset);
    printf("%lu\n", pFile->zgdbHeader->indexCount);

    pFile->zgdbHeader->indexCount = 10;
    pFile->zgdbHeader->freeListOffset = 456;
    saveHeader(pFile);



    closeZgdbFile(pFile);
    return 0;
}
