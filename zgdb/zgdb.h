#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include <time.h>
#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"

#define INDEX_INITIAL_CAPACITY 4

zgdbFile* init(const char* path);

bool finish(zgdbFile* file);

//TODO only for debug, will be removed
documentHeader getDocumentHeader(zgdbFile* file, uint64_t order);

#endif
