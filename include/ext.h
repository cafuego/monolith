/* $Id$ */
/* This file contains all global variables */

extern char	username[L_USERNAME+1];	/* name user enters at login	*/
extern char	hname[255];		/* host user's calling from	*/
extern char	skipping[MAXQUADS];	/* which rooms are skipped	*/
extern int	last_skipped_rm;        /* room # of last skipped rm    */
extern char	temp[60];		/* Name of general temp file	*/
extern char	tmpname[60];
extern char	CLIPFILE[60];		/* ClipBoard-file		*/
extern volatile char	nox;		/* 1 -> no x's, 0 -> x's allowed*/

extern time_t	login_time;		/* time of login */

extern int	line_total;		/* used for -- more (34%) --	*/
extern int	line_count;		/* used for -- more (34%) --	*/
extern unsigned int	curr_line;	/* used by increment()		*/

extern int	curr_rm;		/* Room NUMBER of current room.	*/
extern int	unseen;
extern volatile int idletime;		/* how long user has been idle  */
extern unsigned cmdflags;		/* internal flags		*/

extern room_t   quickroom;
extern user_t	*usersupp;		/* Logged-in user's user-struct	*/

extern config_t config;			/* Peter's config */

/* eof */
