/*
 * wholist.c 
 * $Id$ 
 *
 * Have to make all pointers relative to the shm->first.
 * Because different process have other offsets
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>

#ifdef HAVE_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"

#include "telnet.h"

/*************************************************
* wholist()
*
* Level:
*
*	1 -> normal wholist
*     ( 2 -> ChatMode-wholist )
*	3 -> short wholist
*       4 -> Room-list
*	6 -> friends online list
*
* Buglet fix: $) Ishtar - 951224 - added ^Aa after the ---- line, some
* terminal programs don't change back to default color on a \n
*************************************************/

#define WHOLIST_LINE_LENGTH 200

char *
wholist(int level, const user_t * user)
{

    int tdif, idling, min, hour;
    int j = 0;
    int i;
    char col, *q, *p, line[WHOLIST_LINE_LENGTH];
    time_t t;
    struct tm *tp;
    btmp_t *r;

    if (!shm)
	return "";

    t = time(0);
    tp = localtime(&t);

    /* first malloc some memory! */
    p = xmalloc(WHOLIST_LINE_LENGTH * (shm->user_count + 5));
    strcpy(p, "");

    /* --------------------- print header ----------------------- */

    if (level != 6) {
	if (shm->user_count == 0) {
	    sprintf(p, "\1f\1gNo one is online.\1a\n");
	    return p;
	}
    }
    strcat(p, "\n");

    switch (level) {
	case 1:
	    q = p + strlen(p);
	    sprintf(q, "\1f\1g%2d Users             \1pFlags  \1gLocation          \1pTime \1gDoing\n\1w", shm->user_count);
	    strcat(p, "-------------------------------------------------------------------------------\1a\n");
	    break;
	case 4:
	    strcat(p, "\1f\1gUsername             \1pFlags   Time \1y In Quadrant \1w\n");
	    strcat(p, "---------------------------------------------------------------------\1a\n");
	    break;
	default:
	    break;
    }

    /* ------------------- print online users --------------------- */
    /* from here we start with all the users: */
    /* each time we sprintf() a 'line', and add that to the result */

    i = shm->first;
    while (i != -1) {
	r = &(shm->wholist[i]);
	strcpy(line, "");

	/* several useless variables */
	tdif = time(0) - r->logintime;
	tdif /= 60;
	min = tdif % 60;
	hour = tdif / 60;

	col = 'g';

	switch (level) {
	    case 1:
/*** Normal long wholist ***/

		sprintf(line, "\1f\1%c%-20s ", col, r->username);

		if (r->flags & B_REALIDLE)
		    strcat(line, "\1p[\1gidle\1p] ");

		else if (r->flags & B_AWAY)
		    strcat(line, "\1p[\1raway\1p] ");

		/* new flags */
		else {
		    q = line + strlen(line);
		    sprintf(q, "\1p[%s%s%s"
			    ,((r->flags & B_DONATOR) ? "\1g$" : " ")
			    ,((r->flags & B_XDISABLED) ? "\1g*" : ((r->flags & B_GUIDEFLAGGED) ? "\1r?" : " "))
			    ,((r->flags & B_POSTING) ? "\1y+" : " "));

		    q = line + strlen(line);
		    sprintf(q, "%s\1p] ", (r->flags & B_CLIENTUSER) ? "\1y-" : " ");
		}

		/* show location */
		q = line + strlen(line);
		sprintf(q, "\1g%-16s ", r->remote);

		/* show online time */
		q = line + strlen(line);
		sprintf(q, "\1f\1p%2d:%02d ", hour, min);

		/* show flying, or idle time */
		if (r->flags & B_REALIDLE) {
		    idling = time(0) - r->idletime;
		    idling /= 60;
		    q = line + strlen(line);
		    sprintf(q, "\1a\1gAbducted by aliens for \1y%2.2d:%2.2d\n"
			    ,idling / 60, idling % 60);
		} else {
		    q = line + strlen(line);
		    sprintf(q, "\1g\1f%s\1a\1D\n", r->doing);
		}
		break;

	    case 3:
/*** Short Wholist ***/
		j++;
		sprintf(line, "\1f\1%c%-20s   ", col, r->username);
		if ((j % 3) == 0)
		    strcat(line, "\n");
		break;

	    case 4:
/*** Room-Wholist ***/
		/* username */
		sprintf(line, "\1f\1%c%s%-20s ", col,
		      (user != NULL && user->flags & US_CLIENT) ? "\4" : "",
			r->username);

		/* new flags */
		q = line + strlen(line);
		sprintf(q, "\1p[%s%s%s%s\1p] "
			,((r->flags & B_DONATOR) ? "\1g$" : " ")
			,((r->flags & B_XDISABLED) ? "\1g*" : ((r->flags & B_GUIDEFLAGGED) ? "\1r?" : " "))
			,((r->flags & B_POSTING) ? "\1y+" : ((r->chat != 0) ? "\1y!" : " "))
			,((r->flags & B_CLIENTUSER) ? "\1y-" : " "));

		/* show online time */
		q = line + strlen(line);
		sprintf(q, "\1f\1p%2d:%02d ", hour, min);

		/* current room */
		q = line + strlen(line);
		sprintf(q, "\1f\1y %-36s\n", r->curr_room);

		break;
	}			/* switch */

	strcat(p, line);
	i = r->next;

    }				/* for */

    if ((j % 3) != 0 && (level == 3 || level == 6))
	strcat(p, "\n");

    return p;
}
