/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "routines.h"
#include "sql_utils.h"
#include "sql_llist.h"
#include "sql_convert.h"
#include "sql_express.h"

int
mono_sql_express_add(unsigned int user_id, express_t *x)
{
    MYSQL_RES *res;
    int ret = 0;
    char *message = NULL;

    (void) escape_string(x->message, &message);

    ret = mono_sql_query(&res, "INSERT INTO " X_TABLE " " 
       "(id,user_id,date,sender,recipient,message,sender_priv,override,ack) " 
       "VALUES (0,%u,FROM_UNIXTIME(%u),'%s','%s','%s',%u,%d,%d)",
       user_id,x->time,x->sender,x->recipient,message,x->sender_priv,
       x->override,x->ack);

    (void) mono_sql_u_free_result(res);

    xfree(message);
    return ret;
}

/*
 * Returns a linked list containing messages in an x-log.
 * Points both ways, for reading forward and backwards..
 */
int
mono_sql_express_list_xlog(unsigned int user_id, xlist_t **list)
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0, rows = 0, i = 0;
    xlist_t entry;

    /*
     * Coolest query in the BBS sofar :)
     */
    ret = mono_sql_query(&res, "SELECT date,sender,recipient,message," 
        "sender_priv,override,ack FROM " X_TABLE " WHERE user_id=%u", user_id );

    if (ret == -1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mono_sql_u_free_result(res);
	return -2;
    }

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	/*
         * Get message and add to list.
         */
        if( (entry.x = mono_sql_convert_row_to_x(row)) == NULL ) {
            continue;
        }

	if( mono_sql_ll_add_xlist_to_list(entry, list) == -1 ) {
	    continue;
        }
    }
    mono_sql_u_free_result(res);
    return 0;
}

#ifdef X_LOG_IS_UP_AND_CAN_BE_SEARCHED
int
mono_sql_express_search_xlog(unsigned int user_id, const char *string, sr_list_t **list)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0, rows = 0, i = 0;
    sr_list_t entry;
    char *needle = NULL;

    (void) escape_string(string, &needle);

    if(forum >= 0)
        ret = mono_sql_query( &res,
            "SELECT " \
                "m.message_id,m.forum_id,m.topic_id,f.name,t.name," \
                "u.username,m.alias,m.subject,m.flag " \
            "FROM " M_TABLE " AS m " \
                "LEFT JOIN " U_TABLE " AS u ON u.id=m.author " \
                "LEFT JOIN " F_TABLE " AS f ON f.id=m.forum_id " \
                "LEFT JOIN " T_TABLE " AS t ON m.topic_id=t.topic_id " \
            "WHERE " \
                "(m.content REGEXP '%s' OR m.subject REGEXP '%s') " \
                "AND m.forum_id=%u " \
                "GROUP BY m.message_id " \
            "ORDER BY m.forum_id, m.message_id",
                needle, needle, forum );
    else
        ret = mono_sql_query( &res,
            "SELECT " \
                "m.message_id,m.forum_id,m.topic_id,f.name,t.name," \
                "u.username,m.alias,m.subject,m.flag " \
            "FROM " M_TABLE " AS m " \
                "LEFT JOIN " U_TABLE " AS u ON u.id=m.author " \
                "LEFT JOIN " F_TABLE " AS f ON f.id=m.forum_id " \
                "LEFT JOIN " T_TABLE " AS t ON m.topic_id=t.topic_id " \
            "WHERE " \
                "m.content REGEXP '%s' OR m.subject REGEXP '%s' " \
            "ORDER BY m.forum_id, m.message_id",
                needle, needle );

    xfree(needle);

    if (ret == -1) {
        (void) mono_sql_u_free_result(res);
        return -1;
    }

    if ((rows = mysql_num_rows(res)) == 0) {
        (void) mono_sql_u_free_result(res);
        return 0;
    }

    for (i = 0; i < rows; i++) {
        row = mysql_fetch_row(res);
        if (row == NULL)
            break;
        /*
         * Get result and add to list.
         */
        if( (entry.result = mono_sql_convert_row_to_sr(row)) == NULL ) {
            continue;
        }

        if( mono_sql_ll_add_srlist_to_list(entry, list) == -1) {
            continue;
        }
    }
    (void) mono_sql_u_free_result(res);
    return rows;
}

int
mono_sql_express_expire(unsigned int user_id)
{
    MYSQL_RES *res = NULL;

    /*
     * Delete old eXpress messages from table.
     */
    (void) mono_sql_query( &res, "DELETE FROM " X_TABLE " WHERE user_id=%u AND date<%u",
        user_id, today-14 );
    (void) mono_sql_u_free_result(res);

    return;
}
#endif

/* eof */
