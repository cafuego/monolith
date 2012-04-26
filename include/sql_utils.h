/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern int mono_sql_connect(void);
extern int mono_sql_connected(void);
extern int mono_sql_detach(void);

#ifdef MYSQL_NO_DATA // sql stuff defined */
extern int mono_sql_query(MYSQL_RES ** result, const char *format,...);
extern void mono_sql_free_result(MYSQL_RES *res);
extern void mono_sql_u_free_result(MYSQL_RES *res);
#endif

#ifdef MICHELS_TIME_FUNCS
extern int time_to_datetime(time_t , char * );
extern int time_to_date(time_t , char * );
#endif

/* Small util funcs */
extern int mono_sql_logqueries( void );
extern int mono_sql_logqueries_toggle( void );

extern int escape_string(const char *old_string, char **new_string);
extern const char * mono_mysql_server_info( void );
extern const char * mono_mysql_host_info( void );

extern int mono_sql_test_connection(mysql_t *);

/* eof */
