/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"

#define extern
#include "inter.h"
#undef extern

#include "express.h"
#include "input.h"
#include "routines2.h"

#define DEFAULT_PROTOCOL 0

void
menu_inter()
{
    int i, entries;
    char bbsname[L_BBSNAME + 1];
    char cmd = '\0';
    rbbs_t *bbs;

    cprintf("\n");

    while (cmd != 'q' && cmd != ' ') {

	mono_setdexient();
	entries = 0;
	while ((bbs = mono_getdexient())) {
	    if (entries > 0)
		cprintf("\1f    \1w<\1r%d\1w> \1y%s\n", entries, bbs->name);
	    entries++;
	}
	mono_enddexient();

	cprintf("\n\1g\1fEnter BBS Number, or \1w<\1rq\1w> \1gor \1w<\1rspace\1w>\1g to quit.\n");

	cmd = get_single_quiet("123456789 q");
	switch (cmd) {
	    case 'q':
	    case ' ':
		break;
	    default:
		i = cmd - 48;
		if (i >= entries)
		    continue;

		mono_setdexient();
		while (i-- + 1)
		    bbs = mono_getdexient();
		if (bbs == NULL) {
		    strcpy(bbsname, "Unknown");
		} else {
		    strcpy(bbsname, bbs->name);
		}
		mono_enddexient();
		dexi_wholist(bbsname);
		break;
	}

    }
    return;
}

/*************************************************************
*   port_connect() - internet stuff, establishes connections
*************************************************************/

int
port_connect(const char *bbsname, unsigned int tries)
{

    int clientFd;
    int serverLen;
    rbbs_t *bbs = NULL;
    unsigned int times = 0;

    struct sockaddr_in serverINETAddress;
    struct sockaddr *serverSockAddrPtr;

    struct hostent *hostStruct;
    struct in_addr *hostNode;

    mono_setdexient();
    while (((bbs = mono_getdexient()) != NULL) && strcasecmp(bbs->name, bbsname) != 0) ;
    mono_enddexient();
    if (bbs == NULL)
	return 0;

    serverSockAddrPtr = (struct sockaddr *) &serverINETAddress;
    serverLen = sizeof(serverINETAddress);

    clientFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    serverINETAddress.sin_family = AF_INET;
    serverINETAddress.sin_port = htons(bbs->port);

    hostStruct = gethostbyname(bbs->addr);
    if (hostStruct == NULL)
	return -1;
    hostNode = (struct in_addr *) hostStruct->h_addr;
    serverINETAddress.sin_addr.s_addr = hostNode->s_addr;

    while (times < tries) {
	times++;
	if (!connect(clientFd, serverSockAddrPtr, serverLen))
	    return (clientFd);
	else {

	    (void) cprintf("\1f\1rCould not connect. Retry? \1w(\1gY/n\1w) \1g");
	    (void) fflush(stdout);
	    if (yesno_default(YES) == NO) {
		(void) fflush(stdout);
		break;
	    }
	}
	(void) fflush(stdout);
	sleep(1);
    }
    (void) close(clientFd);
    return (-1);
}

/*************************************************************
* remote_express() - send an eXpress message to dexi server
********************************************************** ***/

int
remote_express(const char *recipient)
{
    int clientFd;
    int i;
    char str[101];
    char send_string[X_BUFFER];
    char override;
    char tmp;
    express_t x;
    char name[L_USERNAME + 1];
    char rbbs[L_BBSNAME + 1];

    if (parse_inter_address(recipient, name, rbbs) == 0) {
	(void) cprintf("\1r\1fInvalid InterBBS name.\1a\n");
	return 0;
    }
    switch (check_remote_user(name, rbbs)) {
	case 0:
	    (void) cprintf("\1r\1fRemote host not responding.\1a\n");
	    return 0;
	case 'Y':
	    (void) cprintf("\1r\1f%s is not online at %s.\1a\n", name, rbbs);
	    return 0;
	case 'U':
	case 'O':
	    (void) cprintf("\1y\1f%s@%s \1ghas disabled %s %s.\n", name, rbbs, config.express, config.x_message_pl);
	    return 0;
	case 'D':
	    (void) cprintf("\1r\1f%s@%s does not wish to receive x's from you.\n", name, rbbs);
	    return 0;
    }

    clientFd = port_connect(rbbs, 3);

    if (clientFd == -1) {
	(void) cprintf("\1r\1fRemote host not responding.\1a\n");
	return 0;
    }
    cprintf("\1c");
    tmp = get_x_lines(send_string, 0);

    override = '3';
    switch (tmp) {
	case 'A':
	    return 1;

	case OR_PING:
	    override = 'P';
	    break;

	case OR_T_TECH:
	case OR_T_ADMIN:
	case OR_T_GUIDE:
	case OR_FEEL:
	case OR_EMOTE:
	    return 1;
    }
    memset(str, 0, sizeof(str));
    (void) sprintf(str, "x%s,%s,%c", name, usersupp->username, override);
    (void) write(clientFd, str, 46);
    (void) write(clientFd, send_string, strlen(send_string) + 1);
    (void) write(clientFd, str, 1);	/* \0 to end body */

    (void) read(clientFd, str, 1);	/* read result */
    (void) close(clientFd);


    switch (str[0]) {

/* forma_ack or so */

	case 'N':
	case 'Y':
	    (void) cprintf("\1f\1y%s \1glogged off from \1y%s \1gbefore you finished typing.\1a\n", name, rbbs);
	    break;
	case 'D':
	    (void) cprintf("\1f\1y%s@%s\1g has disabled %s %s.\1a\n", name, rbbs, config.express, config.x_message_pl);
	    break;
	case 'O':
	    (void) cprintf("\1f\1y%s@%s\1r disabled %s %s before you finished typing.\1a\n", name, rbbs, config.express, config.x_message_pl);
	    break;
	case 'W':
	    if (override == 'P')
		(void) cprintf("\1f\1y%s@%s\1g is \1rbusy\1g right now!\1a\n", name, rbbs);
	    else
		(void) cprintf("\1f\1y%s@%s\1g is \1rbusy\1g and will receive your %s when done.\1a\n", name, rbbs, config.x_message);
	    break;
	case 'A':
	    if (override == 'P')
		(void) cprintf("\1f\1y%s@%s\1g is not busy.\n\1a", name, rbbs);
	    else
		(void) cprintf("\1f\1g%s %s received by \1y%s@%s\1g.\1a\n", config.express, config.x_message, name, rbbs);
	    break;
	case 'K':
	    (void) cprintf("\1f\1g%s has been received, but \1y%s@%s \1gis currently away from keyboard.\1a\n", config.x_message, name, rbbs);
	    break;
	default:
	    cprintf("\1f\1gUnknown acknowledgement \1y'%c'\1g of %s %s.\1a\n", str[0], config.express, config.x_message);
	    break;
    }


    memset(&x, 0, sizeof(express_t));
    strcpy(x.sender, usersupp->username);
    sprintf(x.recipient, "%s@%s", name, rbbs);
    time(&x.time);
    x.override = OR_NORMAL;
    strcpy(x.message, send_string);

    if (xmsgb[0])
	xfree(xmsgb[0]);

    for (i = 1; i < xmsgp; i++)
	xmsgb[i - 1] = xmsgb[i];

    xmsgb[xmsgp - 1] = (express_t *) xmalloc(sizeof(express_t));
    memcpy(xmsgb[xmsgp - 1], &x, sizeof(express_t));

    return 1;

}


/*************************************************************
* dexi_wholist() - read Olymp's wholist
*************************************************************/

int
dexi_wholist(const char *bbsname)
{
    int clientFd, i;
    char str[101];
    FILE *tf;
    char *file;
    int res;

    cprintf("\1f\1g\nConnecting to \1y%s\1g...\1a\n", bbsname);
    fflush(stdout);

    clientFd = port_connect(bbsname, 3);

    if (clientFd == -1) {
	cprintf("\1r\1fRemote host not responding.\1a\n\n");
	return 0;
    }
    file = tempnam(BBSDIR "/tmp", "rwho");

    tf = xfopen(file, "w", FALSE);
    if (!tf) {
	xfree(file);
	return 0;
    }
    strcpy(str, "w");
    write(clientFd, str, 1);

    while ((res = read(clientFd, str, 100)) > 0) {
	for (i = 0; i < res; i++) {
	    if (str[i] != 0) {
		if (str[i] != 10)
		    fprintf(tf, "%c", str[i]);
		else
		    fprintf(tf, "\1D%c", str[i]);
            }
        }

    }
    close(clientFd);
    fclose(tf);
    more(file, 1);
    unlink(file);
    xfree(file);
    return 1;
}

/* this checks the status of user 'name' on bbs 'rbbs' */
int
check_remote_user(const char *name, const char *rbbs)
{
    char str[L_USERNAME * 2 + 6];
    int clientFd;

    clientFd = port_connect(rbbs, 3);

    if (clientFd == -1)
	return 0;

    memset(str, 0, sizeof(str));
    sprintf(str, "x%s,%s,P", name, usersupp->username);
    write(clientFd, str, 45);

    write(clientFd, str, sizeof(str));
    write(clientFd, "", 1);	/* end of header */

    write(clientFd, "", 1);	/* end of message */

    read(clientFd, str, 1);
    close(clientFd);
    return str[0];
}

/*************************************************************
*
* dexi_profile() - profile a person via dexi server
*
*************************************************************/

int
dexi_profile(const char *ruser, const char *rbbs)
{
    int clientFd;
    char *file;
    char str[101];
    unsigned int i, res;
    FILE *tf;

    cprintf("\1f\1gConnecting to \1y%s\1g...\1a\n", rbbs);
    fflush(stdout);

    clientFd = port_connect(rbbs, 3);

    fflush(stdout);
    if (clientFd == -1) {
	cprintf("\1f\1rRemote host not responding.\1a\n");
	return 0;
    }
    file = tempnam(BBSDIR "/tmp", "rprof");

    tf = xfopen(file, "w", FALSE);
    if (!tf) {
	xfree(file);
	return 0;
    }
    (void) sprintf(str, "p%s,%s,%c", ruser, usersupp->username, 4);
    (void) write(clientFd, str, strlen(str) + 1);

    while ((res = read(clientFd, str, 100)) > 0) {
	for (i = 0; i < res; i++) {
	    if (str[i] != 0) {
		if (str[i] != 10)
		    fprintf(tf, "%c", str[i]);
		else
		    fprintf(tf, "\1D%c", str[i]);
            }
        }

    }
    close(clientFd);
    fclose(tf);
    more(file, 1);
    unlink(file);
    xfree(file);
    return 1;
}

int
parse_inter_address(const char *address, char *name, char *bbs)
{
    char rbbs[L_BBSNAME + 1];
    rbbs_t *bbsp;
    unsigned int found = 0;
    char add2[L_USERNAME + L_BBSNAME + 2];
    char *p;

    strcpy(add2, address);
    p = strchr(add2, '@');
    if (p == NULL)
	return 0;
    p++;

    mono_setdexient();
    /* now check if rbbs is a valid bbs identifier */
    while ((bbsp = mono_getdexient()) != NULL) {
	if (strncasecmp(p, bbsp->name, strlen(p)) == 0) {
	    if (!found) {
		found = 1;
		strcpy(rbbs, bbsp->name);
	    } else {
		mono_enddexient();
		return 0;
	    }
	}
    }

    mono_enddexient();
    if (found == 0) {
	return 0;
    }
    strcpy(bbs, rbbs);
    *(p - 1) = 0;
    strcpy(name, add2);
    *(p - 1) = '@';
    return 1;
}


/***************************************************
* InterBBS stuff for system Config.
***************************************************/

void
print_inter_hosts()
{
    unsigned int entries = 0;
    rbbs_t *bbs;

    cprintf("\n\1f\1wBBS's currently accessible via InterBBS:\n\n");

    mono_setdexient();
    while ((bbs = mono_getdexient())) {
	if (entries > 0)
	    cprintf("\t\1f\1g%d. \1y%s\t\1g%s\1a\n", entries, bbs->name, bbs->addr);
	entries++;
    }
    mono_enddexient();
    return;
}

/* eof */
