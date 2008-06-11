INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libc $(all_includes)
METASOURCES = AUTO
AM_CFLAGS = -DVEC_GLIBC
lib_LTLIBRARIES = libfreevec_libc.la
libfreevec_libc_la_LDFLAGS = -module -release $(VERSION)
libfreevec_libc_la_SOURCES = bmove512.c findfirst.S libfreevec.h memccpy.c \
	memchr.c memcmp.c memcpy.c memfrob.c memmove.c mempcpy.c memrchr.c memset.c \
	strcmp.c strlen.c strncmp.c strncpy.c strnlen.c swab.c
