/* $Id$ */
/* todo: set datestamp */

/* for startes, what i want to store here is simple data concerning
 * ql ships */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <string.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "routines.h"
#include "libfriends.h"
#include "sql_utils.h"
#include "sql_forum.h"
#include "sql_message.h"
#include "sql_user.h"

#define extern
#include "sql_userforum.h"
#undef extern

int
mono_sql_uf_unread_room(unsigned int usernum)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT forum_id FROM %s,%s WHERE %s.user_id=%u AND %s.id=%s.forum_id AND %s.lastseen < %s.highest",
			 F_TABLE, UF_TABLE, UF_TABLE, usernum, F_TABLE, UF_TABLE, UF_TABLE, F_TABLE);

    if (ret == -1)
	return -1;

    if (mysql_num_rows(res) == 0) {
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%d", &ret);
    (void) mono_sql_u_free_result(res);

    return ret;
}

int
mono_sql_uf_update_lastseen(unsigned int usernum, unsigned int forum)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0, lastseen = 0;

    ret = mono_sql_query(&res, "SELECT highest FROM %s WHERE id=%u", F_TABLE, forum);
    if (ret == -1) {
	return -1;
    }
    if (mysql_num_rows(res) == 0) {
	return -1;
    }
    row = mysql_fetch_row(res);



    sscanf(row[0], "%u", &lastseen);
    (void) mono_sql_u_free_result(res);

    (void) mono_sql_query(&res, "UPDATE %s SET lastseen=%u WHERE forum_id=%u AND user_id=%u", UF_TABLE, lastseen, forum, usernum);
    (void) mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_get_unread(unsigned int forum, unsigned int lastseen)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT COUNT(*) FROM %s WHERE message_id>%u AND forum_id=%u", M_TABLE, lastseen, forum);
    if (ret == -1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", &ret);
    (void) mono_sql_u_free_result(res);

    return ret;
}

int
add_to_userlist(userlist_t element, userlist_t ** list)
{
    userlist_t *p, *q;

    p = (userlist_t *) xmalloc(sizeof(userlist_t));
    if (p == NULL)
	return -1;

    /* copy, and set as last element */
    *p = element;
    p->next = NULL;

    /* find last element in list */
    q = *list;
    if (q == NULL) {		/* empty list */
	*list = p;		/* put that in as first element */
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 0;
}

int
dest_userlist(userlist_t * list)
{
    userlist_t *p, *q;

    p = list;
    while (p) {
	q = p->next;
	xfree(p);
	p = q;
    }
    return 0;
}

int
dest_forumlist(forumlist_t * list)
{
    forumlist_t *p, *q;

    p = list;
    while (p) {
	q = p->next;
	xfree(p);
	p = q;
    }
    return 0;
}

int
add_to_forumlist(forumlist_t element, forumlist_t ** list)
{
    forumlist_t *p, *q;

    p = (forumlist_t *) xmalloc(sizeof(forumlist_t));
    if (p == NULL)
	return -1;

    /* copy, and set as last element */
    *p = element;
    p->next = NULL;

    /* find last element in list */
    q = *list;
    if (q == NULL) {		/* empty list */
	*list = p;		/* put that in as first element */
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 0;
}

int
mono_sql_uf_add_entry(unsigned int user_id, unsigned int forum_id)
{
    int ret;
    MYSQL_RES *res;

    /* make this into an sql query that also checks if room & user exist */
    ret = mono_sql_query(&res,
			 "SELECT user_id,username FROM %s"
	   "WHERE forum_id=%u AND user_id=%u", UF_TABLE, forum_id, user_id);
    mono_sql_u_free_result(res);

    if (ret != 0) {
	return -1;
    }
    ret = mono_sql_query(&res, "INSERT INTO " UF_TABLE " (user_id,forum_id) VALUES (%u,%u)", user_id, forum_id);
    return ret;
}

int
mono_sql_uf_list_hosts_by_forum(unsigned int forumnumber, userlist_t ** p)
{
    int ret, rows, i;
    unsigned int user_id;
    userlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res,
			 "SELECT u.id,u.username FROM user u,userforum uf "
		       "WHERE forum_id=%u AND uf.user_id=u.id AND host='y' "
			 "ORDER BY u.username", forumnumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", &user_id) == -1)
	    continue;

	e.usernum = user_id;
	strcpy(e.name, row[1]);
	add_to_userlist(e, p);

    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_list_hosts_by_user(unsigned int usernumber, forumlist_t ** p)
{
    int ret, rows, i;
    unsigned int forum_id;
    forumlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT forum_id,f.name FROM %s AS f,%s AS uf WHERE f.id = uf.forum_id AND uf.user_id=%u AND uf.host='y'", F_TABLE, UF_TABLE, usernumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	if (sscanf(row[0], "%u", &forum_id) == -1)
	    continue;

	e.forum_id = forum_id;
	strcpy(e.name, row[1]);
	add_to_forumlist(e, p);
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_is_host(unsigned int usernumber, unsigned int forumnumber)
{
    int ret, rows, host;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT count(*) FROM " UF_TABLE " WHERE (user_id='%u' AND forum_id='%u' AND host='y')", usernumber, forumnumber);

    if (ret == -1) {
	fprintf(stderr, "Error in query.\n");
	return FALSE;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	fprintf(stderr, "internal error\n");
	return FALSE;
    }
    row = mysql_fetch_row(res);
    if (sscanf(row[0], "%d", &host) == -1)
	return FALSE;
    mono_sql_u_free_result(res);
    if (host == 0)
	return FALSE;
    return TRUE;
}


int
mono_sql_uf_is_a_host(unsigned int usernumber)
{
    int ret, rows, host;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT count(*) FROM " UF_TABLE " WHERE (user_id='%u' AND host='y')", usernumber);

    if (ret == -1) {
	fprintf(stderr, "Error in query.\n");
	return FALSE;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	fprintf(stderr, "internal error\n");
	return FALSE;
    }
    row = mysql_fetch_row(res);
    if (sscanf(row[0], "%d", &host) == -1)
	return FALSE;
    mono_sql_u_free_result(res);
    if (host == 0)
	return FALSE;
    return TRUE;
}

int
mono_sql_uf_add_host(unsigned int user_id, unsigned int forum_id)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET "
			 " host='y' WHERE user_id=%u AND forum_id=%u"
			 ,user_id, forum_id);
    mono_sql_u_free_result(res);
    if (ret != 0) {
	fprintf(stderr, "Some sort of error (addhost).\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_remove_host(unsigned int usernumber, unsigned int forumnumber)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET host='n' WHERE (user_id='%u' AND forum_id='%u')", usernumber, forumnumber);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_kill_forum(unsigned int forumnumber)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " UF_TABLE " WHERE (forum_id='%u')", forumnumber);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_kill_user(unsigned int userid)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " UF_TABLE " WHERE (user_id='%u')", userid);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_list_invited_by_forum(unsigned int forumnumber, userlist_t ** p)
{
    int ret, rows, i;
    unsigned int user_id;
    userlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res,
			 "SELECT u.id,u.username "
			 "FROM user u,userforum uf "
	       "WHERE forum_id=%u AND uf.user_id=u.id AND status='invited' "
			 "ORDER BY u.username"
			 ,forumnumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", &user_id) == -1)
	    continue;

	e.usernum = user_id;
	strcpy(e.name, row[1]);
	add_to_userlist(e, p);

    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_list_invited_by_user(unsigned int usernumber, forumlist_t ** p)
{
    int ret, rows, i;
    unsigned int forum_id;
    forumlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT forum_id FROM " UF_TABLE
		      " WHERE user_id=%u AND status='invited'", usernumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	if (sscanf(row[0], "%u", &forum_id) == -1)
	    continue;

	e.forum_id = forum_id;
	/* strcpy( e.forumname, row[1] ); */
	add_to_forumlist(e, p);

	/* do something ! */
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_is_invited(unsigned int usernumber, unsigned int forumnumber)
{
    int ret, rows;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "SELECT count(*) FROM " UF_TABLE
			 " WHERE (user_id='%u' AND forum_id='%u' AND status='invited' )", usernumber, forumnumber);

    if (ret == -1) {
	fprintf(stderr, "No results from query.\n");
	return FALSE;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	fprintf(stderr, "internal error\n");
	mono_sql_u_free_result(res);
	return FALSE;
    }
    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_uf_add_invited(unsigned int user_id, unsigned int forum_id)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET "
			 " status='invited' WHERE user_id=%u AND forum_id=%u"
			 ,user_id, forum_id);

    mono_sql_u_free_result(res);
    if (ret != 0) {
	fprintf(stderr, "Some sort of error (addinvited).\n");
	return -1;
    }
    return 0;
}

int
mono_sql_uf_remove_invited(unsigned int usernumber, unsigned int forumnumber)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET status='normal' WHERE (user_id='%u' AND forum_id='%u')", usernumber, forumnumber);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}


int
mono_sql_uf_list_kicked_by_forum(unsigned int forumnumber, userlist_t ** p)
{
    int ret, rows, i;
    unsigned int user_id;
    userlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res,
			 "SELECT u.id,u.username "
			 "FROM user u,userforum uf "
		"WHERE forum_id=%u AND uf.user_id=u.id AND status='kicked' "
			 "ORDER BY u.username"
			 ,forumnumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", &user_id) == -1)
	    continue;

	e.usernum = user_id;
	strcpy(e.name, row[1]);
	add_to_userlist(e, p);

    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_list_kicked_by_user(unsigned int usernumber, forumlist_t ** p)
{
    int ret, rows, i;
    unsigned int forum_id;
    forumlist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;

    ret = mono_sql_query(&res, "SELECT forum_id FROM " UF_TABLE " WHERE user_id=%u AND status='kicked'", usernumber);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	if (sscanf(row[0], "%u", &forum_id) == -1)
	    continue;

	e.forum_id = forum_id;
	/* strcpy( e.forumname, row[1] ); */
	add_to_forumlist(e, p);

	/* do something ! */
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_is_kicked(unsigned int user_id, unsigned int forum_id)
{
    int ret, rows;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "SELECT count(*) FROM " UF_TABLE " WHERE (user_id='%u' AND forum_id='%u' AND status='kicked')", user_id, forum_id);

    if (ret == -1) {
	fprintf(stderr, "No results from query.\n");
	return FALSE;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	fprintf(stderr, "internal error\n");
	mono_sql_u_free_result(res);
	return FALSE;
    }
    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_uf_add_kicked(unsigned int user_id, unsigned int forum_id)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET "
			 " status='kicked' WHERE user_id=%u AND forum_id=%u"
			 ,user_id, forum_id);
    mono_sql_u_free_result(res);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error (addkicked).\n");
	return -1;
    }
    return 0;
}

int
mono_sql_uf_remove_kicked(unsigned int usernumber, unsigned int forumnumber)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " UF_TABLE " SET status='normal' WHERE (user_id='%u' AND forum_id='%u')", usernumber, forumnumber);

    if (ret != 0) {
	fprintf(stderr, "Some sort of error.\n");
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_uf_new_user(unsigned int user_id)
{
    unsigned int forum_id;
    int ret;
    MYSQL_RES *res;

    for (forum_id = 0; forum_id < MAXQUADS; forum_id++) {
	ret = mono_sql_query(&res,
		   "INSERT INTO userforum (user_id,forum_id) VALUES (%u,%u)"
			     ,user_id, forum_id);
	if (ret == -1)
	    printf("\nInsert error");
    }
    mono_sql_u_free_result(res);
    return 0;
}

#define BUFFSIZE	200

int
mono_sql_uf_whoknows(unsigned int forum_id, char **result)
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int i = 0, j = 0, rows = 0, ret = 0;
    char line[BUFFSIZE];

    ret = mono_sql_query(&res, "SELECT uf.host,u.username FROM %s AS u LEFT JOIN %s "
    " AS uf ON u.id=uf.user_id WHERE uf.forum_id=%u AND uf.status='invited'"
			 ,U_TABLE, UF_TABLE, forum_id);

    if (ret == -1) {
	fprintf(stdout, "Query error\n");
	fflush(stdout);
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mono_sql_u_free_result(res);
	return 0;
    }

    *result = (char *) xmalloc( (rows * BUFFSIZE) * sizeof(char));
    strcpy(*result, "\n");

    for (i = 0; i < rows; i++) {
        j++;
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	sprintf(line, "%s%-24s", (EQ(row[0], "y")) ? "\1r" : "\1g", row[1]);
	if ((j % 3) == 0) {
	    strcat(line, "\n");
	}
        strcat(*result, line);
    }
    strcat(*result, "\n");

    return rows;

}

/* eof */
