/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"

#include "sql_web.h"

extern char *wholist(int level, const user_t * user);

#undef FINGER_DISABLED

int main(int, char *argv[] );

int
main(int argc, char *argv[] )
{
    char *p;

    set_invocation_name(argv[0]);
    mono_setuid("guest");
    chdir(BBSDIR);

#ifdef FINGER_DISABLED
    printf("We are sorry, but the finger service is temporarily disabled.\n");
    fflush(stdout);
    exit(0);
#endif

    mono_connect_shm();
    strremcol(p = wholist(1, NULL));
    mono_detach_shm();

    printf("%s", p);
    fflush(stdout);
    xfree(p);

    mono_sql_connect();
    strremcol(p = mono_sql_web_wholist(1));
    mono_sql_detach();

    printf("%s", p);
    fflush(stdout);
    xfree(p);

    return 0;
}
