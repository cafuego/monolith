/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <mysql.h>

#include "monolith.h"

static MYSQL mp;

int
mono_sql_connect()
{
    if (!(mysql_connect(&mp, "localhost", "kirth", NULL)))
	return -1;

    if (mysql_select_db(&mp, "bbs"))
	return -1;

    return 0;
}

int
mono_sql_detach()
{
    mysql_close(&mp);
    return 0;
}


/* returns TRUE if users exists, FALSE otherwise */
int
mono_sql_checkuser(const char *username)
{
    int i;
    MYSQL_RES *res;
    char query[50];

    sprintf(query, "SELECT username FROM users WHERE username='%s'", username);

    if (mysql_query(&mp, query)) {
	fprintf(stderr, "Error: %s.\n ", mysql_error(&mp));
	return FALSE;
    }
    if (!(res = mysql_store_result(&mp))) {
	fprintf(stderr, "No results from query.\n");
	return FALSE;
    }
    i = mysql_num_rows(res);

    mysql_free_result(res);

    if (i == 1)
	return TRUE;
    else
	return FALSE;
}

int
mono_sql_readuser(const char *username, user_t * data)
{
    unsigned int i = 0;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[50];

    user_t user;

    memset(&user, 0, sizeof(user_t));

    sprintf(query, "SELECT * FROM users WHERE username='%s'", username);

    printf("debug query: %s\n", query);

    if (mysql_query(&mp, query)) {
	fprintf(stderr, "Error: %s.\n ", mysql_error(&mp));
	return -1;
    }
    if (!(res = mysql_store_result(&mp))) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) == 0) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) > 1) {
	fprintf(stderr, "Too many results from query.\n");
	return -1;
    }
    row = mysql_fetch_row(res);
    /* okay. bascially i will have to read all fields, and assign the
     * right variables */

    strcpy(user.username, row[0]);
    strcpy(user.password, row[1]);

    sscanf(row[2], "%ld", &user.validation_key);

    strcpy(user.RGname, row[3]);
    strcpy(user.RGaddr, row[4]);
    strcpy(user.RGzip, row[5]);
    strcpy(user.RGcity, row[6]);
    strcpy(user.RGstate, row[7]);
    strcpy(user.RGcountry, row[8]);
    strcpy(user.RGphone, row[9]);

    date_to_time(row[10], &user.birthday);

    strcpy(user.RGemail, row[11]);
    strcpy(user.RGurl, row[12]);

    strcpy(user.doing, row[13]);

    /* profile 14 */
    /* picture 15 */

    strcpy(user.xtrapflag, row[16]);
    strcpy(user.alias, row[17]);
    strcpy(user.aideline, row[18]);

    memcpy(user.lastseen, row[19], MAXQUADS * sizeof(long));
    memcpy(user.generation, row[20], MAXQUADS * sizeof(char));
    memcpy(user.forget, row[21], MAXQUADS * sizeof(char));
    memcpy(user.roominfo, row[22], MAXQUADS * sizeof(char));

    sscanf(row[23], "%lu", &user.mailnum);
    sscanf(row[24], "%u", &user.flags);
    sscanf(row[25], "%u", &user.priv);
    sscanf(row[26], "%u", &user.config_flags);
    sscanf(row[27], "%u", &user.hidden_info);
    sscanf(row[28], "%u", &user.chat);
    sscanf(row[29], "%u", &user.configuration);
    sscanf(row[30], "%u", &user.timescalled);
    sscanf(row[31], "%u", &user.posted);
    sscanf(row[32], "%lu", &user.x_s);
    sscanf(row[33], "%lu", &user.online);

    datetime_to_time(row[34], &user.firstcall);

    datetime_to_time(row[35], &user.laston_from);
    datetime_to_time(row[36], &user.laston_to);
    strcpy(user.lasthost, row[37]);

    for (i = 0; i < mysql_num_fields(res); i++)
	printf("[%2d]: %s\n", i, row[i]);

    fputc('\n', stdout);

    mysql_free_result(res);

    *data = user;
    return 0;
}

#ifdef NEW
int
mono_sql_writeuser(const user_t * user)
{
    unsigned int i = 0;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[5000], *p;

    sprintf(query, "REPLACE INTO users VALUES (");

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', %ld, ", user->username, user->password, user->validation_key);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '%s', ", user->RGname, user->RGaddr, user->RGzip);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '%s', ", user->RGcity, user->RGstate, user->RGcountry);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '%s', ", user->RGphone, /*b-day */ "", user->RGcountry);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '%s', ", user->RGemail, user->RGurl, user->doing);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '%s', ", /*profile */ "", /*picture */ "", user->xtrapflag);

    p = query + strlen(query);
    sprintf(p, "'%s', '%s', '", user->alias, user->aideline);

    memcpy(query + strlen(query), user.lastseen, MAXQUADS * sizeof(long));

    memcpy(user.lastseen, row[19], MAXQUADS * sizeof(long));
    memcpy(user.generation, row[20], MAXQUADS * sizeof(char));
    memcpy(user.forget, row[21], MAXQUADS * sizeof(char));
    memcpy(user.roominfo, row[22], MAXQUADS * sizeof(char));

    sscanf(row[23], "%lu", &user.mailnum);

    sscanf(row[24], "%u", &user.flags);

    sscanf(row[25], "%u", &user.configuration);

    sscanf(row[26], "%u", &user.timescalled);

    sscanf(row[27], "%u", &user.posted);

    sscanf(row[28], "%lu", &user.x_s);

    sscanf(row[29], "%lu", &user.online);

    datetime_to_time(row[30], &user.firstcall);

    datetime_to_time(row[31], &user.laston_from);
    datetime_to_time(row[32], &user.laston_to);
    strcpy(user.lasthost, row[32]);

    if (mysql_query(&mp, query)) {
	fprintf(stderr, "Error: %s.\n ", mysql_error(&mp));
	return -1;
    }
    if (!(res = mysql_store_result(&mp))) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    mysql_free_result(res);
    return 0;
}
#endif

/* convert MySQL datetime format to UNIX time_t */
/* YYYY-MM-DD HH:MM:SS -> long : */
/* returns 0 on success, -1 on error */
int
datetime_to_time(const char *datetime, time_t * time)
{
    struct tm t;
    time_t i;
    int j;

    j = sscanf(datetime, "%d-%d-%d %d:%d:%d"
      ,&t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);

    if (j != 6)
	return -1;

    i = mktime(&t);

    if (i == -1)
	return -1;

    *time = i;
    return 0;
}

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
	    ,t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

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
