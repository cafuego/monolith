# $Id$ 

noinst_LIBRARIES = libshix.a
libshix_a_SOURCES = libshix.c regerror.c regexp.c regsub.c

#ifdef USE_MYSQL
LDADD = $(MYSQL_LIB) -lmysqlclient
#endif

AM_CFLAGS =-O3 -Wall

INCLUDES = -I../include

#ifdef USE_MYSQL
INCLUDES += $(MYSQL_INCLUDE)
#endif

#CFLAGS += -pedantic -w -W \
#  -Wall -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Werror -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch
