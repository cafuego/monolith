/* $Id$ */

/* defines */
#define EXECUTABLE	BBSDIR "/bin/yawc_port"
#define ALLOWFILE	BBSDIR "/etc/front.ALLOW"
#define PROHIBMSG	BBSDIR "/share/messages/prohib"
#define FULLMSG		BBSDIR "/share/messages/full"

#define QUEUE_FULL	9

/* prototypes		*/
void sttystuff(int sflag);
int main(int, char **);
void de_initialize(void);
void ok_let_in(void);

int wait_in_queue(void);
int rewrite_queue( int echo );
RETSIGTYPE retry(int sig);
RETSIGTYPE log_off(int sig);
void mono_login(void);
char *get_uname(void);
void get_line( char string[], int lim );

