/* $Id$ */
/* message subsystem */

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
#include <fcntl.h>

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
#include "routines.h"
#include "monosql.h"
#include "sql_utils.h"
#include "sql_forum.h"
#include "sql_topic.h"
#include "sql_user.h"
#include "sql_userforum.h"
#ifdef USE_RATING
  #include "sql_rating.h"
#endif
#include "sql_message.h"

static int _mono_sql_mes_add_mes_to_list(mlist_t entry, mlist_t ** list);
static message_t * _mono_sql_mes_row_to_mes(MYSQL_ROW row);

int
mono_sql_mes_add(message_t *message)
{
    MYSQL_RES *res;
    int ret = 0;
    char *alias = NULL, *subject = NULL, *content = NULL, *reply_alias = NULL;

    if( (message->m_id = mono_sql_f_get_new_message_id(message->f_id)) == 0)
        return -1;

    (void) escape_string(message->alias, &alias);
    (void) escape_string(message->subject, &subject);
    (void) escape_string(message->content, &content);
    (void) escape_string(message->reply_alias, &reply_alias);

    /*
     * We add the date here, so it really is the date the message was
     * added to the BBS. TIMESTAMP in the table will not be used, as it might
     * be updated. That's better used to check if a message was modified.
     */
    message->date = time(0);

    ret = mono_sql_query(&res, "INSERT INTO " M_TABLE " (message_id," \
       "forum_id,topic_id,author,alias,subject,content,date,flag," \
       "r_message_id,r_forum_id,r_topic_id,r_author,r_alias," \
       "m_message_id,m_forum_id,m_topic_id,m_author,m_date,m_reason,deleted) " \
       "VALUES (%u,%u,%u,%u,'%s','%s','%s',FROM_UNIXTIME(%u),'%s'," \
       "%u,%u,%u,%u,'%s',%u,%u,%u,%u,FROM_UNIXTIME(%u),'%s','n')",
            message->m_id, message->f_id, message->t_id, message->a_id,
            alias, subject, content, message->date, message->flag,
            message->reply_m_id, message->reply_f_id, message->reply_t_id,
            message->reply_a_id, reply_alias, message->orig_m_id,
            message->orig_f_id, message->orig_t_id, message->orig_a_id,
            message->orig_date, message->mod_reason );

    (void) mysql_free_result(res);

    xfree(alias);
    xfree(subject);
    xfree(content);
    xfree(reply_alias);

#ifdef USE_RATING
    /*
     * To prevent an autor from uprating their own posts.
     */
    (void) mono_sql_rat_add_rating(message->a_id, message->m_id, message->f_id, 0);
#endif

    return ret;
}

int
mono_sql_mes_mark_deleted(unsigned int id, unsigned int forum)
{
    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "UPDATE " M_TABLE " SET deleted='y' WHERE message_id=%u AND forum_id=%u", id, forum );

    (void) mysql_free_result(res);

    return ret;
}
    

int
mono_sql_mes_remove(unsigned int id, unsigned int forum)
{
    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "DELETE FROM " M_TABLE " WHERE message_id=%u AND forum_id=%u", id, forum);
    (void) mysql_free_result(res);

    return ret;
}

int
mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *data)
{

    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    message_t *message;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT m.message_id,m.forum_id,m.topic_id," \
        "m.author,u.username,m.alias,m.subject,m.content,UNIX_TIMESTAMP(m.date)," \
        "AVG(r.score),m.flag,f.name,t.name,m.r_message_id,m.r_forum_id,m.r_topic_id," \
        "m.r_author,ur.username,m.r_alias,fr.name,tr.name,m.m_message_id," \
        "m.m_forum_id,m.m_topic_id,m.m_author,um.username,UNIX_TIMESTAMP(m.m_date)," \
        "fm.name,tm.name,m.m_reason FROM " M_TABLE " AS m " \
        "LEFT JOIN " U_TABLE " AS u ON u.id=m.author " \
        "LEFT JOIN " U_TABLE " AS ur ON ur.id=m.r_author " \
        "LEFT JOIN " U_TABLE " AS um ON um.id=m.m_author " \
        "LEFT JOIN " F_TABLE " AS f ON f.id=m.forum_id " \
        "LEFT JOIN " F_TABLE " AS fr ON fr.id=m.r_forum_id " \
        "LEFT JOIN " F_TABLE " AS fm ON f.id=m.m_forum_id " \
        "LEFT JOIN " T_TABLE " AS t ON t.topic_id=m.topic_id " \
        "LEFT JOIN " T_TABLE " AS tr ON tr.topic_id=m.r_topic_id " \
        "LEFT JOIN " T_TABLE " AS tm ON tm.topic_id=m.m_topic_id" \
        "," R_TABLE " AS r WHERE m.message_id IN(r.message_id) " \
        "AND m.forum_id=%u AND m.message_id=%u", forum, id );

#ifdef OLD_SHIT
    ret = mono_sql_query(&res, "SELECT message_id,topic_id,m.forum_id,author,alias,subject,UNIX_TIMESTAMP(date),type,priv,deleted FROM " M_TABLE " WHERE m.message_id=%u AND m.forum_id=%u", id, forum);
#endif

    if (ret == -1) {
 	(void) mysql_free_result(res);
	return -1;
    }
    if (mysql_num_rows(res) == 0) {
	(void) mysql_free_result(res);
	return -2;
    }
    if (mysql_num_rows(res) > 1) {
	(void) mysql_free_result(res);
	return -3;
    }
    row = mysql_fetch_row(res);
    message = _mono_sql_mes_row_to_mes(row);
    (void) mysql_free_result(res);

    *data = *message;

    return 0;
}

/*
 * Returns a linked list containing messages in a forum.
 * Points both ways, for reading forward and backwards..
 */
int
mono_sql_mes_list_forum(unsigned int forum, unsigned int start, mlist_t ** list)
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0, rows = 0, i = 0;
    mlist_t entry;
    message_t *message;

    /*
     * Coolest query in the BBS sofar :)
     */
    ret = mono_sql_query(&res, "SELECT m.message_id,m.forum_id,m.topic_id," \
        "m.author,u.username,m.alias,m.subject,m.content,UNIX_TIMESTAMP(m.date)," \
        "AVG(r.score),m.flag,f.name,t.name,m.r_message_id,m.r_forum_id,m.r_topic_id," \
        "m.r_author,ur.username,m.r_alias,fr.name,tr.name,m.m_message_id," \
        "m.m_forum_id,m.m_topic_id,m.m_author,um.username,UNIX_TIMESTAMP(m.m_date)," \
        "fm.name,tm.name,m.m_reason FROM " M_TABLE " AS m " \
        "LEFT JOIN " U_TABLE " AS u ON u.id=m.author " \
        "LEFT JOIN " U_TABLE " AS ur ON ur.id=m.r_author " \
        "LEFT JOIN " U_TABLE " AS um ON um.id=m.m_author " \
        "LEFT JOIN " F_TABLE " AS f ON f.id=m.forum_id " \
        "LEFT JOIN " F_TABLE " AS fr ON fr.id=m.r_forum_id " \
        "LEFT JOIN " F_TABLE " AS fm ON f.id=m.m_forum_id " \
        "LEFT JOIN " T_TABLE " AS t ON t.topic_id=m.topic_id " \
        "LEFT JOIN " T_TABLE " AS tr ON tr.topic_id=m.r_topic_id " \
        "LEFT JOIN " T_TABLE " AS tm ON tm.topic_id=m.m_topic_id" \
        "," R_TABLE " AS r WHERE m.message_id IN(r.message_id) " \
        "AND m.forum_id=%u AND m.message_id>%u GROUP BY m.message_id",
            forum, start );

#ifdef OLD_SHIT
    ret = mono_sql_query(&res, "SELECT message_id,topic_id,forum_id,author,alias,subject,UNIX_TIMESTAMP(date) AS date,type,priv,deleted FROM " M_TABLE " WHERE message_id>%u AND forum_id=%u ORDER BY message_id", start, forum);
#endif

    
    if (ret == -1) {
	(void) mysql_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mysql_free_result(res);
	return -2;
    }

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	/*
         * Get message and add to list.
         */
        if( (entry.message = _mono_sql_mes_row_to_mes(row)) == NULL ) {
            continue;
        }

	if( _mono_sql_mes_add_mes_to_list(entry, list) == -1 ) {
	    continue;
        }
    }
    mysql_free_result(res);
    return 0;
}

/*
 * Returns a linked list containing messages in a topic.
 * Points both ways, for reading forward and backwards..
 */
int
mono_sql_mes_list_topic(unsigned int topic, unsigned int start, mlist_t ** list)
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0, rows = 0, i = 0;
    mlist_t entry;

    /*
     * Second coolest query in the BBS sofar.
     */
    ret = mono_sql_query(&res, "SELECT m.message_id,m.forum_id,m.topic_id," \
        "m.author,u.username,m.alias,m.subject,m.content,UNIX_TIMESTAMP(m.date)," \
        "AVG(r.score),m.flag,f.name,t.name,m.r_message_id,m.r_forum_id,m.r_topic_id," \
        "m.r_author,ur.username,m.r_alias,fr.name,tr.name,m.m_message_id," \
        "m.m_forum_id,m.m_topic_id,m.m_author,um.username,UNIX_TIMESTAMP(m.m_date)," \
        "fm.name,tm.name,m.m_reason FROM " M_TABLE " AS m " \
        "LEFT JOIN " U_TABLE " AS u ON u.id=m.author " \
        "LEFT JOIN " U_TABLE " AS ur ON ur.id=m.r_author " \
        "LEFT JOIN " U_TABLE " AS um ON um.id=m.m_author " \
        "LEFT JOIN " F_TABLE " AS f ON f.id=m.forum_id " \
        "LEFT JOIN " F_TABLE " AS fr ON fr.id=m.r_forum_id " \
        "LEFT JOIN " F_TABLE " AS fm ON f.id=m.m_forum_id " \
        "LEFT JOIN " T_TABLE " AS t ON t.topic_id=m.topic_id " \
        "LEFT JOIN " T_TABLE " AS tr ON tr.topic_id=m.r_topic_id " \
        "LEFT JOIN " T_TABLE " AS tm ON tm.topic_id=m.m_topic_id" \
        "," R_TABLE " AS r WHERE m.message_id IN(r.message_id) " \
        "AND m.topic_id=%u AND m.message_id>%u GROUP BY m.message_id",
            topic, start );

#ifdef OLD_SHIT
    ret = mono_sql_query(&res, "SELECT message_id,topic_id,forum_id,author,alias,subject,UNIX_TIMESTAMP(date) AS date,type,priv,deleted FROM " M_TABLE " WHERE message_id>%u AND topic_id=%u ORDER BY message_id", start, topic);
#endif

    if (ret == -1) {
	(void) mysql_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mysql_free_result(res);
	return -1;
    }
    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	/*
         * Get message and add to list.
         */
        if( (entry.message = _mono_sql_mes_row_to_mes(row)) == NULL ) {
            break;
        }
	if (_mono_sql_mes_add_mes_to_list(entry, list) == -1)
	    break;
    }
    mysql_free_result(res);
    return 0;

}

/*
 * Add an entry to the message linked list.
 * Provide support for going in both directions.
 *
 * God, I HATE linked lists.
 */
static int
_mono_sql_mes_add_mes_to_list(mlist_t entry, mlist_t ** list)
{

    mlist_t *p, *q;

    /*
     * Note mono_sql_free_list()
     */
    p = (mlist_t *) xmalloc(sizeof(mlist_t));
    if (p == NULL)
	return -1;

    *p = entry;
    p->next = NULL;
    p->prev = NULL;

    q = *list;
    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
        p->prev = q;
    }
    return 0;
}

void
mono_sql_mes_free_list(mlist_t *list)
{

    mlist_t *ref;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->message->content);
        (void) xfree(list->message);
        (void) xfree(list);
        list = ref;
    }
    return;
}

message_t *
_mono_sql_mes_row_to_mes(MYSQL_ROW row)
{
    message_t *message = NULL;

    message = (message_t *) xmalloc(sizeof(message_t));
    memset(message, 0, sizeof(message_t));

    /*
     * Actual message.
     */
    sscanf(row[0], "%u", &message->m_id);
    sscanf(row[1], "%u", &message->f_id);
    sscanf(row[2], "%u", &message->t_id);
    sscanf(row[3], "%u", &message->author);
    snprintf(message->author, L_USERNAME, "%s", row[4]);
    snprintf(message->alias, L_USERNAME, "%s", row[5]);
    snprintf(message->subject, L_SUBJECT, "%s", row[6]);

    message->content = (char *) xmalloc( strlen(row[7]) + 1 );
    snprintf(message->content, strlen(row[7]), "%s", row[7]);

    sscanf(row[8], "%lu", &message->date);
    sscanf(row[9], "%f", &message->score);
    snprintf(message->flag, L_FLAGNAME, "%s",  row[10]);
    snprintf(message->forum_name, L_QUADNAME, "%s", row[11]);
    snprintf(message->topic_name, L_TOPICNAME, "%s", row[12]);

    /*
     * Reply info.
     */
    sscanf(row[13], "%u", &message->reply_m_id);
    sscanf(row[14], "%u", &message->reply_f_id);
    sscanf(row[15], "%u", &message->reply_t_id);
    sscanf(row[16], "%u", &message->reply_a_id);
    snprintf(message->reply_author, L_USERNAME, "%s", row[17]);
    snprintf(message->reply_alias, L_USERNAME, "%s", row[18]);
    snprintf(message->reply_forum_name, L_QUADNAME, "%s", row[19]);
    snprintf(message->reply_topic_name, L_TOPICNAME, "%s", row[20]);

    /*
     * Modify info.
     */
    sscanf(row[21], "%u", &message->orig_m_id);
    sscanf(row[22], "%u", &message->orig_f_id);
    sscanf(row[23], "%u", &message->orig_t_id);
    sscanf(row[24], "%u", &message->orig_a_id);
    snprintf(message->orig_author, L_USERNAME, "%s", row[25]);
    sscanf(row[26], "%lu", &message->orig_date);
    snprintf(message->orig_forum, L_QUADNAME, "%s", row[27]);
    snprintf(message->orig_topic, L_TOPICNAME, "%s", row[28]);
    snprintf(message->mod_reason, L_REASON, "%s", row[29]);

    return message;
}

/*
mono_sql_mes_search()
{

    mono_sql_query( &res, "SELECT m.message_id AS id,f.name AS forum,u.username AS username,m.subject AS subject FROM %s AS m LEFT JOIN %s AS u ON u.id=m.author LEFT JOIN %s AS f ON f.id=m.forum_id WHERE m.content REGEXP '%s' OR m.subject REGEXP '%s' ORDER BY m.forum_id,m.message_id",
        M_TABLE, U_TABLE, F_TABLE, string, string );

}
*/

/* eof */
