/* $Id$ */
/* todo: set datestamp */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

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
#include "log.h"

#define extern
#include "sql_utils.h"
#undef extern

#define SQLQUERY_BUFFER_SIZE 10
#undef SQL_DEBUG

static MYSQL mp;
static int connected = FALSE;
static int logqueries = FALSE;

static int query_length(const char *format, va_list arg);

int
mono_sql_connect()
{
    if (!(mysql_connect(&mp, "localhost", "root", NULL))) {
	log_it("sql", "could not connect to server (%s) !", mysql_error(&mp));
	return -1;
    }
    if (mysql_select_db(&mp, "bbs")) {
	log_it("sql", "could not connect to database (%s)!", mysql_error(&mp));
	return -1;
    }
    connected = TRUE;
    return 0;
}

int
mono_sql_detach()
{
    mysql_close(&mp);
    connected = FALSE;
    return 0;
}

int
mono_sql_query(MYSQL_RES ** result, const char *format,...)
{
    va_list ptr;
    int ret, length = 0;
    char *query = NULL;
    sigset_t set;

    if (connected == FALSE) {
	fprintf(stderr, "Not connected to database\n");
	return -1;
    }
    /* block signal during query */
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGABRT);
    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0)
	perror("sigprocmask");

    /* determine length, including varargs */
    va_start(ptr, format);
    length = query_length(format, ptr);
    va_end(ptr);

    query = (char *) xmalloc( length * sizeof(char) );

    /* create query string */
    va_start(ptr, format);
    ret = vsnprintf(query, length, format, ptr);
    va_end(ptr);

    /* error check & log */
    if (ret == -1) {		/* query doesn't fit in string. */
	log_it("sql", "too long query: %s", query);
        xfree(query);
	return -1;
    }
#ifdef SQL_DEBUG
    fprintf(stderr, "sql query: %s\n", query);
#endif

    /* do the query */
    ret = mysql_query(&mp, query);

    if (sigprocmask(SIG_UNBLOCK, &set, NULL) < 0)
	perror("sigprocmask");

    if (ret == -1) {
#ifdef SQL_ERROR_SHIT_THAT_MAKES_USERS_WHINE
	fprintf(stderr, "errno: %d error: %s\n", mysql_errno(&mp), mysql_error(&mp));
#endif
        log_it( "sqlerr", "%s", query);
	log_it( "sqlerr", "errno: %d error: %s", mysql_errno(&mp), mysql_error(&mp));
        xfree(query);
	return -1;		/* no results */
    }
    if ( logqueries == TRUE )
        log_it("queries", "%s", query);

    xfree(query);

    *result = mysql_store_result(&mp);

    return 0;
}

int
mono_sql_connected()
{
    return connected;
}

int
mono_sql_logqueries()
{
    return logqueries;
}

int
mono_sql_logqueries_toggle()
{
     logqueries = !logqueries;
     return logqueries;
}

char *
mono_mysql_server_info()
{
    return mysql_get_server_info(&mp);
}

char *
mono_mysql_host_info()
{
    return mysql_get_host_info(&mp);
}

/* this is a useful, if dangerous function */
/* it mallocs its own memroy. be sure to free this !! */
int
escape_string(const char *old_string, char **new_string)
{
    int i, j;
    size_t old_string_len;
    char *ns;

    old_string_len = strlen(old_string);

    if (!old_string || old_string_len < 1)
	return -1;

    ns = (char *) xmalloc(old_string_len * 2);

    if (ns == NULL)
	return -1;

    for (i = 0, j = 0; i < old_string_len; i++) {
	switch (old_string[i]) {
	    case '"':
	    case '\\':
	    case '\'':
		ns[j] = '\\';
		j++;
	}
	ns[j] = old_string[i];
	j++;
    }
    ns[j] = 0;

    *new_string = ns;

    return 0;
}

#ifdef MICHELS_TIME_FUNCS
/* convert MySQL date format to UNIX time_t */
/* YYYY-MM-DD -> long : */
/* returns 0 on success, -1 on error */
int
date_to_time(const char *datetime, time_t * time)
{
    struct tm t;
    time_t i;
    int j;

    j = sscanf(datetime, "%d-%d-%d"
	       ,&t.tm_year, &t.tm_mon, &t.tm_mday);

    t.tm_year -= 1900;
    t.tm_year -= 1;
    t.tm_hour = t.tm_min = t.tm_sec = 0;

    if (j != 6)
	return -1;

    i = mktime(&t);

    if (i == -1)
	return -1;

    *time = i;
    return 0;
}

/* convert UNIX time_t to MySQL datetime format */
/* long -> YYYY-MM-DD HH:MM:SS */
int
time_to_datetime(time_t time, char *datetime)
{
    struct tm t;

    t = *localtime(&time);

    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d"
	    ,t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    return 0;
}

/* convert UNIX time_t to MySQL date format */
/* long -> YYYY-MM-DD HH:MM:SS */
int
time_to_date(time_t time, char *datetime)
{
    struct tm t;

    t = *localtime(&time);

    sprintf(datetime, "%04d-%02d-%02d", t.tm_year, t.tm_mon, t.tm_mday);

    return 0;
}
#endif

static int
query_length(const char *format, va_list arg)
{
    int d, length = 0;
    char c, *s, tmp[20];

    length = strlen(format);

    while(*format) {
        if(*format == '%') {
            strcpy(tmp,"");
            format++;
            switch(*format) {
            case 'u':
                d = va_arg(arg, unsigned int);
                sprintf(tmp, "%u", d);
                length += strlen(tmp);
                break;
            case 'd':
                d = va_arg(arg, int);
                sprintf(tmp, "%d", d);
                length += strlen(tmp);
                break;
            case 'c':
                c = va_arg(arg, char);
                sprintf(tmp, "%c", c);
                length += strlen(tmp);
                break;
            case 's':
                s = va_arg(arg, char *);
                if( s != NULL )
                    length += strlen(s);
                break;
            }
        }
        format++;
    }
    return length;
}

/* eof */
