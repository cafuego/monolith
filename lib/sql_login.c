/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include <mysql.h>

#include "monolith.h"
#include "libuid.h"
#include "monosql.h"

#include "sql_utils.h"
#include "sysconfig.h"

#define extern
#include "sql_login.h"
#undef extern 

#define LOGIN_TABLE	"login"
#define SYSOP_TABLE	"sysop"

/*
 * Writes a log entry into the `login' table.
 */
int
mono_sql_log_logout(unsigned int user_id, time_t login, time_t logout, const char *host, int reason)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "INSERT INTO " LOGIN_TABLE 
     " (user_id,login,logout,online,host,reason) VALUES (%u,FROM_UNIXTIME(%u),FROM_UNIXTIME(%u),SEC_TO_TIME(%u),'%s',%d)"
     , user_id, login, logout, (logout-login), host, reason);

    if (ret == -1)
	return FALSE;

    mysql_free_result(res);
    return TRUE;
}

int
mono_sql_log_sysop(char *event,...)
{

    va_list ap;
    int ret;
    MYSQL_RES *res;
    char log_entry[128];

    va_start(ap, event);
    (void) vsnprintf(log_entry, 128, event, ap);
    va_end(ap);

    ret = mono_sql_query(&res, "INSERT INTO " SYSOP_TABLE " VALUES (0,0,'%s','%s')", mono_uid, log_entry);

    if (ret == -1)
	return FALSE;

    mysql_free_result(res);
    return TRUE;

}
/* eof */
