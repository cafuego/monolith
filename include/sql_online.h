/* $Id$ */

/*
 * protos for sql_online.c
 */

extern int mono_sql_onl_add(unsigned int, const char *);
extern int mono_sql_onl_remove(unsigned int);
extern int mono_sql_onl_status(unsigned int, unsigned int);

#define ONLINE_TABLE	"online"
