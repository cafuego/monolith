/* $Id */

extern int mono_sql_rename_user(unsigned int num, const char *newname);
extern int mono_sql_u_add_user(const char *usernam);
extern int mono_sql_u_kill_user(unsigned int user_id);
extern int mono_sql_u_id2name(unsigned int userid, char *usernam);
extern int mono_sql_u_name2id(const char *usernam, unsigned int *userid);
extern int mono_sql_u_add_user_new(const char *, unsigned int );

extern int mono_sql_u_set_passwd( unsigned int, const char * );
extern int mono_sql_u_check_passwd( unsigned int, const char * );
extern int mono_sql_u_check_user( const char *username );

extern int mono_sql_u_update_registration( unsigned int user_id,
  const char *name, const char *address, const char *zip, const char *city,
  const char *state, const char *country, const char *phone );

extern int mono_sql_u_get_registration( unsigned int user_id,
  char *name, char *address, char *zip, char *city,
  char *state, char *country, char *phone );

extern int mono_sql_read_profile(unsigned int user_id, char ** profile );
extern int mono_sql_write_profile(unsigned int user_id, const char * profile );

extern int mono_sql_u_update_email( unsigned int user_id, const char *email );
extern int mono_sql_u_update_url( unsigned int user_id, const char *url );
extern int mono_sql_u_update_hidden( unsigned int user_id, int hiddeninfo );
extern int mono_sql_u_update_validation( unsigned int user_id, int validation );

extern int mono_sql_u_get_email( unsigned int user_id, char *email );
extern int mono_sql_u_get_url( unsigned int user_id, char *url );
extern int mono_sql_u_get_validation( unsigned int user_id, int *validation );

extern int mono_sql_u_update_x_count( unsigned int user_id, int x_count );
extern int mono_sql_u_increase_x_count( unsigned int user_id );
extern int mono_sql_u_get_x_count( unsigned int user_id, int *x_count );

extern int mono_sql_u_update_post_count( unsigned int user_id, int post_count );
extern int mono_sql_u_increase_post_count( unsigned int user_id );
extern int mono_sql_u_get_post_count( unsigned int user_id, int *post_count );

extern int mono_sql_u_update_login_count( unsigned int user_id, int login_count );
extern int mono_sql_u_increase_login_count( unsigned int user_id );
extern int mono_sql_u_get_login_count( unsigned int user_id, int *login_count );


extern int mono_sql_u_update_awaymsg( unsigned int user_id, const char *awaymsg );
extern int mono_sql_u_get_awaymsg( unsigned int user_id, char *awaymsg );
extern int mono_sql_u_update_doing( unsigned int user_id, const char *doing );
extern int mono_sql_u_get_doing( unsigned int user_id, char *doing );
extern int mono_sql_u_update_xtrapflag( unsigned int user_id, const char *xtrapflag );
extern int mono_sql_u_get_xtrapflag( unsigned int user_id, char *xtrapflag );

#define U_TABLE	"user"
/* eof */
