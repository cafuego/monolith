/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_ASM_TERMBITS_H
  #undef HAVE_TERMBITS_H
  #include <asm/termbits.h>
#else
  #undef HAVE_ASM_TERMBITS_H
  #ifdef HAVE_TERMBITS_H
    #include <termbits.h>
  #endif
#endif

#include <time.h>
#include <unistd.h>

#ifdef HAVE_MYSQL_H
  #undef HAVE_MYSQL_MYSQL_H
  #include <mysql.h>
#else
  #ifdef HAVE_MYSQL_MYSQL_H
    #undef HAVE_MYSQL_H
    #include <mysql/mysql.h>
  #endif
#endif

#include "monolith.h"
#include "libmono.h"
#include "setup.h"

#define extern
#include "front.h"
#undef extern

char hname[255], username[L_USERNAME + 1];
unsigned int maxallow;
int my_position, successful_pre_login = 0, allowed_skip_queue = 0;
pid_t mypid;
char *l_user[] = {
    "Alien species", "Fish species", "LaMeR NiCk", "Cyborg id", "Username"
};

static void get_hostname(void);

/*************************************************
* main()
*************************************************/

int
main(int argc, char **argv)
{

    FILE *fp;
    char host[255], badness = 0, temp[85], tmpallow[5];

    set_invocation_name(argv[0]);
    mono_setuid("guest");

#ifdef DEBUG
    printf("[Starting front program]\n");
#endif

    printf("%c%c%c", IAC, WILL, TELOPT_SGA);
    printf("%c%c%c", IAC, WILL, TELOPT_ECHO);

    fflush(stdout);
    umask(0007);
    chdir(BBSDIR);

    signal(SIGHUP, log_off);
    signal(SIGPIPE, log_off);
    signal(SIGALRM, retry);

    sttybbs(0);

    printf("\r");
    printf("Welcome to %s. (%s)\n", BBSNAME, VERSION);
    fflush(stdout);

    /* if the bbs is down no reason to do anything else */
    if (fexists(DOWNFILE)) {
	system("/bin/cat " DOWNFILE);
	fflush(stdout);
	sleep(5);
	exit(1);
    }
    mypid = getpid();

    /* use ALLOWFILE to determine maximum number of users, else
     * set a default from sysconfig.h */
    fp = xfopen(ALLOWFILE, "r", FALSE);
    if (fp != NULL) {
	fgets(tmpallow, 5, fp);
	maxallow = atoi(tmpallow);
	fclose(fp);
    } else
	maxallow = MAXUSERS;

    /* check to see if we're coming from a blacklisted host */

    get_hostname();

#ifdef DEBUG
    /* michel disabled this, because he doesn't understand what
       it does, and doesn't think it works */
    printf("[Checking if from blacklisted host]\n");

    fp = xfopen(LOCKOUTFILE, "r", FALSE);
    if (fp != NULL) {
	while (fgets(temp, 84, fp)) {
	    if (temp[0] == '#' || strlen(temp) <= 2)
		continue;	/* #'s are like / *'s           */

	    sscanf(temp, "%c %s\n", &badness, host);
	}
	fclose(fp);
    }
#endif
    mono_connect_shm();

    if (shm->user_count >= maxallow) {	/* || shm->queue_count > 0) */
	printf("\n\r\n\rSorry, the BBS is full at the moment. Please try again later.\n\r\n\r");
        mono_detach_shm();
	exit(0);
    }
    /* sends the SIGALRM signal after 30 secs */
    alarm(30);			/* 30 seconds between autoupdates */
    ok_let_in();
    return 0;
}

/*************************************************
* ok_let_in()
* lets the user in to the BBS...
*************************************************/
void
ok_let_in()
{
    de_initialize();

#ifdef DEBUG
    printf("[Trynig to start main program]\n");
#endif

    execl(EXECUTABLE, EXECUTABLE, hname, username, 0);
    /* if we can't find yawc just quietly die */
    printf("Horror! The bbs programme has been deleted!\r\n");
    log_off(0);
}

/*************************************************
* wait_in_queue()
*************************************************/

int
wait_in_queue()
{

    int cmd;

    if (rewrite_queue(1) == QUEUE_FULL)
	/* add myself to the Queue at first     */
/*    return QUEUE_FULL; */

	retry(1);

    for (;;) {
	cmd = getc(stdin);

	if (cmd == 3 || cmd == 4) {
	    /* ctrl-c or ctrl-d to get out */
	    printf("\nLeaving right now...\n");
	    rewrite_queue(2);
	    log_off(0);
	}
	if (cmd == 10 || cmd == 13) {
	    retry(1);
	}
	if (cmd == 'L') {	/* <L>ogin from the Queue and skip it
				 * if I'm allowed to.                   */
	    mono_login();
	    retry(1);
	}
    }
}



/*************************************************
* rewrite_queue()
*
* in_what_way:	1 -> put myself at the end of it
*		2 -> remove myself: i'm leaving
*		3 -> put myself at the top!
*		4 -> rewrite the queue, remove
*		     only the eventual ghosts.
*************************************************/

int
rewrite_queue(int echo)
{

    unsigned int i, j;
    int new_qc = 0;
    mono_queue_t tmpq;

    if (shm->queue_count >= MAXQUEUED && echo == 1) {
	printf("*** Sorry, Monolith BBS is full right now! ***\r\n\n");
	printf(" There are %d users online which is currently the maximum.\n\r", MAXUSERS);
	return QUEUE_FULL;
    }
    mono_lock_queue(1);		/* lock it                      */

    for (i = 0; i < shm->queue_count; i++) {
	if (kill(shm->queue[i].pid, 0) != 0
	    || (shm->queue[i].pid == mypid && echo == 2)) {
	    for (j = i; j < shm->queue_count - 1; j++)
		bcopy(&(shm->queue[j + 1]), &(shm->queue[j]), sizeof(mono_queue_t));
	    continue;
	}
	new_qc++;

	if (shm->queue[i].pid == mypid)
	    my_position = new_qc;
    }

    shm->queue_count = new_qc;


    if (echo == 1) {		/* if we're putting myself in   */
	strncpy(shm->queue[shm->queue_count].host, hname, L_HOSTNAME);
	shm->queue[shm->queue_count].pid = mypid;
	shm->queue[shm->queue_count].flags = 0;
	shm->queue_count += 1;
    }
    if (echo == 3) {		/* if we're jumping to the top  */
	/* first backup my own entry      */
	bcopy(&(shm->queue[my_position - 1]), &tmpq, sizeof(mono_queue_t));

	/* push down everybody above me   */
	for (i = my_position - 1; i > 0; i--)
	    bcopy(&(shm->queue[i - 1]), &(shm->queue[i]), sizeof(mono_queue_t));

	/* then put back me at the top    */
	bcopy(&tmpq, &(shm->queue[0]), sizeof(mono_queue_t));

    }
    mono_lock_queue(2);		/* unlock it                    */

    return 1;

}



/*************************************************
* retry()
*
* this function is run every 30 seconds, to find
* out if somebody either has left the BBS or the
* queue, in a bad way.
* it is also called when the user hits <enter> and
* when a user leaves the BBS.
*************************************************/

RETSIGTYPE
retry(int sig)
{

    time_t t;
    struct tm *tp;

    sig++;			/* to stop gcc warning */

    rewrite_queue(4);		/* remove ghosts                */

    if (my_position == 1) {	/* if I'm first in the queue    */
	if (shm->user_count < maxallow) {	/* I should be let in by now!   */
	    rewrite_queue(2);
	    putchar('\007');
	    ok_let_in();
	}
    }
    if (successful_pre_login == 1 && allowed_skip_queue == 1) {
	if (shm->user_count < MAXUSERS) {
	    printf("...and then we skipped the entire AirLock. Welcome to Monolith BBS...\n");
	    rewrite_queue(2);
	    putchar('\007');
	    ok_let_in();
	} else if (my_position != 1) {	/* we jump to the queue-top     */
	    printf("Sorry, but the BBS is _completely_ full: I can't even squeeze in a\n");
	    printf("little spotty Emperor right now.\n");
	    printf("But you'll be put in in front of the AirLock so relax for a minute or two...\n");
	    rewrite_queue(3);
	}
    }
    time(&t);
    tp = localtime(&t);

    if (shm->queue_count == 1)
	printf("(%.2d:%.2d) You're the only one in the AirLock right now.\n",
	       tp->tm_hour, tp->tm_min);
    else
	printf("(%.2d:%.2d) There are %d users in the AirLock; you're #%d.\n",
	       tp->tm_hour, tp->tm_min, shm->queue_count, my_position);

    signal(SIGALRM, retry);	/* do this again                */
    alarm(30);
    return;
}


/*************************************************
* mono_login()
*************************************************/

void
mono_login()
{

    char pwtest[20];
    char tmp_user[L_USERNAME + 1];
    user_t *user;
    int i = 0;

    if (successful_pre_login == 1)
	return;

    successful_pre_login = 0;
    allowed_skip_queue = 0;

    (void)srand(42);
    i = rand() % 7;
    switch(i) {
      case 1:
      case 2:
      case 3:
      case 4:
        printf("\r%s: ", l_user[i]);
        break;
      default:
        printf("\r%s: ", l_user[0]);
        break;
    }

    strcpy(tmp_user, get_uname());

    if (!strlen(tmp_user))
	return;

    printf("\rPassword: ");
    get_line(pwtest, -19);

    if ((user = readuser(tmp_user)) == NULL) {
	printf("Incorrect login.\n");	/* no such user...              */
	strcpy(username, "");
	xfree(user);
	return;
    }
    if ( mono_sql_u_check_passwd( user->usernum, pwtest ) == TRUE ) {
	successful_pre_login = 1;
	strcpy(username, tmp_user);	/* added this - Lisa */
	printf("\n [ Logged In ]\n\n");

	if (user->priv >= PRIV_SYSOP)
	    allowed_skip_queue = 1;
    } else {
	printf("Incorrect login.\n");
	strcpy(username, "");
	strcpy(tmp_user, "");	/* just in case, prolly not needed - Lisa */
    }
    xfree(user);

}

/*************************************************
* get_uname()
*************************************************/

char *
get_uname()
{

    static char pbuf[L_USERNAME + 1];
    register char *p, c;
    int upflag, fflag, invalid = 0, a;

    for (a = 0; a < L_USERNAME; a++)
	pbuf[a] = 0;

    for (;;) {
	upflag = fflag = 1;
	a = 0;

	p = pbuf;

	for (;;) {
	    c = getc(stdin);

	    if (c == 127)
		c = '\b';

	    if (c == 13 || c == 10)
		break;

	    if (c == ' ' && (fflag || upflag))
		continue;

	    if (c == '\b' || c == ' ' || isalpha(c))
		invalid = 0;

	    else {
		invalid++;
		if (invalid++)
		    tcflush(0, TCIFLUSH);
		continue;
	    }

	    do
		if (c == '\b' && p > pbuf) {
		    printf("\b \b");
		    --p;
		    upflag = (p == pbuf || *(p - 1) == ' ');
		    if (p == pbuf)
			fflag = 1;

		    pbuf[strlen(pbuf)] = 0;
		    *p = 0;
		} else if ((p < &pbuf[20] && (isalpha(c) || c == ' '))) {
		    fflag = 0;

		    if (upflag && islower(c))
			c -= 32;

		    upflag = 0;	/* hack to make space working after '*' f.e. */

		    if (c == ' ')
			upflag = 1;
		    *p++ = c;
		    putchar(c);
		}
	    while ((c == 24 || c == 23) && p > pbuf) ;

	}

	*p = 0;
	break;
    }

    printf("\r\n");

    if (p > pbuf && p[-1] == ' ')
	p[-1] = 0;

    return (pbuf);

}

/*************************************************
* get_line()
*************************************************/
void
get_line(char string[], int lim)
{

    int a, b;
    char flag;
    unsigned int length;

    flag = 0;

    if (lim < 0)
	flag = 1;
    length = abs(lim);

    strcpy(string, "");

    for (;;) {
	a = getc(stdin);

	if (a == 127)
	    a = '\b';

	if ((a < 32) && (a != 8) && (a != 13) && (a != 10))
	    a = 32;

	if ((a == 8) && (strlen(string) == 0))	/* backspace at beginning of line */
	    continue;

	if ((a != 13) && (a != 8) && (strlen(string) == length))
	    continue;

	if ((a == 8) && (string[0] != 0)) {
	    string[strlen(string) - 1] = 0;
	    putc(8, stdout);
	    putc(32, stdout);
	    putc(8, stdout);
	    continue;
	}
	if ((a == 13) || (a == 10)) {
	    putc(13, stdout);
	    putc(10, stdout);
	    return;
	}
	b = strlen(string);
	string[b] = a;
	string[b + 1] = 0;

	if (flag == 0)
	    putc(a, stdout);
	if (flag == 1)
	    putc('.', stdout);
    }
}

/*************************************************
* log_off()
*************************************************/
RETSIGTYPE
log_off(int sig)
{
    sig++;			/* to stop compiler warnings */
    de_initialize();
    exit(0);
}


void
get_hostname()
{

    struct hostent *host;
    char *site_num;
    int size;
    struct sockaddr_in acc_addr;
    unsigned long int addr;

    size = sizeof(struct sockaddr_in);
    getpeername(0, (struct sockaddr *) &acc_addr, &size);

    site_num = inet_ntoa(acc_addr.sin_addr);
    addr = inet_addr(site_num);

    if ((host = gethostbyaddr((char *) &addr, 4, AF_INET)) != NULL)
	strncpy(hname, host->h_name, 254 );
    else
	strncpy(hname, site_num, 254 );

    return;
}

/*************************************************
* de_initialize()
*************************************************/

void
de_initialize()
{
    mono_detach_shm();
    sttybbs(1);			/* restore the old params?      */
    signal(SIGHUP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGALRM, SIG_DFL);

    return;
}

/* eof */
