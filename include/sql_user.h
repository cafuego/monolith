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

#define U_TABLE	"user"
/* eof */
