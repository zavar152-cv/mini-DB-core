cmake_minimum_required(VERSION 3.10)
project(ZGDBProject C)

set(CMAKE_C_STANDARD 17)

add_executable(ZGDBProject tests.c zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h)
