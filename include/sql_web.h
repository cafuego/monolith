/* $Id$ */

/*
 * protos for sql_web.c
 */

int mono_sql_web_send_x(unsigned int, unsigned int, const char *);
int mono_sql_web_remove_read(unsigned int);
int mono_sql_web_cleanup(unsigned int);
int mono_sql_web_get_online(wu_list_t ** list);
char * mono_sql_web_wholist(void);

#define WEB_X_TABLE	"webx"
