/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern int mono_sql_query(MYSQL_RES ** result, const char *format,...);

extern int mono_sql_connect(void);
extern int mono_sql_connected(void);
extern int mono_sql_detach(void);
extern void mono_sql_u_free_result(MYSQL_RES *res);

#ifdef MICHELS_TIME_FUNCS
extern int time_to_datetime(time_t , char * );
extern int time_to_date(time_t , char * );
#endif

/* Small util funcs */
extern int mono_sql_logqueries( void );
extern int mono_sql_logqueries_toggle( void );

extern int escape_string(const char *old_string, char **new_string);
extern char * mono_mysql_server_info( void );
extern char * mono_mysql_host_info( void );

extern int mono_sql_test_connection(mysql_t *);

/* eof */
