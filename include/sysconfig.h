/* $Id$ */

/* UID and GID of bbs account */
#define BBSUID          333     /* bbs's uid from /etc/passwd file	*/
#define BBSGID          333     /* bbs's gid from /etc/group file	*/

/* where bbs's homedir is */
#define BBSDIR		"/usr/bbs/"	

/* the <shift-e>ditor */
#define EDITOR		BBSDIR "bin/BBSpico"	

/* Crypt-key for the passwords */
#define CRYPTKEY	"Gu"	

/* Number of rooms in system */
#define MAXQUADS	150

#define SQL_SERVER	"localhost"
#define SQL_USER	"root"
#define SQL_PASSWORD	NULL
#define SQL_DATABASE	"bbs"
