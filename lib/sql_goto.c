/*
 * $Id$
 *
 * sql_goto.c - Reads random goto from the goto database.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "monolith.h"

#include "extra.h"

#include "monosql.h"
#include "routines.h"
#include "sql_utils.h"
#include "sql_goto.h"

char *
mono_sql_random_goto(void)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *string;

    (void) mono_sql_query(&res, "SELECT goto FROM " GOTO_TABLE " ORDER BY RAND() LIMIT 1");

    if (mysql_num_rows(res) != 1) {
	(void) mono_sql_u_free_result(res);
	return NULL;
    }

    row = mysql_fetch_row(res);

    string = (char *) xmalloc(strlen(row[0]) + 1);
    strcpy(string, row[0]);
    (void) mono_sql_u_free_result(res);

    return string;
}

/*
 * No error checking *wince*
 */
int
mono_sql_add_goto(char *thegoto)
{

    char *mygoto = NULL;
    MYSQL_RES *res;

    (void) escape_string(thegoto, &mygoto);

    (void) mono_sql_query(&res, "INSERT INTO " GOTO_TABLE " VALUES(0,'%s')", mygoto);

    mono_sql_u_free_result(res);
    xfree(mygoto);

    return 0;

}
