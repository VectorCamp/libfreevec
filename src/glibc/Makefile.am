INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/glibc $(all_includes)
METASOURCES = AUTO
AM_CFLAGS = -DVEC_GLIBC
lib_LTLIBRARIES = libfreevec_glibc.la
libfreevec_glibc_la_LDFLAGS = -module
libfreevec_glibc_la_SOURCES = common.c findfirst.S libfreevec.h memccpy.c \
	memchr.c memcmp.c memcpy.c memfrob.c mempcpy.c memrchr.c memset.c strlen.c \
	strnlen.c
