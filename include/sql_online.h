/* $Id$ */

/*
 * protos for sql_online.c
 */

int mono_sql_onl_add(unsigned int, const char *,const char *);
int mono_sql_onl_remove(unsigned int, const char *);
int mono_sql_onl_status(unsigned int, unsigned int);
int mono_sql_onl_doing(const char *, const char *);
int mono_sql_onl_check_user(const char *);

#define ONLINE_TABLE	"online"
