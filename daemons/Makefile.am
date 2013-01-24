
bin_PROGRAMS = server front
server_SOURCES = server.c
front_SOURCES = front.c

AM_CFLAGS= -Wall 

#ifdef USE_MYSQL
AM_CPPFLAGS = $(MYSQL_INCLUDE)
#endif

LDADD = -L../lib -lyawc
#ifdef USE_MYSQL
LDADD += $(MYSQL_LIB) -lmysqlclient 
#else
LDADD += 
#endif

#CFLAGS += -W \
#  -Wall -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch
