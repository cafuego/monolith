/* $Id$ */

/* prototypes */
extern user_type *readuser( const char * );
extern int writeuser( user_type *, int );
extern int check_user( const char * );
extern int write_profile( const char *, char * );
extern char *read_profile( const char * );
extern char *name2file( char * );
extern int del_user( const char * );
extern int rename_user( const char *from, const char *to );
extern int isbad( const char * );
extern char * read_regis ( const user_t *, int );
extern char * getuserdir( const char * );
extern int check_password( const user_t *user, const char *password );
extern int set_password( user_t *user, const char *password );
extern unsigned int get_new_usernum(const char *, unsigned int *);
