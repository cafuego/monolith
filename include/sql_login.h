/* $Id$ */

/*
 * protos for sql_log.c   *argh*  this doesn't even exist anymore.
 */

extern int mono_sql_log_logout(unsigned int, time_t, time_t, const char *, int );
extern int sql_log_logout(unsigned int user_id, time_t login, time_t logout, const char *host, int reason);
extern int sql_log_sysop(char *event,...);
