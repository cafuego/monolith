/* $Id$ */

/* functions */
extern int mono_setuid( const char * );
extern char *mono_getuid( void );
extern int set_invocation_name( const char *name );

/* variables */
extern char mono_uid[L_USERNAME+1];
extern char *program_invocation_name;
extern char *program_invocation_short_name;

/* eof */
