INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/preload $(all_includes)
METASOURCES = AUTO
AM_CFLAGS = -DVEC_GLIBC
lib_LTLIBRARIES = libfreevec_preload.la
libfreevec_preload_la_LDFLAGS = -module -release $(VERSION)
libfreevec_preload_la_SOURCES = findfirst.S libfreevec.h memccpy.c memchr.c \
	memcmp.c memcpy.c memfrob.c memmove.c mempcpy.c memrchr.c strcmp.c strlen.c \
	strncmp.c strncpy.c strnlen.c swab.c
