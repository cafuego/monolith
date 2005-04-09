/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"

#include "btmp.h"
#include "express.h"
#include "registration.h"
#include "dexi2.h"

void log_off(int sig);
extern char *wholist(int level, const user_t * user);

extern char rbbs[20];

/*************************************************
* print_user_stats()
*************************************************/

void
print_user_stats(user_t * user)
{

    unsigned int a;
    int control = 0, found_RA_rooms = 0;
    time_t timecall, curtime;
    char *p, *profile;
    btmp_t *bp;

    time(&curtime);
    timecall = ((curtime - user->laston_from) / 60);

    control = 0;

    printf("\n\1y\1f%s\1g ", user->username);

    if (strcmp(user->username, "Guest") == 0) {
	printf("\nThe Guest User.\n\1");
	return;
    }
    if (user->priv & PRIV_WIZARD)
	printf(EXTWIZARDTITLE);
    if (user->priv & PRIV_SYSOP)
	printf(EXTSYSOPTITLE);
    if (user->priv & PRIV_TECHNICIAN)
	printf(EXTPROGRAMMERTITLE);
    if (user->flags & US_ROOMAIDE)
	printf(EXTROOMAIDETITLE);
    if (user->flags & US_GUIDE)
	printf(EXTGUIDETITLE);
    if (!(user->priv & PRIV_VALIDATED))
	printf("* Unvalidated * ");
    if (user->priv & PRIV_DEGRADED)
	printf("\1wDegraded ");
    if (user->priv & PRIV_TWIT)
	printf("\1w*CURSED* ");
    if (user->priv & PRIV_DELETED)
	printf("\1w[Deleted] ");
    if (user->xtrapflag)
	printf("\1f\1y%s", user->xtrapflag);
    if (user->flags & US_COOL)
	printf("\1w*COOL AS ICE* ");
    if (user->flags & US_LAG)
	printf("\1y*LAG* ");
    if (user->flags & US_DONATOR)
	printf("\1b\1f$ DONATOR $ ");

    printf("\1a\1g\1D\n");

    p = read_regis(user, 0);
    printf(p);
    xfree(p);

    printf("\1f\1cFlying: %s\1a   ", user->doing);

    if (user->flags & US_ROOMAIDE)
	for (a = 0; a < 5; a++)
	    if (user->RA_rooms[a] >= 0) {
		if (found_RA_rooms == 0) {
		    printf("\n\1c\1fQuadrant Leader in: \1g");
		    found_RA_rooms = 1;
		}
		printf("%d ", user->RA_rooms[a]);
	    }
    printf("\1a\1f\1c\n");

    /* IS ONLINE */
    if (mono_return_pid(user->username) != -1) {
	printf("\1rONLINE \1cfor \1y%ld:%2.2ld \1c", timecall / 60, timecall % 60);
	if (!(user->flags & US_HIDDENHOST))
	    printf("from \1r%s %s\1c"
	    ,user->lasthost, user->flags & US_HIDDENHOST ? "(hidden)" : "");
	printf("\n");
    } else
	/* IS NOT ONLINE */
    {
	a = timecall / 60 / 24;
	printf("Last on \1g%s \1cto \1g%s \1p( \1y%d\1p day%s ago )\1c",
	       printdate(user->laston_from, 1),
	       printdate(user->laston_to, 2), a, (a == 1) ? "" : "s");
	if (!(user->flags & US_HIDDENHOST))
	    printf(" from \1r%s %s\1c"
	    ,user->lasthost, user->flags & US_HIDDENHOST ? "(hidden)" : "");
	printf("\n");
    }

    printf("\n");

    if (user->flags & US_GUIDE) {
	bp = mono_read_btmp(user->username);
	if (bp && bp->flags & B_GUIDEFLAGGED)
	    printf("\1g*** is available to help others ***\n");
	xfree(bp);
    }
    /* --- PROFILE --- */
    profile = read_profile(user->username);
    printf("%s\n", read_profile(user->username));
    free(profile);
    return;
}

/* wholist */
void
show_online(int level)
{
    char *p;
    p = wholist(level, NULL);
    printf("%s", p);
    free(p);
    return;
}

void
intersendx(const char *to, const char *sender, char send_string[], char override)
{

    express_t *Sendxs;
    btmp_t *tuser = NULL;
    int timeout;

/* -------------------- decide who to send it to ------------------ */

    if (!check_user(to)) {
	putchar('X');		/* no such user */
	log_off(0);
    }
    tuser = mono_read_btmp(to);
    if (tuser == NULL) {
	putchar('Y');
	log_off(0);
    }
    Sendxs = mono_find_xslot(tuser->username);
    for (timeout = 0; timeout < 20; timeout++) {
	usleep(100000);
	if (Sendxs->override == OR_FREE)
	    break;
    }
    /* * ----------------- make the sendx structure ------------------ */
    if (override == OR_PING)
	Sendxs->override = override;
    else
	Sendxs->override = OR_INTER;
    Sendxs->ack = NO_ACK;
    strcpy(Sendxs->recipient, to);
    Sendxs->sender_priv = 0;
    strcpy(Sendxs->sender, sender);

    strcat(Sendxs->sender, "@");
    strcat(Sendxs->sender, rbbs);

    strcpy(Sendxs->message, send_string);
    time(&Sendxs->time);

    kill(tuser->pid, SIGIO);

    for (timeout = 0; timeout < 20; timeout++) {
	usleep(100000);
	if (Sendxs->ack != NO_ACK) {
	    if (Sendxs->ack == ACK_AWAY)
		putchar('K');
	    if (Sendxs->ack == ACK_NOTBUSY)
		putchar(ACK_RECEIVED);
	    else
		putchar(Sendxs->ack);
	    break;
	}
    }
    if (Sendxs->ack == NO_ACK) {
	if (mono_return_pid(to) == -1)
	    putchar('N');
	else
	    putchar('U');
    }
    return;

}

/* eof */
