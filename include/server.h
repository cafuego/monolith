/* $Id$ */

/*
 * Generic telnet protocol #defines, along with some special protocol that is
 * spoken between the YAWC BBS and a connected client.  You don't want to
 * change this, it depends on this being the same on both ends, and you can't
 * change what the BBS end does. 
 */

#define	IAC	255		/* interpret as command:		*/

/* BBS client additions */

#define CLIENT  0xa0            /* Is a client				*/
#define BLOCK   0xa1            /* Start block mode transfer		*/
#define G_STR   0xa2            /* Get string (arg = length)		*/
#define G_NAME  0xa3            /* Get name (arg = type)		*/

/*** other definitions */

#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

#define BBSEXECUTABLE   BBSDIR "bin/yawc_client"
#define MOTD		BBSDIR "share/motd" 
#define ALLOWFILE	BBSDIR "etc/front.ALLOW"
#define PROHIBMSG	BBSDIR "share/messages/prohib"
#define FULLMSG		BBSDIR "share/messages/full"

/* prototypes */
int main(int, char **);
void the_works(void);
void ok_let_in(void);
void get_hostname(void);
int wait_in_queue(void);
int rewrite_queue( int in_what_way );
int get_btmp(void);
void mono_login(void);
char *get_name(void);
RETSIGTYPE retry(int);
void getline(char *string, int lim, int frog);
int getblock(void);
int netget(void);
void logoff( int womble );

/* eof */
