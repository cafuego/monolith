/* $Id$ */

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
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"
#include "setup.h"
#include "chat.h"
#include "friends.h"
#include "routines2.h"

#define extern
#include "wholist.h"
#undef extern


#undef NO_WHOLIST

/*************************************************
* wholist()
*
* Level:
*
*	1 -> normal wholist
*       2 -> Chat-wholist 
*	3 -> short wholist
*       4 -> Room-list

*************************************************/

#define WHOLIST_LINE_LENGTH 200

char *
wholist(int level, const user_t * user)
{

    int tdif, idling, min, hour;
    int j = 0;
    int i;
    char col[5], *q, *p, line[WHOLIST_LINE_LENGTH];
    char string[20];
    time_t t;
    struct tm *tp;
    btmp_t *r;

    if (!shm) {
	mono_errno = E_NOSHM;
	return "";
    }
    t = time(0);
    tp = localtime(&t);

    if (tp == NULL)
	return "";

    /* first malloc some memory! */
    p = (char *) xmalloc(WHOLIST_LINE_LENGTH * (shm->user_count + 5));
    strcpy(p, "");

    /* --------------------- print header ----------------------- */

    if (level == 3) {
	if (shm->user_count == 1) {
	    (void) sprintf(p, "\n\1g\1fYou are all alone online at \1y%02d:%02d\1g local time.\n", tp->tm_hour, tp->tm_min);
	} else if (shm->user_count == 0) {
	    (void) sprintf(p, "\1f\1gHey! What do you think you are doing here?\1a\n");
	    return p;
	} else if (shm->queue_count <= 0) {
	    (void) sprintf(p, "\n\1g\1fThere are \1y%u\1g %s online"
			   " at \1y%02d:%02d\1g local time.\n",
		  shm->user_count, config.user_pl, tp->tm_hour, tp->tm_min);
	} else {
	    (void) sprintf(p, "\n\1g\1fThere are \1y%ud\1g %s online"
	     " \1c( \1y%ud \1cqueued ) \1gat \1y%02d:%02d\1g local time.\n",
			   shm->user_count, config.user_pl, shm->queue_count, tp->tm_hour, tp->tm_min);
	}
    }
    strcat(p, "\n");

    (void) sprintf(string, "%u %s", shm->user_count, (shm->user_count == 1) ? config.user : config.user_pl);

    switch (level) {
	case 1:
	    (void) sprintf(line, "\1f\1g%-19s  \1pFlags  \1g%-17s \1pTime \1g%-28s\n\1w", string, config.location, config.doing);
	    strcat(p, line);
	    strcat(p, "-------------------------------------------------------------------------------\1a\n");
	    break;
	case 2:
	    (void) sprintf(line, "\1f\1g%-16s  \1p%s\n\1g", string, config.chatmode);
	    strcat(p, line);
	    strcat(p, "-------------------------------------------------------------------------------\1a\n");
	    break;
	case 4:
	    (void) sprintf(line, "\1f\1g%-20s \1pFlags   Time \1y In %s\n", string, config.forum);
	    strcat(p, line);
	    strcat(p, "---------------------------------------------------------------------\1a\n");
	    break;
	default:
	    break;
    }

    /* ------------------- print online users --------------------- */
    /* from here we start with all the users: */
    /* each time we sprintf() a 'line', and add that to the result */

    (void) mono_lock_shm(1);
    i = shm->first;
    while (i != -1) {
	int xing, chat, silc;

	r = &(shm->wholist[i]);
	strcpy(line, "");

	/* several useless variables */
	tdif = time(0) - r->logintime;
	tdif /= 60;
	min = tdif % 60;
	hour = tdif / 60;
	xing = (EQ(r->x_ing_to, username) || EQ(r->x_ing_to, "Everyone"));
	silc = (EQ(r->x_ing_to, "SILC") && (usersupp->priv >= PRIV_SYSOP));
	chat = (is_chat_subscribed(user->chat, r->x_ing_to));
	if ((user != NULL) && is_cached_friend(r->username))
	    (void) sprintf(col, "%s", FRIENDCOL);
	else
	    (void) sprintf(col, "%s", USERCOL);

	switch (level) {
	    case 1:
/*** Normal long wholist ***/

		(void) sprintf(line, "\1a\1f%s%s%-20s ", col,
		      (user != NULL && user->flags & US_CLIENT) ? "\4" : "",
			       r->username);

		if (r->flags & B_REALIDLE)
		    strcat(line, "\1p[\1gidle\1p] ");

		else if (r->flags & B_AWAY)
		    strcat(line, "\1p[\1raway\1p] ");

		else if (r->flags & B_LOCK)
		    strcat(line, "\1p[\1clock\1p] ");

		/* new flags */
		else {
		    q = line + strlen(line);
		    (void) sprintf(q, "\1p[%s%s"
				   ,((r->flags & B_DONATOR) ? "\1p$" : " ")
   ,((r->flags & B_XDISABLED) ? "\1g*" : ((r->flags & B_GUIDEFLAGGED) ? "\1r?" : " ")));
		    if (r->flags & B_POSTING)
			strcat(q, "\1y+");
		    else if (xing)
			strcat(q, "\1yx");
		    else if (chat)
			strcat(q, "\1pc");
		    else if (r->flags & B_INFOUPDATED)
			strcat(q, "\1gi");
		    else if (silc)
			strcat(q, "\1w.");
		    else
			strcat(q, " ");

		    q = line + strlen(line);
		    j = cached_name_to_x(r->username);

		    if ((user != NULL) && (j >= 0)) {
			(void) sprintf(q, "\1c%d\1p] ", j);
		    } else {
			(void) sprintf(q, "%s\1p] ", (r->flags & B_CLIENTUSER) ? "\1y-" : " ");
		    }
		}

		/* show location */
		q = line + strlen(line);

		if ((user != NULL) && (user->flags & US_IPWHOLIST))
		    (void) sprintf(q, "\1g%-16s ", r->ip_address);
		else
		    (void) sprintf(q, "\1g%-16s ", r->remote);

		/* show online time */
		q = line + strlen(line);
		(void) sprintf(q, "\1f\1p%2d:%02d ", hour, min);

		/* show flying, or idle time */
		if (r->flags & B_REALIDLE) {
		    idling = time(0) - r->idletime;
		    idling /= 60;
		    q = line + strlen(line);
		    (void) sprintf(q, "\1a\1g%s for \1y%2.2d:%2.2d\1a\n"
				   ,config.idle
				   ,idling / 60, idling % 60);
		} else {
		    q = line + strlen(line);
		    (void) sprintf(q, "\1g\1f%s\1D\1a\n", r->doing);
		}
		break;


	    case 2:
/*** Chat  Wholist ***/
		q = line;
		(void) sprintf(q, "\1f%s%-17s \1p", col, r->username);
		for (i = 0; i < 10; i++) {
		    if (r->chat & (1 << i)) {
			q = line + strlen(line);
			(void) sprintf(q, "%s ", shm->holodeck[i].name);
		    }
		}
		strcat(line, "\n");
		break;
	    case 3:
/*** Short Wholist ***/
		j++;
		(void) sprintf(line, "\1p[%s%s%s\1p] "	/* green $ is boring */
			       ,((r->flags & B_DONATOR) ? "\1p$" : " ")
			       ,((r->flags & B_XDISABLED) ? "\1g*" : ((r->flags & B_GUIDEFLAGGED) ? "\1r?" : " "))
			       ,((r->flags & B_POSTING) ? "\1y+" : ((xing) ? "\1yx" : ((chat) ? "\1pc" : " "))));
		q = line + strlen(line);
		(void) sprintf(q, "\1f%s%-18s ", col, r->username);
		if ((j % 3) == 0)
		    strcat(line, "\n");
		break;

	    case 4:
/*** Room-Wholist ***/
		/* username */
		(void) sprintf(line, "\1f%s%-20s ", col, r->username);

		/* new flags */
		q = line + strlen(line);
		(void) sprintf(q, "\1p[%s%s%s%s\1p] "
			       ,((r->flags & B_DONATOR) ? "\1p$" : " ")
			       ,((r->flags & B_XDISABLED) ? "\1g*" : ((r->flags & B_GUIDEFLAGGED) ? "\1r?" : " "))
			       ,((r->flags & B_POSTING) ? "\1y+" : " ")
			       ,((r->flags & B_CLIENTUSER) ? "\1y-" : " "));

		/* show online time */
		q = line + strlen(line);
		(void) sprintf(q, "\1f\1p%2d:%02d ", hour, min);

		/* current room */
		q = line + strlen(line);
		(void) sprintf(q, "\1f\1y %-36s\n", r->curr_room);
		break;
	}			/* switch */

	strcat(p, line);

	i = r->next;

    }				/* for */
    (void) mono_lock_shm(0);

    if ((j % 3) != 0 && (level == 3 || level == 6))
	strcat(p, "\n");

    return p;
}
