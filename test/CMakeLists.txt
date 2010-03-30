#this is just a basic CMakeLists.txt, for more information see the cmake manpage

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBFREEVEC")

add_executable(test-memcpy test_memcpy.c run_memcpy_test.c common.c)

target_link_libraries(test-memcpy string_static)

add_executable(prepare_data prepare_data.c)
