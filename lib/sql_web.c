/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
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
#include "sql_web.h"
#undef extern 

/*
 * Adds an entry into the `online' table.
 */
int
mono_sql_web_send_x(unsigned int sender, unsigned int recipient, const char *message)
{

    int ret;
    MYSQL_RES *res;
    char *fmt_message = NULL;

    (void) escape_string( message, &fmt_message );

    ret = mono_sql_query(&res, "INSERT INTO " WEB_X_TABLE 
     " (sender,recipient,message,date,status) VALUES (%u,'%s','%s',NOW(),'unread')"
     , sender,recipient,message);

    if (ret == -1) {
        xfree(fmt_message);
	return FALSE;
    }

    mono_sql_u_free_result(res);
    xfree(fmt_message);
    return TRUE;
}

int
mono_sql_web_remove_read(unsigned int user_id)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " WEB_X_TABLE 
     " WHERE sender=%u AND status='read'", user_id );

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_web_cleanup(unsigned int user_id)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " WEB_X_TABLE 
     " WHERE sender=%u OR recipient=%u", user_id, user_id);

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

/* eof */
