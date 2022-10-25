cmake_minimum_required(VERSION 3.10)
project(ZGDBProject C)

set(CMAKE_C_STANDARD 17)

add_executable(ZGDBProject zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h zgdb/index/list/freelist.c zgdb/index/list/freelist.h zgdb/schema/zgdbschema.c zgdb/schema/zgdbschema.h tests/freelist/freelisttest.c)
