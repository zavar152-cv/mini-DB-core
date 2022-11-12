cmake_minimum_required(VERSION 3.10)
project(ZGDBProject C)

set(CMAKE_C_STANDARD 17)

if (WIN32)
    add_executable(ZGDBProject mman-win32/mman.c mman-win32/mman.h zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h zgdb/index/list/freelist.c zgdb/index/list/freelist.h zgdb/schema/zgdbschema.c zgdb/schema/zgdbschema.h zgdb/zgdb.c zgdb/zgdb.h tests/mmap/mmaptest.c)
endif (WIN32)

if (UNIX)
    add_executable(ZGDBProject zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h zgdb/index/list/freelist.c zgdb/index/list/freelist.h zgdb/schema/zgdbschema.c zgdb/schema/zgdbschema.h zgdb/zgdb.c zgdb/zgdb.h tests/freelist/freelisttest.c)
endif (UNIX)

