#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <time.h>
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

#include "commands.h"
#include "input.h"
#include "routines2.h"
#include "chat.h"
#include "clipboard.h"
#include "friends.h"
#include "display_message.h"
#include "express.h"
#include "inter.h"
#include "main.h"
#include "qc.h"
#include "rooms.h"
#include "setup.h"
#include "statusbar.h"
#include "enter_message.h"
#include "help.h"
#include "fun.h"
#include "messages.h"
#include "usertools.h"
#include "uadmin.h"

#define extern
#include "read_menu.h"
#undef extern

/* ---------------------------------------------------- */

static long get_read_start_number(const long, const int);
static void long_prompt(long, int);
static long numeric_read(const long);
static int set_read_bounds(long *, long *);
static int validate_read_command(int);
static unsigned int ungoto_message_id(const unsigned int);
static unsigned int ungoto_forum_id(const unsigned int);
static int no_new_posts_here(const long, const int, const long);
static void ungoto(void);

int not_my_post = TRUE;

/* ---------------------------------------------------- */

void
new_message_system(void)
{


#ifdef CONVERT_STUFF_YET_AGAIN_ARGH_COMMA_ARGH
//    int j;
    //    for (j = 0; j < MAXQUADS; j++) {
    //      cprintf("\nquad %d\n", j);
    //      convert_message_base(j);
    //   }
    //   logoff(0);

#else

    short_prompt();

#endif

}

void
short_prompt(void)
{
    char tempstr[40];
    int t;
    struct tm *tp = NULL;
    time_t now;
    int cmd = 0;

    for (;;) {

	are_there_held_xs();	/* HERE! in main menu, mark as not busy */
	display_short_prompt();

#ifdef SUPERHERO
	cmd = get_single_quiet("~aAbBcCdefEFGHiIjJkKlLMNOPQqrRsSTUvVwWxXYZ0123456789!<>-_+:#.,*\"@`$^&a([]% /?\005\006\011\014\016\022\030\'");
#else
	cmd = get_single_quiet("~aAbBcCdefEFGHiIjJkKlLMNOPQqrRsSTUvVwWxXYZ0123456789!<>-_+:#.,*\"@$^&a([]% /?\005\006\011\014\016\022\030\'");
#endif

	cmd = validate_read_command(cmd);	/* priv check */

	if (mono_return_pid(who_am_i(NULL)) == -1) {
	    cprintf("\1f\1rAYIEE!!! \1gHmmm... you're a ghost user at the moment.\n");
	    cprintf("\1f\1gFixing that right now... ");
	    mono_add_loggedin(usersupp);
	    cprintf("\1f\1gokay, there ya go!\n");
	}
	time(&now);
	tp = localtime(&now);

	switch (cmd) {

	    case 0:
	    case -1:
		break;

	    case 'a':
		if (usersupp->priv & (PRIV_WIZARD | PRIV_SYSOP)) {
		    cprintf("\1f\1wAdmin cmd: \1a");
		    sysop_menu();
		} else if (is_ql(who_am_i(NULL), quickroom)) {
		    cprintf(ROOMAIDETITLE " cmd: \1a");
		    roomaide_menu();
		} else {
		    cprintf("\1f\1rI'm sorry Dave, but I can't do that.\1a");
		}
		break;

	    case 'A':
		toggle_away();
		break;

	    case '~':
	    case ':':
		cprintf("\1f\1gEmote.\1a\n");
		express(2);
		break;

	    case 'b':
		cprintf("\1f\1gRead messages backwards.\1a\n");
		long_prompt(0, -1);
		break;

	    case 'B':
		toggle_beeps();
		break;

	    case 'c':
		nox = 1;
		cprintf("\1gPress \1w<\1rshift-c\1w>\1g to access the config menu.\n");
		express(3);
		break;

	    case 'C':
		cprintf("\1f\1gConfig: \1a");
		config_menu();
		writeuser(usersupp, 0);
		break;

	    case 'd':
		cprintf("\1f\1pSubscribe to %s: \1a", config.chatmode);
		chat_subscribe();
		break;

	    case 'e':
		cprintf("\1f\1gEnter message.\1a\n");
		enter_message(curr_rm, EDIT_NORMAL, NO_BANNER, NULL);
		break;

	    case 'E':
		cprintf("\1f\1gEnter Editor-message.\1a\n");
		status_bar_off();
		enter_message(curr_rm, EDIT_EDITOR, NO_BANNER, NULL);
		status_bar_on();
		break;

	    case 005:		/* <ctrl-e> */
		nox = 1;
		enter_admin_message();
		break;

	    case 'f':
		cprintf("\1f\1gRead messages forward.\1a\n");
		long_prompt(0, 1);
		break;

	    case 'F':
		cprintf("\1f\1gYour friends online.\1a\n");
		friends_online();
		break;

	    case 006:
#ifndef FRIENDS_CACHE_DEBUG
		cprintf("\1f\1gYour friends online.\1a\n");
		friends_online();
#else
		{
		    char *cachedump;
		    cachedump = show_user_cache();
		    more_string(cachedump);
		    xfree(cachedump);
		}
#endif
		break;

	    case 'G':
		cmdflags &= ~C_ROOMLOCK;
		skipping[curr_rm] = 0;
		ungoto_message_id(usersupp->lastseen[curr_rm]);
		ungoto_forum_id(curr_rm);
		mark_as_read(curr_rm);
		writeuser(usersupp, 0);
		gotonext();
		break;

	    case 'H':
		nox = TRUE;
		cprintf("\1f\1gHelpfiles.\1a\n");
		help_topics();
		break;

	    case 'i':
		cprintf("\1f\1g%s Info.\1a\n", config.forum);
		display_message(curr_rm, 0, DISPLAY_INFO);
		break;

	    case 'I':
		toggle_interbbs();
		break;

	    case 'J':
		cprintf("\1f\1rJump:\nNon Destructive Jump to %s name/number: \1a", config.forum);
		fflush(stdout);
		skipping[curr_rm] = 0;
		jump(0);
		break;

	    case 'j':
		cprintf("\1f\1gJump to %s name/number: \1a", config.forum);
		fflush(stdout);
		ungoto_message_id(usersupp->lastseen[curr_rm]);
		ungoto_forum_id(curr_rm);
		skipping[curr_rm] = 0;
		jump(1);
		break;

	    case 'K':
		cprintf("\1f\1gAll %s\1w:\n", config.forum_pl);
		show_known_rooms(0);
		break;

	    case 'k':
		cprintf("\1f\1gKnown %s.\n", config.forum_pl);
		show_known_rooms(1);
		break;

	    case 12:		/* <ctrl-l> */
	    case 18:		/* <ctrl-r> */
		cprintf("c");
		fflush(stdout);
		break;

	    case 'L':
		cprintf("\1f\1g%s with unread %s.\n", config.forum_pl, config.message_pl);
		show_known_rooms(2);
		break;

	    case 'l':
		cprintf("\1f\1gLogout.\1a");
		if (user_terminate() == TRUE)
		    return;
		break;

	    case 'M':
		cprintf("\1f\1gMisc: \1a");
		fflush(stdout);
		nox = 1;
		misc_menu();
		continue;

	    case 'N':		/* second parm of no_new_posts_here is direction, which is   */
	    case 32:		/* understood to be forward in main menu, although undefined */
		if (no_new_posts_here(readquad(curr_rm).highest, 1, usersupp->lastseen[curr_rm])) {
		    if (cmdflags & C_ROOMLOCK)
			random_goto();
		    else {
			gotonext();
		    }
		}
		long_prompt(0, 1);
		break;


	    case '\016':	/* umm this is ctrl-n, not ctrl-d */
		cprintf("\1f\1rCtrl\1w-\1rN\1w-\1gEnter %s.\1a\n", config.message);
		enter_message(curr_rm, EDIT_CTRLD, NO_BANNER, NULL);
		break;

	    case 'O':
		cprintf("\1f\1gRead old %s.\1a\n", config.message_pl);
		long_prompt(0, -1);
		break;

	    case 'P':
		nox = 1;
		cprintf("\1f\1gProfile an %s.\1a\n", config.user);
		profile_user();
		break;

	    case 'Q':
		nox = 1;
		cprintf("\1f\1rAsk a Question.\1a\n");
		express(1);
		break;

	    case 'q':
		nox = 1;
		q_menu();
		break;

	    case 'r':
		cprintf("\1f\1gRead %s reverse.\1a\n", config.message_pl);
		long_prompt(0, -1);
		break;

	    case 'R':
		cprintf("\1f\1gInterBBS Wholist.\1a\n");
		nox = 1;
		menu_inter();
		break;

	    case 'S':
		nox = 1;
		cprintf("\1f\1gSearch %s\n", config.message_pl);
		search_via_sql(curr_rm);
		break;

	    case 's':
		cprintf("\1f\1gSkipping this %s.\n\n\1a", config.forum);
		cmdflags &= ~C_ROOMLOCK;
		skiproom();
		break;

	    case 'T':
		cprintf("\1f\1gDate: \1w%s\1a", printdate(time(0), 0));
		break;

	    case 'U':
		cprintf("\1f\1gUngoto. \1w(\1gbuggy\1w)\1a");
		fflush(stdout);
		ungoto();
		break;

	    case 'v':
		nox = 1;
		express(-1);
		break;

	    case 'V':
		cprintf("\1f\1gVote.\n");
		voting_booth();
		break;

	    case ',':		/* Holodeck Wholist */
		cprintf("\1f\1g%s Wholist.\n", config.chatmode);
		show_online(2);
		break;

	    case 'w':
		cprintf(_("\1f\1gWhich %s are online?\1a\n"), config.user_pl);
		show_online(1);
		break;

	    case 'W':
		cprintf("\1f\1gShort Wholist.\1a\n");
		show_online(3);
		break;

            case '^':		/* <ctrl-w> */
	        nox = 1;
	        cprintf("\1f\1gSend \1pWeb \1g%s %s.\1a\n", config.express, config.x_message);
                express(-4);
                break;
                
	    case '!':
		nox = 1;
		feeling();	/* the feeling menu */
		break;

	    case 'x':
		nox = 1;
		cprintf("\1f\1gSend %s %s.\1a\n", config.express, config.x_message);
		express(0);
		if (((int) rand() % 500000) == 42) {
		    cprintf("\1f\1b\n*** \1g%s %s from \1yThe House Spirit \1gto \1y%s \1gat \1w(\1g%02d:%02d\1w) \1b***\1a\n", config.express, config.x_message, usersupp->username, tp->tm_hour, tp->tm_min);
		    cprintf("\1a\1c>*chomp* *chomp*\n>Your %s %s tasted great!\n", config.express, config.x_message);
		} else if (((int) rand() % 500000) == 42) {
		    cprintf("\1f\1b\n*** \1g%s %s from \1yCthulhu \1gto \1y%s \1gat \1w(\1g%02d:%02d\1w) \1b***\1a\n", config.express, config.x_message, usersupp->username, tp->tm_hour, tp->tm_min);
		    cprintf("\1a\1c>Feed me!\1a\n");
		}
		break;

	    case 'X':
		change_express(0);
		break;

	    case 030:		/* <ctrl-x> */
		cprintf("\1f\1gRead %s-Log.\1a\n", config.express);
		old_express();
		break;

	    case 'Y':
		cprintf("\1f\1gYell-menu.\1a \n");
		nox = TRUE;
		yell_menu();
		break;

	    case 'Z':
		cprintf("\1f\1gZap %s.\1a\n", config.forum);
		forget();
		break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		nox = 1;
		express(cmd - 38);
		break;

	    case '-':
		cprintf("\1f\1gJump a number of %s back from the Last Read %s.\1a\n", config.message_pl, config.message);
		cprintf("\1f\1gHow many %s back shall I jump?\1c ", config.message_pl);
		fflush(stdout);
		getline(tempstr, 4, 1);
		t = atoi(tempstr);
		if (t <= 0)
		    cprintf("\1f\1rThat is not a valid entry.\1a\n");
		else
		    long_prompt(-t, 1);
		break;

	    case '_':
		cprintf("\1f\1rJump a number of %s back from the Last %s in this %s.\1a\n", config.message_pl, config.message, config.forum);
		cprintf("\1f\1gHow many %s back shall I jump?\1c ", config.message_pl);
		fflush(stdout);
		getline(tempstr, 4, 1);
		t = atoi(tempstr);
		if (t <= 0)
		    cprintf("\1f\1rThat is not a valid entry.\1a\n");
		else {
		    leave_n_unread_posts(curr_rm, t);
		    long_prompt(0, 1);
		}
		break;


	    case '+':
		nox = 1;
		roll_the_bones();
		break;

	    case '.':
		if (usersupp->priv & (PRIV_WIZARD | PRIV_SYSOP))
		    send_silc();
		break;

	    case '*':
		nox = 1;
		clip_board();
		break;

	    case '@':
		IFNSYSOP break;
		cprintf("\n");
		more(ADMINLIST, 1);
		break;

	    case '&':
		cprintf("\1f\1gRandom Quote!\n");
		sprintf(tempstr, "%s/quote%d", QUOTEDIR, ((int) random() % 60) + 1);
		more(tempstr, 1);
		break;

	    case '$':
		cprintf("\1a\1f\1gDonator List.\1a\n");
		more(DONATORLIST, 1);
		cprintf("\n");
		break;

#ifdef SUPERHERO
	    case '`':
		cprintf("\1a\1f\1gGenerate Superhero.\1a\1n");
		crap(3, "", 0);
		cprintf("\n");
		break;
#endif

	    case '\'':
		nox = 1;
		literature_menu();
		break;

	    case '(':
#ifdef QC_ENABLE
		nox = TRUE;
		qc_user_menu(0);
#endif
		break;

	    case ']':
		fflush(stdout);
		crap(1, "zippy", 0);
		break;

	    case '"':
		cprintf("\1f\1rQuote %s %s.\1a\n", config.express, config.x_message_pl);
		quoted_Xmsgs();
		break;

	    case '%':
		change_atho(0);
		break;

	    case '<':
		cprintf("\1f\1gChange your Friends-list.\1a\n\n");
		menu_friend(FRIEND);
		break;

	    case '>':
		cprintf("\1f\1gChange your Enemylist.\1a\n\n");
		menu_friend(ENEMY);
		break;

	    case '?':
		online_help('s');
		break;

	    case '/':
		cprintf("\1f\1gList all commands.\1a\n");
		more(MENUDIR "/menu_commands", 1);
		break;

	    case '#':
		if ((t = numeric_read(-1)) > 0)		/* use -1 as there's no current */
		    long_prompt(t, 1);	/* post in main menu */
		break;
	}
    }				/* for (;;) */
}


void
long_prompt(long number, int direction)
{
    int read_command, read_position_modified;
    long start, current, temp_lowest, temp_highest;

    if ((start = get_read_start_number(number, direction)) == -6666)
	return;			/* found no new posts */
    if (start < 0)
	start = 0;		/* ugly fix for users getting  bigboote-d */

    set_read_bounds(&temp_lowest, &temp_highest);

/***********  Main Read Loop  ***********/

    for (current = start;
	 (current >= temp_lowest) && (current <= (temp_highest));
	 current += direction) {

	/* see if new posts while we were reading, x-ing, flaming, etc.. */
	if (temp_highest >= current - 3)	/* but only worry about it at last posts */
	    set_read_bounds(&temp_lowest, &temp_highest);

	read_command = display_message(curr_rm, current, DISPLAY_NORMAL);

	if (current > usersupp->lastseen[curr_rm])
	    usersupp->lastseen[curr_rm] = current;	/* update lastseen */
/*
            mono_sql_ut_update_lastseen( usersupp->usernum, curr_rm, 0, current );
*/

	if (read_command == 0)
	    continue;

	read_position_modified = FALSE;

/***********  Main Read Command Loop  ***********/

	while (!read_position_modified) {

	    show_long_prompt(curr_rm, current, direction);
	    are_there_held_xs();	/* check if there are x-es HERE!! */

	    if (usersupp->flags & US_NOPROMPT)	/* dump quad to screen */
		break;

	    not_my_post = strcmp(usersupp->username, message_reply_name(NULL));

	    read_command = get_single_quiet("1234567890 aAbBcCdDeEfFgGhHiIjJlkKLpPrRqQmMnNsStTvVWwxXyYzZ?!#\006\005\014\018\022\030<>%\":~,.*");

	    read_command = validate_read_command(read_command);		/* priv check */

	    switch (read_command) {

		case -1:	/* we're an asshole, guest, newbie, etc..  and we */
		    break;	/* can't use this command */

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		    nox = 1;
		    express(read_command - 38);
		    break;

		case 'a':
		    cprintf("\1f\1gAgain.\1a\n");
		    current -= direction;
		    read_position_modified = TRUE;
		    break;

		case 'A':
		    toggle_away();
		    cprintf("\n");
		    break;

		case 'b':
		    cprintf("\1f\1gBackwards\1a\n");
		    direction = -1;
		    read_position_modified = TRUE;
		    break;

		case 'B':
		    cprintf("\1f\1gBackwards from the end\1a\n");
		    current = temp_highest + 1;
		    direction = -1;
		    read_position_modified = TRUE;
		    break;

		case 'C':
		    cprintf("\1f\1gCopy this %s.\n", config.message);
		    copy_message_wrapper(current, not_my_post, TRUE);
		    break;

		case 'c':
		    nox = 1;
		    cprintf("\1gPress \1w<\1rshift-c\1w>\1g to access the config menu.\n");
		    express(3);
		    break;

		case '*':
		    cprintf("\1f\1gClip %s.\n", config.message);
		    display_message(curr_rm, current, DISPLAY_2_CLIP);
		    break;

		case 'd':
		    cprintf("\1f\1pChannel Config: \1a");
		    chat_subscribe();
		    break;

		case 'D':
		    delete_message_wrapper(current, not_my_post);
		    read_position_modified = TRUE;
		    break;

		case 'e':
		    cprintf("\1f\1gEnter %s.\1a\n", config.message);
		    direction = 1;
		    enter_message(curr_rm, EDIT_NORMAL, NO_BANNER, NULL);
		    break;

		case 'E':
		    cprintf("\1f\1gEnter Editor-%s.\1a\n", config.message);
		    direction = 1;
		    status_bar_off();
		    enter_message(curr_rm, EDIT_EDITOR, NO_BANNER, NULL);
		    status_bar_on();
		    break;

		case 005:
		    nox = 1;
		    enter_admin_message();
		    break;

		case 'f':
		case 'F':
		    cprintf("\1f\1gForward.\1a\n");
		    direction = 1;
		    read_position_modified = TRUE;
		    break;

		case '':
		    cprintf("\1f\1gFriends Online.\1a\n");
		    friends_online();
		    break;

		case 'g':
		case 'G':
		    cprintf("\1f\1gGoto.\1a\n");
		    cmdflags &= ~C_ROOMLOCK;
		    ungoto_message_id(current);
		    ungoto_forum_id(curr_rm);
		    mark_as_read(curr_rm);
		    gotonext();
		    return;

		case 'h':
		case 'H':
		    nox = TRUE;
		    cprintf("\1f\1gHelpfiles.\1a\n\n");
		    help_topics();
		    break;

		case 'i':
		    cprintf("\1f\1g%s Info.\1a\n", config.forum);
		    display_message(curr_rm, 0, DISPLAY_INFO);
		    break;

		case 'I':
		    toggle_interbbs();
		    cprintf("\n");
		    break;

		case 'J':
		    cprintf("\1f\1rJump:\nNon Destructive Jump to %s name/number\1w: \1c", config.forum);
		    fflush(stdout);
		    if (!jump(0))
			break;
		    return;

		case 'j':
		    cprintf("\1f\1gJump to %s name/number\1w: \1c", config.forum);
		    ungoto_message_id(current);
		    ungoto_forum_id(curr_rm);
		    fflush(stdout);
		    if (!jump(1))
			break;
		    return;

		case 'k':
		    cprintf("\1f\1gKnown %s\1w:\n\n", config.forum_pl);
		    show_known_rooms(1);
		    break;

		case 'K':
		    cprintf("\1f\1gAll %s\1w:\n\n", config.forum_pl);
		    show_known_rooms(0);
		    break;

		case 'L':
		    cprintf("\1f\1g%s with unread %s.\n", config.forum_pl, config.message_pl);
		    show_known_rooms(2);
		    break;

		case 'l':
		    cprintf("\1f\1gLogout.\1a");
		    if (user_terminate() == TRUE)
			logoff(0);
		    break;

		case 'm':
		    cprintf("\1f\1gMisc: \1a");
		    nox = 1;
		    misc_menu();
		    break;

		case 'M':
		    copy_message_wrapper(current, not_my_post, FALSE);
		    read_position_modified = TRUE;
		    break;

		case 'n':
		case 'N':
		case ' ':
		    cprintf("\1f\1gNext.\1a\n");
		    if (no_new_posts_here(temp_highest, direction, current)) {
			return;
		    }
		    read_position_modified = TRUE;
		    break;

		case '\016':	/* this is ctrl-n  */
		    direction = 1;
		    nox = 1;
		    cprintf("\1f\1rCtrl\1w-\1rN\1w-\1gEnter message.\1a\n");
		    enter_message(curr_rm, EDIT_CTRLD, NO_BANNER, NULL);
		    break;

		case 'p':
		    cprintf("\n");
		    profile_user();
		    cprintf("\n");
		    break;

		case 'P':
		    lookup_anon_author(current);
		    break;

		case 'q':
		    nox = 1;
		    q_menu();
		    break;

		case 'Q':
		    nox = 1;
		    cprintf("\1f\1rAsk a Question.\1a\n");
		    express(1);
		    break;

		case 'r':
		    direction = 1;
		    cprintf("\1f\1gReply.\1a\n");
		    enter_message(curr_rm, REPLY_MODE, NO_BANNER, NULL);
		    break;

		case 'R':
		    cprintf("\1f\1gRate %s.\1a\n", config.message);
		    rate_message(NULL, current, curr_rm);
		    break;

		case 'S':
		    nox = 1;
		    cprintf("\1f\1wSearch %s\n", config.message_pl);
		    search_via_sql(curr_rm);
		    break;

		case 's':
		    cprintf("\1f\1gStop.\1a\n");
		    return;

		case 't':
		    cprintf("\1f\1gDate: \1w%s\1a\n", printdate(time(0), 0));
		    break;

		case 'T':
		    IFSYSOP {
			cprintf("\1f\1r Trash Message.\1a\n");
			message_move(curr_rm, TRASH_FORUM, current, "");
			read_position_modified = TRUE;
		    }
		    else
		    cprintf("\1f\1gDate: \1w%s\1a\n", printdate(time(0), 0));
		    break;

		case 'v':
		case 'V':
		    nox = TRUE;
		    express(-1);
		    break;

		case 'W':
		    cprintf("\1f\1gShort Wholist.\1a\n");
		    show_online(3);
		    cprintf("\n");
		    break;

		case 'w':
		    cprintf("\1f\1gWhich aliens are online?\1a\n");
		    show_online(1);
		    cprintf("\n");
		    break;

		case 'x':
		    nox = TRUE;
		    cprintf("\1f\1gSend %s %s.\1a\n", config.express, config.x_message);
		    express(0);
		    break;

		case 'X':
		    change_express(0);
		    cprintf("\n");
		    break;

		case 'y':
		case 'Y':
		    cprintf("\1f\1gYell-menu.\1a \n");
		    nox = TRUE;
		    yell_menu();
		    break;

		case 'z':	/* ugly, probably shouldn't be allowed at long */
		case 'Z':	/* prompt */
		    cprintf("\1f\1gZap %s.\1a\n", config.forum);
		    forget();
		    direction = 1;
		    start = get_read_start_number(0, direction);
		    if (start == -6666)
			return;
		    if (start < 0)
			start = 0;
		    current = start - 1;
		    set_read_bounds(&temp_lowest, &temp_highest);
		    read_position_modified = TRUE;
		    break;

		case 12:	/* <ctrl-l> */
		case 18:	/* <ctrl-r> */
		    cprintf("c");
		    fflush(stdout);
		    break;

		case 030:	/* ctrl-x for xlog */
		    cprintf("\1f\1gRead X-Log.\1a\n");
		    old_express();
		    break;

		case '?':
		    online_help('l');
		    break;

		case '"':
		    cprintf("\1f\1rQuote %s %ss.\1a\n", config.express, config.x_message);
		    quoted_Xmsgs();
		    break;

		case '%':
		    change_atho(0);
		    break;

		case '#':
		    direction = 1;
		    current = numeric_read(current);
		    break;

		case '!':
		    nox = 1;
		    feeling();	/* the feeling menu */
		    break;

		case '<':
		    cprintf("\1f\1gChange your Friends-list.\1a\n\n");
		    menu_friend(FRIEND);
		    break;

		case '>':
		    cprintf("\1f\1gChange your Enemylist.\1a\n\n");
		    menu_friend(ENEMY);
		    break;

		case ',':
		    show_online(2);
		    break;

		case '.':
		    IFSYSOP
			send_silc();
		    break;

		case '~':
		case ':':
		    nox = 1;
		    cprintf("\1f\1gEmote.\1a\n");
		    express(2);
		    break;

		default:	/* lessee if this gets rid of the input-buffer crash */
		    cprintf("\n");
		    read_position_modified = TRUE;
		    break;
	    }			/* switch */
	}			/* while */
    }				/* for */

}

long
get_read_start_number(const long read_number, const int read_direction)
{
    long start_at;
    room_t scratch;

    read_forum( curr_rm, &scratch );

    /* mail uses the userfile, NOT the quickroom */
    if (curr_rm == 1) {
	if (usersupp->lastseen[curr_rm] <= usersupp->mailnum)
	    start_at = usersupp->lastseen[curr_rm] + 1;
	else
	    start_at = usersupp->mailnum;

    }
    /* make sure lastseen is something reasonable */
    else {
	start_at = usersupp->lastseen[curr_rm] + 1;
	if (usersupp->lastseen[curr_rm] < scratch.lowest)
	    usersupp->lastseen[curr_rm] = scratch.lowest;
	else if (usersupp->lastseen[curr_rm] > scratch.highest)
	    usersupp->lastseen[curr_rm] = scratch.highest;
    }
#ifdef DEBUG
    cprintf("\ndebug: modified lastseen = %d\n", usersupp->lastseen[curr_rm]);
#endif

    if (read_number > 0)	/* check if we want to read a specific number */
	start_at = read_number;
    else if (read_number < 0)	/* check if we're reading old messages */
	start_at += read_number;

    if (read_direction < 0)
	start_at--;

    /* check if message still exists */
    if (curr_rm != 1) {
	if (start_at < scratch.lowest)
	    start_at = scratch.lowest;
	if (start_at > scratch.highest)
	    start_at = -6666;
    }
    return start_at;
}

long
numeric_read(const long current_post)
{
    room_t quad;
    unsigned int highest_id, lowest_id = 0;
    char tempstr[12];
    long templong;

    if (curr_rm == MAIL_FORUM)
	highest_id = usersupp->mailnum;
    else {
        read_forum( curr_rm, &quad );
	highest_id = quad.highest;
	lowest_id = quad.lowest;
    }
    cprintf("\1f\1gJump to a specific message number.\1a\n");
    cprintf("\1f\1gWhich message would you like to read? \1a");
    fflush(stdout);
    getline(tempstr, 9, 1);
    templong = atol(tempstr);
    if (templong > highest_id) {
	cprintf("\1f\1rThat doesn't seem to be a post number, Dave.\1a\n");
	return current_post;
    }
    if (templong < lowest_id)
	templong = lowest_id;
    return templong;
}

int
set_read_bounds(long *lower_bound, long *upper_bound)
{
    room_t quad;

    read_forum( curr_rm, &quad );
    if (curr_rm == MAIL_FORUM) {
	*lower_bound = 0;
	*upper_bound = usersupp->mailnum;
    } else {
	*lower_bound = quad.lowest;
	*upper_bound = quad.highest;
    }
    return 0;
}

int
validate_read_command(int read_command)
{
    IFTWIT
    {				/* make nasty users repent for what they have done */
	if (strchr("1234567890!AcCdDeEIMrRqQxXvVwWzZ\005\030!%:~.*&(+\'", read_command))
	    if (!((curr_rm == 13) && (strchr("eE", read_command)))) {
		more(TWITMSG, 1);
		read_command = -1;
	    }
    }
    else
    IFGUEST
    {				/* more bofh functions, different message */
	if (strchr("1234567890!cCdDeEIMmrRqQvVxXyYzZ(%\"\006\005\030!<>:~.*", read_command)) {
	    more(GUESTMSG, 1);
	    read_command = -1;
	}
    }
    else
    IFUNVALID
    {				/* no x related functions for unvalidated users */
	if (strchr("1234567890AcdDeEIrRxXvV+\005\030\014!:~.*", read_command)) {
	    more(UNVALIDMSG, 0);
	    read_command = -1;
	}
    }
    else
    IFDEGRADED
    {				/* remove some functions degraded users have */
	if (strchr("1234567890AcdDeEIrRvVxX(+\005\030!:~.\'*", read_command)) {
	    more(DEGRADEDMSG, 0);
	    read_command = -1;
	}
    }
    if (usersupp->priv & PRIV_DELETED) {
	more("share/messages/deleted_goodbye", 1);
	cprintf("\1f\1g\nPress any key to log off...\1a");
	inkey();
	logoff(0);
    }
    return read_command;

}

unsigned int
ungoto_forum_id(const unsigned int forum_id)
{
    static unsigned int ungoto_f_id = 0;
    unsigned int ret = 0;

    if (forum_id == 0 && curr_rm != 0)
	ret = ungoto_f_id;

    ungoto_f_id = forum_id;
    return ret;
}

unsigned int
ungoto_message_id(const unsigned int current_id)
{
    static unsigned int ungoto_m_id = 0;
    unsigned int ret = 1;

    if (current_id == 0)
	ret = ungoto_m_id;

    ungoto_m_id = current_id;
    return ret;
}

int
no_new_posts_here(const long highest, const int direction, const long current)
{
    return
	(((curr_rm == MAIL_FORUM) && (direction > 0) && (current == usersupp->mailnum)) ||
	 ((curr_rm != MAIL_FORUM) && (direction > 0) && (current == highest))) ? 1 : 0;
}

void
display_short_prompt(void)
{
    static room_t quad;
    static int last_prompt_forum = -1;

    if (curr_rm != last_prompt_forum) {
        read_forum( curr_rm, &quad );
	last_prompt_forum = curr_rm;
    }
    if (curr_rm == MAIL_FORUM) {
	cprintf("\1a\n\1f\1w%d.\1%c%s's %s\1w> ", curr_rm,
		(quad.flags & QR_PRIVATE) ? 'r' :
		(quad.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'g',
		usersupp->username,
		quad.name);
    } else {
	cprintf("\1a\n\1f\1w%d.\1%c%s\1w> ", curr_rm,
		(quad.flags & QR_PRIVATE) ? 'r' :
		(quad.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'y',
		quad.name);
    }
    fflush(stdout);
}


void
ungoto(void)
{
    curr_rm = ungoto_forum_id(0);
    gotocurr();
    if (curr_rm != 0)
	usersupp->lastseen[curr_rm] = ungoto_message_id(0);

}

#ifdef DUMP_ALL_POSTS_INTO_SQL
void
bingle(unsigned int forum)
{
    room_t quad;
    int i = 0;

    if (forum == 1)
        return;
    quad = readquad(forum);

    if(!(quad.flags & QR_INUSE ))
        return;

    cprintf("\rWorking on %d.%s...", forum, quad.name); fflush(stdout);
    for (i = quad.lowest; i <= quad.highest; i++) {
       (void)copy_message_to_sql(forum,i);
    }
    cprintf(" done.\n"); fflush(stdout);

}

void
copy_message_to_sql(unsigned int forum, unsigned int message)
{

    message_header_t *header;
    char filename[1000], content[1000];

    header = (message_header_t *) xmalloc(sizeof(message_header_t));
    memset(header, 0, sizeof(message_header_t));

    message_header_filename(&filename, forum, message);
    if ((read_message_header(filename, header)) == -1)
	return;
    sprintf(content, "/usr/bbs/save/forums/%d/%d.t", forum, message);

    /* Now save it. */
    (void) save_to_sql(header, content);
    xfree(header);

    return;
}
#endif
/*eof*/
