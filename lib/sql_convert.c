/*
 * $Id$
 *
 * Contains conversion fucntions from MYSQL_ROW to custom types.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

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

#define extern
#include "sql_convert.h"
#undef extern

/*
 * Convert MYQSL_ROW to message_t.
 */
message_t *
mono_sql_convert_row_to_mes(MYSQL_ROW row)
{
    message_t *message = NULL;

    message = (message_t *) xmalloc(sizeof(message_t));

    if(message== NULL)
        return NULL;

    memset(message, 0, sizeof(message_t));

    /*
     * Actual message.
     */
    sscanf(row[0], "%u", &message->m_id);
    sscanf(row[1], "%u", &message->f_id);
    sscanf(row[2], "%u", &message->t_id);
    sscanf(row[3], "%u", &message->a_id);
    snprintf(message->author, L_USERNAME, "%s", row[4]);
    snprintf(message->alias, L_USERNAME, "%s", row[5]);
    snprintf(message->subject, L_SUBJECT, "%s", row[6]);

    message->content = (char *) xmalloc( strlen(row[7]) + 1 );
    snprintf(message->content, strlen(row[7]), "%s", row[7]);

    sscanf(row[8], "%lu", &message->date);
    snprintf(message->flag, L_FLAGNAME, "%s",  row[9]);
    snprintf(message->forum_name, L_QUADNAME, "%s", row[10]);
    snprintf(message->topic_name, L_TOPICNAME, "%s", row[11]);

    /*
     * Reply info.
     */
    sscanf(row[12], "%u", &message->reply_m_id);
    sscanf(row[13], "%u", &message->reply_f_id);
    sscanf(row[14], "%u", &message->reply_t_id);
    sscanf(row[15], "%u", &message->reply_a_id);
    snprintf(message->reply_author, L_USERNAME, "%s", row[16]);
    snprintf(message->reply_alias, L_USERNAME, "%s", row[17]);
    snprintf(message->reply_forum_name, L_QUADNAME, "%s", row[18]);
    snprintf(message->reply_topic_name, L_TOPICNAME, "%s", row[19]);

    /*
     * Modify info.
     */
    sscanf(row[20], "%u", &message->orig_m_id);
    sscanf(row[21], "%u", &message->orig_f_id);
    sscanf(row[22], "%u", &message->orig_t_id);
    sscanf(row[23], "%u", &message->orig_a_id);
    snprintf(message->orig_author, L_USERNAME, "%s", row[24]);
    sscanf(row[25], "%lu", &message->orig_date);
    snprintf(message->orig_forum, L_QUADNAME, "%s", row[26]);
    snprintf(message->orig_topic, L_TOPICNAME, "%s", row[27]);
    snprintf(message->mod_reason, L_REASON, "%s", row[28]);

    return message;
}

/*
 * Convert MYSQL_ROW to sr_t.
 */
sr_t *
mono_sql_convert_row_to_sr(MYSQL_ROW row)
{
    sr_t *result = NULL;

    result = (sr_t *) xmalloc(sizeof(sr_t));

    if(result == NULL)
        return NULL;

    memset(result, 0, sizeof(sr_t));

    sscanf(row[0], "%u", &result->m_id);
    sscanf(row[1], "%u", &result->f_id);
    snprintf(result->forum, L_QUADNAME, "%s", row[2]);
    snprintf(result->author, L_USERNAME, "%s", row[3]);
    snprintf(result->alias, L_USERNAME, "%s", row[4]);
    snprintf(result->subject, L_SUBJECT, "%s", row[5]);
    snprintf(result->flag, L_FLAGNAME, "%s", row[6]);

    return result;
}

/*
 * Convert MYSQL_ROW to express_t.
 */
express_t *
mono_sql_convert_row_to_x(MYSQL_ROW row)
{
    express_t *x = NULL;

    x = (express_t *) xmalloc(sizeof(express_t));

    if(x == NULL)
        return NULL;

    memset(x, 0, sizeof(express_t));

    sscanf(row[0], "%lu", &x->time);
    snprintf(x->sender, L_USERNAME+L_BBSNAME+2, "%s", row[1]);
    snprintf(x->recipient, L_USERNAME+L_BBSNAME+2, "%s", row[2]);
    snprintf(x->message, X_BUFFER, "%s", row[3]);
    sscanf(row[4], "%u", &x->sender_priv);
    sscanf(row[5], "%d", &x->override);
    sscanf(row[6], "%d", &x->ack);

    return x;
}
/* eof */
