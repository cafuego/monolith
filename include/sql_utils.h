/* $Id$ */

#ifndef MYSQL_RES
#include <mysql.h>
#endif
extern int mono_sql_query(MYSQL_RES ** result, const char *format,...);

extern int mono_sql_connect(void);
extern int mono_sql_connected(void);
extern int mono_sql_detach(void);

extern int time_to_datetime(time_t time, char *datetime);
extern int time_to_date(time_t time, char *datetime);

extern int escape_string(const char *old_string, char **new_string);

/* eof */
