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
#include "sql_userforum.h"

#include "sql_message.h"

static int _mono_sql_mes_add_num_to_list(mlist_t entry, mlist_t ** list);
static void _mono_sql_mes_save(const char *tmpfile, const char *filename);
static char * _mono_sql_mes_make_file(unsigned int forum, unsigned int num);

int
mono_sql_mes_add(message_t *message, unsigned int forum, const char *tmpfile)
{
    MYSQL_RES *res;
    int ret = 0;
    char *alias = NULL, *subject = NULL, message_file[100];

    if( (message->num = mono_sql_f_get_new_message_id(forum)) == 0)
        return -1;

    strcpy(message_file, _mono_sql_mes_make_file(forum, message->num));
    strcpy(message->content, message_file);
    (void) _mono_sql_mes_save(tmpfile, message_file);

    (void) escape_string(message->alias, &alias);
    (void) escape_string(message->subject, &subject);

    /*
     * We add the date here, so it really is the date the message was
     * added to the BBS. TIMESTAMP in the table will not be used, as it might
     * be updated. That's better used to check if a message was modified.
     */
    message->date = time(0);

    ret = mono_sql_query(&res, "INSERT INTO " M_TABLE " (message_id,topic_id,forum_id,author,alias,subject,date,content,type,priv,deleted) VALUES (%u,%u,%u,%u,'%s','%s',FROM_UNIXTIME(%u),'%s','%s','%s','%c')",
        message->num, message->topic, message->forum, message->author,
        alias, subject, message->date, message->content, message->type,
        message->priv, message->deleted );

    (void) mysql_free_result(res);
    xfree(alias);
    xfree(subject);

    return ret;

}

static char *
_mono_sql_mes_make_file(unsigned int forum, unsigned int number)
{
    static char filename[100];

    (void) snprintf(filename, 100, FILEDIR "/forum.%u/message.%u", forum, number);
    return filename;
}

static void
_mono_sql_mes_save(const char *tmpfile, const char *filename)
{
    int fd = 0;
    struct stat buf;
    char *buffer = NULL;
    
    old = open(tmpfile, O_RDONLY);
    buffer = (char *) xmalloc(buf.st_size);
    (void) read(old, buffer, buf.st_size);
    (void) close(old);

    new = open(filename, O_RDWR | O_CREAT);
    (void) write(new, &buffer, strlen(buffer));
    (void) close(new);
    xfree(buffer);

    (void) unlink(tmpfile);
    return;
}

int
mono_sql_mes_mark_deleted(unsigned int id, unsigned int forum)
{
    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "UPDATE " M_TABLE " SET deleted='y' WHERE message_id=%d AND forum_id=%d", id, forum );

    (void) mysql_free_result(res);

    return ret;
}
    

int
mono_sql_mes_remove(unsigned int id, unsigned int forum)
{
    MYSQL_RES *res;
    int ret = 0;

    ret = mono_sql_query(&res, "DELETE FROM " M_TABLE " WHERE message_id=%d AND forum_id=%d", id, forum);
    (void) mysql_free_result(res);

    return ret;
}

int
mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *data)
{

    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    message_t message;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT message_id,topic_id,forum_id,author,alias,subject,UNIX_TIMESTAMP(date),content,type,priv,deleted FROM " M_TABLE " WHERE message_id=%u AND forum_id=%u", id, forum);

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

    /*
     * Determine size.
     */

    memset(&message, 0, sizeof(message_t));

    /*
     * Admin info
     */
    message.num = atoi(row[0]);
    sscanf( row[0], "%d", &message.num);
    message.topic = atoi(row[1]);
    message.forum = atoi(row[2]);

    /*
     * Header info
     */
    sscanf( row[3], "%u", &message.author);
    sprintf(message.alias, row[4]);
    sprintf(message.subject, row[5]);
    sscanf( row[6], "%lu", &message.date);

    /*
     * Content
     */
    sprintf(message.content, row[7]);

    /*
     * And more admin info.
     */
    sscanf( row[8], "%s", message.type);
    sscanf( row[9], "%s", message.priv);
    sscanf( row[10], "%c", &message.deleted);

    (void) mysql_free_result(res);

    *data = message;

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

    ret = mono_sql_query(&res, "SELECT (message_id) FROM " M_TABLE " WHERE forum_id=%d AND message_id>=%d ORDER BY message_id", forum, start);

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
	/* Get message_id */
	if (sscanf(row[0], "%u", &(entry.id)) == -1)
	    continue;
	/* Add this one to the linked list */
	if (_mono_sql_mes_add_num_to_list(entry, list) == -1)
	    break;
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

    ret = mono_sql_query(&res, "SELECT FROM " M_TABLE " WHERE topic_id=%d AND message_id>=%d ORDER BY message_id", topic, start);

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
	/* Get message_id */
	if (sscanf(row[0], "%u", &(entry.id)) == -1)
	    continue;
	/* Add this one to the linked list */
	if (_mono_sql_mes_add_num_to_list(entry, list) == -1)
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
_mono_sql_mes_add_num_to_list(mlist_t entry, mlist_t ** list)
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
        xfree(list);
        list = ref;
    }
    return;
}

/* eof */
