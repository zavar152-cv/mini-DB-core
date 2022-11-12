#include <stdlib.h>
#include "zgdbindex.h"

uint64_t createIndex(zgdbFile* file) {
    zgdbIndex index;
    index.flag = INDEX_NEW;
    index.offset = 0;
    off_t offset = (long long) (sizeof(zgdbHeader) + (file->zgdbHeader.indexCount) * sizeof(zgdbIndex));
    fseeko(file->file, offset, SEEK_SET);
    fwrite(&index, sizeof(zgdbIndex), 1, file->file);
    file->zgdbHeader.indexCount++;
    saveHeader(file);
    return 0;
}

uint64_t createIndexes(zgdbFile* file, uint64_t n) {
    zgdbIndex index[n];
    for (int i = 0; i < n; ++i) {
        index[i].flag = INDEX_NEW;
        index[i].offset = 0;
    }
    off_t offset = (long long) (sizeof(zgdbHeader) + (file->zgdbHeader.indexCount) * sizeof(zgdbIndex));
    fseeko(file->file, offset, SEEK_SET);
    fwrite(&index, sizeof(zgdbIndex), n, file->file);
    file->zgdbHeader.indexCount += n;
    saveHeader(file);
    return 0;
}

zgdbIndex getIndex(zgdbFile* file, uint64_t order) {
    zgdbIndex index;
    if (order > file->zgdbHeader.indexCount - 1) {
        index.flag = INDEX_INVALID;
        index.offset = 0;
        return index;
    }
    off_t offset = (long long) (sizeof(zgdbHeader) + order * sizeof(zgdbIndex));
    fseeko(file->file, offset, SEEK_SET);
    fread(&index, sizeof(zgdbIndex), 1, file->file);
    return index;
}

bool attachIndexToBlock(zgdbFile* file, uint64_t order, off_t blockOffset) {
    zgdbIndex index = getIndex(file, order);
    if (index.flag == INDEX_INVALID)
        return false;
    index.flag = INDEX_ALIVE;
    index.offset = blockOffset;
    off_t offset = (long long) (sizeof(zgdbHeader) + order * sizeof(zgdbIndex));
    fseeko(file->file, offset, SEEK_SET);
    fwrite(&index, sizeof(zgdbIndex), 1, file->file);
    return true;
}