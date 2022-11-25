#include "zgdbdata.h"

bool isRootDocument(document document) {
    return document.isRoot;
}

bool hasChild(document document) {
    return document.header.indexSon != 0;
}

bool hasBrother(document document) {
    return document.header.indexBrother != 0;
}

documentId generateId(off_t offset) {
    uint32_t now = (uint32_t) time(NULL);
    documentId id = {.timestamp = now, .offset = offset};
    return id;
}

off_t getElementSize(element cur) {
    off_t s = 0;
    s += sizeof(uint8_t);
    s += 13 * sizeof(char);
    switch (cur.type) {
        case TYPE_BOOLEAN:
            s += sizeof(uint8_t);
            break;
        case TYPE_INT:
            s += sizeof(int32_t);
            break;
        case TYPE_DOUBLE:
            s += sizeof(double);
            break;
        case TYPE_TEXT:
            s += sizeof(uint32_t);
            s += (off_t) (cur.textValue.size * sizeof(char));
            break;
    }
    return s;
}

off_t writeElement(zgdbFile* file, element cur) {
    off_t t = ftello(file->file);
    fwrite(&cur.type, sizeof(uint8_t), 1, file->file);
    fwrite(&cur.key, 13 * sizeof(char), 1, file->file);
    switch (cur.type) {
        case TYPE_BOOLEAN:
            fwrite(&cur.booleanValue, sizeof(uint8_t), 1, file->file);
            break;
        case TYPE_INT:
            fwrite(&cur.integerValue, sizeof(int32_t), 1, file->file);
            break;
        case TYPE_DOUBLE:
            fwrite(&cur.doubleValue, sizeof(double), 1, file->file);
            break;
        case TYPE_TEXT:
            fwrite(&cur.textValue.size, sizeof(uint32_t), 1, file->file);
            fwrite(&cur.textValue.data, cur.textValue.size * sizeof(char), 1, file->file);
            break;
    }
    return ftello(file->file) - t;
}

element* readElement(zgdbFile* file) {
    element* cur = malloc(sizeof(element));
    fread(&cur->type, sizeof(uint8_t), 1, file->file);
    fread(&cur->key, 13 * sizeof(char), 1, file->file);
    switch (cur->type) {
        case TYPE_BOOLEAN:
            fread(&cur->booleanValue, sizeof(uint8_t), 1, file->file);
            break;
        case TYPE_INT:
            fread(&cur->integerValue, sizeof(int32_t), 1, file->file);
            break;
        case TYPE_DOUBLE:
            fread(&cur->doubleValue, sizeof(double), 1, file->file);
            break;
        case TYPE_TEXT:
            fread(&cur->textValue.size, sizeof(uint32_t), 1, file->file);
            fread(&cur->textValue.data, cur->textValue.size * sizeof(char), 1, file->file);
            break;
    }
    return cur;
}