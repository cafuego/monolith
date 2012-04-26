# $Id$ 

noinst_LIBRARIES = libicq.a

libicq_a_SOURCES = icqlib.c

AM_CFLAGS = -O3 -Wall -g
AM_CPPFLAGS = -I../include

#ifdef USE_MYSQL
AM_CPPFLAGS += $(MYSQL_INCLUDE)
#endif

#-w -W -pedantic \
#  -Wall -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Werror -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch

