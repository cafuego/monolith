
bin_PROGRAMS = dexi purge ubl finger makeemp ccat initgoto ql_sql fsql convert initdb profile
ql_sql SOURCES = ql_sql.c

dexi_SOURCES = dexi.c dexi2.c wholist.c

initdb_SOURCES = initdb.c

convert_SOURCES = convert.c

purge_SOURCES = purge.c

finger_SOURCES = finger.c wholist.c

makeemp_SOURCES = makeemp.c

ccat_SOURCES = ccat.c
ccat_LDADD =

ubl_SOURCES = ubl.c

fsql_SOURCES = fsql.c

initgoto_SOURCES = initgoto.c

profile_SOURCES = profile.c

# this works:
LDADD = -lyawc $(MYSQL_LIB) -L../lib -lmysqlclient

# this does not work
# LDADD = -L../lib -lyawc -L/usr/local/mysql/lib -lintl

CFLAGS = -g -O3 -Wall -I../include

INCLUDES += -I../include
#ifdef USE_MYSQL
INCLUDES += $(MYSQL_INCLUDE)
#endif
