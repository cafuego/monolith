/* $Id$ */
/* todo: set datestamp */

/* for startes, what i want to store here is simple data concerning
 * ql ships */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "libmono.h"
#include "monosql.h"
#include "sql_utils.h"
#include "sql_usertopic.h"

#define UT_TABLE "usertopic"

int
mono_sql_ut_add_entry(unsigned int user_id, unsigned int topic_id)
{
    int ret;
    MYSQL_RES *res;

    /* make this into an sql query that also checks if room & user exist */
    ret = mono_sql_query(&res,
			 "SELECT user_id,username FROM usertopic"
		     "WHERE topic_id=%u AND user_id=%u", topic_id, user_id);
    mysql_free_result(res);

    if (ret != 0) {
	return -1;
    }
    ret = mono_sql_query(&res, "INSERT INTO " UT_TABLE " (user_id,topic_id) VALUES (%u,%u)", user_id, topic_id);
    mysql_free_result(res);
    return ret;
}

int
mono_sql_ut_kill_topic(unsigned int topicnumber)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " UT_TABLE " WHERE (topic_id='%d')", topicnumber);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mysql_free_result(res);
    return 0;
}

int
mono_sql_ut_kill_user(unsigned int userid)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " UT_TABLE " WHERE (user_id='%d')", userid);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mysql_free_result(res);
    return 0;
}

int
mono_sql_ut_new_user(unsigned int user_id)
{
    unsigned int topic_id;
    int ret;
    MYSQL_RES *res;

    for (topic_id = 0; topic_id < MAXQUADS; topic_id++) {
	ret = mono_sql_query(&res,
		   "INSERT INTO usertopic (user_id,topic_id) VALUES (%u,%u)"
			     ,user_id, topic_id);
	if (ret == -1)
	    printf("\nInsert error");
    }
    mysql_free_result(res);
    return 0;
}

/* eof */
