/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* define SQL_CONFIGS */

#define HAVE_FUN_C 1

#include <stdio.h>
#include <sys/signal.h>
#include <string.h>
#include <stdlib.h>
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

#include "monolith.h"
#include "libmono.h"

#include "telnet.h"
#include "ext.h"
#include "libcache.h"
#include "setup.h"
#include "sql_config.h"
#include "sql_userforum.h"

#define extern
#include "commands.h"
#undef extern

#include "bbsconfig.h"
#include "clipboard.h"
#include "express.h"
#include "inter.h"
#include "input.h"
#include "friends.h"
#include "fun.h"
#include "chat.h"
#include "main.h"
#include "menu.h"
#include "help.h"
#include "key.h"
#include "rooms.h"
#include "messages.h"
#include "sql_goto.h"
#include "statusbar.h"
#include "usertools.h"
#include "registration.h"
#include "routines2.h"
#include "uadmin.h"
#include "quadcont.h"

static void userlists(void);
static void lock_terminal(void);
static void unlock_terminal(void);

#define NUMBER_COLOR 'w'

void
which_room(const char *plus)
{
    if (strcmp(quickroom.name, "Mail") == 0) {
	cprintf("\1a\n\1f\1%c%d.\1%c%s's %s\1w> %s", NUMBER_COLOR, curr_rm,
		(quickroom.flags & QR_PRIVATE) ? 'r' :
		(quickroom.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'g',
		usersupp->username,
		quickroom.name, plus);
    } else {
	cprintf("\1a\n\1f\1%c%d.\1%c%s\1w> %s", NUMBER_COLOR, curr_rm,
		(quickroom.flags & QR_PRIVATE) ? 'r' :
		(quickroom.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'y',
		quickroom.name, plus);
    }
    if (!(usersupp->flags & US_NOCMDHELP) && !(strlen(plus))) {
	cprintf("\n%s\n", SHORT_HELPPROMPT);
	cprintf("\1f\1gEnter command\1w ->\1c ");
    }
    fflush(stdout);
}

/*************************************************
* main_menu()
*************************************************/

void
main_menu()
{
    char tempstr[40];
    int t;
    struct tm *tp = NULL;
    time_t now;
    int cmd = 0;

    for (;;) {

#ifdef DEBUG_MEMORY_LEAK
/*      cprintf("\n\1a\1wMemory Debug: %d objects, Total xmalloc() calls: %lu \1a", allocated_ctr, allocated_total); */
#endif

	are_there_held_xs();	/* HERE! in main menu, mark as not busy */
	which_room("");
	cmd = get_single_quiet("~aAbBcCdefEFGHiIjJkKlLMNOPQqrRsSTUvVwWxXYZ0123456789!<>-_+:#.,*\"@`$&a([]% /?\005\006\011\014\016\022\030\'");

	cmd = validate_read_command(cmd);	/* priv check */

	if (mono_return_pid(usersupp->username) == -1) {
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
		} else if (is_ql(usersupp->username, quickroom)) {
		    cprintf(ROOMAIDETITLE " cmd: \1a");
		    roomaide_menu();
		} else {
		    cprintf("\1f\1rI'm sorry Dave, but I cannot do that.\1a");
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
		read_menu(0, -1);
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
		entmsg(MES_NORMAL, EDIT_NORMAL);
		break;

	    case 'E':
		cprintf("\1f\1gEnter Editor-message.\1a\n");
		status_bar_off();
		entmsg(MES_NORMAL, EDIT_EDITOR);
		status_bar_on();
		break;

	    case 005:		/* <ctrl-e> */
		nox = 1;
		enter_message_with_title();
		break;

	    case 'f':
		cprintf("\1f\1gRead messages forward.\1a\n");
		read_menu(0, 1);
		break;

	    case 'F':
		  cprintf("\1f\1gYour friends online.\1a\n");
	  	  friends_online();
	  	  break;

	    case 006: {
		 char *argh;

		 if (1 /* usersupp->priv >= PRIV_TECHNICIAN */) {

		     cprintf("\1f\1gCache debug: \n");
		     argh = show_user_cache();
		    more_string(argh);
		    xfree(argh);
		 } else {
		     cprintf("\1f\1gYour friends online.\1a\n");
		     friends_online();
		 }
		 break;
	    }

	    case 'G':
		cmdflags &= ~C_ROOMLOCK;
		skipping[curr_rm] = 0;
		storeug(curr_rm);
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
		cprintf("\1f\1g%s Info.\1a\n", config.forum_pl);
		show_desc(curr_rm);
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
		skipping[curr_rm] = 0;
		jump(1);
		break;

	    case 'K':
		cprintf("\1f\1gAll %ss\1w:\n", config.forum);
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
		if (no_new_posts_here(read_quad(curr_rm).highest, 1, usersupp->lastseen[curr_rm])) {
		    if (cmdflags & C_ROOMLOCK)
			random_goto();
		    else {
			gotonext();
		    }
		}
		read_menu(0, 1);
		break;


	    case '\016':	/* umm this is ctrl-n, not ctrl-d */
		cprintf("\1f\1rCtrl\1w-\1rN\1w-\1gEnter message.\1a\n");
		entmsg(MES_NORMAL, EDIT_CTRLD);
		break;

	    case 'O':
		cprintf("\1f\1gRead old messages.\1a\n");
		read_menu(0, -1);
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
		cprintf("\1f\1gRead messages reverse.\1a\n");
		read_menu(0, -1);
		break;

	    case 'R':
		cprintf("\1f\1gInterBBS Wholist.\1a\n");
		nox = 1;
		menu_inter();
		break;

	    case 'S':
		cprintf("\1f\1gSearch.\n");
		search();
		break;

	    case 's':
		cprintf("\1f\1gSkipping this %s.\n\n\1a", config.forum);
		cmdflags &= ~C_ROOMLOCK;
		skiproom();
		break;

	    case 'T':
		cprintf("\1f\1gDate: \1w%s \1f\1w(\1gCET\1w)\1a", printdate(time(0), 0));
		break;

	    case 'U':
		cprintf("\1f\1gUngoto.\1a");
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
		cprintf("\1f\1gChannel Wholist.\n");
		show_online(2);
		break;

	    case 'w':
		cprintf("\1f\1gWhich %s are online?\1a\n", config.user_pl);
		show_online(1);
		break;

	    case 'W':
		cprintf("\1f\1gShort Wholist.\1a\n");
		show_online(3);
		break;

	    case '!':
		nox = 1;
		feeling();	/* the feeling menu */
		break;

	    case 'x':
		nox = 1;
		cprintf("\1f\1gSend %s %s.\1a\n", config.express, config.x_message);
		express(0);
		if (((int) random() % 500000) == 42) {
		    cprintf("\1f\1b\n*** \1g%s %s from \1yThe House Spirit \1gto \1y%s \1gat \1w(\1g%02d:%02d\1w) \1b***\1a\n", config.express, config.x_message, usersupp->username, tp->tm_hour, tp->tm_min);
		    cprintf("\1a\1c>*chomp* *chomp*\n>Your %s %s tasted great!\n", config.express, config.x_message);
		} else if (((int) random() % 500000) == 42) {
		    cprintf("\1f\1b\n*** \1g%s %s from \1yCthulhu \1gto \1y%s \1gat \1w(\1g%02d:%02d\1w) \1b***\1a\n", config.express, config.x_message, usersupp->username, tp->tm_hour, tp->tm_min);
		    cprintf("\1a\1c>Feed me!\1a\n");
		}
		break;

	    case 'X':
		change_express(0);
		break;

	    case 030:		/* <ctrl-x> */
		cprintf("\1f\1gRead X-Log.\1a\n");
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
		cprintf("\1f\1gJump a number of posts back from the Last Read post.\1a\n");
		cprintf("\1f\1gHow many messages back shall I jump?\1a ");
		fflush(stdout);
		getline(tempstr, 4, 1);
		t = atoi(tempstr);
		if (t <= 0)
		    cprintf("\1f\1rThat is not a valid entry.\1a\n");
		else
		    read_menu(-t, 1);
		break;

	    case '_':
		cprintf("\1f\1rJump a number of posts back from the Last Post in this quad.\1a\n");
		cprintf("\1f\1gHow many messages back shall I jump?\1a ");
		fflush(stdout);
		getline(tempstr, 4, 1);
		t = atoi(tempstr);
		if (t <= 0)
		    cprintf("\1f\1rThat is not a valid entry.\1a\n");
		else {
		    leave_n_unread_posts(curr_rm, t);
		    read_menu(0, 1);
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

	    case '`':
		cprintf("\1a\1f\1gGenerate Superhero.\1a\1n");
		crap(3, "", 0);
		cprintf("\n");
		break;

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
		cprintf("\1f\1rQuote %s %ss.\1a\n", config.express, config.x_message);
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
//		cprintf("\1f\1rHelp!\1a\n");
//		more(MENUDIR "/menu_main", 1);
		online_help('s');
		break;

	    case '/':
		cprintf("\1f\1gList all commands.\1a\n");
		more(MENUDIR "/menu_commands", 1);
		break;

	    case '#':
		if ((t = numeric_read(-1)) > 0)		/* use -1 as there's no current */
		    read_menu(t, 1);	/* post in main menu */
		break;
	}
    }				/* for (;;) */
}

/*************************************************
 * 
 * feeling() - turn it into a menu.... 
 * 
 *************************************************/

void
feeling()
{

    register char cmd = '\0';

    cprintf("\1f\1gSend Feeling Message.\1a\n");
    if (usersupp->flags & US_EXPERT)
	cprintf("\1f\1gPress \1w<\1r?\1w>\1g for a list of available feelings.\1a\n");
    if ((usersupp->flags & US_COOL) || (usersupp->priv & (PRIV_SYSOP | PRIV_WIZARD)))
	cprintf("\1f\1gYou're cool. Press \1w<\1rf\1w>\1g to send the \1cFREEZE\1g feeling.\1a\n");

    while ((cmd != SP) && (cmd != 13)) {
	which_room("\1f\1gFeeling: \1a");
	IFNEXPERT
	{
	    cprintf("\1f\1gFeeling Options.\1a\n");
	    more(MENUDIR "/menu_feelings", 1);
	    which_room("\1f\1gFeeling: \1a");
	}

	cmd = get_single_quiet("12abcdefFgGhHiklmnoOpPrRstTuwzZ\r ?");

	if (cmd != '?')
	    cprintf("\1f\1c%c\n", cmd);

	switch (cmd) {
	    case ' ':
	    case 13:
		return;

	    case '?':
		cprintf("\1f\1gFeeling Options.\1a");
		more(MENUDIR "/menu_feelings", 1);
		break;

/* Peter put all feeling under a specific letter instead of a number */

	    case 'e':
		express(20);
		break;
	    case 'w':
		express(21);
		break;
	    case 'z':
		express(22);
		break;
	    case 'h':
		express(23);
		break;
	    case 'l':
		express(24);
		break;
	    case 'r':
		express(25);
		break;
	    case 's':
		express(26);
		break;
	    case 'u':
		express(27);
		break;
	    case 'b':
		express(28);
		break;
	    case 'm':
		express(29);
		break;
	    case 'o':
		express(30);
		break;
	    case 'f':
		if (usersupp->flags & US_COOL)
		    express(31);
		break;
	    case 'i':
		express(32);
		break;
	    case 'k':
		express(33);
		break;
	    case 't':
		express(34);
		break;
	    case 'g':
		express(35);
		break;

	    case '1':
		express(36);
		break;

	    case '2':
		express(37);
		break;

	    case 'p':
		express(38);
		break;
	    case 'a':
		express(39);
		break;

	    case 'R':
		express(40);
		break;

	    case 'd':
		express(41);
		break;
	    case 'G':
		express(42);
		break;
	    case 'P':
		express(43);
		break;
	    case 'O':
		express(44);
		break;
	    case 'n':
		express(45);
		break;
	    case 'c':
		express(46);
		break;
	    case 'Z':
		express(47);
		break;
	    case 'T':
		express(48);
		break;
	    case 'H':
		express(49);
		break;
	    case 'F':
		express(50);
		break;

	    default:
		cprintf("\1f\1rSomething went wrong here...\1a\n");
		return;
	}
    }
}

/*
 * sysop_menu()
 */
void
sysop_menu()
{

    register char cmd = '\0';

    /* extra security */
    if ((usersupp->priv & (PRIV_WIZARD | PRIV_SYSOP)) == 0)
	return;

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("\1f\1wHelp\1a\n");
	    more(MENUDIR "/menu_admin", 1);
	    which_room("\1f\1wAdmin cmd: \1a");
	}

	cmd = get_single_quiet("BCEFgGKNOQRTU\r\b ?");

	if (strchr("BCEFgOP", cmd))
	    nox = 1;		/* is busy, wants no x's */

	switch (cmd) {

	    case 'B':
		cprintf("\1f\1rMake a broadcast.\1a\n");
		express(-2);
		break;

	    case 'C':
		cprintf("\1f\1pConfig cmd:\1a\n");
                bbsconfig_menu();
		break;

	    case 'E':
		if (usersupp->priv & PRIV_WIZARD) {
		    cprintf("\1fEmperor cmd: \1a");
		    emperor_menu();
		    continue;
		} else {
		    cprintf("\1f\1rI'm sorry, Dave, but I cannot do that.\1a");
		    continue;
		}

#ifdef FILE_POST
	    case 'F':
		cprintf("\1f\1wPost a file in this room.\1a\n");
		cprintf("\1f\1wFilename:\1a ");
		fflush(stdout);
		getline(tempstr, 50, 1);
		if (!fexists(tempstr))
		    break;
		post_file(tempstr, "", "", MES_NORMAL);
#endif
		break;

            case 'g':
		cprintf("\1f\1rAdd a random goto.\1a");
                add_goto();
		break;

	    case 'G':
		cprintf("\1f\1rRemove Ghosts from the Wholist.\1a");
		mono_remove_ghosts();
                break;

	    case 'K':
		cprintf("\1f\1r%s List.\n", config.forum_pl);
		show_online(4);
		break;


	    case 'N':
		cprintf("\1f\1rNoteBook \1w(\1rpersonal\1w)\1a\n");
		notebook(2);
		break;

	    case 'R':
		cprintf("\1f\1r%s cmd: \1a", config.forum);
		sysoproom_menu();
		continue;

	    case 'T':
		nox = 1;
	       break;;

	    case 'U':
		cprintf("\1f\1r%s cmd: \1a", config.user);
		sysopuser_menu();
		continue;

	    case 'W':
		cprintf("\1f\1rWho's on Localhost?\1a\n");
		system("w");
		return;

	    case '?':
		cprintf("\1f\1wHelp\1a\n");
		more(MENUDIR "/menu_admin", 1);
		break;

	    default:
		back(11);
		return;
	}
	which_room("\1f\1wAdmin cmd: \1a");
    }
}

void
bbsconfig_menu()
{
    register char cmd = '\0';

    /* extra security */
    if ((usersupp->priv & (PRIV_WIZARD | PRIV_SYSOP)) == 0)
        return;

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
        IFNEXPERT
        {
            cprintf("\1f\1wHelp\1a\n");
            more(MENUDIR "/menu_bbsconfig", 1);
            which_room("\1f\1wAdmin cmd: \1pConfig cmd: \1c");
        }

        cmd = get_single_quiet("CDEQ ?\r\b ?");

        if (strchr("CDE", cmd))
            nox = 1;            /* is busy, wants no x's */

        switch (cmd) {

            case 'C':
                cprintf("\1f\1rCreate a BBS Config.\1a\n");
                (void) create_config();
                break;

            case 'D':
                cprintf("\1f\1rDelete a BBS Config.\1a\n");
                (void) delete_config();
                break;

            case 'E':
                cprintf("\1f\1rEdit a BBS Config.\1a\n");
                (void) edit_config();
                break;

            case '?':
                cprintf("\1f\1wHelp\1a\n");
                more(MENUDIR "/menu_bbsconfig", 1);
                break;

            default:
                back(23);
                return;
        }
        which_room("\1f\1wAdmin cmd: \1pConfig cmd: \1c");
    }
    return;
}

/*************************************************
* sysoproom_menu()
*************************************************/

void
sysoproom_menu()
{

    register char cmd = '\0';

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("\1f\1w(\1rRoom Options\1w)\1a\n");
	    more(MENUDIR "/menu_room", 1);
	    which_room("\1f\1pAdmin cmd: \1rRoom cmd: \1a");
	}

	cmd = get_single_quiet("CDEHiILkKNPTWZQG\r\b ?");

	if (strchr("CLDEHiIkKTZQ", cmd))
	    nox = 1;		/* is busy... */

	switch (cmd) {

	    case 'C':
		cprintf("\1f\1rCreate new %s.\1a\n", config.forum);
		create_room();
		break;

	    case 'D':
		cprintf("\1f\1rChange %s Description.\1a\n", config.forum);
		change_roominfo();
		break;

	    case 'E':
		cprintf("\1f\1rEdit %s.\1a\n", config.forum);
		editroom();
		break;

	    case 'G':
		cprintf("\1f\1rGenerate low-traffic %s post.\1a\n", config.forum);
		low_traffic_quad_list();
		break;

	    case 'H':
		edithosts_menu();
	 	break;

	    case 'i':
		cprintf("\1f\1rInvite user to %s.\1a\n", config.forum);
		do_invite();
		break;

            case 'I': 
                invite_menu();
                break;
           

	    case 'k':
		cprintf("\1f\1rKickout user from %s.\1a\n", config.forum);
		do_kickout();
		break;

            case 'K': 
	kickout_menu();
                break;
            

	    case 'L':
		cprintf("\1f\1pList of %ss.\1a\n", config.forum);
		look_into_quickroom(1);
		break;

	    case 'N':
		cprintf("\1f\1rNoteBook-utility.\1a\n");
		notebook(1);
		break;

	    case 'P':
		cprintf("\1f\1rPost as " ROOMAIDETITLE ".\1a\n");
		entmsg(MES_ROOMAIDE, EDIT_NORMAL);
		break;

	    case 'Q':
#ifdef QC_ADMIN_ENABLE
		cprintf("\1f\1rQuadcont Menu.\1a\n");
		qc_menu();
#endif
		break;

	    case 'T':
		cprintf("\1f\1rType of room.\1a\n");
		print_type();
		break;

	    case 'W':
		cprintf("\1f\1rWho Knows %s.\1a\n", config.forum);
		whoknows();
		break;

	    case 'Z':
		cprintf("\1f\1rDelete this %s.\1a\n", config.forum);
		killroom();
		break;

	    case '?':
		cprintf("\1f\1w(\1r%s Options\1w)\1a\n", config.forum);
		more(MENUDIR "/menu_room", 1);
		break;

	    default:
		back(14);
		return;
	}
	which_room("\1f\1wAdmin cmd: \1rQuadrant cmd: \1a");
    }
    return;
}


/*************************************************
* sysopuser_menu()
*************************************************/

void
sysopuser_menu()
{

    register char cmd = '\0';
    user_t *tmpuser;
    char work[90];
    char *p;

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("\1f\1w(\1wUser Options\1w)\1a\n");
	    more(MENUDIR "/menu_user", 1);
	    which_room("\1f\1wAdmin cmd: \1rUser cmd: \1a");
	}

	cmd = get_single_quiet("EkKLnv\r\b ?");

	if (strchr("BEKLvn", cmd))
	    nox = 1;

	switch (cmd) {
	    case 'E':
		cprintf("\1f\1rEdit a user.\1a\n");
		useradmin();
		break;

	    case 'n': 
		namechange();
		cprintf("\nPress a key.. \n");
		inkey();
		break;

	    case 'k':
		cprintf("\1f\1rKICK a user off the BBS\1a\n");
		kickout_user();
		break;


	    case 'K':
		cprintf("\1f\1rSee the KnownQuadsList for a user.\1a\n");
		cprintf("\1f\1gUser: \1c");
		p = get_name(2);
		if (!check_user(p)) {
		    printf("\1r\1fNo such user.\n");
		    break;
		} else if ((tmpuser = readuser(p)) != NULL) {
		    p = known_rooms_list(tmpuser, 1);
		    more_string(p);
		    xfree(p);
		    xfree(tmpuser);
		} else
		    printf("\1r\1fCan't read userfile.\n");
		break;

	    case 'L':
		cprintf("\1f\1rHave a perv at the Userlists.\1a\n");
		userlists();
		break;

	    case 'v':
		cprintf("\1f\1gVerify users' emailaddress.\n");
		cprintf("\1f\1rEnter user: \1a\1c");
		if ((tmpuser = readuser(get_name(2))) != NULL) {
		    cprintf("\1f\1rChecking...\1a\1c\n");
		    fflush(stdout);
		    sprintf(work, "/usr/local/bin/vrfy %s", tmpuser->RGemail);
		    system(work);
		    xfree(tmpuser);
		} else {
		    cprintf("\1f\1rNo such user.\1a\n");
		}
		break;

	    case '?':
		cprintf("\1f\1w(\1rUser Options\1w)\1a\n");
		more(MENUDIR "/menu_user", 1);
		break;

	    default:
		back(11);
		return;
	}

	which_room("\1f\1wAdmin cmd: \1rUser cmd: \1a");

    }
}



/*************************************************
* roomaide_menu()
*************************************************/

void
roomaide_menu()
{

    register char cmd = '\0';

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	if (usersupp->flags & US_ADMINHELP) {
	    cprintf("\1f\1w(" ROOMAIDETITLE " Options\1w)\1a\n");
	    more(QUADRANT "/index", 1);
	    which_room(ROOMAIDETITLE " cmd: \1a");
	}
	cmd = get_single_quiet("DEiIkKNPRTW\r\b ?");

	if (strchr("DEIK", cmd))
	    nox = 1;

	switch (cmd) {

	    case 'D':
		cprintf("\1f\1wEdit %s Description.\1a\n", config.forum);
		change_roominfo();
		break;

	    case 'E':
		cprintf("\1f\1wEdit %s.\1a\n", config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/edit", 1);
		editroom();
		break;

	    case 'i':
		cprintf("\1f\1wInvite %s to %s.\1a\n", config.user, config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/invite", 1);
		do_invite();
		break;

            case 'I': 
	invite_menu();
                break;
            

	    case 'k':
		cprintf("\1f\1wKickout %s from %s.\1a\n", config.user, config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/kickout", 1);
		do_kickout();
		break;

            case 'K': 
	         kickout_menu();
 break;

	    case 'N':
		cprintf("\1f\1wNotebook-utility.\1a\n");
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/notebook", 1);
		notebook(1);
		break;

	    case 'P':
		cprintf("\1f\1wPost as " ROOMAIDETITLE ".\1a\n");
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/post", 1);
		entmsg(MES_ROOMAIDE, EDIT_NORMAL);
		break;

	    case 'T':
		cprintf("\1f\1wType of %s.\1a\n", config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/type", 1);
		print_type();
		break;

	    case 'W':
		cprintf("\1f\1wWho Knows %s.\1a\n", config.forum);
		whoknows();
		break;

	    case '?':
		cprintf("\1f\1w(" ROOMAIDETITLE " Options\1w)\1a\n");
		more(QUADRANT "/index", 1);
		break;

	    default:
		back(strlen(ROOMAIDETITLE) + 13);
		return;
	}
	which_room(ROOMAIDETITLE " cmd: \1a");

    }
}



/*************************************************
* emperor_menu()
*************************************************/

void
emperor_menu()
{

    register char cmd = '\0';

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("\1f\1w(" WIZARDTITLE " Options\1p)\1a\n");
	    more(MENUDIR "/menu_emp", 1);
	    which_room("\1f\1wAdmin cmd: " WIZARDTITLE " cmd: \1a");
	}

	cmd = get_single_quiet("bBFrR*\r\b ?");

	if (strchr("bBDEU", cmd))
	    nox = 1;

	switch (cmd) {

	    case 'b':
		cprintf("\1f\1wMake an \1pImportant\1w broadcast!\1a\n");
		express(-3);
		break;

	    case 'B':
		cprintf("\1f\1rBOOT ALL USERS!\1a\n");
		emergency_boot_all_users();
		break;

	    case 'r':
		cprintf("\1f\1w%s switching!\1a\n", config.forum);
		move_rooms();
		break;

	    case 'R':
		cprintf("\1f\1bReset a %s.\1a\n", config.forum);
		reset_room();
		break;

	    case '?':
		cprintf("\1f\1p(" WIZARDTITLE " Options\1p)\1a\n");
		more(MENUDIR "/menu_emp", 1);
		break;

	    default:
		back(13);
		return;
	}

	which_room("\1f\1wAdmin cmd: Emperor cmd: \1a");
    }
}


/*************************************************
* config_menu()
*************************************************/

void
config_menu()
{

    char newb[40];
    register char cmd = '\0';
    int i = 1;

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("(Config Options)\n");
	    more(MENUDIR "/menu_config", 1);
	    which_room("\1gConfig: \1w\1f");
	}

	cmd = get_single_quiet("AabBCcDEFHiIKLOPsSTWY\r\b ?/");

	IFTWIT
	    if (strchr("acFDKLHIW", cmd)) {
	    more(TWITMSG, 1);
	    continue;
	}
	if (strchr("AbaCcEFDKIPTWY?", cmd))
	    nox = 1;

	switch (cmd) {

	    case 'A':
		cprintf("\1f\1gChange address.\n");
		if ((usersupp->priv & PRIV_DEGRADED) || (!(usersupp->priv & PRIV_VALIDATED))) {
		    enter_reginfo();
		} else {
		    cprintf("\1y\1fYou can not change your address info once you are validated.\n");
		    cprintf("\1y(You can \1w<\1ry\1w>\1yell to ask the sysops to change it.)\n");
		}
		break;

	    case 'a':
		cprintf("\1f\1gChange your alias.\n");
		while (i)
		    i = change_alias();
		break;

	    case 'b':
		cprintf("\1f\1gChange birthday/date.\n");
		modify_birthday(&(usersupp->birthday));
		writeuser(usersupp, 0);
		break;

	    case 'B':
		cprintf("\1f\1gBorg.\n");
		cprintf("\n\1f\1yYou will be assimilated!\nResistance is futile!\n");
		break;

	    case 'C':
		    cprintf("\1f\1gChange configuration.\n\n");
		    cprintf("\1f\1gYour configuration is set to \1y`%s'\1g.\n", config.bbsname);
		    cprintf("\1f\1gAre you sure you want to change this? \1w(\1gy\1w/\1gn\1w) \1c");
		    if (yesno() == NO)
			break;
		    cprintf("\n");

                    usersupp->configuration = select_config( "\n\1f\1gUse configuration\1w: \1c" );
		    (void) mono_sql_read_config(usersupp->configuration, &config);

		    cprintf("\1f\1gConfiguration set to \1y`%s'\1g.\n", config.bbsname);
		    break;

	    case 'c':
#ifdef CLIENTSRC
		client_input(CONFIG, 0);
		while (netget(0) != '\n') ;
#else
		cprintf("\1f\1rYou can't configure the CLient when you are not using it.\n");
#endif
		break;



	    case 'Y':
	    case 'D':		/* First announce, then rip out of the code  */
		cprintf("\1f\1gChange %s.\n", config.doing);
		change_flying();
		break;

	    case 'E':
		cprintf("\1f\1gChange your Enemylist.\1a\n\n");
		menu_friend(ENEMY);
		break;

	    case 'F':
		cprintf("\1f\1gChange your Friendslist.\1a\n\n");
		menu_friend(FRIEND);
		break;

	    case 'H':
	    case 'L':
		cprintf("\1f\1gChange %s.\n", LOCATION);
		change_host(usersupp);
		break;

	    case 'i':
		cprintf("\1f\1gChange Info (Profile).\n");
		nox = 1;
		cprintf("Are you sure you want to change your profile (y/n)");
		if (yesno() == YES) {
		    change_profile(usersupp);
		} else {
		    cprintf("Okay, not changing it then.\n");
		}
		break;

	    case 'I':
		cprintf("Edit Info (Profile).\n");
		nox = 1;
		cprintf("Are you sure you want to edit your profile (y/n)\n");
		if (yesno() == YES) {
		    edit_profile(usersupp);
		} else {
		    cprintf("Okay, not changing it then.\n");
		}
		break;

	    case 'K':
		cprintf("\1f\1gKey Menu.\n");
		key_menu();
		break;

	    case 'O':
		cprintf("\1f\1gOptions.\n");
		menu_options();
		writeuser(usersupp, 0);
		break;

	    case 'P':
		cprintf("\1f\1yChange Password.\n");
		do_changepw();
		break;

	    case 'S':
	    case 's':
		cprintf("\1f\1pSubscribe to channel: \1a");
		chat_subscribe();
		break;

	    case 'T':
		cprintf("Change termtype.\n");
		cprintf("New termtype (%s): ", getenv("TERM"));
		fflush(stdout);
		getline(newb, 15, 1);
		if (strlen(newb) > 0)
		    setenv("TERM", newb, 1);
		else
		    cprintf("Not changed.\n");
		break;

	    case 'W':
		cprintf("fgChange WWW url.\n");
		change_url();
		break;

	    case '/':

		cprintf("(Config Options)\n");
		more(MENUDIR "/menu_config", 1);
		break;

	    case '?':
		online_help('c');
		break;

	    default:
		back(8);
		return;
	}
	which_room("\1gConfig: \1w\1f");
    }
}

/*************************************************
* misc_menu()
*************************************************/

void
misc_menu()
{

    int i = 0;
    register char cmd = '\0';

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("(Miscellaneous Options)\n");
	    more(MENUDIR "/menu_misc", 1);
	    which_room("\1gMisc: \1w");
	}

	cmd = get_single_quiet("lLmMrsxz\r\b ?/");

	switch (cmd) {
            case 'l':
            case 'L':
                nox = 1;
                cprintf("\1f\1gLock Terminal.\n");
                lock_terminal();
                break;

	    case 'm':
		nox = 1;
		cprintf("\1f\1cMail Direct.\n");
		direct_mail();
		break;

	    case 'M':
		cprintf("\1f\1cMark messages as read.\n");
		cprintf("\1f\1gAre you \1rSURE\1g you want to mark ALL messages as read? \1w(\1gy\1w/\1gn\1w) \1c");
		if (yesno() == YES) {
		    for (i = 0; i < MAXQUADS; i++) {
			mark_as_read(i);
		    }
		    cprintf("\1f\1gThe entire BBS has been marked as read.\n");
		    writeuser(usersupp, 0);
		}
		break;

	    case 'r':
		cprintf("\1f\1c%sLock myself.\n", config.forum);
		cmdflags ^= C_ROOMLOCK;
		cprintf("\1gYou are now %s this %s.\n", (cmdflags & C_ROOMLOCK) ? "locked in" : "released from", config.forum);
		break;

	    case 's':
		cprintf("\1f\1cSystem Configuration.\n");
		print_system_config();
		break;

	    case 'x':
		cprintf("\1f\1cReset Lastseen in this %s.\n", config.forum);
		reset_lastseen();
		break;

	    case 'z':
		cprintf("\1f\1cZap all %ss.\n", config.forum);
		zap_all();
		break;


	    case '/':
		cprintf("\1f\1w(\1gMisc. Options\1w)\n");
		more(MENUDIR "/menu_misc", 1);
		break;
 
	    case '?':
		online_help('i');
		break;

	    default:
		back(6);
		return;
	}
	which_room("\1f\1gMisc: \1c");

    }
}

/*************************************************
* userlists()
*************************************************/

static void
userlists()
{
    int cmd;

    for (;;) {
	cprintf("\n  \1w1. \1gAdminList\n");
	cprintf("  \1w2. \1gNewUserList\n");
	cprintf("  \1w3. \1gPurgeList\n");
	cprintf("  \1w4. \1gCursed&DeletedList\n");

	cprintf("\n  \1yOption: ");

	cmd = get_single("1234\rQ \b");

	cprintf("\1w");

	switch (cmd) {
	    case '1':
		more(ADMINLISTSYS, 1);
		break;

	    case '2':
		more(NEWUSERLIST, 0);
		break;

	    case '3':
		more(PURGELIST, 0);
		break;

	    case '4':
		more(TWITLIST, 0);
		break;

	    default:
		return;
	}

	putchar('\n');
    }

}

static void
lock_terminal()
{
    cmdflags ^= C_LOCK;
    mono_change_online(usersupp->username, " ", 17);
    cprintf("c");
    fflush(stdout);
    cprintf("\n\1f\1w[ \1gTerminal locked for \1y%s@%s \1w]", usersupp->username, config.bbsname );
    fflush(stdout);
    inkey();
    unlock_terminal();
    cmdflags ^= C_LOCK;
    mono_change_online(usersupp->username, " ", 17);
    return;
}

static void
unlock_terminal()
{
    char pwtest[20];
    unsigned int done = FALSE, failures = 0;

    do {
        cprintf("c");
        fflush(stdout);
        cprintf("\n\1f\1gUnlock terminal.\1a");
        if(failures)
            cprintf(" \1f\1w(\1r%d failure%s\1w)\1a", failures, (failures > 1) ? "s" : "" );
        cprintf("\n\n");
        cprintf("\1f\1gPlease enter your password\1w: \1c");
        (void) getline(pwtest, -19, 1);

        if (strlen(pwtest) == 0) {
            cprintf("");
            fflush(stdout);
            failures++;
            done = FALSE;
        } else {
            if ( mono_sql_u_check_passwd( usersupp->usernum, pwtest ) == TRUE ) {
               done = TRUE;
            } else {
               fflush(stdout);
               cprintf("");
               failures++;
               done = FALSE;
            }
        }
        if( failures >= 4 ) {
            cprintf("\n\n\1f\1rToo many failures, logging off!\1a\n");
            logoff(0);
         }
    } while( done != TRUE );

    return;
}
