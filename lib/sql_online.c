/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

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
#include "monosql.h"

#include "sql_utils.h"

#define extern
#include "sql_online.h"
#undef extern 

/*
 * Adds an entry into the `online' table.
 */
int
mono_sql_onl_add(unsigned int user_id, const char *interface)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "INSERT INTO " ONLINE_TABLE 
     " (user_id,interface) VALUES (%u,'%s')"
     , user_id, interface);

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_onl_remove(unsigned int user_id)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " ONLINE_TABLE 
     " WHERE user_id=%u", user_id);

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_onl_status(unsigned int user_id, unsigned int status)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " ONLINE_TABLE 
     " SET status='%s' WHERE user_id=%u", user_id, (status) ? "yes" : "no");

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}


/* eof */
