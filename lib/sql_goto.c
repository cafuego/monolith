/*
 * $Id$
 *
 * sql_goto.c - Reads random goto from the goto database.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_MYSQL_H
  #undef HAVE_MYSQL_MYSQL_H
  #include <mysql.h>
#else
  #ifdef HAVE_MYSQL_MYSQL_H
    #undef HAVE_MYSQL_H
    #include <mysql/mysql.h>
  #endif
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
    int num = 0;
    char *string;

    (void) mono_sql_query(&res, "SELECT COUNT(*) FROM " GOTO_TABLE);
    
    if ((mysql_num_rows(res)) != 1) {
	(void) mysql_free_result(res);
	return NULL;
    }

    row = mysql_fetch_row(res);
    num = atoi(row[0]);
    (void) mysql_free_result(res);

    (void) mono_sql_query(&res, "SELECT goto FROM " GOTO_TABLE " WHERE ID=%d", 
		((rand() % num) + 1));

    if (mysql_num_rows(res) != 1) {
	(void) mysql_free_result(res);
	return NULL;
    }

    row = mysql_fetch_row(res);

    string = (char *) xmalloc(strlen(row[0]) + 1);
    strcpy(string, row[0]);
    (void) mysql_free_result(res);

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

    mysql_free_result(res);
    xfree(mygoto);

    return 0;

}
