/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

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

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "monolith.h"
#include "libmono.h"
#include "setup.h"
#include "version.h"

#define extern
#include "server.h"
#undef extern

char hostname[255];
char username[L_USERNAME + 1];
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
    char badness = 0, host[50], temp[85];

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    printf("\rWelcome to %s.\n\rRunning %s\n", BBSNAME, BBS_VERSION);
    fflush(stdout);

    chdir(BBSDIR);

    /* if the bbs is down no reason to do anything else */
    if (fexists(DOWNFILE)) {
	system("/bin/cat " DOWNFILE);
	fflush(stdout);
	sleep(5);
	logoff(0);
    }
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

    execl(BBSEXECUTABLE, BBSEXECUTABLE, hostname, username, (char *)NULL );
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
    size_t size;

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
* logoff()
*************************************************/

void
logoff(int womble)
{
    fclose(netofp);
    fclose(netifp);
    exit(0);
}
