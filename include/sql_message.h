/*
 * $Id$
 */
extern int mono_sql_mes_add(message_t *message, const char *tmpfile);
extern char * mono_sql_mes_make_file(unsigned int forum, unsigned int num);
extern int mono_sql_mes_remove(unsigned int id, unsigned int forum);
extern int mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *message);
extern int mono_sql_mes_list_forum(unsigned int forum, unsigned int start, mlist_t ** list);
extern int mono_sql_mes_list_topic(unsigned int topic, unsigned int start, mlist_t ** list);
extern void mono_sql_mes_free_list(mlist_t *);

#define M_TABLE "message"
#define FILEDIR BBSDIR "/save/messages"
