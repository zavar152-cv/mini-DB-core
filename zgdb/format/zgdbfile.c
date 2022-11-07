#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include "zgdbfile.h"

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

uint32_t getZgdbFormat() {
    unsigned char format[] = {'Z', 'G', 'D', 'B'};
    return (uint32_t)format[3] << 24 |
           (uint32_t)format[2] << 16 |
           (uint32_t)format[1] << 8  |
           (uint32_t)format[0];
}

uint8_t getVersion() {
    return 1;
}

zgdbFile* loadOrCreateZgdbFile(const char* path) {//TODO printf errors
    if (file_exists(path)) {
        FILE* file = fopen(path, "rb+");
        zgdbHeader header;
        fseeko(file, 0, SEEK_SET);
        fread(&header, sizeof(zgdbHeader), 1, file);
        zgdbFile* zgdb = (zgdbFile*) malloc(sizeof(zgdbFile));
        if(header.zgdbType != getZgdbFormat())
            return NULL;
        if(header.version != getVersion())
            return NULL;
        zgdb->zgdbHeader = header;
        zgdb->file = file;
        return zgdb;
    } else {
        FILE* file = fopen(path, "wb+");
        zgdbHeader* header = (zgdbHeader*) malloc(sizeof(zgdbHeader));
        header->indexCount = 0;
        header->freeListOffset = 0;
        header->zgdbType = getZgdbFormat();
        header->betweenSpace = 0;
        header->version = 1;
        header->fileSize = sizeof(zgdbHeader);
        fseeko(file, 0, SEEK_SET);
        fwrite(header, sizeof(zgdbHeader), 1, file);
        zgdbFile* zgdb = (zgdbFile*) malloc(sizeof(zgdbFile));
        zgdb->zgdbHeader = *header;
        zgdb->file = file;
        return zgdb;
    }
}

uint8_t closeZgdbFile(zgdbFile* file) {
    fclose(file->file);
    free(file);
    return 0;
}

void saveHeader(zgdbFile* file) {
    fseeko(file->file, 0, SEEK_SET);
    zgdbHeader toFile = file->zgdbHeader;
    fwrite(&toFile, sizeof(zgdbHeader), 1, file->file);
}
