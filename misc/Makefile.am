
bin_PROGRAMS = dexi purge ubl finger makeemp ccat initgoto ql_sql fsql convert initdb
ql_sql SOURCES = ql_sql.c
dexi_SOURCES = dexi.c dexi2.c wholist.c
initdb_SOURCES = initdb.c
convert_SOURCES = convert.c
purge_SOURCES = purge.c
finger_SOURCES = finger.c wholist.c
makeemp_SOURCES = makeemp.c
ccat_SOURCES = ccat.c
ubl_SOURCES = ubl.c
fsql_SOURCES = fsql.c
initgoto_SOURCES = initgoto.c

LDADD = -L../lib -lyawc -L/usr/local/mysql/lib 

CFLAGS = -I/usr/local/include/mysql -O3 -Wall
