
bin_PROGRAMS = server front
server_SOURCES = server.c
front_SOURCES = front.c

CFLAGS = -O3 -Wall -DBBSDIR=\"$(prefix)\" -DBBSNAME=\"$(BBSNAME)\"

#ifdef USE_MYSQL
CFLAGS += $(MYSQL_INCLUDE)
#endif

#ifdef USE_MYSQL
LDADD = $(MYSQL_LIB) -L../lib -lmysqlclient -lyawc
#else
LDADD = $(MYSQL_LIB) -L../lib -lyawc -lmysqlclient
#endif

#CFLAGS += -W \
#  -Wall -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch
