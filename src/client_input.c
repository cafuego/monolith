/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>

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
#include "telnet.h"
#include "input.h"
#include "express.h"
#include "main.h"
#include "routines2.h"
#include "statusbar.h"

#define MAXIDLE 45

long sync_byte;
int client_flags = 0;

#define CF_POSTMARK 1
#define CF_CLIENT_IDET 2

int clientidle;

static void sendfile(const char *filename);
static void getblock(void);
static void getfile(const char *);
void connect_socket(void);
void unidle(int how);
int real_netget(void);

/*************************************************
* get_name()
*
* Used for getting names (usernames, roomnames,
* etc.) Capitalizes first letter of word auto-
* magically. Does different things depending on
* the value of quit_priv. The name is then
* returned to the caller. 
* 
* IF
* quit_priv = 1  ->  one can use CTRL-D to break
*		     the input...
*
* quit_priv = 2  ->  get a 20 characters name
*		     (UserName) with TABfeature
*
* quit_priv = 3  ->  get a 40 characters name
*		     (RoomName); changed so that
*                    you can use *;:)( and so on.
*		     ...now also with TABfeature!
*
* quit_priv = 5  ->  the same as 2, but with
*		     numbers aswell. (used for
*		     <p>rofiling and <aue>diting)
*
*************************************************/

char *
get_name(int quit_priv)
{

    static char pbuf[L_USERNAME + L_BBSNAME + 2];
    size_t d;

    client_input(G_NAME, quit_priv);

    for (d = 0; d <= sizeof(pbuf) /*was 40 */ &&pbuf[d - 1] != '\n'; d++)
	pbuf[d] = netget(0);

    pbuf[d - 1] = '\0';

    return pbuf;
}

/*************************************************
* xgetline()
*************************************************/

int
xgetline(char *string, int lim, int nocol)
{
    char *str;
    int cnt;

    nox = 1;
    strcpy(string, "");

    str = string;

    (void) client_input(G_STR, lim);

    /* kirth added `cnt' variable to stop buffer overflows */
    for (cnt = 0; *(str - 1) != '\n' && cnt < abs(lim); cnt++)
	*(str++) = netget(0);

    *(str - 1) = '\0';

    if (nocol)
	strremcol(str);

    return 0;
}

/*************************************************
* client_input()
*************************************************/

int
client_input(int command, int argument)
{

    char one, two, three;

    /*
     * now code the (int) how_many into char's to send over to the CLient.
     */

    one = sync_byte >> 16;
    two = (sync_byte - (one << 16)) >> 8;
    three = sync_byte - (one << 16) - (two << 8);

    putchar(IAC);		/* * Is A Command                 */
    putchar(command);		/* * G_POST etc. etc.             */
    putchar(argument);		/* * argument to the command      */
    putchar(one);
    putchar(two);
    putchar(three);

    fflush(stdout);		/* send'em over right now */
    getblock();
    return 0;
}


/*************************************************
* send_update_to_client()
*************************************************/
void
send_update_to_client()
{
    putchar(IAC);
    putchar(UPDATE);
    putchar((usersupp->flags & US_ANSI) ? 1 : 0);
    putchar((usersupp->flags & US_NOFLASH) ? 1 : 0);
    putchar((usersupp->flags & US_NOBOLDCOLORS) ? 1 : 0);

}

/*************************************************
* sendfile()
*
* sends a file to the CLient, with the appropriate
* command-char's. 
*
*************************************************/

static void
sendfile(const char *filename)
{

    FILE *fp;
    int a;

    fp = xfopen(filename, "r", FALSE);
    if (!fp)
	return;

    (void) cprintf("%c%c", IAC, FILE_S);

    for (;;) {
	if ((a = getc(fp)) < 0)
	    break;

	(void) putchar(a);
    }

    (void) fclose(fp);
    (void) cprintf("%c%c", IAC, FILE_E);
    return;
}



/*************************************************
* getfile()
*
* get a file from the CLient.
*************************************************/

static void
getfile(const char *filename)
{

    int a;
    FILE *fp;

    fp = xfopen(filename, "w", FALSE);
    if (!fp)
	return;

    while ((a = netget(0)) != '\0')
	(void) putc(a, fp);

    (void) fclose(fp);
    return;
}



/*************************************************
* connect_socket()
*************************************************/

void
connect_socket()
{

    /*
     * we'll now send over the START-command so that the CLient
     * will synchronize with our sync_byte.
     *
     * ...this was removed successfully (?) by KHaglund when it became obvious
     *    it is not needed.
     */

    (void) putchar(IAC);
    (void) putchar(START);

    sync_byte = 1;		/* * was 1 before KHTEST */

}



/*************************************************
*
* get_buffer()				-KHaglund
*
* I wonder why such a strange name for this
* function?
*
* how:	1 -> normal message.
*	2 -> upload message.
*       3 -> editor message.
*
*	5 -> reply'ing, but with no difference.
*       9 -> normal entering, FORCED!
*
*************************************************/

int
get_buffer(FILE * fp, int how, int *lines)
{

    int a = -1;
    int edithow;

    if (how == 5 || how == 9)
	edithow = EDIT_NORMAL;
    else
	edithow = how;

    *lines = 0;

    for (;;) {
	(void) client_input(G_POST, edithow);

	while ((a = netget(0)) != 4) {
	    putc(a, fp);
	    if (a == 10 || a == 13)
		*lines += 1;
	}

	a = netget(0);

	if (a != 's' && a != 'S' && how == 9) {		/*
							 * if the user did 
							 * not save it 
							 */
	    cprintf("\1f\1rYou MUST write this %s!\n\n", config.message);
	    continue;
	}
	break;
    }

    return a;
}



/*************************************************
*
* get_x_lines()
*
*************************************************/

int
get_x_lines(char *xstring, int X_PARAM)
{

    char *p, *n, override = OR_NORMAL;
    int d;

    client_input(G_LINES, 1);

    n = p = xstring;

    *p++ = '>';

    for (d = 1; d < X_LENGTH * 80 && *(p - 1); d++) {
	*p++ = netget(0);

	if (*(p - 1) == '\n') {
	    *p++ = '>';
	    d++;
	    if (strncmp(n, ">GUARD", 6) == 0 && usersupp->priv >= PRIV_TECHNICIAN && X_PARAM != -2) {
		override = OR_T_TECH;
		p = n + 1;
	    } else if (strncmp(n, ">ADMIN", 6) == 0 && usersupp->priv >= PRIV_SYSOP && X_PARAM != -2) {
		override = OR_T_ADMIN;
		p = n + 1;
	    } else if (strncmp(n, ">SYSGUIDE", 6) == 0 && (usersupp->flags & US_GUIDE) && X_PARAM != -2) {
		override = OR_T_GUIDE;
		p = n + 1;
	    } else if (strncmp(n, ">SYS", 4) == 0 && (usersupp->flags & US_GUIDE) && X_PARAM != -2) {
		override = OR_T_GUIDE;
		p = n + 1;
	    } else if (strncmp(n, ">NOWHEREVOICE", 13) == 0 && usersupp->priv >= PRIV_SYSOP && (X_PARAM == -2 || X_PARAM == -3)) {
		override = OR_NOWHERE;
		p = n + 1;
	    } else if (strncmp(n, ">CTHULHU", 7) == 0 && usersupp->priv >= PRIV_SYSOP && (X_PARAM == -2 || X_PARAM == -3)) {
		override = OR_SILENT;
		p = n + 1;
	    } else if (strncmp(n, ">FISH", 5) == 0 && usersupp->priv >= PRIV_SYSOP && (X_PARAM == -2 || X_PARAM == -3)) {
		override = OR_FISH;
		p = n + 1;
	    } else if (strncmp(n, ">LLAMA", 6) == 0 && usersupp->priv >= PRIV_SYSOP && (X_PARAM == -2 || X_PARAM == -3)) {
		override = OR_LLAMA;
		p = n + 1;
	    } else if (strncmp(n, ">SHOUT", 6) == 0 && usersupp->priv >= PRIV_SYSOP && (X_PARAM == -2 || X_PARAM == -3)) {
		override = OR_SHOUT;
		p = n + 1;
	    } else if (strncmp(n, ">ABORT", 6) == 0) {
		override = 'A';
		break;
	    } else if (strncmp(n, ">DELETELINE", 11) == 0)
		p = n + 1;
	    else
		n = p - 1;

	}
    }

    /*
     * now get rid of the extra '>' and '\n' at the end.
     */

    *(p - 4) = '\0';

    if (xstring[0] == '\0')
	return OR_PING;

    return (override);

}

/*************************************************
* editor_edit()
*************************************************/
int
editor_edit(const char *fname)
{

    nox = 1;
    status_bar_off();

    /* send the old file to the CLient */
    sendfile(fname);

    putchar(IAC);
    putchar(EDIT_S);
    fflush(stdout);

    /* wait until the CLient is finished */
    getblock();

    /* get the finished file from the CLient */
    getfile(fname);

    idletime = 0;
    mono_change_online(who_am_i(NULL), "", 3);

    status_bar_on();
    return 1;
}

/*************************************************
* cprintf()
*************************************************/
int
cprintf(const char *fmt,...)
{
    va_list args;
    int i;

    fflush(stdout);
    va_start(args, fmt);
    i = vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    return i;

}

/*************************************************
* inkey()
*************************************************/

int
inkey()
{

    int i = -5;

    /* kirth added the j variable to ignore all LF's that
     * are sent after a CR */

    fflush(stdout);

    if ((nox == 0) && (xmsgp < XLIMIT))
	are_there_held_xs();	/* there are new x's  */

    while (i < 0 || i > DEL)
	i = netget(0);

    if (i == DEL)
	i = BS;

/*
 * if (i == LF)
 * i = CR;
 */

    return i;
}

int
flush_input()
{
    tcflush(0, TCIFLUSH);
    return 0;
}


/*************************************************
* getblock()				(31-03-95)
*
* used to receive the IAC+BLOCK that the CLient
* sends out when it wants us to stop ignoring what
* it sends.
*
* this is now handled by the state machine -Flint 
*************************************************/

void
getblock()
{
    netget(1);			/* let netget do the work =) -Flint */
}

/*******************************************************************
* real_netget() - the actual network access, excluded from netget()
********************************************************************/
int
real_netget()
{
    
#ifdef DITWERKTNIET
    size_t ret;
    char buf[2];
#endif
    int i = -1;
    int err_ctr = 0;
#ifdef DITWERKNIET
    while (i < 0) {
	ret = read(0, buf, 1);
	i = buf[0];
	if (i == EOF)
	    logoff(ULOG_NORMAL);
    }
    return i;
#endif

    do {
	i = fgetc(stdin);
	if (i == EOF) {
	    if (errno == EINTR) {

/* hack to try and catch runaway clients */
		err_ctr++;
		if (err_ctr > 1000)
		    logoff(ULOG_NORMAL);

		continue;
	    } else
		logoff(ULOG_NORMAL);
        }
    } while (i < 0);
    return i;

#ifdef ZUT
//    here we try to convert the thing to read. 
    //    lots of nasty things happen on this bbs with signals, 
    //   and read gives a friendly EINTR if that happens, so it
    //  doesn't crash

    do {
	ret = read(0, buf, 1);
	if (ret == -1 && errno == EINTR)
	    continue;
	i = buf[0];
    }
    while (i > DEL || i == 0 || i == LF);

    do {
	ret = read(0, buf, 1);
	if (ret == -1) {
	    log_it("errors", "netget() read error: %s", strerror(errno));
	    switch (errno) {
		case EINTR:
		    continue;
		case EBADF:
		case EIO:
		    raise(SIGHUP);
		    break;
		default:
		    log_it("errors", "strange read error: %s", strerror(errno));
		    raise(SIGHUP);
		    break;
	    }
	}
    } while (ret == -1);
    if (ret != 1)
	printf("vaag");
    return buf[0];
#endif
}

/*******************************************************************
* unidle() - remove the idle flags from the wholist
********************************************************************/
void
unidle(int how)
{
    if (idletime >= 2)
	mono_change_online(who_am_i(NULL), "", 3);	/* remove idleflag from wholist */

    idletime = 0;		/* user is active again */

    if (how)			/* user is really active, not only CLient-reported */
	clientidle = 0;
}

/*******************************************************************
* netget()
*
* Includes an expandable IAC state machine. I've made it so that it
* can take EVERY IAC the PR-DWD CLient could possibly send although
* it probably will never do. Just to avoid bad surprises.
*
* skip_sync_byte: 0 -> increase sync_byte
*                 1 -> don't increase sync_byte, act as getblock
********************************************************************/

int
netget(char skip_sync_byte)
{
    int a = 0;
    int state = TS_DATA;

    for (;;) {			/* state machine */
	a = real_netget();

	if ((state != TS_IAC) && (!skip_sync_byte))
	    sync_byte++;

	switch (state) {
	    case TS_DATA:
		switch (a) {
		    case IAC:
			state = TS_IAC;
			break;
		    default:
			if (!skip_sync_byte) {	/* not in getblock mode */
			    unidle(1);
			    return (a);
			}
		}
		break;		/* end TS_DATA */

	    case TS_IAC:
		switch (a) {
		    case BLOCK:
			state = TS_DATA;
			if (skip_sync_byte) {	/* getblock mode? then return now. */
			    unidle(1);
			    return (0);		/* no return value expected by getblock() */
			}
		    case CLIENT:
			state = TS_DATA;	/* ignored; maybe a unidle(1) could go here? */
			break;
		    case DO:
			state = TS_DO;
			break;
		    case DONT:
			state = TS_DONT;
			break;
		    case POST_K:	/* will be ignored      */
			a = real_netget();	/* trash post number    */
			a = real_netget();	/* trash closing #17    */
			if (!skip_sync_byte)
			    sync_byte += 2;
			state = TS_DATA;
			break;
		    case SB:
			state = TS_SB;
			break;
		    case WILL:
			state = TS_WILL;
			break;
		    case WONT:
			state = TS_WONT;
			break;
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_IAC */

	    case TS_DO:
		switch (a) {
		    case TELOPT_LOGOUT:
			logoff(ULOG_NORMAL);	/* no more break or state setting necessary */

		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_DO */

	    case TS_DONT:
		switch (a) {
		    case TELOPT_LOGOUT:
			if (clientidle <= MAXIDLE)	/* requested "no-logout" for too long? */
			    unidle(0);	/* don't log me out, I'm not idle!     */
			else {
			    cprintf("\n\7\1f\1w[\1yBBS\1w: \1gPlease return to the BBS or the connection will be closed.\1w]\n");
			    fflush(stdout);
			}
			state = TS_DATA;
			break;
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_DONT */

	    case TS_WILL:
		switch (a) {
		    case CLIENT_OPTIONS:	/* negotiate post marking & idle detection */
			cprintf("%c%c%c%c%c%c%c%c%c%c", IAC, SB, CLIENT_OPTIONS,
				POST_MARK, ON, IAC, SB, CLIENT_OPTIONS, TELOPT_LOGOUT, ON);
			state = TS_DATA;
			break;
		    case TELOPT_BBSPOST:	/* these are ignored right now */
		    case TELOPT_NAWS:
		    case TELOPT_TIMING_MARK:
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_WILL */

	    case TS_WONT:
		switch (a) {
		    case CLIENT_OPTIONS:
			/* no action currently */
			state = TS_DATA;
			break;
		    case TELOPT_BBSPOST:	/* these are ignored right now */
		    case TELOPT_NAWS:
		    case TELOPT_TIMING_MARK:
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_WONT */

	    case TS_SB:
		switch (a) {
		    case CLIENT_OPTIONS:
			state = TS_SB_CLIENT_OPTIONS;
			break;
		    case TELOPT_NAWS:	/* negotiation about window size */
			a = real_netget();	/* ignore 3 \0's */
			a = real_netget();
			a = real_netget();
			a = real_netget();	/* this is the current screenlength. You    */
			/*  could / should use it, e.g. copy it to  */
			/* usersupp->screenlength.                  */
			a = real_netget();	/* IAC SE, ignored */
			a = real_netget();
			if (!skip_sync_byte)
			    sync_byte += 5;	/* 6 bytes - IAC = 5 ;) */
			state = TS_DATA;
			break;
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_SB */

	    case TS_SB_CLIENT_OPTIONS:
		switch (a) {
		    case POST_MARK:
			if (real_netget() == OK)
			    client_flags |= CF_POSTMARK;
			state = TS_DATA;
			break;
		    case TELOPT_LOGOUT:
			if (real_netget() == OK)
			    client_flags |= CF_CLIENT_IDET;
			state = TS_DATA;
			break;
		    default:
			state = TS_DATA;
			break;
		}
		break;		/* end TS_SB_CLIENT_OPTIONS */

	}			/* end of main switch */
    }				/* end of main loop */
}

/*--------------------------------------------------------------------------

  Here are the other changes necessary to make the features work. 


* post marking:
    Go into the readmsgs() (messages.c). Search for the line

      e=read_message(fullroom.FRpos[a], fullroom.FRnum[a]);

    and replace it with this:

#ifdef CLIENTSRC
    if(client_flags & CF_POSTMARK)
    {
      putchar(IAC);
      putchar(POST_S);
    }
#endif
      e=read_message(fullroom.FRpos[a], fullroom.FRnum[a]);
#ifdef CLIENTSRC
    if(client_flags & CF_POSTMARK)
    {
      putchar(IAC);
      putchar(POST_E);
    }
#endif


* CLient timeout feature:

    open yawc.c and go to sleeping(). replace thsi (timeout section)

#else
    if( idletime == TIMEOUTCLIENT && usersupp->priv < PRIV_SYSOP )
#endif

    with this:

#else
    if( idletime == TIMEOUTCLIENT && usersupp->priv < PRIV_SYSOP )
    {
      cprintf("%c%c%c", IAC, WILL, TELOPT_LOGOUT);
      fflush( stdout );
    }
    if( idletime >= TIMEOUTCLIENT + 1 && usersupp->priv < PRIV_SYSOP )
#endif


  That's it, I hope I didn't forget anything important. Enjoy - Flint. =)

-----------------------------------------------------------------------------*/
