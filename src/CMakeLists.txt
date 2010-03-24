#this is just a basic CMakeLists.txt, for more information see the cmake manpage

#add definitions, compiler switches, etc.
add_definitions(-Wall -O2 -finline-functions -fivopts -frename-registers -mtune=athlon64)

include_directories(include)

subdirs(string stdlib)
