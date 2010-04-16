#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBFREEVEC")

# libstring tests
add_executable(test-memcpy test_memcpy.c run_memcpy_test.c common.c)
add_executable(test-memcpy_aligned test_memcpy_aligned.c run_memcpy_test.c common.c)
add_executable(test-mempcpy test_mempcpy.c run_mempcpy_test.c common.c)
add_executable(test-memset test_memset.c run_memset_test.c common.c)
add_executable(test-swab test_swab.c run_swab_test.c common.c)
target_link_libraries(test-memcpy string_static)
target_link_libraries(test-memcpy_aligned string_static)
target_link_libraries(test-mempcpy string_static)
target_link_libraries(test-memset string_static)
target_link_libraries(test-swab string_static)

# libmath float tests
add_executable(test-cosf test_cosf.c run_cosf_test.c common.c)
add_executable(test-sinf test_sinf.c run_sinf_test.c common.c)
add_executable(test-expf test_expf.c run_expf_test.c common.c)
target_link_libraries(test-cosf math_static m)
target_link_libraries(test-sinf math_static m)
target_link_libraries(test-expf math_static m)

# libmath double tests
add_executable(test-cos test_cos.c run_cos_test.c common.c)
add_executable(test-sin test_sin.c run_sin_test.c common.c)
add_executable(test-exp test_exp.c run_exp_test.c common.c)
target_link_libraries(test-cos math_static m)
target_link_libraries(test-sin math_static m)
target_link_libraries(test-exp math_static m)

add_executable(prepare_data prepare_data.c)
