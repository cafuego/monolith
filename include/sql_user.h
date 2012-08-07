/* $Id */

int mono_sql_rename_user(unsigned int num, const char *newname);
int mono_sql_u_add_user(const char *usernam);
int mono_sql_u_kill_user(unsigned int user_id);
int mono_sql_u_id2name(unsigned int userid, char *usernam);
int mono_sql_u_name2id(const char *usernam, unsigned int *userid);
int mono_sql_u_add_user_new(const char *, unsigned int );

int mono_sql_u_set_passwd( unsigned int, const char * );
int mono_sql_u_check_passwd( unsigned int, const char * );
int mono_sql_u_check_user( const char *username );

int mono_sql_u_update_registration( unsigned int user_id,
  const char *name, const char *address, const char *zip, const char *city,
  const char *state, const char *country, const char *phone );

int mono_sql_u_get_registration( unsigned int user_id,
  char *name, char *address, char *zip, char *city,
  char *state, char *country, char *phone );

int mono_sql_read_profile(unsigned int user_id, char ** profile );
int mono_sql_write_profile(unsigned int user_id, const char * profile );

int mono_sql_u_update_email( unsigned int user_id, const char *email );
int mono_sql_u_update_url( unsigned int user_id, const char *url );
int mono_sql_u_update_hidden( unsigned int user_id, int hiddeninfo );
int mono_sql_u_update_validation( unsigned int user_id, int validation );

int mono_sql_u_get_email( unsigned int user_id, char *email );
int mono_sql_u_get_url( unsigned int user_id, char *url );
int mono_sql_u_get_hidden(unsigned int user_id, int *hiddeninfo);
int mono_sql_u_get_validation( unsigned int user_id, unsigned int *validation );

int mono_sql_u_update_x_count( unsigned int user_id, unsigned int x_count );
int mono_sql_u_increase_x_count( unsigned int user_id );
int mono_sql_u_get_x_count( unsigned int user_id, unsigned int *x_count );

int mono_sql_u_update_post_count( unsigned int user_id, unsigned int post_count );
int mono_sql_u_increase_post_count( unsigned int user_id );
int mono_sql_u_get_post_count( unsigned int user_id, unsigned int *post_count );

int mono_sql_u_update_login_count( unsigned int user_id, unsigned int login_count );
int mono_sql_u_increase_login_count( unsigned int user_id );
int mono_sql_u_get_login_count( unsigned int user_id, unsigned int *login_count );


int mono_sql_u_update_awaymsg( unsigned int user_id, const char *awaymsg );
int mono_sql_u_get_awaymsg( unsigned int user_id, char *awaymsg );
int mono_sql_u_update_doing( unsigned int user_id, const char *doing );
int mono_sql_u_get_doing( unsigned int user_id, char *doing );
int mono_sql_u_update_xtrapflag( unsigned int user_id, const char *xtrapflag );
int mono_sql_u_get_xtrapflag( unsigned int user_id, char *xtrapflag );

#ifdef HAVE_MEMFROB
void *memfrob(void *s, size_t n);
#endif

#define U_TABLE	"user"
/* eof */
