/* $Id */

extern int mono_sql_u_add_user_new(const char *username, unsigned int num);
extern int mono_sql_rename_user(unsigned int num, const char *newname);
extern int mono_sql_u_add_user(const char *username);
extern int mono_sql_u_kill_user(unsigned int user_id);
extern int mono_sql_u_id2name(unsigned int userid, char *username);
extern int mono_sql_u_name2id(const char *username, unsigned int *userid);

/* eof */
