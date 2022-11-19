#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include <time.h>
#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"
#include "data/result/resultlist.h"

#define INDEX_INITIAL_CAPACITY 10

zgdbFile* init(const char* path);

bool finish(zgdbFile* file);

//TODO only for debug, will be removed
documentHeader getDocumentHeader(zgdbFile* file, uint64_t order);

resultList* findIfFromRoot(zgdbFile* file, bool (* predicate)(document));

resultList* findIfFromDocument(zgdbFile* file, bool (* predicate)(document), document document);

#endif
