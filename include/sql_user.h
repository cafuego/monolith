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

#define U_TABLE	"user"
/* eof */
