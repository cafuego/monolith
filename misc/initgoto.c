/*
 * $Id$
 *
 * This eats a flat ascii file with one goto per line as command line
 * argument. 's must be quotes with a \, as must all \'s.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#ifdef NOTDEFINED

#include "monolith.h"
#include "libmono.h"
#include "routines.h"
#include "monosql.h"

#endif

#define BUFSIZE		3000

int
main( int argc, char *argv[])
{
#ifdef NOTDEFINED
    FILE *fp;
    MYSQL_RES *res;
    char buffer[BUFSIZE];

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    fp = xfopen(argv[1], "r", FALSE);
    if (fp == NULL)
	return FALSE;

    (void) mono_sql_connect();

    while (fgets(buffer, BUFSIZE, fp) != NULL) {

	/* remove trailing '\n' */
	buffer[strlen(buffer) - 1] = '\0';

        if((mono_sql_query(&res,"INSERT INTO goto VALUES(0,'%s')", buffer)) == -1 )
            printf("Error saving!\n");
        else
            printf("Saved\n");

         mono_sql_u_free_result(res);
    }
    (void) mono_sql_detach();
    fclose(fp);
    return TRUE;
#else
return 1;
#endif
}
