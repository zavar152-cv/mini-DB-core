#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"
#include "data/result/resultlist.h"
#include "data/treestack/treestack.h"
#include "data/iterator/eliterator.h"
#include "data/iterator/dociterator.h"

#define INDEX_INITIAL_CAPACITY 20
#define INDEX_MULTIPLIER 2

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

zgdbFile* init(const char* path);

bool finish(zgdbFile* file);

updateElementStatus updateElement(zgdbFile* file, document doc, char* key, char* input);

createStatus createDocument(zgdbFile* file, const char* name, documentSchema* schema, document parent);

void deleteDocument(zgdbFile* file, document doc);

void forEachDocument(zgdbFile* file, void (* consumer)(document, zgdbFile*), document start);

void printDocumentElements(zgdbFile* file, document document);

resultList findIfFromRoot(zgdbFile* file, bool (* predicate)(document));

resultList findIfFromDocument(zgdbFile* file, bool (* predicate)(document), document document);

resultList join(zgdbFile* file, document parent);

#endif
