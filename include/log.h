/* $Id */

extern int log_it ( const char *type, const char *event,...);
extern int log_user( const user_t *user, const char *hostname, char offstat);

extern /* volatile */ int mono_errno;

#define E_NOUSER	1	/* no such user */
#define E_NOTONLINE	2	/* user is not online */
#define E_NOMEM		3	/* not enough memory */
#define E_NOQUAD	4	/* no such quadrant */
#define E_NOMESG	5	/* no such message */
#define E_NOFILE	6       /* no such file */
#define E_NOPERM	7	/* no permission to do something */
#define E_NOSHM		8 	/* not connected to shm */
#define E_KICKED	9	/* user is kicked from room */
#define E_FEXISTS	10	/* file already exits */
