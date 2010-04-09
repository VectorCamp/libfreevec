#this is just a basic CMakeLists.txt, for more information see the cmake manpage


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

include_directories(include)

subdirs(string stdlib math)
