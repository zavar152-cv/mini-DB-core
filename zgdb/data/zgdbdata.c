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
        case TYPE_TEXT: {
            div_t divRes = div((int) cur.textValue.size, CHUNK_SIZE);
            int chunks = divRes.quot;
            if(divRes.rem != 0)
                chunks++;
            printf("Chunks: %d\n", chunks);

            s += sizeof(firstTextChunk);
            int count = 0;
            for (count = 0; count < chunks; ++count) {
                s += sizeof(uint8_t);
                s += sizeof(textChunk);
            }
            break;
        }
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
        case TYPE_TEXT: {
            div_t divRes = div((int) cur.textValue.size, CHUNK_SIZE);
            int chunks = divRes.quot;
            if(divRes.rem != 0)
                chunks++;
            printf("Chunks: %d\n", chunks);

            firstTextChunk firstChunk = {.size = cur.textValue.size, .nextOffset = ftello(file->file) + sizeof(firstTextChunk)};
            fwrite(&firstChunk, sizeof(firstTextChunk), 1, file->file);
            int count = 0;
            textChunk temp;
            uint8_t chunkType = TYPE_TEXT_CHUNK;
            for (count = 0; count < chunks; ++count) {
                memset(temp.data, 0, CHUNK_SIZE);
                if(count == chunks - 1) {
                    temp.nextOffset = 0;
                    for (int i = 0; i < divRes.rem; ++i) {
                        temp.data[i] = cur.textValue.data[chunks * count + i];
                    }
                } else {
                    temp.nextOffset = ftello(file->file) + sizeof(uint8_t) + sizeof(textChunk);
                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                        temp.data[i] = cur.textValue.data[chunks * count + i];
                    }
                }
                fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                fwrite(&temp, sizeof(temp), 1, file->file);
            }
            break;
        }
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
        case TYPE_TEXT: {
            firstTextChunk firstChunk;
            fread(&firstChunk, sizeof(firstTextChunk), 1, file->file);

            div_t divRes = div((int) firstChunk.size, CHUNK_SIZE);
            int chunks = divRes.quot;
            if(divRes.rem != 0)
                chunks++;
            printf("Chunks: %d\n", chunks);
            int count = 0;
            textChunk temp;
            uint8_t chunkType;
            uint32_t nextOffset = firstChunk.nextOffset;
            char buf[firstChunk.size];

            for (count = 0; count < chunks; ++count) {
                fseeko(file->file, nextOffset, SEEK_SET);
                fread(&chunkType, sizeof(uint8_t), 1, file->file);
                fread(&temp, sizeof(textChunk), 1, file->file);
                nextOffset = temp.nextOffset;
                for (int i = 0; i < CHUNK_SIZE; ++i) {
                    buf[chunks * count + i] = temp.data[i];
                }
            }
            cur->textValue.size = firstChunk.size;
            strcpy(cur->textValue.data, buf);
            break;
        }
    }
    return cur;
}