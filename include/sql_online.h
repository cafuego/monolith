/* $Id$ */

/*
 * protos for sql_online.c
 */

extern int mono_sql_onl_add(unsigned int, const char *,const char *);
extern int mono_sql_onl_remove(unsigned int, const char *);
extern int mono_sql_onl_status(unsigned int, unsigned int);
extern int mono_sql_onl_doing(const char *, const char *);

#define ONLINE_TABLE	"online"
