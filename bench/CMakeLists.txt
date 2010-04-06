#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBFREEVEC")

find_path( SQLITE3_INCLUDE_DIR sqlite3.h  )
find_library( SQLITE3_LIBRARY_RELEASE sqlite3 )

include_directories(../test ${SQLITE3_INCLUDE_DIR})

add_executable(bench-memcpy bench_memcpy.c run_memcpy_bench.c bench_common.c)

target_link_libraries(bench-memcpy string_static ${SQLITE3_LIBRARY_RELEASE})
