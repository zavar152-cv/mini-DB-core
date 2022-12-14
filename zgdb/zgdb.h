#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"
#include "data/result/resultlist.h"
#include "data/result/elresultlist.h"
#include "data/treestack/treestack.h"
#include "data/iterator/eliterator.h"
#include "data/iterator/dociterator.h"
#include "data/filter/zgdbfilter.h"

#define INDEX_INITIAL_CAPACITY 50000
#define INDEX_MULTIPLIER 4
#define COPY_BUFFER_SIZE 64

typedef enum updateElementStatus {
    UPDATE_OK = 0,
    ELEMENT_NOT_FOUND = 1,
    INVALID_NAME = 2,
    TYPE_PARSE_ERROR = 3
} updateElementStatus;

typedef enum createStatus {
    CREATE_OK = 0,
    OUT_OF_INDEX = 1,
    CREATE_FAILED = 2
} createStatus;

typedef enum str2intStatus {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2intStatus;

typedef enum str2doubleStatus {
    STR2DOUBLE_SUCCESS,
    STR2DOUBLE_INCONVERTIBLE
} str2doubleStatus;

typedef struct findIfResult {
    resultType type;
    resultList documentList;
    eLresultList elementList;
} findIfResult;

zgdbFile* init(const char* path);

bool finish(zgdbFile* file);

updateElementStatus updateElement(zgdbFile* file, document doc, char* key, char* input);

createStatus createDocument(zgdbFile* file, const char* name, documentSchema* schema, document parent);

void deleteDocument(zgdbFile* file, document doc);

void forEachDocument(zgdbFile* file, void (* consumer)(document, zgdbFile*), document start);

void printDocumentElements(zgdbFile* file, document document);

findIfResult findIfFromRoot(zgdbFile* file, path p);

resultList join(zgdbFile* file, document parent);

#endif
