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
#else
  #ifdef HAVE_ASM_TERMBITS_H
    #undef HAVE_TERMBITS_H
    #include <asm/termbits.h>
  #else
    #undef HAVE_ASM_TERMBITS_H
    #ifdef HAVE_TERMBITS_H
      #include <termbits.h>
    #endif
  #endif
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

#define extern
#include "front.h"
#undef extern

char hname[255], username[L_USERNAME + 1];
unsigned int maxallow;
int my_position, successful_pre_login = 0;
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
    char tmpallow[5];

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

    sttybbs(0);

    printf("\r");
    printf("Welcome to %s.\n\rRunning %s\n", BBSNAME, BBS_VERSION);
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
