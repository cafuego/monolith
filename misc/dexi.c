/* $Id$ */

/**************************************************************************
   BBS Data EXchange Interface server; some routines taken from YAWC files
   It will install itself at Port 1996 to provide some BBS communication.
   (C) Flint 1996. ;) Edited by Kirth@monolith
***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifdef HAVE_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"

#include "registration.h"
#include "dexi2.h"

int main(int, char **);
static void xmsg(void);
static void profile_user(void);
static void errormsg(char param);
void log_off(int sig);
static char *get_hostname(void);

char cmd, site[16], rbbs[20];
int color;

/* log_off() - detach shared memory.
 */
void
log_off(int sig)
{
    sig++;
    fflush(stdout);
    mono_detach_shm();
    exit(0);
}

/* void profile_user() 
 */
static void
profile_user()
{
    char command[L_USERNAME * 2 + 3], *charp, buf;
    user_t *structure;
    int res;

    strcpy(command, "");
    charp = command;
    while (((res = read(0, &buf, 1)) == 1) && (buf != 0)) {
	/* michel now has a ^D to end the message, should be a \0 */
	*charp = buf;
	charp++;
    }

    charp = strchr(command, ',');
    *charp = 0;
    charp++;

    if (check_user(command) == FALSE) {
	printf("No such user.\n");
	return;
    }
    color = 1;
    structure = readuser(command);
    print_user_stats(structure);
    free(structure);
    return;
}

/*************************************************************************
* errormsg() - sends back error message if an unknown command is received
**************************************************************************/
static void
errormsg(char param)
{
    fprintf(stderr, "\n\1rUnknown command '%c'.\nClosing connection.\n", param);
}

/**************************************************************************
* xmsg() - reads X-Msg, sends it to the recipient and returns the ack byte
* after express() from yawc, changed by Flint :)
***************************************************************************/

static void
xmsg()
{
    char *sender, *recipient, *p, override, buf;
    char send_string[X_BUFFER], aaa[50];
    int res;

    color = 1;

    p = aaa;
#ifdef NEW
    while (((res = read(0, &buf, 1)) == 1) && (buf != 0)) {
	*p = buf;
	p++;
    }
    *p = 0;

#else
    read(0, aaa, 45);
#endif
    /* format: x-recipient,x-sender,override */

    recipient = aaa;
    sender = strchr(aaa, ',');
    *sender = 0;
    sender++;

    p = strchr(sender, ',');
    *p = 0;
    override = *++p;

    strcpy(send_string, "");	/* reads x body */
    p = send_string;
    while (((res = read(0, &buf, 1)) > 0) && (buf != 0)) {
	*p = buf;
	p++;
    }
    *p = 0;			/* add terminating 0 */

    /* okay, check stuff, then start building an x structure */
    intersendx(recipient, sender, send_string, override);

}

/*
 * main()
 */
int
main(int argc, char **argv)
{
    rbbs_t *dex;

    set_invocation_name(argv[0]);

    /* ignore all signals */
    signal(SIGILL, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGIOT, SIG_IGN);
    signal(SIGBUS, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);	/* KH-addition to get rid of Zombie's */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGALRM, log_off);
    alarm(180);			/* kill myself if still alive after 3 minutes */

    chdir(BBSDIR);

    strcpy(site, get_hostname());

    mono_setdexient();
    while ((dex = mono_getdexient())) {
	if (dex == NULL) {
	    cmd = 'w';
	    break;
	}
	if (strcmp(site, dex->addr) == 0) {
	    strcpy(rbbs, dex->name);
	    read(0, &cmd, 1);
	    break;
	}
    }
    mono_enddexient();

    mono_connect_shm();

    switch (cmd) {
	case 'w':
	    show_online(1);
	    break;
	case 'p':
	    profile_user();
	    break;
	case 'x':
	    xmsg();
	    break;
	default:
	    errormsg(cmd);
	    break;
    }

    log_off(0);
    return 0;
}

/*
 * char *get_hostname()
 * returns the hostname of the remote system.
 */
static char *
get_hostname()
{
    int size;
    struct sockaddr_in acc_addr;

    size = sizeof(struct sockaddr_in);
    /* YUCK! $) Ishtar 960806 */
    getpeername(0, (struct sockaddr *) &acc_addr, &size);
    /* int  getpeername(int  s, struct sockaddr *name, int *namelen); */
    return inet_ntoa(acc_addr.sin_addr);
}

/* eof */
