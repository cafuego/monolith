/* $Id$ */

/*
 * protos for sql_log.c
 */

extern int sql_log_logout(unsigned int user_id, time_t login, time_t logout, const char *host, int reason);
extern int sql_log_sysop(char *event,...);
