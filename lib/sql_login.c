/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

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

    mono_sql_u_free_result(res);
    return TRUE;
}

/* eof */
