
# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
noinst_HEADERS = libfreevec.h macros/memchr.h macros/memcmp.h macros/memcpy.h macros/memfrob.h \
                    macros/memset.h macros/strcmp.h macros/strcpy.h macros/strlen.h macros/swab.h
pkglib_LTLIBRARIES = libfreevec.la
libfreevec_la_SOURCES = bcmp.c bcopy.c bmove512.c bzero.c memccpy.c memchr.c \
		memcmp.c memcpy.c memfrob.c memmove.c mempcpy.c memrchr.c memset.c strcmp.c \
		strcpy.c strfill.c strfry.c strlen.c strncmp.c strnlen.c swab.c printvec.c \
	common.c
libfreevec_la_LDFLAGS = -module

SUBDIRS = glibc
