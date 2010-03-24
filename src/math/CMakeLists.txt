#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(SOURCES ${SOURCES} cosf.c sinf.c tanf.c coshf.c sinhf.c tanhf.c expf.c)

include_directories(arch)
add_library(math_static STATIC ${SOURCES})
add_library(math_shared SHARED ${SOURCES})

target_link_libraries(math_shared math_static)

SET_TARGET_PROPERTIES(math_shared PROPERTIES COMPILE_FLAGS "-DTEST_LIBC")