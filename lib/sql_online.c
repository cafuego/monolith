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
#include "routines.h"

#include "sql_user.h"
#include "sql_utils.h"

#define extern
#include "sql_online.h"
#undef extern 

/*
 * Adds an entry into the `online' table.
 */
int
mono_sql_onl_add(unsigned int user_id, const char *interface, const char *doing)
{

    int ret;
    MYSQL_RES *res;
    char *fmt_doing = NULL;

    (void) escape_string( doing, &fmt_doing );

    ret = mono_sql_query(&res, "INSERT INTO " ONLINE_TABLE 
     " (user_id,interface,doing) VALUES (%u,'%s','%s')"
     , user_id, interface,doing);

    if (ret == -1) {
        xfree(fmt_doing);
	return FALSE;
    }

    mono_sql_u_free_result(res);
    xfree(fmt_doing);
    return TRUE;
}

int
mono_sql_onl_remove(unsigned int user_id, const char *interface)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " ONLINE_TABLE 
     " WHERE user_id=%u AND interface='%s'", user_id, interface);

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

int
mono_sql_onl_doing(const char* username, const char *doing)
{

    int ret;
    MYSQL_RES *res;
    unsigned int user_id = 0;
    char *fmt_doing = NULL;

    (void) mono_sql_u_name2id( username, &user_id );
    (void) escape_string( doing, &fmt_doing );

    ret = mono_sql_query(&res, "UPDATE " ONLINE_TABLE 
     " SET doing='%s' WHERE user_id=%u", user_id, fmt_doing);

    if (ret == -1) {
        xfree(fmt_doing);
	return FALSE;
    }

    mono_sql_u_free_result(res);
    xfree(fmt_doing);
    return TRUE;
}

int
mono_sql_onl_check_user(const char *username)
{

    int ret;
    MYSQL_RES *res;
    unsigned int user_id = 0;

    (void) mono_sql_u_name2id( username, &user_id );

    ret = mono_sql_query(&res, "SELECT * FROM " ONLINE_TABLE 
     " WHERE user_id=%u AND interface='web'", user_id);

    if (ret == -1)
	return FALSE;

    if( mysql_num_rows(res) == 0 ) {
        mono_sql_u_free_result(res);
        return FALSE;
    }

    mono_sql_u_free_result(res);
    return TRUE;

}

/* eof */
