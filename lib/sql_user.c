/* $Id$ */
/* let's start out simple. we need something to find out quickly the 
 * usernumber of a specific user, so, basically this only does a
 * translatoin from user<->number */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>	/* for strncpy */
#include <assert.h>
#include <unistd.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

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
#include "sql_user.h"
#undef extern

/* ADD USER */
int
mono_sql_u_add_user_new(const char *username, unsigned int num)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "INSERT INTO " U_TABLE " (username,id) VALUES ('%s', %u)", username, num);

    return ret;
}

int
mono_sql_rename_user(unsigned int num, const char *newname)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "UPDATE " U_TABLE " SET username='%s' WHERE id=%u", newname, num);

    return ret;
}


/* ADD USER */
int
mono_sql_u_add_user(const char *username)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "INSERT INTO " U_TABLE " (username) VALUES ('%s' )", username);

    return ret;
}

/* SET PASSWORD */
int
mono_sql_u_set_passwd( unsigned int user_id, const char *passwd )
{
    MYSQL_RES *res;
    int ret;
    char cryp[14];
    char salt[3];

    /* generate salt */
    strcpy( salt, "Lu" );

    strcpy( cryp, crypt( passwd, salt ) );

    ret = mono_sql_query(&res, "UPDATE " U_TABLE " SET password='%s' WHERE id=%u", cryp, user_id);

    return ret;
}


int
mono_sql_u_check_passwd( unsigned int user_id, const char *passwd )
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret;
    char salt[2];
    char typed[14];

    ret = mono_sql_query(&res, "SELECT password FROM " U_TABLE " WHERE id=%u", user_id);

    if ( ret == -1 ) {
	fprintf(stderr, "Query Error.\n");
        return FALSE;
    }

    if ( mysql_num_rows(res) != 1 ) {
	fprintf(stderr, "Not enough results Error.\n");
        return FALSE;
    }
    row = mysql_fetch_row(res);

    if ( strlen( row[0] ) < 3 ) {
	fprintf(stderr, "Error getting results.\n");
        return FALSE;
    }

    strncpy( salt, row[0], 2 );
    salt[2] = '\0';

    strcpy( typed, crypt( passwd, salt ) );
    ret = strcmp( row[0], typed );

/* michel: putting this free statement here gives a segfaul.t
     don't know why, but it should really be here */
/*    mono_sql_u_free_result(res); */

    if ( ret == 0 ) return TRUE;
    return FALSE;
}

/* REMOVE BY USERID */
int
mono_sql_u_kill_user(unsigned int user_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " U_TABLE " WHERE (id='%u')", user_id);

    return ret;
}

int
mono_sql_u_check_user( const char *username )
{
    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "SELECT id FROM " U_TABLE " WHERE username='%s'", username);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return FALSE;
    }
    if (mysql_num_rows(res) != 1) {
	return FALSE;
    }
    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_u_id2name(unsigned int userid, char *username)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT username FROM " U_TABLE " WHERE id=%u", userid);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) != 1) {
	return -1;
    }
    row = mysql_fetch_row(res);
    strcpy(username, row[0]);
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_name2id(const char *username, unsigned int *userid)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;

    assert(username != NULL);

    i = mono_sql_query(&res, "SELECT id FROM " U_TABLE " WHERE username='%s'", username);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) != 1) {
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", userid);
    mono_sql_u_free_result(res);
    return 0;

}

/* eof */
