/*
 * $Id$
 */
int mono_sql_mes_get_unread(unsigned int forum, unsigned int lastseen);
int mono_sql_mes_add(message_t *message, unsigned int forum);
int mono_sql_mes_remove(unsigned int id, unsigned int forum);
int mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *message);
int mono_sql_mes_list_forum(unsigned int forum, unsigned int start, mlist_t ** list);
int mono_sql_mes_list_topic(unsigned int topic, unsigned int start, mlist_t ** list);

