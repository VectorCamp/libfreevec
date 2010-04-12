#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(SOURCES cosf.c sinf.c tanf.c coshf.c sinhf.c tanhf.c expf.c cos.c sin.c exp.c)

include_directories(arch)
add_library(math_static STATIC ${SOURCES})
add_library(math_shared SHARED ${SOURCES})

add_dependencies(math_shared math_common.h trig.h)
add_dependencies(math_static math_common.h trig.h)

target_link_libraries(math_shared math_static)

SET_TARGET_PROPERTIES(math_shared PROPERTIES COMPILE_FLAGS "-DLIBFREEVEC_BUILD_AS_LIBM")
