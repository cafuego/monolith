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

#define Q_CLIENTUSER    1	/* i am a client-user                   */

unsigned int maxallow, my_position;
char hostname[255];
char username[L_USERNAME + 1];
int successful_pre_login = 0;
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
    printf("Welcome to %s.\n\rRunning %s\n", BBSNAME, BBS_VERSION);

    fflush(stdout);

    chdir(BBSDIR);

    /* if the bbs is down no reason to do anything else */
    if (fexists(DOWNFILE)) {
	system("/bin/cat " DOWNFILE);
	fflush(stdout);
	sleep(5);
	logoff(0);
    }
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
