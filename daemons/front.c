/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

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

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include <time.h>
#include <unistd.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "libmono.h"
#include "setup.h"
#include "version.h"
#include "front.h"

char hname[ L_HOSTNAME + 1];
char username[L_USERNAME + 1];
unsigned int maxallow;
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
    int ret;

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    printf("%c%c%c", IAC, WILL, TELOPT_SGA);
    printf("%c%c%c", IAC, WILL, TELOPT_ECHO);

    fflush(stdout);
    umask(0007);

    ret = chdir(BBSDIR);
    if ( ret != 0 ) { exit(1); }

    signal(SIGHUP, log_off);
    signal(SIGPIPE, log_off);

    sttybbs(0);

    printf("\r");
    printf("Welcome to %s.\n\rRunning %s\n\r", BBSNAME, BBS_VERSION);
    fflush(stdout);

    /* if the bbs is down no reason to do anything else */
    if (fexists(DOWNFILE)) {
	ret = system("/bin/cat " DOWNFILE);
	fflush(stdout);
	sleep(5);
	exit(1);
    }

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

    de_initialize();

#ifdef DEBUG
    printf("[Trying to start main program]\n");
#endif

    execl(EXECUTABLE, EXECUTABLE, hname, username, (char *)NULL );
    /* if we can't find yawc just quietly die */
    printf("Horror! The bbs programme has been deleted!\r\n");
    log_off(0);
    return 0;
}

/*************************************************
* log_off()
*************************************************/
void
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
    size_t size;
    struct sockaddr_in acc_addr;
    unsigned long int addr;

    size = sizeof(struct sockaddr_in);
    getpeername(0, (struct sockaddr *) &acc_addr, &size);

    site_num = inet_ntoa(acc_addr.sin_addr);
    addr = inet_addr(site_num);

    if ((host = gethostbyaddr((char *) &addr, 4, AF_INET)) != NULL)
	strncpy(hname, host->h_name, L_HOSTNAME );
    else
	strncpy(hname, site_num, L_HOSTNAME );

    return;
}

/*************************************************
* de_initialize()
*************************************************/

void
de_initialize()
{
    sttybbs(1);			/* restore the old params?      */
    signal(SIGHUP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGALRM, SIG_DFL);

    return;
}

/* eof */
