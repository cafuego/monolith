/* $Id$ */
/* let's start out simple. we need something to find out quickly the 
 * usernumber of a specific user, so, basically this only does a
 * translatoin from user<->number */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	/* for strncpy */
#include <assert.h>
#include <unistd.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "monolith.h"
#include "monosql.h"
#include "sql_utils.h"

#include "routines.h"

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

    if ( (ret = mysql_num_rows(res)) != 1 ) {
	fprintf(stderr, "The infamous \"Not enough results Error\"\n\r");
        fflush(stderr);
        mono_sql_u_free_result(res);
        return FALSE;
    }
    row = mysql_fetch_row(res);

    if ( strlen( row[0] ) < 3 ) {
	fprintf(stderr, "Error getting results.\n");
        fflush(stderr);
        mono_sql_u_free_result(res);
        return FALSE;
    }

    strncpy( salt, row[0], 2 );
    salt[2] = '\0';

    strcpy( typed, crypt( passwd, salt ) );
    ret = strcmp( row[0], typed );

/* michel: putting this free statement here gives a segfaul.t
     don't know why, but it should really be here */
     /* mono_sql_u_free_result(res); */

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
    strncpy(username, row[0], L_USERNAME );
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_name2id(const char *username, unsigned int *userid)
{
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    assert(username != NULL);

    ret = mono_sql_query(&res, "SELECT id FROM " U_TABLE " WHERE username='%s'", username);

    if ( ret == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }

    if (mysql_num_rows(res) != 1) {
	return -1;
    }

    row = mysql_fetch_row(res);
    ret = sscanf(row[0], "%u", userid);
    mono_sql_u_free_result(res);
    return 0;

}

int
mono_sql_read_profile(unsigned int user_id, char ** profile )
{   
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT profile FROM " U_TABLE " WHERE id=%u", user_id);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    if (mysql_num_rows(res) != 1) {
	return -1;
    }

    /* copy entire proflie into 'p' here */
    row = mysql_fetch_row(res);

    *profile = (char *) xmalloc( strlen(row[0]) * sizeof(char) );

    strcpy( *profile, row[0] );

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_write_profile(unsigned int user_id, const char * profile )
{   
    int i;
    MYSQL_RES *res;
    char *p2;

    i = escape_string( profile, &p2 );

    if (i == -1) {
	fprintf(stderr, "Can't escape query.\n");
        return -1;
    }   

    i = mono_sql_query(&res, "UPDATE " U_TABLE " set profile='%s' WHERE id=%u", p2, user_id);

    xfree( p2 );

//    if (i == -1) {
//	  fprintf(stderr, "No results from query.\n");
//        return -1;
//    }   

//    if (mysql_num_rows(res) != 1) {
//	return -1;
//    }

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_update_registration( unsigned int user_id,
		const char *name,
		const char *address,
		const char *zip,
		const char *city,
		const char *state,
		const char *country,
		const char *phone
) {

    int i;
    MYSQL_RES *res;
    char *p1, *p2, *p3, *p4, *p5, *p6, *p7;
    
    i = escape_string( name, &p1 );
    i = escape_string( address, &p2 );
    i = escape_string( zip, &p3 );
    i = escape_string( city, &p4 );
    i = escape_string( state, &p5 );
    i = escape_string( country, &p6 );
    i = escape_string( phone, &p7 );

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set name='%s', "
	" address='%s', "
	" zip='%s', "
	" city='%s', "
	" state='%s', "
	" country='%s', "
	" phone='%s'"
	" WHERE id=%u", p1, p2, p3, p4, p5, p6, p7, user_id );

    xfree( p1 ); xfree( p2 ); xfree( p3 ); xfree( p4 );
    xfree( p5 ); xfree( p6 ); xfree( p7 );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_registration( unsigned int user_id,
		char *name, char *address, char *zip, char *city, 
		char *state, char *country, char *phone
) {

    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT "
	" name,address,zip,city,state,country,phone "
	" FROM " U_TABLE
	" WHERE id=%u", user_id );

    if (ret == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);

    ret = snprintf( name, 80, "%s", row[0]);
    ret = snprintf( address, 80, "%s", row[1]);
    ret = snprintf( zip, 80, "%s", row[2]);
    ret = snprintf( city, 80, "%s", row[3]);
    ret = snprintf( state, 80, "%s", row[4]);
    ret = snprintf( country, 80, "%s", row[5]);
    ret = snprintf( phone, 80, "%s", row[6]);

    mono_sql_u_free_result(res);

    return 0;
}


int
mono_sql_u_update_email( unsigned int user_id, const char *email )
{

    int i;
    MYSQL_RES *res;
    char *p2;

    i = escape_string( email, &p2 );

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set email='%s' "
	" WHERE id=%u", p2, user_id );

    xfree( p2 );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_email( unsigned int user_id, char *email )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT email FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    ret = snprintf( email, 80, "%s", row[0]);
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_update_url( unsigned int user_id, const char *url )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set url='%s' "
	" WHERE id=%u", url, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_url( unsigned int user_id, char *url )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT url FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    ret = snprintf( url, 100, "%s", row[0]);
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_update_hidden( unsigned int user_id, int hiddeninfo )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set hiddeninfo=MAKE_SET('%d','name','address','city','country','phone','email','url','birthday','zip','state') "
	" WHERE id=%u", hiddeninfo, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;

}

int
mono_sql_u_get_hidden( unsigned int user_id, int *hiddeninfo )
{

    int i;
    MYSQL_RES *res;
    /* MYSQL_ROW *row; */

    return -1;

    i = mono_sql_query(&res, "SELECT hiddeninfo FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;

}

int
mono_sql_u_update_validation( unsigned int user_id, int validation )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set validation='%d' "
	" WHERE id=%u", validation, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_validation( unsigned int user_id, int *validation )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT validation FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);

    ret = sscanf( row[0], "%d", validation );
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_update_x_count( unsigned int user_id, int x_count )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set x_count='%ud' "
	" WHERE id=%u", x_count, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_x_count( unsigned int user_id, int *x_count )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT x_count FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);

    ret = sscanf( row[0], "%ud", x_count );
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_increase_x_count( unsigned int user_id )
{
    int i;
    /* int ret; */
    MYSQL_RES *res;
    /* MYSQL_ROW row; */

    i = mono_sql_query(&res, "UPDATE " U_TABLE
	" set x_count=x_count+1 WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    return 0;
}


int
mono_sql_u_update_post_count( unsigned int user_id, int post_count )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set post_count='%ud' "
	" WHERE id=%u", post_count, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_post_count( unsigned int user_id, int *post_count )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT post_count FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);

    ret = sscanf( row[0], "%ud", post_count );
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_increase_post_count( unsigned int user_id )
{
    int i;
    /* int ret; */
    MYSQL_RES *res;
    /* MYSQL_ROW row; */

    i = mono_sql_query(&res, "UPDATE " U_TABLE
	" set post_count=post_count+1 WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    return 0;
}


int
mono_sql_u_update_login_count( unsigned int user_id, int login_count )
{

    int i;
    MYSQL_RES *res;

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set login_count='%ud' "
	" WHERE id=%u", login_count, user_id );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_login_count( unsigned int user_id, int *login_count )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT login_count FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);

    ret = sscanf( row[0], "%ud", login_count );
    mono_sql_u_free_result(res);

    return 0;
}

int
mono_sql_u_increase_login_count( unsigned int user_id )
{
    int i;
    /* int ret; */
    MYSQL_RES *res;
    /* MYSQL_ROW row; */

    i = mono_sql_query(&res, "UPDATE " U_TABLE
	" set login_count=login_count+1 WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    return 0;
}

/* eof */
int
mono_sql_u_update_awaymsg( unsigned int user_id, const char *awaymsg )
{

    int i, ret;
    MYSQL_RES *res;

    char *p;


    ret = escape_string( awaymsg, &p );

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set awaymsg='%s' "
	" WHERE id=%u", p, user_id );

    xfree( p );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_awaymsg( unsigned int user_id, char *awaymsg )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT awaymsg FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    ret = snprintf( awaymsg, L_AWAYMSG, "%s", row[0]);
    mono_sql_u_free_result(res);

    return 0;
}
/* eof */
int
mono_sql_u_update_doing( unsigned int user_id, const char *doing )
{

    int i;
    MYSQL_RES *res;
	char *p;

    i = escape_string( doing, &p );

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set doing='%s' "
	" WHERE id=%u", p, user_id );

    xfree( p );

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_doing( unsigned int user_id, char *doing )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT doing FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    ret = snprintf( doing, L_AWAYMSG, "%s", row[0]);
    mono_sql_u_free_result(res);

    return 0;
}
int
mono_sql_u_update_xtrapflag( unsigned int user_id, const char *xtrapflag )
{

    int i;
    MYSQL_RES *res;
    char *p;
    int ret;
    
    ret = escape_string( xtrapflag, &p );

    i = mono_sql_query(&res, "UPDATE " U_TABLE 
        " set xtrapflag='%s' "
	" WHERE id=%u", p, user_id );

    xfree( p);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
        return -1;
    }   

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_u_get_xtrapflag( unsigned int user_id, char *xtrapflag )
{
    int i;
    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    i = mono_sql_query(&res, "SELECT xtrapflag FROM " U_TABLE 
	" WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    ret = snprintf( xtrapflag, L_AWAYMSG, "%s", row[0]);
    mono_sql_u_free_result(res);

    return 0;
}

#ifdef USE_ICQ

/* SET ICQ NUMBER */
int
mono_sql_u_set_icq_number( unsigned int user_id, unsigned long number )
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "UPDATE " U_TABLE " SET icq_number=%lu WHERE id=%u", number, user_id);

    return ret;
}

/* SET ICQ PASSWORD */
int
mono_sql_u_set_icq_pass( unsigned int user_id, char *pass )
{
    MYSQL_RES *res;
    int ret;
    char *tmp_pass = NULL;

#ifdef HAVE_MEMFROB
    /*
     * Do at least a weak attempt at making the password unreadable if we can.
     */
    (void) memfrob(pass, strlen(pass));
#endif
    ret = escape_string( pass, &tmp_pass );

    ret = mono_sql_query(&res, "UPDATE " U_TABLE " SET icq_pass='%s' WHERE id=%u", tmp_pass, user_id);

    xfree(tmp_pass);

    return ret;
}

unsigned long
mono_sql_u_icq_get_number( unsigned int user_id )
{

    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    unsigned long icq_num = 0;

    i = mono_sql_query(&res, "SELECT icq_number FROM " U_TABLE
        " WHERE id=%u", user_id );

    if (i == -1) {
        fprintf(stderr, "No results from query.\n");
        return -1;
    }

    if (mysql_num_rows(res) != 1) {
        return -1;
    }

    row = mysql_fetch_row(res);
    icq_num = atol(row[0]);
    mono_sql_u_free_result(res);

    return icq_num;
}

#endif
