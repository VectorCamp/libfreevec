project(libfreevec)

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 2.6)

if (CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(CMAKE_C_FLAGS "-fPIC -DLINUX64")
	message("Enabling 64-bit support")
endif (CMAKE_SIZEOF_VOID_P MATCHES "8")

if(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_SYSTEM_NAME MATCHES Linux)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

		option(LIBFREEVEC_SSE "Enable/Disable SSE optimizations" OFF)
		if(LIBFREEVEC_SSE)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -DSIMD_ENGINE=sse")
			message("Enabling SSE optimizations")
		endif(LIBFREEVEC_SSE)

		option(LIBFREEVEC_SSE2 "Enable/Disable SSE2 optimizations" OFF)
		if(LIBFREEVEC_SSE2)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -DSIMD_ENGINE=sse2")
			message("Enabling SSE2 optimizations")
		endif(LIBFREEVEC_SSE2)

		option(LIBFREEVEC_SSE3 "Enable/Disable SSE3 optimizations" OFF)
		if(LIBFREEVEC_SSE3)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3 -DSIMD_ENGINE=sse3")
			message("Enabling SSE3 optimizations")
		endif(LIBFREEVEC_SSE3)

		option(LIBFREEVEC_ALTIVEC "Enable/Disable AltiVec optimizations" OFF)
		if(LIBFREEVEC_ALTIVEC)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maltivec -mabi=altivec -DSIMD_ENGINE=altivec")
			message("Enabling AltiVec optimizations")
		endif(LIBFREEVEC_ALTIVEC)

		option(LIBFREEVEC_CELL "Enable/Disable Cell SPU optimizations" OFF)
		if(LIBFREEVEC_CELL)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSIMD_ENGINE=cell-spu")
			message("Enabling Cell SPU optimizations")
		endif(LIBFREEVEC_CELL)

		option(LIBFREEVEC_NEON "Enable/Disable ARM NEON optimizations" OFF)
		if(LIBFREEVEC_NEON)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -DSIMD_ENGINE=neon")
			message("Enabling ARM NEON optimizations")
		endif(LIBFREEVEC_NEON)
	endif(CMAKE_SYSTEM_NAME MATCHES Linux)
endif(CMAKE_COMPILER_IS_GNUCXX)

subdirs(src)
