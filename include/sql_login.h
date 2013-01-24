/* $Id$ */

/*
 * protos for sql_log.c   *argh*  this doesn't even exist anymore.
 */

int mono_sql_log_logout(unsigned int, time_t, time_t, const char *, int );
int sql_log_logout(unsigned int user_id, time_t login, time_t logout, const char *host, int reason);
int sql_log_sysop(char *event,...);
