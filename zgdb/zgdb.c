#include "zgdb.h"

void createRootDocument(zgdbFile* file, off_t offset);

zgdbFile* init(const char* path) {
    zgdbFile* pFile = loadOrCreateZgdbFile(path);
    if (pFile->zgdbHeader.indexCount == 0) {
        createIndexes(pFile, INDEX_INITIAL_CAPACITY);
        off_t offset = (off_t) (pFile->zgdbHeader.indexCount * sizeof(zgdbIndex) + sizeof(zgdbHeader));
        createRootDocument(pFile, offset);
        attachIndexToBlock(pFile, 0, offset);
    }
    pFile->pIndexesMmap = (char *) mmap(NULL, pFile->zgdbHeader.indexCount * sizeof(zgdbIndex),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               fileno(pFile->file), 0) + sizeof(zgdbHeader);
    return pFile;
}

bool finish(zgdbFile* file) {
    munmap(file->pIndexesMmap, file->zgdbHeader.indexCount * sizeof(zgdbIndex));
    return closeZgdbFile(file);
}

documentId generateId(off_t offset) {
    uint32_t now = (uint32_t)time(NULL);
    documentId id = {.timestamp = now, .offset = offset};
    return id;
}

void createRootDocument(zgdbFile* file, off_t offset) {
    fseeko(file->file, offset, SEEK_SET);
    documentId id = generateId(offset);
    documentHeader header = {.id = id, .size = sizeof(documentHeader),
                             .attrCount = 0, .indexAttached = 0, .indexBrother = 0,
                             .indexSon = 0, .name = "root"};

    fwrite(&header, sizeof(documentHeader), 1, file->file);
    file->zgdbHeader.fileSize += sizeof(documentHeader);
    saveHeader(file);
}

documentHeader getDocumentHeader(zgdbFile* file, uint64_t order) {
    zgdbIndex index = getIndex(file, order);
    fseeko(file->file, index.offset, SEEK_SET);
    documentHeader header;
    fread(&header, sizeof(documentHeader), 1, file->file);
    return header;
}

