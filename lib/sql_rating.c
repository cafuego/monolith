/*
 * $Id$
 *
 * Message rating system a la grouplens in slrn.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_RATING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

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
#include "libmono.h"
#include "sql_rating.h"
#include "sql_utils.h"

int
mono_sql_rat_add_rating(unsigned int user_id, unsigned int message_id, unsigned int forum_id, int rating)
{
    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "INSERT INTO %s (user_id,message_id,forum_id,score) VALUES (%u,%u,%u,%d)",
        R_TABLE, user_id, message_id, forum_id, rating);

    (void) mono_sql_u_free_result(res);

    return ret;
}

float
mono_sql_rat_get_rating(unsigned int message_id, unsigned int forum_id)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = -1;
    float rating = 0;

    ret = mono_sql_query(&res, "SELECT AVG(score) FROM %s WHERE forum_id=%u AND message_id=%u",
        R_TABLE, forum_id, message_id );

    if (ret == -1) {
        (void) mono_sql_u_free_result(res);
        return 0;
    }
    if (mysql_num_rows(res) == 0) {
        (void) mono_sql_u_free_result(res);
        return 0;
    }
    if (mysql_num_rows(res) > 1) {
        (void) mono_sql_u_free_result(res);
        return 0;
    }
    row = mysql_fetch_row(res);

    if( (sscanf(row[0], "%f", &rating)) != 1)
         rating = 0;

    (void) mono_sql_u_free_result(res);

    return rating;
}

int
mono_sql_rat_check_rating(unsigned int user_id, unsigned int message_id, unsigned int forum_id)
{

    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT score FROM %s WHERE user_id=%u AND message_id=%u AND forum_id=%u",
        R_TABLE, user_id, message_id, forum_id);

    if(ret != -1)
        if(mysql_num_rows(res) != 0) {
            (void) mono_sql_u_free_result(res);
            return -1;
        }
    (void) mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_rat_erase_forum(unsigned int forum)
{

    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "DELETE FROM %s WHERE forum_id=%u", R_TABLE, forum);

    if(ret != 0)
	(void) log_it("sqlerr", "Unable to delete ratings for quad %d.", forum );

    (void) mono_sql_u_free_result(res);
    return ret;
}

#endif /* USE_RATING */
