/*
 * $Id$
 *
 * sql_goto.c - Reads random goto from the goto database.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>

#include "monolith.h"

#include "extra.h"

#include "monosql.h"
#include "routines.h"
#include "sql_utils.h"
#include "sql_goto.h"

int
mono_sql_random_goto(char *string)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int num = 0;

    /*
     * Empty the bastard, just in case.
     */
    mono_sql_query(&res, "SELECT COUNT(*) FROM %s", GOTO_TABLE);

    /*
     * See how many gotos we have and squeal if none.
     */
    if ((mysql_num_rows(res)) == 0) {
	(void) mysql_free_result(res);
	return -1;
    }
    if (mysql_num_rows(res) > 1) {
	(void) mysql_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    num = atoi(row[0]);
    (void) mysql_free_result(res);

    /*
     * Read a random goto.
     */
    mono_sql_query(&res, "SELECT * FROM %s WHERE ID=%d", GOTO_TABLE, (rand() % (num - 1)) + 1);

    /*
     * No goto found???
     */
    if (mysql_num_rows(res) == 0) {
	(void) mysql_free_result(res);
	return -1;
    }
    /*
     * * Too many gotos found. *eep*
     */
    if (mysql_num_rows(res) > 1) {
	(void) mysql_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);

    /*
     * Skip the id (row 0)
     */
    sprintf(string, row[1]);

    (void) mysql_free_result(res);

    return 0;
}

int
mono_sql_add_goto(char *thegoto)
{

    char *mygoto = NULL;
    MYSQL_RES *res;

    (void) escape_string(thegoto, &mygoto);

    (void) mono_sql_query(&res, "INSERT INTO %s VALUES(0,'%s')", GOTO_TABLE, mygoto);

    mysql_free_result(res);
    xfree(mygoto);

    return 0;

}
