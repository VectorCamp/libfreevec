#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBFREEVEC")
if(LIBFREEVEC_NEON)
    set(ASM_SRC glibc-memcpy-neon.S)
    #set(ASM_SRC arm_neon_memcpy1.S)
    set_source_files_properties(${ASM_SRC} PROPERTIES LANGUAGE C)
    set_source_files_properties(${ASM_SRC} PROPERTIES COMPILE_FLAGS "-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp")
endif(LIBFREEVEC_NEON)

find_path( SQLITE3_INCLUDE_DIR sqlite3.h  )
find_library( SQLITE3_LIBRARY_RELEASE sqlite3 )

include_directories(../test ${SQLITE3_INCLUDE_DIR})

add_executable(bench-memcpy bench_memcpy.c run_memcpy_bench.c bench_common.c ${ASM_SRC})
add_executable(bench-mempcpy bench_mempcpy.c run_mempcpy_bench.c bench_common.c ${ASM_SRC})
add_executable(bench-memset bench_memset.c run_memset_bench.c bench_common.c ${ASM_SRC})
add_executable(bench-swab bench_swab.c run_swab_bench.c bench_common.c ${ASM_SRC})

target_link_libraries(bench-memcpy string_static ${SQLITE3_LIBRARY_RELEASE})
target_link_libraries(bench-mempcpy string_static ${SQLITE3_LIBRARY_RELEASE})
target_link_libraries(bench-memset string_static ${SQLITE3_LIBRARY_RELEASE})
target_link_libraries(bench-swab string_static ${SQLITE3_LIBRARY_RELEASE})
