/*
 * $Id$
 */

extern int mono_sql_read_config( int num, config_t *data );
int mono_sql_save_config( int num, config_t *config );
int mono_sql_delete_config( int num );
int mono_sql_config_exists( const char *name );


#define CONFIG_TABLE	"config"
