/*
 * $Id$
 */

extern int mono_sql_read_config( int, config_t *);
extern int mono_sql_save_config( int, config_t *);
extern int mono_sql_delete_config( int );
extern int mono_sql_config_exists( const char *);


#define CONFIG_TABLE	"config"
