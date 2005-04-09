/* $Id$ */
/* new chatmodes */
/* the idea: DW-ish like talker `channels' */

/* You can tune into a channel, and receive the messages being broadcast
 * on there, or X to the channel */

/* Concept is pretty simple, basic problems: 
 * - channel management (can users create any c hannel?)
 * - how do we store the list of tuned in channels
 * - bitfields in a long integer?
 */

/* basically we need in this file a function to subscribe to the 
 * different channels. And something to setup the different channels
 * and lastly something to view which users are on which channel 
 * also , chat channels should be stored in the userfile */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

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
#include "chat.h"
#undef extern

#include "routines2.h"

int
is_chat_subscribed(int chat, const char *channel)
{
    int i, chan, sub;

    if (channel == NULL)
	return FALSE;

    for (chan = -1, i = 0; i <= 9; i++)
	if (EQ(shm->holodeck[i].name, channel)) {
	    chan = i;
	    break;
	}
    if (chan == -1)
	return FALSE;

    sub = chat & (1 << chan);

    return sub ? TRUE : FALSE;
}

void
chat_subscribe()
{
    int i, c;
    int oldchat;
    char string[50];

    cprintf("\n");

    oldchat = usersupp->chat;

    do {

	cprintf("\n\1f");
	for (i = 0; i <= 9; i++) {
	    if (strlen(shm->holodeck[i].name)) {
		cprintf("\1w[\1r%c\1w] \1g%d. \1p%s\n"
			,(usersupp->chat & (1 << i)) ? '*' : ' ', i + 1
			,shm->holodeck[i].name);
	    }
	}

	cprintf(_("\n\1gSelect a number, <n> for none, or <space> to quit:\1c "));

	c = get_single("123456789nq \r\n");
	if (c >= '1' && c <= '9')
	    usersupp->chat ^= 1 << (c - '0' - 1);
	if (c == 'n')
	    usersupp->chat = 0;

    } while (c != 'q' && c != ' ' && c != '\r' && c != '\n');

    /* if it has been modified, change online status */
    if (usersupp->chat != oldchat) {
	sprintf(string, "%ud", usersupp->chat);
	mono_change_online(usersupp->username, string, 11);
    }
    return;
}
