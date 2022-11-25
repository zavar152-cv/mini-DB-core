cmake_minimum_required(VERSION 3.10)
project(ZGDBProject C)

set(CMAKE_C_STANDARD 17)

include_directories("zgdb" "lib")

set(GENERAL_SRC zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h
        zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h zgdb/index/list/freelist.c zgdb/index/list/freelist.h
        zgdb/schema/zgdbschema.c zgdb/schema/zgdbschema.h zgdb/zgdb.c zgdb/zgdb.h zgdb/data/result/resultlist.c zgdb/data/result/resultlist.h
        zgdb/data/treestack/treestack.c zgdb/data/treestack/treestack.h zgdb/data/iterator/eliterator.c zgdb/data/iterator/eliterator.h)

set(MAIN tests/iterator/iteratortest.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-ms-bitfields")
if (WIN32)
    set(WIN_SRC lib/mman-win32/mman.c lib/mman-win32/mman.h)
    add_executable(ZGDBProject ${WIN_SRC} ${GENERAL_SRC} ${MAIN})
endif (WIN32)

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address")
    add_executable(ZGDBProject ${GENERAL_SRC} ${MAIN})
endif (UNIX)

