/*
 * $Id$
 */
extern int mono_sql_mes_unread_room(unsigned int usernum);
extern int mono_sql_mes_get_unread(unsigned int forum, unsigned int lastseen);
extern int mono_sql_mes_add(message_t *message, unsigned int forum);
extern int mono_sql_mes_remove(unsigned int id, unsigned int forum);
extern int mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *message);
extern int mono_sql_mes_list_forum(unsigned int forum, unsigned int start, mlist_t ** list);
extern int mono_sql_mes_list_topic(unsigned int topic, unsigned int start, mlist_t ** list);

#define M_TABLE "message"
