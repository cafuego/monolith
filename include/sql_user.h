/* $Id */

extern int mono_sql_u_add_user_new(const char *username, unsigned int num);
extern int mono_sql_rename_user(unsigned int num, const char *newname);
extern int mono_sql_u_add_user(const char *username);
extern int mono_sql_u_kill_user(unsigned int user_id);
extern int mono_sql_u_id2name(unsigned int userid, char *username);
extern int mono_sql_u_name2id(const char *username, unsigned int *userid);
extern int mono_sql_u_add_user_new(const char *, unsigned int );

extern int mono_sql_u_set_passwd( unsigned int, const char * );
extern int mono_sql_u_check_passwd( unsigned int, const char * );

#define U_TABLE	"user"
/* eof */
