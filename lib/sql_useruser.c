/* $Id$ */
/* todo: set datestamp */

/* to get this thing to work, we need: */
/* 1) list friends/enemy list */
/* 2) add to friends/enemy list */
/* 3) remove from friends/enemy list */
/* 4) check if on friends/enemy list */
/* 5) kill friends/enemy list */
/* 6) we need quickx support, user->number and number->user */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>

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
#include "libfriends.h"
#include "monosql.h"
#include "sql_utils.h"
#include "sql_useruser.h"

#define UU_TABLE "useruser"

/* list all list entries for a certain person */
int
mono_sql_uu_read_list(unsigned int user_id, friend_t ** first, int flag)
{
    int i, rows;
    MYSQL_RES *res;
    MYSQL_ROW row;
    friend_t q;
    char status[10];

    if (flag & L_FRIEND) {
	strcpy(status, "friend");
    }
    if (flag & L_ENEMY) {
	strcpy(status, "enemy");
    }
    i = mono_sql_query(&res, "SELECT friend_id,quickx,username FROM useruser,user  WHERE user_id=%u AND status='%s' AND friend_id=id ORDER BY username", user_id, status);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    } else {
	rows = mysql_num_rows(res);

	*first = NULL;

	for (i = 0; i < rows; i++) {
	    row = mysql_fetch_row(res);
	    if (row == NULL)
		break;
	    /* needed results are quickx & number */
	    if (sscanf(row[0], "%u", &(q.usernum)) == -1)
		continue;
	    if (sscanf(row[1], "%d", &(q.quickx)) == -1)
		continue;
	    strcpy(q.name, row[2]);
	    add_friend_to_list(q, first);
	}
	mono_sql_u_free_result(res);
	return 0;
    }
}

/* this is very nasty and needs to be rewritten! */
int
mono_sql_uu_write_list(unsigned int user_id, friend_t * const first)
{
    friend_t *p;

    /* mono_sql_uu_clear_list(user_id); */

    p = first;
    while (p) {
	mono_sql_uu_add_entry(user_id, p->usernum, p->flags);
	if (p->quickx != -1) {
	    mono_sql_uu_add_quickx(user_id, p->usernum, p->quickx);
	}
	p = p->next;
    }
    return 0;
}

/* check if a certain user has a certain status with another user */
int
mono_sql_uu_is_on_list(unsigned int user_id, unsigned int friend_id, int flags)
{
    int ret;
    MYSQL_RES *res;
    int count;
    char status[10];

    if (flags & L_FRIEND) {
	strcpy(status, "friend");
    }
    if (flags & L_ENEMY) {
	strcpy(status, "enemy");
    }
    ret = mono_sql_query(&res, "SELECT user_id FROM " UU_TABLE
		  " WHERE (user_id='%u' AND friend_id='%u' AND status='%s')"
			 ,user_id, friend_id, status);

    if (ret == -1) {
	return FALSE;
    } else {
	count = mysql_num_rows(res);
	mono_sql_u_free_result(res);
	if (count == 1)
	    return TRUE;
	else
	    return FALSE;
    }
}

/* add entry */
int
mono_sql_uu_add_entry(unsigned int user_id, unsigned int friend_id, unsigned int flag)
{
    MYSQL_RES *res;
    int ret;
    char status[10];

    if (flag & L_FRIEND) {
	strcpy(status, "friend");
    }
    if (flag & L_ENEMY) {
	strcpy(status, "enemy");
    }
    ret = mono_sql_query(&res, "INSERT INTO " UU_TABLE " (user_id,friend_id,status) VALUES (%u, %u, '%s')\n", user_id, friend_id, status);
    return ret;
}

/* remove entry */
int
mono_sql_uu_remove_entry(unsigned int user_id, unsigned int friend_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " UU_TABLE " WHERE (user_id='%u' AND friend_id='%u')", user_id, friend_id);
    return ret;
}

/* kill an entire part of the lits for a certain user */
int
mono_sql_uu_clear_list(unsigned int user_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " UU_TABLE " WHERE (user_id='%u')", user_id);
    return ret;
}

int
mono_sql_uu_clear_list_by_type(unsigned int user_id, int flag)
{
    MYSQL_RES *res;
    int ret;
    char status[10];

    if (flag & L_FRIEND) {
	strcpy(status, "friend");
    }
    if (flag & L_ENEMY) {
	strcpy(status, "enemy");
    }
    ret = mono_sql_query(&res, "DELETE FROM " UU_TABLE " WHERE (user_id='%u' AND status='%s')", user_id, status);
    return ret;
}

/* kill an entire user */
int
mono_sql_uu_kill_user(unsigned int user_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " UU_TABLE " WHERE (user_id='%u' OR friend_id='%u')", user_id, user_id);
    return ret;
}

int
mono_sql_uu_add_quickx(unsigned int user_id, unsigned int friend_id, int quickx)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "UPDATE " UU_TABLE " SET quickx='%d' WHERE (user_id='%u' AND friend_id='%u')", quickx, user_id, friend_id);
    return ret;
}

int
mono_sql_uu_remove_quickx(unsigned int user_id, unsigned int friend_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "UPDATE " UU_TABLE " SET quickx='-1' WHERE (user_id='%u' AND friend_id='%u')", user_id, friend_id);
    return ret;
}

/* translates quickx number -> usernumber */
int
mono_sql_uu_quickx2user(unsigned int user_id, int quickx, unsigned int *friend_id)
{

    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT friend_id FROM " UU_TABLE " WHERE (user_id='%u' AND quickx='%u')", user_id, quickx);

    if (ret == -1) {
	mono_sql_u_free_result(res);
	return -1;
    } else {
	row = mysql_fetch_row(res);
	if (row)
	    sscanf(row[0], "%u", friend_id);
	mono_sql_u_free_result(res);
	return 0;
    }
}


/* translates       usernumber -> quickx number */
int
mono_sql_uu_user2quickx(unsigned int user_id, unsigned int friend_id, int *quickx)
{

    int ret;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT quickx FROM " UU_TABLE " WHERE (user_id='%u' AND friend_id='%u')", user_id, friend_id);

    if (ret == -1) {
	return -1;
    } else {
	row = mysql_fetch_row(res);
	if (row)
	    sscanf(row[0], "%d", quickx);
	mono_sql_u_free_result(res);
	return 0;
    }
}
