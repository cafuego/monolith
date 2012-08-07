/*
 * $Id$
 */

int mono_sql_read_config( int, config_t *);
int mono_sql_save_config( int, config_t *);
int mono_sql_delete_config( int );
int mono_sql_config_exists( const char *);


#define CONFIG_TABLE	"config"
