/* $Id$ */

/*
 * protos for sql_web.c
 */

int mono_sql_web_send_x(unsigned int, unsigned int, const char *, const char *);
int mono_sql_web_remove_read(unsigned int);
int mono_sql_web_cleanup(unsigned int);
int mono_sql_web_get_online(wu_list_t ** list);
char * mono_sql_web_wholist(void);
int mono_sql_web_get_xes(unsigned int user_id, wx_list_t ** list);
void mono_sql_web_mark_wx_read(wx_list_t *list);

#define WEB_X_TABLE	"webx"
