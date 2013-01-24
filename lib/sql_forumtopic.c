/* $Id$ */
/* todo: set datestamp */

/* for startes, what i want to store here is simple data concerning
 * ql ships */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "routines.h"
#include "libfriends.h"
#include "sql_utils.h"
#include "sql_forum.h"
#include "sql_topic.h"
#include "sql_message.h"
#include "sql_forumtopic.h"

int
add_to_topiclist(topiclist_t element, topiclist_t ** list)
{
    topiclist_t *p, *q;

    p = (topiclist_t *) xmalloc(sizeof(topiclist_t));
    if (p == NULL)
        return -1;

    /* copy, and set as last element */
    *p = element;
    p->next = NULL;

    /* find last element in list */
    q = *list;
    if (q == NULL) {            /* empty list */
        *list = p;              /* put that in as first element */
    } else {
        while (q->next != NULL)
            q = q->next;
        q->next = p;
    }
    return 0;
}

int
dest_topiclist(topiclist_t * list)
{
    topiclist_t *p, *q;

    p = list;
    while (p) {
        q = p->next;
        xfree(p);
        p = q;
    }
    return 0;
}

int
mono_sql_ft_list_topics_by_forum(unsigned int forum_id, topiclist_t ** p)
{
    int ret, i;
    unsigned int topic_id;
    topiclist_t e;
    MYSQL_RES *res;
    MYSQL_ROW row;
    my_ulonglong rows;

    ret = mono_sql_query(&res, "SELECT topic_id FROM " FT_TABLE
		      " WHERE forum_id=%u", forum_id);

    if (ret == -1) {
	return -1;
    }
    rows = mysql_num_rows(res);

    *p = NULL;

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	if (sscanf(row[0], "%u", &topic_id) == -1)
	    continue;

	e.topic_id = topic_id;
	/* strcpy( e.forumname, row[1] ); */
	add_to_topiclist(e, p);

	/* do something ! */
    }
    mono_sql_u_free_result(res);
    return 0;
}

#ifdef ZUT
int
mono_sql_ft_is_invited(unsigned int topicnumber, unsigned int forumnumber)
{
    int ret, rows;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "SELECT count(*) FROM " FT_TABLE
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
mono_sql_ft_add_invited(unsigned int topic_id, unsigned int forum_id)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " FT_TABLE " SET "
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
#endif


/* eof */
