/* $Id$ */
/*
 * That statusbar was given to us by Jassyca BBS and Janni just asked
 * me if I would put acknowledgements in to them, and so I'm doing that
 * now. THANK YOU JASSYCA FOR THE STATUSBAR!!! WILL YOU MARRY ME???
 * <heh> We like it though.. thanx. :)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include "monolith.h"
#include "ext.h"
#include "setup.h"

#define extern
#include "statusbar.h"
#undef extern


#include "btmp.h"
#include "chat.h"
#include "routines.h"

int bartmpoff;

void
statusbar(char *outstr)
{
    char message[100];
    time_t t;
    struct tm *tp;

    if (!(usersupp->flags & US_ANSI))
	return;
    if (!(usersupp->flags & US_STATUSBAR))
	return;

#ifdef CLIENTSRC
    if (bartmpoff == 1)
	return;
#endif /* CLIENTSRC */

    if (outstr == NULL) {
	sprintf(message, "\1b%s\1d", config.bbsname);
    } else {
	strcpy(message, outstr);
    }
    t = time(NULL);
    tp = localtime(&t);

    SAVE_CURSOR;

    cprintf("[1;1f\1W\1d\n[1;1f");
    cprintf("\1a\1f\1W\1d");
    cprintf("  %-42s Currently online: \1b%3d \1dUser%c. \1b%02d\1e:\1a\1f\1W\1b%02d \1a",
	    message, shm->user_count, ((shm->user_count == 1) ? ' ' : 's'), tp->tm_hour, tp->tm_min);

    RESTORE_CURSOR;

    fflush(stdout);
    return;
}


void
status_bar_on()
{
    /* start the status bar, if wanted */
    if (usersupp->flags & US_STATUSBAR) {
	cprintf("7");		/* save cursor position */
	cprintf("[2;%dr", usersupp->screenlength);
	cprintf("8");		/* restore cursor and color */
	statusbar(NULL);
    }
    bartmpoff = 0;
}

void
status_bar_off()
{
    if (usersupp->flags & US_STATUSBAR) {
	cprintf("7");
	cprintf("[1;%dr", usersupp->screenlength);
	cprintf("8");
#ifdef CLIENTSRC
	bartmpoff = 1;
#endif /* CLIENTSRC */
    }
    return;
}

void
holodeck_statusbar_on()
{
/*
 * if (usersupp->flags & US_STATUSBAR) {
 * SAVE_CURSOR;
 * cprintf("[%d;1f\1W\1b%d;1f", usersupp->screenlength - 1, usersupp->screenlength - 1 );
 * cprintf("\1f\1W \1b%-20s                                  ", config.bbsname );
 * RESTORE_CURSOR;
 * }
 */
    return;
}

void
holodeck_statusbar_off()
{
    if (usersupp->flags & US_STATUSBAR) {
	SAVE_CURSOR;
	cprintf("[1;%dr", usersupp->screenlength);
	RESTORE_CURSOR;
    }
    return;
}

void
switch_window(int how)
{

    char message[100];
    time_t t;
    struct tm *tp;

    t = time(NULL);
    tp = localtime(&t);

    if (usersupp->flags & US_STATUSBAR) {

	switch (how) {
	    case UPPER:
		SAVE_CURSOR;
		cprintf("[%d;1f", usersupp->screenlength - 3);
		RESTORE_CURSOR;
		break;
	    case LOWER:
		cprintf("[%d;1f", usersupp->screenlength);
		fflush(stdout);
		break;
	    case STATUS:
		SAVE_CURSOR;
		sprintf(message, "\1b%s \1r[\1d%s\1r]", config.bbsname, config.chatmode);
		cprintf("[%d;1f\1W\1d\n", usersupp->screenlength - 2);
		cprintf("\1a\1f\1W\1d");
		cprintf("  %-42s Currently online: \1b%3d \1dUser%c. \1b%02d\1e:\1a\1f\1W\1b%02d \1a",
			message, shm->user_count, ((shm->user_count == 1) ? ' ' : 's'), tp->tm_hour, tp->tm_min);
		RESTORE_CURSOR;
		break;
	    default:
		break;
	}
    }
    return;
}
