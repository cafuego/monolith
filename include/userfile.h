/* $Id$ */

/* prototypes */
user_type *readuser( const char * );
int writeuser( user_type *, int );
int check_user( const char * );
int write_profile( const char *, char * );
char *read_profile( const char * );
char *name2file( char * );
int del_user( const char * );
int rename_user( const char *from, const char *to );
int isbad( const char * );
char * read_regis ( const user_t *, int );
char * getuserdir( const char * );
int check_password( const user_t *user, const char *password );
int get_new_usernum(const char *, user_id_t *);
int del_sql_user(user_id_t);
int mono_lock_usernum(int);
