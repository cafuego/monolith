/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"
#include "setup.h"

#define extern
#include "server.h"
#undef extern

#define Q_CLIENTUSER    1	/* i am a client-user                   */

unsigned int maxallow, my_position;
char hostname[255];
char username[L_USERNAME + 1];
int successful_pre_login = 0;
int allowed_skip_queue = 0;
pid_t mypid;
FILE *netofp, *netifp;
struct sockaddr_in acc_addr;
char *l_user[] = {
    "Alien species", "Fish species", "LaMeR NiCk", "Cyborg id", "Username"
};

/* ---------------------------------------------------------------- */
int
main(int argc, char **argv)
{

    unsigned char inpstr[80];
    int len;
    FILE *fp;
    char badness = 0, host[50], temp[85], tmpallow[3];

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    printf("\r");
    printf("Welcome to %s. (%s)\n", BBSNAME, VERSION);

    fflush(stdout);

    chdir(BBSDIR);

    /* if the bbs is down no reason to do anything else */
    if (fexists(DOWNFILE)) {
	system("/bin/cat " DOWNFILE);
	fflush(stdout);
	sleep(5);
	logoff(0);
    }
    signal(SIGALRM, retry);
    alarm(30);			/* 30 seconds between autoupdates */
    mypid = getpid();

    /* use ALLOWFILE to determine maximum number of users, else
     * set a default from sysconfig.h */

    fp = xfopen(ALLOWFILE, "r", FALSE);
    if (fp != NULL) {
	fgets(tmpallow, 3, fp);
	sscanf(tmpallow, "%d", &maxallow);
	fclose(fp);
    } else
	maxallow = MAXUSERS;

    get_hostname();

    /* check to see if we're coming from a blacklisted host */

    fp = xfopen(LOCKOUTFILE, "r", FALSE);
    if (fp != NULL) {
	while (fgets(temp, 84, fp)) {
	    if (temp[0] == '#' || strlen(temp) <= 2)
		continue;	/* #'s are like / *'s           */

	    sscanf(temp, "%c %s\n", &badness, /*& */ host);
	    if (strstr(hostname, host) && badness == 'X') {
		fclose(fp);
		printfile(PROHIBMSG);
		sleep(1);
		logoff(0);
	    }
	}
	fclose(fp);
    }
    mono_connect_shm();

    while (1) {
	bzero(&inpstr, sizeof(inpstr));

	if ((len = read(0 /*connecting_sock */ , inpstr, sizeof(inpstr))) == -1) {
	    printf("Monolith Server: connecting_sock read failed! Exiting.\n");
	    logoff(0);
	}
	if (inpstr[0] == IAC && inpstr[1] == CLIENT) {
	    /*
	     * * the CLient has now sent out a "can I connect?"-query to the
	     * * server, and since we have recieved it, we continue to fix
	     * * her the real BBS-program connected to her socket.
	     */
	    ok_let_in();
	}
	sleep(1);
    }
    return 0;
}



/*************************************************
* ok_let_in()
*************************************************/

void
ok_let_in()
{

    mono_detach_shm();
    execl(BBSEXECUTABLE, BBSEXECUTABLE, hostname, username, 0);
    logoff(0);
}

/*************************************************
* get_hostname()
*************************************************/

void
get_hostname()
{

    struct hostent *host;
    char site_num[80];
    int size;

/* according to the manpage, this should be a unsgnd long int, not just
 * an unsigned int.              kirth */

    unsigned long int addr;

    size = sizeof(struct sockaddr_in);
    getpeername(0, (struct sockaddr *) &acc_addr, &size);
    /* int  getpeername(int  s, struct sockaddr *name, int *namelen); */

    strcpy(site_num, (char *) inet_ntoa(acc_addr.sin_addr));
    /* get the number of the site, and put it in 'site_num' */
    addr = inet_addr(site_num);
    /* now translate this back into binary data */

    /* i don't know if gethostbyaddr returns a pointer to a local static
     * string...if it doesn't, it should be malloced here..that could
     * cause the problems we're having now..... i'll change this a bit to
     * make sure of that.
     */

    if ((host = gethostbyaddr((char *) &addr, 4, AF_INET)) != NULL)
	strcpy(hostname, host->h_name);
    else
	strcpy(hostname, site_num);

    return;
}

/*************************************************
* wait_in_queue()
*************************************************/
int
wait_in_queue()
{

    int cmd;

    if (rewrite_queue(1) == 9)	/* add myself to the Queue        */
	return 9;

    retry(1);

    for (;;) {
	cmd = netget();

	if (cmd == 3 || cmd == 4) {
	    /* <Ctrl-C> or <Ctrl-D> exits the Queue. */
	    printf("\nQuitting right now...\n");
	    rewrite_queue(2);
	    logoff(0);
	}
	if (cmd == 10 || cmd == 13) {	/* <Enter> retries              */
	    retry(1);
	}
	if (cmd == 'L') {	/* <L>ogin from the Queue and skip it
				 * if I'm allowed to.                   */
	    mono_login();
	    retry(1);
	}
    }
    if (rewrite_queue(2) == 9)
	return 9;

    return 1;

}

/*************************************************
* rewrite_queue()
*
* in_what_way:	1 -> put myself at the end of it
*		2 -> remove myself: i'm leaving
*		3 -> put myself at the top!
*		4 -> rewrite the queue, remove
*		     only the eventual ghosts.
*
* Achtung: if in_what_way == 1 here in the CLient-
* "front", we do not put the user at the end of
* the Queue. We put him after the last CLientuser
* instead - CLientusers can jump past normal users
* in the queue.
*************************************************/
int
rewrite_queue(int in_what_way)
{
    int new_qc = 0, benefit = 0;
    unsigned int i, j;
    mono_queue_t tmpq;

    if (shm->queue_count >= MAXQUEUED && in_what_way == 1) {
	printf("*** Sorry, even the Queue to the BBS is full right now! ***\n");
	printf("\n    Try again later.\n");
	return (9);
    }
    mono_lock_queue(1);		/* lock it                      */

    for (i = 0; i < shm->queue_count; i++) {
	if (kill(shm->queue[i].pid, 0) != 0
	    || (shm->queue[i].pid == mypid && in_what_way == 2)) {
	    for (j = i; j < shm->queue_count - 1; j++)
		bcopy(&(shm->queue[j + 1]), &(shm->queue[j]), sizeof(mono_queue_t));
	    continue;
	}
	new_qc++;

	if (shm->queue[i].pid == mypid)
	    my_position = new_qc;

    }

    shm->queue_count = new_qc;


    if (in_what_way == 1) {	/* if we're putting myself in   */
	/*
	 * we will now search for the position that we'll use: we are allowed
	 * to jump past every telnet-user as we're using the CLient.
	 */

	for (i = shm->queue_count - 1; i > 0; i--) {
	    if (shm->queue[i].flags & Q_CLIENTUSER)
		break;
	    benefit++;
	}

	if (benefit > 0) {
	    printf(" *** Hey, smart move to use the CLient: you've jumped past\n");
	    printf(" *** %d users in the Queue this way.\n\n", benefit);
	    fflush(stdout);
	}
	my_position = i + 1;

	/*
	 * we now know where we'll be, so we push down every user below us one
	 * step.
	 */

	for (i = shm->queue_count; i > my_position; i--)
	    bcopy(&(shm->queue[i - 1]), &(shm->queue[i]), sizeof(mono_queue_t));

	strcpy(shm->queue[my_position].host, hostname);
	shm->queue[my_position].pid = mypid;
	shm->queue[my_position].flags = Q_CLIENTUSER;

	shm->queue_count += 1;
    }
    if (in_what_way == 3) {	/* if we're jumping to the top    */

	/* first backup my own entry      */
	bcopy(&(shm->queue[my_position - 1]), &tmpq, sizeof(mono_queue_t));

	/* push down everybody above me   */
	for (i = my_position - 1; i > 0; i--)
	    bcopy(&(shm->queue[i - 1]), &(shm->queue[i]), sizeof(mono_queue_t));

	/* then put back me at the top    */
	bcopy(&tmpq, &(shm->queue[0]), sizeof(mono_queue_t));

    }
    mono_lock_queue(2);		/* unlock it                    */


    return (1);

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

    long t;
    struct tm *tp;

    rewrite_queue(4);		/* remove ghosts                */
    if (my_position == 1) {	/* if I'm first in the queue    */
	if (shm->user_count < maxallow) {	/* I should be let in by now!   */
	    rewrite_queue(2);
	    ok_let_in();
	}
    }
    if (my_position > shm->queue_count) {
	printf("\n*** Oopsie. There seems to be some sort of problem here right now,\n");
	printf("    you are being thrown off the Queue. Sorry - try again and post in Bugs> about this!\n\n");
	fflush(stdout);
	rewrite_queue(2);
	logoff(0);
    }
    if (successful_pre_login == 1 && allowed_skip_queue == 1) {
	if (shm->user_count < MAXUSERS) {
	    printf("---SCHWOOSCH!--- There you skipped the whole Queue... (c:\n");
	    fflush(stdout);
	    rewrite_queue(2);
	    ok_let_in();
	} else if (my_position != 1) {	/* we jump to the queue-top     */
	    printf("Sorry, but the BBS is _completely_ full: I can't even squeeze in a\n");
	    printf("little Sysop right now.\n");
	    printf("But you'll be put at the top of the queue so relax for a minute or two...\n");
	    fflush(stdout);
	    rewrite_queue(3);
	}
    }
    time(&t);
    tp = (struct tm *) localtime(&t);

    if (shm->queue_count == 1)
	printf("(%.2d:%.2d) You're the only one in the queue right now.\n",
	       tp->tm_hour, tp->tm_min);
    else
	printf("(%.2d:%.2d) There are %d users in the queue; you're #%d.\n",
	       tp->tm_hour, tp->tm_min, shm->queue_count, my_position);

    fflush(stdout);

    if (sig != 1) {
	signal(SIGALRM, retry);	/* do this again                */
	alarm(30);
    }
}

/*************************************************
* login()
*************************************************/

void
mono_login()
{
    char pwtest[20];
    char testuser[23];
    user_t *user;
    int i = 0;

    if (successful_pre_login == 1) {
	printf("You can only login once.\n");
	return;
    }
    successful_pre_login = 0;
    allowed_skip_queue = 0;

    (void)srand( 42 );
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


    fflush(stdout);
    strcpy(testuser, get_name());

    if (!strlen(testuser))
	return;

    printf("\rPassword: ");
    fflush(stdout);
    getline(pwtest, -19, 0);

    if ((user = readuser(testuser)) == 0) {
	printf("Incorrect login.\n");	/* no such user...      */
	strcpy(testuser, "");
	return;
    }
    if ( mono_sql_u_check_passwd( user->usernum, pwtest ) == TRUE ) {
	successful_pre_login = 1;
	printf("\nYou are now logged in.\n\n");
	strcpy(username, testuser);

	if (user->priv >= PRIV_SYSOP)
	    allowed_skip_queue = 1;
    } else {
	printf("Incorrect login.\n");
	strcpy(testuser, "");
    }

    xfree(user);
}

/*************************************************
* get_name()
*************************************************/

char *
get_name()
{

    static char pbuf[21];
    int a;

    for (a = 0; a < 21; a++)
	pbuf[a] = 0;

    putc(IAC, netofp);
    putc(G_NAME, netofp);
    putc(2, netofp);
    putc(0, netofp);		/* these three 0's are the sync_byte... */
    putc(0, netofp);
    putc(0, netofp);
    fflush(netofp);

    getblock();

    for (a = 0; a < 21 && pbuf[a - 1] != '\n'; a++)
	pbuf[a] = netget();

    pbuf[a - 1] = '\0';

    return pbuf;
}

/*************************************************
* getline()
*************************************************/

void
getline(char *string, int lim, int frog)
{
    /* char *string;                  Pointer to string buffer             
     * int lim;                       Maximum length; if negative, no-show */

    char *str;

    str = string;

    putc(IAC, netofp);		/* send "get string"-command to the CLient */
    putc(G_STR, netofp);
    putc(lim, netofp);
    putc(0, netofp);		/* substitute for sync_byte             */
    putc(0, netofp);
    putc(0, netofp);
    fflush(netofp);

    getblock();			/* wait for the CLient to send out BLOCK */


    *(str++) = netget();	/* get first character */

    while (*str != '\n')
	*(str++) = netget();
    *str = '\0';

}

/*************************************************
* getblock()
*************************************************/
int
getblock()
{

    int a = 0, b;

    for (;;) {
	b = a;
	a = netget();
	if (b == IAC && a == BLOCK)
	    return 1;
    }
}



/*************************************************
* netget()
*************************************************/
int
netget()
{

    int a;

    if ((a = getc(netifp)) == EOF)
	logoff(0);

    return a;
}

/*************************************************
* logoff()
*************************************************/

void
logoff(int womble)
{
    fclose(netofp);
    fclose(netifp);
    mono_detach_shm();
    exit(0);
}
