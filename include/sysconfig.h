/* $Id$ */

/* UID and GID of bbs account */
#define BBSUID          333     /* bbs's uid from /etc/passwd file	*/
#define BBSGID          333     /* bbs's gid from /etc/group file	*/

/* the <shift-e>ditor */
/* #define EDITOR		BBSDIR "/bin/BBSpico"	 */
#define EDITOR		BBSDIR "/bin/nano"
#define TMPDIR		BBSDIR "/tmp/"

/* Crypt-key for the passwords */
#define CRYPTKEY	"Gu"	

/* Number of rooms in system */
#define MAXQUADS	150

#define SQL_SERVER	"210.8.200.9"
#define SQL_USER	"monolith"
#define SQL_PASSWORD	"bing"
#define SQL_DATABASE	"monolith"
