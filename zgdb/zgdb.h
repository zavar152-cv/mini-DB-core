#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include <time.h>
#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"
#include "data/result/resultlist.h"
#include "data/treestack/treestack.h"

#define INDEX_INITIAL_CAPACITY 10
#define INDEX_MULTIPLIER 2
#define ELEMENTS_LOAD_CAPACITY 4

zgdbFile* init(const char* path);

bool finish(zgdbFile* file);

//TODO only for debug, will be removed
documentHeader getDocumentHeader(zgdbFile* file, uint64_t order);

void createDocument(zgdbFile* file, const char* name, documentSchema schema, document parent);

void deleteDocument(zgdbFile* file, document document);

void forEachDocument(zgdbFile* file, void (* consumer)(document), document start);

void printDocumentElements(zgdbFile* file, document document);

resultList findIfFromRoot(zgdbFile* file, bool (* predicate)(document));

resultList findIfFromDocument(zgdbFile* file, bool (* predicate)(document), document document);

#endif
