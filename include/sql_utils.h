/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int mono_sql_connect(void);
int mono_sql_connected(void);
int mono_sql_detach(void);

#ifdef MYSQL_NO_DATA /* sql stuff defined */
int mono_sql_query(MYSQL_RES ** result, const char *format,...);
void mono_sql_free_result(MYSQL_RES *res);
void mono_sql_u_free_result(MYSQL_RES *res);
#endif

#ifdef MICHELS_TIME_FUNCS
int time_to_datetime(time_t , char * );
int time_to_date(time_t , char * );
#endif

/* Small util funcs */
int mono_sql_logqueries( void );
int mono_sql_logqueries_toggle( void );

int escape_string(const char *old_string, char **new_string);
const char * mono_mysql_server_info( void );
const char * mono_mysql_host_info( void );

int mono_sql_test_connection(mysql_t *);

/* eof */
