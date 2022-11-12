#include "zgdb.h"

zgdbFile* init(const char* path) {
    zgdbFile* pFile = loadOrCreateZgdbFile(path);
    if (pFile->zgdbHeader.indexCount == 0) {
        createIndexes(pFile, INDEX_INITIAL_CAPACITY);
    }
    pFile->pIndexesMmap = (char *) mmap(NULL, pFile->zgdbHeader.indexCount * sizeof(zgdbIndex),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               fileno(pFile->file), 0) + sizeof(zgdbHeader);
    return pFile;
}