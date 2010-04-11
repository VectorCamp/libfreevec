project(libfreevec)

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS_DEBUG "-g3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -finline-functions")

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
if(cmake_build_type_tolower STREQUAL "debug")
    set(CMAKE_BUILD_TYPE "Debug")
    message("Enabling Debug mode")
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_DEBUG})
else(cmake_build_type_tolower STREQUAL "debug")
    set(CMAKE_BUILD_TYPE "Release")
    message("Enabling Release mode")
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_RELEASE})
endif(cmake_build_type_tolower STREQUAL "debug")

if (CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DLINUX64")
	message("Enabling 64-bit support")
endif (CMAKE_SIZEOF_VOID_P MATCHES "8")

if(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_SYSTEM_NAME MATCHES Linux)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

		option(LIBFREEVEC_SSE "Enable/Disable SSE optimizations" OFF)
		if(LIBFREEVEC_SSE)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -DLIBFREEVEC_SIMD_ENGINE=sse -DLIBFREEVEC_SSE")
			message("Enabling SSE optimizations")
		endif(LIBFREEVEC_SSE)

		option(LIBFREEVEC_SSE2 "Enable/Disable SSE2 optimizations" OFF)
		if(LIBFREEVEC_SSE2)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -DLIBFREEVEC_SIMD_ENGINE=sse2")
			message("Enabling SSE2 optimizations")
		endif(LIBFREEVEC_SSE2)

		option(LIBFREEVEC_SSE3 "Enable/Disable SSE3 optimizations" OFF)
		if(LIBFREEVEC_SSE3)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3 -DLIBFREEVEC_SIMD_ENGINE=sse3")
			message("Enabling SSE3 optimizations")
		endif(LIBFREEVEC_SSE3)

		option(LIBFREEVEC_ALTIVEC "Enable/Disable AltiVec optimizations" OFF)
		if(LIBFREEVEC_ALTIVEC)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maltivec -mabi=altivec -flax-vector-conversions -DLIBFREEVEC_SIMD_ENGINE=altivec -DLIBFREEVEC_ALTIVEC")
			message("Enabling AltiVec optimizations")
		endif(LIBFREEVEC_ALTIVEC)

		option(LIBFREEVEC_NEON "Enable/Disable ARM NEON optimizations" OFF)

		if(LIBFREEVEC_NEON)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -DLIBFREEVEC_SIMD_ENGINE=neon -DLIBFREEVEC_NEON")
			message("Enabling ARM NEON optimizations")
		endif(LIBFREEVEC_NEON)
	endif(CMAKE_SYSTEM_NAME MATCHES Linux)
endif(CMAKE_COMPILER_IS_GNUCC)

subdirs(src)

option(LIBFREEVEC_TESTS "Enable/Disable Tests" ON)
if(LIBFREEVEC_TESTS)
    add_subdirectory(test)
    message("Enabling tests")
endif()

option(LIBFREEVEC_BENCH "Enable/Disable Benchmarking" ON)
if(LIBFREEVEC_BENCH)
    add_subdirectory(bench)
    message("Enabling benchmarks")
endif()
