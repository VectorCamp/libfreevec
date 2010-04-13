#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(SOURCES memcpy.c mempcpy.c memset.c swab.c)

include_directories(arch)
add_library(string_shared SHARED ${SOURCES})
add_library(string_static STATIC ${SOURCES})

target_link_libraries(string_shared string_static)

SET_TARGET_PROPERTIES(string_shared PROPERTIES COMPILE_FLAGS "-DLIBFREEVEC_BUILD_AS_LIBC")
