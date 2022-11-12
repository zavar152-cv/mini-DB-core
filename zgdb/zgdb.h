#ifndef ZGDBPROJECT_ZGDB_H
#define ZGDBPROJECT_ZGDB_H

#include "format/zgdbfile.h"
#include "index/zgdbindex.h"

#define INDEX_INITIAL_CAPACITY 40

zgdbFile* init(const char* path);

#endif
