/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
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
#include "setup.h"

#define extern
#include "input.h"
#undef extern

#include "express.h"
#include "main.h"
#include "rooms.h"
#include "routines2.h"

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
*		     (UserName) with TABfeature!
*
* quit_priv = 3  ->  get a 40 characters name
*		     (RoomName); changed so that
*                    you can use *;:)( and so on.
*		     ...now also with TABfeature!
*
* quit_priv = 5  ->  the same as 2, but with
*		     numbers aswell. (used for
*		     <p>rofiling and <aue>diting)
*************************************************/

char *
get_name(int quit_priv)
{

#define VALIDCHARS "1234567890;:.,_-+=*!&()/'?@$"
    /* these are  useable when quit_priv == 3 */

    static char pbuf[L_USERNAME + L_BBSNAME + 2];	/* basically the string buffer */
    register char *p;		/* pointer current position in buffer  */
    register char c;		/* character that is entered */

    room_t qkr;			/* room info for tab completion */
    int upflag;			/* TRUE if next char has to be uppercase */
    int fflag, pblen = 0, invalid = 0;
    unsigned int a = 0;		/* counter for tab completion */
    int i = 0;
    btmp_t *bp = NULL;

    memset(pbuf, 0, 41);

    for (;;) {
	upflag = fflag = TRUE;
	if (shm)
	    i = shm->first;
	a = 0;
	p = pbuf;

	/* -- get a key -- */
	for (;;) {
	    c = inkey();

	    if (c == '\n' || c == '\r')
		break;

	    if (c == CTRL_U || c == CTRL_X) {
		back(strlen(pbuf));
		strcpy(pbuf, "");	/* clear the pbuf buffer        */
		p = pbuf;
		pblen = '\0';
		upflag = fflag = TRUE;
		continue;
	    }
	    if (c == CTRL_D && quit_priv == 1) {
		pbuf[0] = CTRL_D;
		pbuf[1] = '\0';
		return pbuf;
	    }
	    /* TAB-feature, username  */
	    if (shm && (c == 9) && (quit_priv == 2 || quit_priv == 5)) {
		while (i != -1) {
		    bp = &(shm->wholist[i]);
		    if (strncasecmp(pbuf, bp->username, pblen) == 0) {
			if (EQ(pbuf, bp->username)) {
			    break;
			    /* complete match already, do nothing */
			} else {
			    /* partial match */
			    back(strlen(pbuf));
			    strcpy(pbuf, bp->username);
			    p = pbuf + strlen(pbuf);
			    cprintf("%s", pbuf);
			    break;
			}
		    }
		    i = bp->next;
		}
		if (bp->next == -1)
		    i = shm->first;
		continue;

	    } else if (c == 9 && quit_priv == 3) {
		/* TAB-feature with roomname */
		for (; a < MAXQUADS; a++) {
                    read_forum( a, &qkr );
		    if ((qkr.flags & QR_INUSE)
			&& strncmp(pbuf, qkr.name, pblen) == 0
			&& i_may_read_forum(a)
			) {
			back(strlen(pbuf));
			strcpy(pbuf, qkr.name);
			p = pbuf + strlen(pbuf);
			cprintf("%s", pbuf);
			break;
		    }
		}
		if (a >= MAXQUADS - 1)
		    a = 0;
		continue;
	    }
	    if (c == '_')
		c = ' ';

	    if (c == ' ' && (fflag || upflag))
		continue;

	    if (c == '\b' || c == ' ' || isalpha(c) || (isdigit(c) &&
	       quit_priv == 5) || ((c == '@' || c == ',') && quit_priv == 5)
		|| (strchr(VALIDCHARS, c) && (quit_priv == 3)))
		invalid = 0;
	    else {
		flush_input();
		continue;
	    }

	    if (c == '\b' && p > pbuf) {
		back(1);
		p--;
		pblen--;
		upflag = (p == pbuf || *(p - 1) == ' ');
		if (p == pbuf)
		    fflag = 1;

		pbuf[strlen(pbuf)] = 0;
		*p = 0;
	    } else if ((p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : L_USERNAME]
			&& (isalpha(c) || (isdigit(c) && quit_priv == 5) || ((c == '@' || c == ',') && quit_priv == 5) ||
		  c == ' ' || (strchr(VALIDCHARS, c) && quit_priv == 3)))) {
		fflag = 0;
		if (upflag && islower(c))
		    c = toupper(c);

		upflag = 0;	/* hack to make space working after '*'
				 * f.e. */

		if (c == ' ')
		    upflag = 1;
		*p++ = c;
		(void) putchar(c);
	    }
	    pblen = strlen(pbuf);

	}
	*p = '\0';
	break;
    }

    if (p > pbuf || quit_priv >= 2)
	cprintf("\r\n");

    if (p > pbuf && p[-1] == ' ')
	p[-1] = 0;


    return pbuf;

}

/**************************************************
 * getline( char *string, int lim, int nocol )
 *
 * char *string;		Pointer to string buffer
 * int lim;		Maximum length; if negative, no-show
 * int nocol;		flag: 1 -> no colors allowed
 *************************************************/
int
getline(char *string, int lim, int nocol)
{
    int a;
    char noshow = FALSE;
    char *p;
    size_t length;

    length = abs(lim);
    if (lim < 0)
	noshow = TRUE;

    strcpy(string, "");
    p = string;

    for (;;) {
	a = inkey();
        a &= 127;

	/* ^U or BS at beginning of line */
	if ((a == CTRL_U || a == CTRL_X || a == BS) && strlen(string) == 0)
	    continue;

	/* ^U someplace else on line    */
	if (a == CTRL_U || a == CTRL_X) {
	    while (p-- > string) {
		if (*(p - 1) != CTRL_A) {
		    back(1);
		} else
		    p--;
	    }
	    cprintf("\1a");
	    strcpy(string, "");	/* clear the string buffer      */
	    p = string;
	    continue;
	}
	if (a == 13 || a == 10) {
	    (void) putchar(13);
	    (void) putchar(10);
	    return 0;
	}
	if (a == 8) {
	    *(--p) = '\0';
	    /* color-hack: if the character before the one that you're going
	     * to backspace-erase is ^A, delete them both.  */

	    if (*(p - 1) != '\1') {
		back(1);
	    } else {
		*(--p) = '\0';	/* delete ^A */
		cprintf("\1a\1f");	/* reset the colors     */
	    }
	    continue;
	}
	if ((strlen(string) >= length))
	    continue;

	if (a == '\1' && nocol == FALSE) {
	    cprintf("Color: ");
	    a = inkey() & 127;
	    cprintf("\b\b\b\b\b\b\b       \b\b\b\b\b\b\b");

	    if (a < 32 || a > 126) {	/* if not-allowed characters    */
		(void) putchar('\007');
		continue;
	    }
	    *p++ = '\1';
	    *p++ = a;
	    *p = '\0';

	    cprintf(p - 2);	/* print out the chosen color   */
	    cprintf(" \b");
	    continue;
	}
	if ((a < 32) && (a != 8))
	    a = 32;

	*p++ = a;
	*p = '\0';

	if (noshow == TRUE)
	    (void) putchar('.');
	else
	    (void) putchar(a);
    }
    return 0;
}

/*************************************************
* get_string_wr()
* length = length of line
* result = pointer to string to store result
* line = line of message, line=0 -> first line
* flag: 1 -> not used for X's --what's this?
*************************************************/

int
get_string_wr(int lim, char *result, int line, int flag)
{

    static char wrap[80];
    char *rest;
    register char *p, *q;
    register int c;
    size_t length;

    length = lim;

    if (line == 0)
	strcpy(wrap, "");

    /* we get something wrapped from the prev. line */
    if (*wrap) {
	cprintf("%s", wrap);
	strcpy(result, wrap);
	p = result + strlen(wrap);
	*wrap = '\0';
    } else {
	p = result;
	*p = '\0';
    }

    for (;;) {
	c = inkey();

	/* check for deletion characters at beginning of line */
	if ((c == BS || c == DEL || c == CTRL_U || c == CTRL_X) && (p == result)) {
	    continue;
	}
	if (c == BS || c == DEL) {
	    *(--p) = '\0';
	    /* color-hack: if the character before the one that you're going
	     * to backspace-erase is ^A, delete them both.  */

	    if (*(p - 1) != CTRL_A) {
		back(1);
	    } else {
		*(--p) = '\0';	/* delete ^A */
		cprintf("\1a\1f");	/* reset the colors     */
	    }
	    continue;
	}
	/* erase whole line */
	if (c == CTRL_U || c == CTRL_X) {
	    while (p-- > result) {
		if (*(p - 1) != CTRL_A) {
		    back(1);
		} else
		    p--;
	    }
	    strcpy(result, "");
	    cprintf("\1a");
	    p = result;
	    continue;
	}
	/* return/newline, end of message */
	if (c == '\n' || c == '\r')
	    break;

	/* if this is the last character on last line of x, ignore all */
	if ((line == X_LENGTH - 1) && (flag != 1) && (strlen(result) >= length))
	    continue;

	/* color-character */
	if (c == CTRL_A) {
	    c = inkey();
	    *p++ = CTRL_A;	/* put the <ctrl-a>-char first; */
	    *p++ = c;		/* ...then the "real" colorcharacter. */
	    *p = '\0';
	    cprintf("%s", p - 2);
	    continue;
	}
	/* unprintable */
	if (!isprint(c)) {
	    continue;
	}
	/* print normal character */
	if (strlen(result) < length) {
	    *p++ = c;
	    *p++ = '\0';
	    p--;
	    (void) putchar(c);
	    continue;
	}
	/* we are now at the last character on the line, and checking
	 * if we should wrap. */

	/* space, just jump to next line */
	if (c == ' ')
	    break;

	/* find the first space searching backwards */
	for (q = p; *q != ' ' && q > result; q--) ;

	/* found a space */
	if (q > result) {
	    *q = '\0';
	    q++;
	    for (rest = wrap; q < p; q++) {
		back(1);
		*rest++ = *q;
	    }
	    *rest++ = c;
	    *rest = '\0';
	}
	break;
    }				/* for( ;; ) */
    *p = '\0';
    cprintf("\n");
    return 0;
}

/*************************************************
*
* get_buffer()				-KHaglund
*
* how:	1 -> normal entering
*	2 -> Ctrl-D-entering
*	3 -> Editor
*
*	5 -> reply, nothing different.
*	9 -> FORCED normal entering.
*

* this is horrible and needs to be separated
* the way a posts is entered should be different
* from the type of post

*************************************************/

int
get_buffer(FILE * outputfp, int how, int *lines)
{
    FILE *fp, *fp2;
    char string[80], tmpfname[40];
    int edithow, a, b, cmd, next_stop_menu = 0;

    if (how == 5 || how == 9)
	edithow = EDIT_NORMAL;
    else
	edithow = how;

    strcpy(tmpfname, tempnam(BBSDIR "tmp", "getbuf"));

    fp = xfopen(tmpfname, "w+", FALSE);
    if (fp == NULL)
	return -1;

    for (;;) {
	switch (edithow) {
	    case EDIT_NORMAL:
		/* basically, get input with wrap, save in *fp */
		for (;;) {
		    get_string_wr(79, string, 1, 1);
		    if (strlen(string) < 1) {
			next_stop_menu = 1;
			break;
		    }
		    fprintf(fp, "%s\n", string);
		}
		break;

	    case EDIT_CTRLD:
		for (;;) {
		    a = inkey();

		    if (a == 255)
			a = 32;
		    if (a == 13)
			a = 10;


		    if (a != 4) {
			(void) putc(a, fp);
			(void) putchar(a);
		    }
		    if (a == 10)
			(void) putchar(13);

		    if (a == 4) {	/* CtrlD */
			next_stop_menu = 1;
			break;
		    }
		}
		break;

	    case EDIT_EDITOR:
		(void) editor_edit(tmpfname);
		next_stop_menu = 1;
		break;
	}

	if (next_stop_menu) {
	    next_stop_menu = 0;

	    (void) fflush(fp);

	    cprintf("\1a\1f\1r<\1wS\1r>\1gave \1r<\1wA\1r>\1gbort \1r<\1wC\1r>\1gontinue \1w<\1rE\1w>\1gdit \1r<\1wI\1r>\1gnsert Clip \1r<\1wP\1r>\1grint \1w-> ");

	    cmd = get_single_quiet("ACDEINPS");

	    switch (cmd) {
		case 'A':
		    cprintf("\1f\1rAbort.\n");

		    if (how == 9) {
			cprintf("1\f\1rYou MUST write this %s!\n", config.message);
			break;
		    }
		    cprintf("\1f\1gAre you sure? \1w(\1gy\1w/\1gn\1w) \1c");
		    if (yesno() == NO)
			break;
		    (void) unlink(tmpfname);
		    (void) fclose(fp);
		    return 'a';

		case 'C':
		    cprintf("\1f\1gContinue.\n");
		    break;

		case 'D':
		    cprintf("\1f\1gContinue until \1w<\1rCTRL-D\1w)\n");
		    edithow = EDIT_CTRLD;
		    break;

		case 'E':
		    cprintf("\1f\1gContinue in Editor.\n");
		    edithow = EDIT_EDITOR;
		    break;

		case 'I':
		    cprintf("\1f\1gInsert ClipBoard.\n");
		    fp2 = xfopen(CLIPFILE, "r", FALSE);
		    if (fp2 != NULL) {
			while (b = getc(fp2), b > 0)
			    (void) putc(b, fp);

			fprintf(fp, "\1a\1c");
			(void) fclose(fp2);
		    }
		    break;

		case 'N':
		    cprintf("\1f\1gContinue.\n");
		    edithow = EDIT_NORMAL;
		    break;

		case 'P':
		    cprintf("\1f\1gPrint %s.\n", config.message);
		    curr_line = 1;
		    (void) fflush(fp);
		    more(tmpfname, 1);
		    break;

		case 'S':
		    cprintf("\1f\1gSave %s.\n", config.message);
		    *lines = 0;

		    (void) fseek(fp, 0, 0);
		    while (b = getc(fp), b > 0) {
			(void) putc(b, outputfp);
			if (b == 10 || b == 13)		/* newline */
			    *lines += 1;
		    }

		    if (*lines > 0) {
			(void) unlink(tmpfname);
			(void) fclose(fp);
			return ('s');
		    }
		    cprintf("\1f\1rI refuse to save an empty %s. Press \1w<\1rA\1w>\1r to abort.\n", config.message);
		    break;

		default:
                    log_it("errors", "went clueless in normal_input() due to '%c'", cmd);
		    cprintf("\1f\1gYou cannot press \1w<\1r%c\1w>\1g at this prompt.\n", cmd);
		    break;
	    }
	    cprintf("\1a\1g");
	} else
	    break;

    }

    return 0;
}

/*************************************************
* get_x_lines()
*************************************************/
int
get_x_lines(char *xstring, int X_PARAM)
{

    int a, i;
    char sstring[X_LENGTH][80], *p, override = ' ';

    for (i = 0; (i < X_LENGTH) && (i ? *sstring[i - 1] : TRUE); i++) {
	(void) putchar('>');
	get_string_wr(78, sstring[i], i, 0);

	if (strcmp(sstring[i], "ABORT") == 0) {
	    return ('A');
	}
	if ((strcmp(sstring[i], "PING") == 0) && (X_PARAM != -2))
	    return OR_PING;

	if ((strcmp(sstring[i], "GUARD") == 0) && (usersupp->priv >= PRIV_TECHNICIAN) && X_PARAM != -2) {
	    cprintf("\1f\1gMarked as %s%s\1g in the %s %s.\1a\n", PROGRAMMERCOL, config.programmer, config.express, config.x_message);
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_T_TECH;
	}
	if ((strcmp(sstring[i], "ADMIN") == 0) && (usersupp->priv >= PRIV_SYSOP) && X_PARAM != -2) {
	    cprintf("\1f\1gMarked as %s%s\1g in the %s %s.\1a\n", ADMINCOL, config.admin, config.express, config.x_message);
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_T_ADMIN;
	}
	if ((strcmp(sstring[i], "SYSGUIDE") == 0 || strcmp(sstring[i], "SYS") == 0) &&
	    (usersupp->flags & US_GUIDE) && X_PARAM != -2) {
	    cprintf("\1f\1gMarked as %s%s\1g in the %s %s.\1a\n", GUIDECOL, config.guide, config.express, config.x_message);
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_T_GUIDE;
	}
	if ((strcmp(sstring[i], "CTHULHU") == 0) && (X_PARAM == -2)) {
	    cprintf("\1wSending a \1rCTHULHU \1wBroadCast!\1a\1c\n");
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_SILENT;
	}
	if ((strcmp(sstring[i], "FISH") == 0) && (X_PARAM == -2)) {
	    cprintf("\1f\1gSquids! Squids!\1a\n");
	    cprintf("\1c");
	    i--;
	    override = OR_FISH;
	}
	if ((strcmp(sstring[i], "LLAMA") == 0) && (X_PARAM == -2)) {
	    cprintf("\1f\1gAnnouncing via the intercom.\1a\n");
	    cprintf("\1c");
	    i--;
	    override = OR_LLAMA;
	}
	if ((strcmp(sstring[i], "SHOUT") == 0) && (X_PARAM == -2)) {
	    cprintf("\1f\1gSHOUTING in general!\1a\n");
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_SHOUT;
	}
	if ((strcmp(sstring[i], "NOWHEREVOICE") == 0) && (X_PARAM == -2)) {
	    cprintf("\1f\1gSending a \1rHouse Spirit\1g broadcast.\1a\n");
	    cprintf("\1c");	/* make user input cyan */
	    i--;
	    override = OR_NOWHERE;
	}
	if ((strcmp(sstring[i], "DELETELINE") == 0) && (i > 0)) {
	    cprintf("\1f\1rOk; this and the previous line are deleted.\1c\n");
	    i -= 2;
	}
    }

    if (!*sstring[0]) {
	if (X_PARAM == -2)
	    return 'A';
	else
	    return OR_PING;
    }
    p = xstring;

    for (i = 0; (i < X_LENGTH) && *sstring[i]; i++) {
	*p++ = '>';

	for (a = 0; sstring[i][a]; a++)
	    *p++ = sstring[i][a];

	*p++ = '\n';
    }

    *p = 0;

    return override;

}

/*************************************************
* ColourChar() - by DWD. (spelt correctly ;)
* (Oh shure! complane bout mine spelling! ;) -KH)
* Modelled on color() by KH. Internal use only.
*************************************************/
void
ColourChar(char key)
{
    if (usersupp->flags & US_ANSI) {
#ifdef CLIENTSRC
        (void) save_colour(key);
#endif
	switch (key) {
	    case 'd':
		cprintf("[30m");
		break;		/* dark   textcolor       */
	    case 'r':
		cprintf("[31m");
		break;		/* red    textcolor       */
	    case 'g':
		cprintf("[32m");
		break;		/* green  textcolor       */
	    case 'y':
		cprintf("[33m");
		break;		/* yellow textcolor       */
	    case 'b':
		cprintf("[34m");
		break;		/* blue   textcolor       */
	    case 'p':
		cprintf("[35m");
		break;		/* purple textcolor       */
	    case 'c':
		cprintf("[36m");
		break;		/* cyan   textcolor       */
	    case 'w':
		cprintf("[37m");
		break;		/* white  textcolor       */
	    case 'D':
		cprintf("[40m");
		break;		/* dark   backgroundcolor */
	    case 'R':
		cprintf("[41m");
		break;		/* red    backgroundcolor */
	    case 'G':
		cprintf("[42m");
		break;		/* green  backgroundcolor */
	    case 'Y':
		cprintf("[43m");
		break;		/* yellow backgroundcolor */
	    case 'B':
		cprintf("[44m");
		break;		/* blue   backgroundcolor */
	    case 'P':
		cprintf("[45m");
		break;		/* purple backgroundcolor */
	    case 'C':
		cprintf("[46m");
		break;		/* cyan   backgroundcolor */
	    case 'W':
		cprintf("[47m");
		break;		/* white  backgroundcolor */
	    case 'a':
		cprintf("[0m");
		break;		/* RESET   attribute      */
	    case 'f':
		if ((usersupp->flags & US_NOBOLDCOLORS) == 0)
		    cprintf("[1m");
		break;		/* BOLD    attribute      */
	    case 'u':
		cprintf("[4m");
		break;		/* UNDERLINED attribute   */
	    case 'e':
		if ((usersupp->flags & US_NOFLASH) == 0)
		    cprintf("[5m");
		break;		/* FLASH   attribute      */
	    case 'i':
		cprintf("[7m");
		break;		/* INVERSE attribute      */
	    case 'h':
		cprintf("[8m");
		break;		/* HIDDEN  attribute      */
	    default:
		break;
	}
    }

    return;
}


/*************************************************
*
* editor_edit()
*
* b            contains the PID of the editor-
*              process.
* editor_exit  contains the exit-status that was
*	       made by the editor at exit.
*
* the editor needs to exit(1) when a file was
* made&saved, and exit(0) when it was aborted.
* exit(1) seems to give editor_exit a value of
* 256 (!) of all things - i haven't got the
* foggiest why, but that's how it is. BBSpico
* already has all of this in it, i only tell you
* this incase you hate pico and want something
* else. like vi. ;)
*
*************************************************/

int
editor_edit(const char *fname)
{
    int a, b, editor_exit;
    char aaa[51];

    nox = 1;

    a = fork();			/* split into two processes     */

    if (a == 0) {		/* daughterprocess, does the work */
	restore_term();
	sprintf(aaa, "-r%s", quickroom.name);	/* to see the
						 * roomname */
	cprintf("\1a\1f\1gTrying to start the PICO editor now: \1p%s\1g.\1a\n", fname);

	execl(EDITOR, EDITOR, aaa, fname, NULL);	/* PR: added NULL
							 * parameter ! */
	cprintf("execl FAILED\n");
	exit(0);		/* PR bugfix? Tell parent editing is
				 * aborted */
    } else if (a > 0)		/* motherprocess that waits     */
	do {
	    editor_exit = -999;
	    b = wait(&editor_exit);
#ifdef PR__DEBUG
	    cprintf("wait: PID: %d, status %x\n", b, editor_exit);
#endif
	}
	while (((b != a) && (b >= 0)) || ((editor_exit != 256) && (editor_exit != 0)));

    sttybbs(0);

    idletime = 0;
    mono_change_online(who_am_i(NULL), "", 3);

    return ((editor_exit == 256) ? 1 : (editor_exit == 0) ? 0 : -1);
    /* returns 1 at saved msg, 0 at aborted, -1 at problems */

}


/*************************************************
* inkey()
*************************************************/

int
inkey()
{

    int i = -5;
    static int j;

    /* kirth added the j variable to ignore all LF's that
     * are sent after a CR */

    (void) fflush(stdout);

    if ((nox == 0) && (xmsgp < XLIMIT))
	are_there_held_xs();	/* inkey() */

    if (idletime > 0)
	idletime = 0;

    do {
#ifdef DITWERKTOOKNIET
	ret = read(0, buf, 1);
	if (ret == -1) {
	    if (errno == EINTR) /* interrupted by signal */
		continue;
	    else
		logoff( ULOG_PROBLEM ); /* logoff */
	} else if (ret == 0) {
	    /* should return 0 when connection is closed */
            logoff( ULOG_DROP ); /* logoff */
	}
	i = buf[0];
#else
        i = fgetc(stdin);
        if(i == EOF) {
            if(errno == EINTR) { /* we got some kind of x */
                i = 0;
                continue;
            } else {
                logoff( ULOG_PROBLEM ); /* logoff! */
            }
        }
#endif 
    }
    while (i > DEL || i == 0 || (j == CR && i == LF));

    j = i;

    if (idletime >= 2)
	mono_change_online(who_am_i(NULL), "", 3);	/* remove idleflag from wholist */

    /* the LAG flag!! *evilgrin* */
    if (usersupp->flags & US_LAG)
	usleep(rand() % 500000);

    if (i == DEL)
	i = BS;

    return i;
}

int
flush_input()
{
    tcflush(0, TCIFLUSH);
    return 0;
}

/* eof */
