#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include <time.h>
#include "data/zgdbdata.h"
#include "schema/zgdbschema.h"
#include "format/zgdbfile.h"
#include "index/zgdbindex.h"

#define INDEX_INITIAL_CAPACITY 4

zgdbFile* init(const char* path);

//TODO will be replaced, only for debug
documentHeader getDocumentHeader(zgdbFile* file, uint64_t order);

#endif
