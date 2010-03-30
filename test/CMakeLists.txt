#this is just a basic CMakeLists.txt, for more information see the cmake manpage

link_libraries(string_static)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBFREEVEC")

add_executable(test-memcpy test_memcpy.c run_memcpy_test.c common.c)
add_executable(prepare_data prepare_data.c)
