/*
 * $Id$
 *
 * Contains conversion fucntions from MYSQL_ROW to custom types.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

#ifdef extern
#include "sql_convert.h"
#endif

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
    sscanf(row[2], "%u", &result->t_id);
    snprintf(result->forum, L_QUADNAME, "%s", row[3]);
    snprintf(result->topic, L_TOPICNAME, "%s", row[4]);
    snprintf(result->author, L_USERNAME, "%s", row[5]);
    snprintf(result->subject, L_SUBJECT, "%s", row[6]);
    sscanf(row[7], "%u", &result->score);

    return result;
}
/* eof */
