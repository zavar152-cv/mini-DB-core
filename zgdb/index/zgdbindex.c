#include <stdlib.h>
#include "zgdbindex.h"

uint64_t createIndex(zgdbFile* file) {
    zgdbIndex index;
    index.flag = INDEX_NEW;
    index.offset = 0;
    long offset = (long) (sizeof(zgdbHeader) + (file->zgdbHeader.indexCount) * sizeof(zgdbIndex));
    fseek(file->file, offset, SEEK_SET);
    fwrite(&index, sizeof(zgdbIndex), 1, file->file);
    file->zgdbHeader.indexCount++;
    saveHeader(file);
    return 0;
}

zgdbIndex* getIndex(zgdbFile* file, uint64_t order) {
    zgdbIndex index;
    long offset = (long) (sizeof(zgdbHeader) + order * sizeof(zgdbIndex));
    fseek(file->file, offset, SEEK_SET);
    fread(&index, sizeof(zgdbIndex), 1, file->file);
    printf("%lu", index.offset);
    return NULL;
}