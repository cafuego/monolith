
bin_PROGRAMS = purge ubl finger makeemp ccat initgoto ql_sql fsql convert initdb profile
ql_sql SOURCES = ql_sql.c

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

LDADD = -lyawc $(MYSQL_LIB) -L../lib -lmysqlclient

AM_CFLAGS = -Wall  -g

#ifdef USE_MYSQL
AM_CPPFLAGS = $(MYSQL_INCLUDE)
#endif
