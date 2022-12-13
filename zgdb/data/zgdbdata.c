#include "zgdbdata.h"
#include "index/zgdbindex.h"

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

//@Deprecated
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
#ifdef DEBUG_OUTPUT
            printf("Chunks: %d\n", chunks);
#endif
            s += sizeof(firstTextChunk);
            break;
        }
    }
    return s;
}

off_t writeElement(zgdbFile* file, element cur, uint64_t firstToastIndex) {
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
#ifdef DEBUG_OUTPUT
            printf("Write chunks: %d\n", chunks);
#endif
            off_t rec = ftello(file->file);
            zgdbIndex indexToast = getIndex(file, firstToastIndex);
            toast tempToast;
            fseeko(file->file, indexToast.offset, SEEK_SET);
            fread(&tempToast, sizeof(toast), 1, file->file);
            fseeko(file->file, rec, SEEK_SET);

            off_t nextOffset = (off_t) (tempToast.used * (sizeof(uint8_t) + sizeof(textChunk)));

            firstTextChunk firstChunk = {.size = cur.textValue.size, .offsetInToast = nextOffset};//offset in toast after header
            fwrite(&firstChunk, sizeof(firstTextChunk), 1, file->file);

            rec = ftello(file->file);
            fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);

            textChunk tempTextChunk;
            uint8_t chunkType = TYPE_TEXT_CHUNK;
            for (size_t count = 0; count < chunks; ++count) {
                memset(tempTextChunk.data, 0, CHUNK_SIZE);
                if(count == chunks - 1) {
                    tempTextChunk.nextOffset = 0;
                    tempTextChunk.toastIndex = firstToastIndex;
                    for (int i = 0; i < divRes.rem; ++i) {
                        tempTextChunk.data[i] = cur.textValue.data[CHUNK_SIZE * count + i];
                    }
                } else {
                    tempTextChunk.nextOffset = (off_t) ((tempToast.used + count + 1) * (sizeof(uint8_t) + sizeof(textChunk)));//offset in toast after header
                    tempTextChunk.toastIndex = firstToastIndex;
                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                        tempTextChunk.data[i] = cur.textValue.data[CHUNK_SIZE * count + i];
                    }
                }
                fwrite(&chunkType, sizeof(uint8_t), 1, file->file);
                fwrite(&tempTextChunk, sizeof(textChunk), 1, file->file);
            }
            tempToast.used += chunks;
            fseeko(file->file, indexToast.offset, SEEK_SET);
            fwrite(&tempToast, sizeof(toast), 1, file->file);
            fseeko(file->file, rec, SEEK_SET);
            break;
        }
    }
    return ftello(file->file) - t;
}

//only for debug
element* readElement(zgdbFile* file, document doc) {
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
            off_t rec = ftello(file->file);

            div_t divRes = div((int) firstChunk.size, CHUNK_SIZE);
            int chunks = divRes.quot;
            if(divRes.rem != 0)
                chunks++;
#ifdef DEBUG_OUTPUT
            printf("Read chunks: %d\n", chunks);
#endif
            zgdbIndex indexToast = getIndex(file, doc.header.firstToastIndex);
            toast tempToast;
            fseeko(file->file, indexToast.offset, SEEK_SET);
            fread(&tempToast, sizeof(toast), 1, file->file);
            char buf[firstChunk.size];
            memset(buf, 0, firstChunk.size);
            textChunk tempChunk;
            uint8_t chunkType;
            off_t nextOffset = firstChunk.offsetInToast;

            for (size_t count = 0; count < chunks; ++count) {
                fseeko(file->file, (off_t) (indexToast.offset + sizeof(toast) + nextOffset), SEEK_SET);
                memset(tempChunk.data, 0, CHUNK_SIZE);

                fread(&chunkType, sizeof(uint8_t), 1, file->file);
                fread(&tempChunk, sizeof(textChunk), 1, file->file);
                if(count == chunks - 1) {
                    for (int i = 0; i < divRes.rem; ++i) {
                        buf[CHUNK_SIZE * count + i] = tempChunk.data[i];
                    }
                } else {
                    for (int i = 0; i < CHUNK_SIZE; ++i) {
                        buf[CHUNK_SIZE * count + i] = tempChunk.data[i];
                    }
                }
                if(tempChunk.toastIndex == tempToast.indexAttached) {
                    nextOffset = tempChunk.nextOffset;
                } else {
                    indexToast = getIndex(file, tempChunk.toastIndex);
                    fseeko(file->file, indexToast.offset, SEEK_SET);
                    fread(&tempToast, sizeof(toast), 1, file->file);
                    nextOffset = tempChunk.nextOffset;
                }
            }

            cur->textValue.size = firstChunk.size;
            cur->textValue.data = malloc(cur->textValue.size);
            strcpy(cur->textValue.data, buf);
            fseeko(file->file, rec, SEEK_SET);
            break;
        }
    }
    return cur;
}

documentHeader getDocumentHeader(zgdbFile* file, uint64_t order) {
    zgdbIndex index = getIndex(file, order);
    fseeko(file->file, index.offset, SEEK_SET);
    documentHeader header;
    fread(&header, sizeof(documentHeader), 1, file->file);
    return header;
}

document getRootDocument(zgdbFile* file) {
    document rootDoc;
    rootDoc.header = getDocumentHeader(file, 0);
    rootDoc.isRoot = true;
    rootDoc.indexParent = 0;
    return rootDoc;
}

bool isRootDocumentHeader(documentHeader header) {
    return strcmp(header.name, "root") == 0 && header.indexAttached == 0 && header.attrCount == 0 &&
           header.indexBrother == 0;
}