/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"
#include "sql_userforum.h"
#include "ext.h"
#include "setup.h"
#include "telnet.h"

#define extern
#include "usertools.h"
#undef extern

#include "express.h"
#include "help.h"
#include "friends.h"
#include "input.h"
#include "inter.h"
#include "main.h"
#include "menu.h"
#include "messages.h"
#include "newuser.h"
#include "registration.h"
#include "routines2.h"
#include "statusbar.h"
#include "uadmin.h"
#include "wholist.h"

char profile_default[L_USERNAME + L_BBSNAME + 2] = "";

/* PR additions for debugging */
#define UID_MathFox 23
#define PR_DEBUG (usersupp->usernum == UID_MathFox)

/* static prototypes */

static void check_passwd(void);

   /* config options statics */
static void set_usersupp_flag(const unsigned int, const long, const char *);
static void set_usersupp_config_flag(const unsigned int, const long, const char *);
static void hidden_info_menu(const unsigned int, const long, const char *);
static void _set_screenlength(const unsigned int, const long, const char *);
static void _tgl_status_bar(const unsigned int, const long, const char *);
static void _tgl_use_alias(const unsigned int, const long, const char *);
static void _tgl_silc(const unsigned int, const long, const char *);

/* code */

/*************************************************
* profile_user()
*************************************************/

void
profile_user(void)
{
    char p_name[L_BBSNAME + L_USERNAME + 2];
    user_t *user;
    char rbbs[L_BBSNAME + 1], ruser[L_USERNAME + 1];

    if (strlen(profile_default))
	cprintf("\1f\1gProfile user \1w(\1y%s\1w): \1c", profile_default);
    else
	cprintf("\1g\1fProfile user\1w: \1c");

    strncpy(p_name, get_name(5), L_BBSNAME + L_USERNAME - 2);
    p_name[L_BBSNAME + L_USERNAME - 1] = '\0';	/* no overflow, please */

    if (!strlen(p_name) && !strlen(profile_default))
	strcpy(p_name, usersupp->username);
    else if (!strlen(p_name))
	strcpy(p_name, profile_default);

    if (strchr(p_name, '@') != NULL) {
	if (parse_inter_address(p_name, ruser, rbbs) == 0) {
	    cprintf("\1r\1fNot a valid InterBBS name.\n");
	    return;
	}
	dexi_profile(ruser, rbbs);
	sprintf(profile_default, "%s@%s", ruser, rbbs);
	return;
    }
    if (check_user(p_name) == TRUE) {
	user = readuser(p_name);
	print_user_stats(user, usersupp);
	xfree(user);
	strcpy(profile_default, p_name);
    } else
	cprintf("\1f\1rNo such user.\1a\n");
    return;
}

/*************************************************
* menu_options
*************************************************/

void
menu_options(void)
{				/* configure user account */

    char tempstr[100];
    MENU_DECLARE;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title, "\n\1f\1w[\1gConfiguration Menu\1w]\n\n");
	strcpy(tempstr, "");
	sprintf(tempstr, "Screenlength: \1w[\1r%2d\1w] \1glines", usersupp->screenlength);

	MENU_ADDITEM(set_usersupp_flag, US_ANSI, 0, "ANSI Colour Display",
		     "tiv", "ANSI Colours Enabled",
		     "1", (usersupp->flags & US_ANSI) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_EXPERT, 0, "Expert User Mode",
		     "tiv", "Expert User Mode",
		     "2", (usersupp->flags & US_EXPERT) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_LASTOLD, 0,
		     "Viewing of last old post",
		     "tiv", "See last old post",
		     "3", (usersupp->flags & US_LASTOLD) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_NOPROMPT, 1,
		     "Prompting after each post",
		     "tiv", "Prompt after each post",
		     "4", (usersupp->flags & US_NOPROMPT) ? "0" : "1");
	MENU_ADDITEM(set_usersupp_flag, US_PAUSE, 0, "Pause after each screen",
		     "tiv", "Pause after each screenfull",
		     "5", (usersupp->flags & US_PAUSE) ? "1" : "0");
	MENU_ADDITEM(hidden_info_menu, 0, 0, "",
		     "ti", "Toggle Hidden Info \1w[\1rmenu\1w]", "6");

	MENU_ADDITEM(_set_screenlength, 0, 0, "", "ti", tempstr, "7");
	MENU_ADDITEM(set_usersupp_flag, US_NOFLASH, 1,
		     "Display of flashing text",
		     "tiv", "Flashing Text Enabled",
		     "8", (usersupp->flags & US_NOFLASH) ? "0" : "1");
	MENU_ADDITEM(set_usersupp_flag, US_NOBOLDCOLORS, 1,
		     "Display of bold text",
		     "tiv", "Bold Text Enabled",
		     "9", (usersupp->flags & US_NOBOLDCOLORS) ? "0" : "1");
	MENU_ADDITEM(set_usersupp_flag, US_XOFF, 0,
		     "X-message disable at login",
		     "tiv", "Disabled X-es at login",
		     "A", (usersupp->flags & US_XOFF) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_NOTIFY_FR, 0,
		     "Receiving friendslist logon notices",
		     "tiv", "Receive logon/off notifies",
		     "B", (usersupp->flags & US_NOTIFY_FR) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_SHIX, 0, "SHIX Filter",
		     "tiv", "SHIX Filter Enabled",
		     "C", (usersupp->flags & US_SHIX) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_HIDDENHOST, 0, "Hide your hostname",
		     "tiv", "Hide your hostname",
		     "D", (usersupp->flags & US_HIDDENHOST) ? "1" : "0");
	MENU_ADDITEM(_tgl_status_bar, 0, 0, "", "tiv", "Status bar",
		     "E", (usersupp->flags & US_STATUSBAR) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_NOINTERXS, 1,
		     "InterBBS access",
		     "tiv", "InterBBS Access Enabled",
		     "F", (usersupp->flags & US_NOINTERXS) ? "0" : "1");
	MENU_ADDITEM(set_usersupp_flag, US_NOCMDHELP, 1,
		     "Extra command help"
		     ,"tiv", "Extra command help Enabled",
		     "G", (usersupp->flags & US_NOCMDHELP) ? "0" : "1");
	MENU_ADDITEM(set_usersupp_flag, US_IPWHOLIST, 0,
		     "Showing hostnames in the wholist",
		     "tiv", "Show hostnames in the wholist",
		     "H", (usersupp->flags & US_IPWHOLIST) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_config_flag, CO_SHOWFRIENDS, 0,
		     "Showing of friends online at login",
		     "tiv", "Show friends online at login",
		"I", (usersupp->config_flags & CO_SHOWFRIENDS) ? "1" : "0");
	MENU_ADDITEM(_tgl_use_alias, 0, 0, "",
		     "tiv", "Use a default alias",
		   "J", (usersupp->config_flags & CO_USEALIAS) ? "1" : "0");
	MENU_ADDITEM(set_usersupp_flag, US_NOTIFY_ALL, 0,
		     "Receipt of all login event notifies",
		     "tiv", "Receive all logon events",
		     "K", (usersupp->flags & US_NOTIFY_ALL) ? "1" : "0");
//      MENU_ADDITEM(_tgl_friend_notify,"tiv", "Receive friend logon notifies", 
	//              "L", (usersupp->flags & US_NOTIFY_FR) ? "1" : "0");

	if ((usersupp->flags & US_ROOMAIDE) || (usersupp->priv >= PRIV_SYSOP)
	    || (usersupp->flags & US_GUIDE))
	    MENU_ADDITEM(set_usersupp_flag, US_ADMINHELP, 0,
			 "Extra Admin Help",
			 "tiv", "Extra Admin Help Enabled",
			 "M", (usersupp->flags & US_ADMINHELP) ? "1" : "0");

	if (usersupp->priv >= PRIV_SYSOP)
	    MENU_ADDITEM(_tgl_silc, C_NOSILC, 0, "SILC Disable",
			 "tiv", "SILC Enabled",
			 "N", (cmdflags & C_NOSILC) ? "0" : "1");
        MENU_ADDITEM(online_help_wrapper, 0, 0, "o",
			 "tiv", "Online Help", 
			 "?", "0");

#ifdef KNOB_FILTER
	if (EQ(usersupp->username, "Guest")) {
	    cprintf(" \1f\1w[\1r%c\1w] \1gO. Bing Filter enabled.\1a\n", (usersupp->config_flags & CO_WHAKFILTER) ? '*' : ' ');
	}
#endif

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
#ifdef CLIENTSRC
    send_update_to_client();
#endif
    return;

}

/*************************************************
* print_user_stats()
*************************************************/

void
print_user_stats(const user_t * user, const user_t * viewing_user)
{

    FILE *fp;
    char work[L_USERNAME + strlen(USERDIR) + 10];
    register char cmd = '\0';
    int control = 0, found_RA_rooms = 0;
    time_t timecall, curtime;
    unsigned int a;
    btmp_t *record;

    if ((EQ(user->username, "Guest")) && (usersupp->priv < PRIV_SYSOP)) {
	cprintf("\n\1f\1gThe Guest User.\n\1a");
	return;
    }
    curtime = time(0);
    timecall = ((curtime - user->laston_from) / 60);

    if (EQ(viewing_user->username, user->username))
	control = 1;

    cprintf("\n\1y\1f%s\1g ", user->username);

    if (user->priv & PRIV_WIZARD)
	cprintf("\1w(*** %s%s \1w***) ", WIZARDCOL, config.wizard);
    else if (user->priv & PRIV_SYSOP)
	cprintf("\1w(** %s%s \1w**) ", SYSOPCOL, config.sysop);
    else if (user->priv & PRIV_TECHNICIAN)
	cprintf("\1w(* %s%s \1w*) ", PROGRAMMERCOL, config.programmer);
    else if (user->flags & US_ROOMAIDE)
	cprintf("\1w(* %s%s \1w*) ", ROOMAIDECOL, config.roomaide);
    if (user->flags & US_GUIDE)
	cprintf("\1w( %s%s \1w) ", GUIDECOL, config.guide);
    if (user->priv & PRIV_NEWBIE)
	cprintf("\1w- \1pNEW \1w- ", GUIDECOL, config.guide);
    cprintf("\1f");
    if (!(user->priv & PRIV_VALIDATED))
	cprintf("\1y* Unvalidated * ");
    if (user->priv & PRIV_DEGRADED)
	cprintf("\1w[ \1rDegraded \1w] ");
    if (user->priv & PRIV_TWIT)
	cprintf("\1w[ \1rCURSED \1w] ");
    if (user->priv & PRIV_DELETED)
	cprintf("\1w[ \1rDeleted \1w] ");
    if (user->xtrapflag)
	cprintf("\1f%s", user->xtrapflag);
    if (user->flags & US_COOL)
	cprintf("\1f\1w*COOL AS ICE* ");
    if (user->flags & US_DONATOR)
	cprintf("\1g$ DONATOR $ ");

    IFSYSOP {
	if (user->flags & US_LAG)
	    cprintf("\1r[ \1wLAG \1r] ");
    }
    IFWIZARD {
	if (user->flags & US_IAMBAD)
	    cprintf("\1r[ \1wX-LOG\1r ] ");
    }
    cprintf("\n\1a\1g");

    dis_regis(user, ((viewing_user->priv >= PRIV_SYSOP) || control));

    cprintf("\1f\1cFlying: %s\1a\n", user->doing);

    if ((viewing_user->priv >= PRIV_SYSOP) && (strlen(user->aideline) > 0))
	cprintf("\1f\1yAideline:%s\n", user->aideline);
    if (viewing_user->priv >= PRIV_SYSOP) {
	if (user->configuration >= 0 && user->configuration < MAXCONFIGS)
	    cprintf("\1f\1gConfiguration\1w: \1y%s\n", shm->config[user->configuration].bbsname);
    }
    if (user->flags & US_ROOMAIDE){
	for (a = 0; a < 5; a++)
	    if (user->RA_rooms[a] >= 0) {
		if (found_RA_rooms == 0) {
		    cprintf("\1c\1f%s in: \1g", ROOMAIDE);
		    found_RA_rooms = 1;
		}
		cprintf("%d ", user->RA_rooms[a]);
	    }
    cprintf("\1a\1f\1c\n");
//    IFSYSOP mono_sql_uf_list_hosts_by_user( user->usernum );
}


    /* IS ONLINE */
    if (mono_return_pid(user->username) != -1) {
	cprintf("\1rONLINE \1cfor \1y%ld:%2.2ld \1c", timecall / 60, timecall % 60);
	if (!(user->flags & US_HIDDENHOST) || control ||
	    viewing_user->priv >= PRIV_SYSOP)
	    cprintf("from \1r%s %s\1c"
	    ,user->lasthost, user->flags & US_HIDDENHOST ? "(hidden)" : "");
	cprintf("\n");
    } else
	/* IS NOT ONLINE */
    {
	cprintf("Last on \1g%s ", printdate(user->laston_from, 1));
	cprintf("\1cto \1g%s \1p", printdate(user->laston_to, 2));
	if ((a = (timecall / 60 / 24)))
	    cprintf("(\1y%d\1p day%s ago)\1c", a, (a == 1) ? "" : "s");
	else
	    cprintf("(\1ytoday\1p)\1c");

	if (!(user->flags & US_HIDDENHOST) || control ||
	    viewing_user->priv >= PRIV_SYSOP)
	    cprintf(" from \1r%s %s\1c"
	    ,user->lasthost, user->flags & US_HIDDENHOST ? "(hidden)" : "");
	cprintf("\n");
    }

    if (!user->birthday.day) {
	/* do nothing */ ;
    } else if (user->hidden_info & H_BIRTHDAY) {
	if (viewing_user->priv >= PRIV_SYSOP) {
	    cprintf("\1a\1c(Birthday: \1g");
	    print_birthday(user->birthday);
	    cprintf(")\1f\1c ");
	} else {
	    /* do nothing */ ;
	}
    } else {
	cprintf("Birthday: \1g");
	print_birthday(user->birthday);
	cprintf("\1c ");
    }

    if (viewing_user->priv >= PRIV_SYSOP) {	/* a sysop profiles */
	cprintf("First login: \1g%s\1c\nLogins: \1g%-5d \1cPosts: \1g%-5d \1cOnlinetime: \1g%2ld:%2.2ld   \1cPriv: \1g%5d  \1cX's: \1g%5ld \n",
		printdate(user->firstcall, 1), user->timescalled,
		user->posted, user->online / 60, user->online % 60,
		user->priv, user->x_s);
	cprintf("\1cUsernum: \1y%d\1c\n", user->usernum);
    } else if (control) {	/* you profile yourself */
	cprintf("First login: \1g%s\1c\nLogins: \1g%-5d \1cPosts: \1g%-5d \1cOnlinetime: \1g%2ld:%2.2ld \1cX's: \1g%5ld \n",
		printdate(user->firstcall, 1), user->timescalled,
	     user->posted, user->online / 60, user->online % 60, user->x_s);
    } else			/* other people profile */
	cprintf("\n");


    record = mono_read_btmp(user->username);
    if (record != NULL) {
	if ((user->flags & US_GUIDE) && (record->flags & B_GUIDEFLAGGED))
	    cprintf("\1g*** is available to help others ***\n");
	xfree(record);
    }
    /* --- PROFILE --- */
    if (EQ(viewing_user->username, user->username))
	sprintf(work, "%s/profile", getuserdir(user->username));
    else {
	sprintf(work, "%s/.profile", getuserdir(user->username));
	if (!fexists(work))
	    sprintf(work, "%s/profile", getuserdir(user->username));
    }
    if (!fexists(work))
	return;
    fp = xfopen(work, "r", FALSE);
    if (!fp)
	return;

    if (file_line_len(fp) > (viewing_user->screenlength - 10)) {
	fclose(fp);
	cprintf("\n\1a\1f\1gPress \1w<\1rSPACE\1w>\1g to continue or \1w<\1rs\1w>\1g to skip... \1a");
	cmd = get_single_quiet(" s");
	switch (cmd) {
	    case ' ':
		cprintf("\1f\1g Continue.\1a\n");
		more(work, TRUE);
		cprintf("\n");
		break;

	    case 's':
		cprintf("\1f\1g Skip.\1a\n");

	}
    } else {
	fclose(fp);
	more(work, TRUE);
	cprintf("\n");
    }
    return;
}
/*************************************************
* change_atho( int i )
*
* i =  1 enable
* i =  0 toggle
* i = -1 disable
*************************************************/

void
change_atho(int i)
{

    btmp_t *tmpbtmp;
    int change = 0;

    if ((!(usersupp->flags & US_GUIDE) &&
	 !(usersupp->priv >= PRIV_TECHNICIAN)) && i == 0) {
	cprintf("\1gYou're not a %s!", GUIDE);
	return;
    }
    tmpbtmp = mono_read_btmp(usersupp->username);

    if (tmpbtmp == NULL)
	return;

    if ((tmpbtmp->flags & B_GUIDEFLAGGED) && (i == 0 || i == -1)) {
	if (usersupp->flags & US_ADMINHELP)
	    more(HELPTERM "/toggle_off", 1);
	mono_change_online(usersupp->username, "", -7);		/* deguideflagize */
	change = -1;
    } else if ((!(tmpbtmp->flags & B_GUIDEFLAGGED)) && (i == 0 || i == 1)) {
	if (usersupp->flags & US_ADMINHELP)
	    more(HELPTERM "/toggle_on", 1);
	mono_change_online(usersupp->username, "", 7);	/* guideflagize */
	change = 1;
    }
    if (change != 0) {
	cprintf("\1f\1wYou are now %smarked as 'available to help others'.\1a"
		,(change == -1) ? "\1rnot\1w " : "");
    }
    xfree(tmpbtmp);
    return;
}

/*********************************************************************/
/*  test_ansi_colours:                                               */
/*     checks to see if user is colourcode challenged    -russ       */
/*********************************************************************/

void
test_ansi_colours(user_t * user)
{
    if (user->flags & US_ANSI)	/* if set to on, turn it off */
	user->flags ^= US_ANSI;
    cprintf("\n\n\nTesting ANSI colour mode.. Currently set to: %s.\n",
	    (user->flags & US_ANSI) ? "ON" : "OFF");
    cprintf("\n\nTest Paragraph 1:\n   Now, we're going to check and see if your terminal programme is\n");
    cprintf("   capable of displaying colours properly.  This paragraph is being\n");
    cprintf("   displayed with the ANSI colours off.  This paragraph should\n");
    cprintf("   look correct.\n\n");
    cprintf("Press a key to continue.. ");
    inkey();
    user->flags ^= US_ANSI;
    cprintf("\n\n\1f\1cToggling ANSI colour mode configuration.. Currently set to: %s\1c.\1a\n",
	    (user->flags & US_ANSI) ? "\1rON" : "\1gOFF");
    cprintf("\n\nTest Paragraph 2:\n\1f\1g   Now, we're going to check and see if your terminal programme is\n");
    cprintf("   capable of displaying colours properly.  This paragraph is being\n");
    cprintf("   displayed with the \1cA\1bN\1cS\1gI colours \1rON\1g. This paragraph might\n");
    cprintf("   not look correct.\1a\n\n");

    user->flags ^= US_ANSI;

    cprintf("-  In the second paragraph if you see things like  w^[[31;0m  and display is   -\n");
    cprintf("-  NOT in color, then I'm sorry, your terminal programme can't handle colour.  -\n\n");

    cprintf("Did the Test Paragraph 2 display properly? (y/n) ");
    if (yesno() == YES)
	user->flags ^= US_ANSI;
    cprintf("\n\n\1f\1gOk.. your ANSI colour mode configuration has been set to: %s\1g.\1a\n",
	    (user->flags & US_ANSI) ? "\1rON" : "\1gOFF");
    return;
}

/*************************************************
* edit_profile()
*************************************************/

void
edit_profile(const user_t * user)
{
    char work[80];

    sprintf(work, "%s/profile", getuserdir(user->username));
    editor_edit(work);
    return;

}
/*************************************************
* change_profile()
*************************************************/

void
change_profile(const user_t * user)
{
    int a;
    char work[560];		/* WTH did keep this on 400 GRRRR */

#ifndef CLIENTSRC
    char tempstr[300];
#endif

    cprintf("\1yYou now have seven lines to create a nice profile of yourself.\1g\n\n");

#ifdef CLIENTSRC
    client_input(G_LINES, 2);

    for (a = 0; a < 560 && (a == 0 || work[a - 1]); a++)
	work[a] = netget(0);
    work[a - 3] = '\0';
#else
    strcpy(work, "");
    for (a = 0; a < 7 && (a == 0 || tempstr[0]); a++) {
	cprintf(">");
	get_string_wr(78, tempstr, a, 1);
	strcat(work, tempstr);
	strcat(work, "\n");	/* better within the brackets: PR */
    }
#endif
    if (write_profile(user->username, work) == -1) {
	cprintf("\1r\1fCould not save your profile.\n\1a");
    }
    return;
}

/*************************************************
* change_flying()
*************************************************/

void
change_flying(void)
{
    cprintf("\1gYour current %s is: \1y`%s\1a'\n", config.doing, usersupp->doing);
    cprintf("\1f\1gDo you want to change this? \1w(\1gy\1w/\1gn\1w)\1c ");
    if (yesno() == YES) {
	cprintf("\1gEnter the new %s\1w: \1f\1c", config.doing);
	getline(usersupp->doing, 28, 0);
	usersupp->doing[28] = '\0';
	writeuser(usersupp, 0);
	mono_change_online(usersupp->username, usersupp->doing, 14);
	cprintf("\1a");
    }
    return;
}

/*************************************************
* change_url()
*************************************************/

void
change_url(void)
{
    char p[RGurlLEN];

    cprintf("\1gYour current url is: \1y`%s'\n", usersupp->RGurl);
    cprintf("\1gDo you want to change this? \1w(\1gy\1w/\1gn\1w)\1c ");
    if (yesno() == YES) {
	cprintf("\1f\1gEnter the new url\1w: \1chttp://");
	getline(p, RGurlLEN - 7, TRUE);
	if (strlen(p) < 5) {
	    strcpy(usersupp->RGurl, "");
	    cprintf("\1f\1gURL not set.\n");
	} else
	    sprintf(usersupp->RGurl, "http://%s", p);

	writeuser(usersupp, 0);
	cprintf("\1a");
    }
    return;
}
/**************************************************
* yell_menu()
**************************************************/

void
yell_menu(void)
{

    register char cmd = '\0';

    cprintf("\n\1f\1gPress \1w<\1rd\1w>\1g to delete your account.\1a\n");
    cprintf("\1f\1gPress \1w<\1rY\1w>\1g to send a Yell to the Administrators.\1a\n");
    cprintf("\1f\1gPress \1w<\1rq\1w>\1g to quit...\1a\n\n");
    cprintf("\1f\1gChoice\1w: \1r");

    cmd = get_single_quiet("dqY ");
    switch (cmd) {

	case 'Y':
	    cprintf("\1f\1g1\1w: \1gYell.\1a\n");
	    yell();
	    break;

	case 'd':
	    cprintf("\1f\1g2\1w: \1gDo you \1rreally\1g want to delete your account? ");
	    if (yesno() == NO) {
		cprintf("\1f\1gAccount deletion aborted.\1a\n");
		yell_menu();
		return;
	    }
	    check_passwd();
	    break;

	case ' ':
	case 'q':
	    cprintf("\1f\1gQuit.\1a");
	    break;
    }
    cprintf("\n");
    return;
}

/**********************************************
* check_passwd()
***********************************************/

static void
check_passwd(void)
{
    char pwtest[20];

    cprintf("\r\1f\1gPlease enter your password\1w: \1g");
    getline(pwtest, -19, 1);

    if (strlen(pwtest) < 1) {
	cprintf("\1f\1rAccount deletion aborted.\1a");
	yell_menu();
	return;
    }
    if (check_password(usersupp, pwtest) == TRUE) {
	usersupp->priv ^= PRIV_DELETED;
	writeuser(usersupp, 0);
/*** nasty message that tells you not to come back... ***/
	more("share/messages/deleted_goodbye", 1);
	cprintf("\1f\1g\nPress any key to log off...\1a");
	inkey();
	logoff(0);
    } else {
	cprintf("\1f\1rIncorrect Code.\1a\n");
	yell_menu();
    }
    return;
}

/**********************************************
* q_menu()
**********************************************/

void
q_menu(void)
{

    register char cmd = '\0';

    nox = 1;			/* disable x's in q_menu */

    cprintf("\n\1f\1gPress \1w<\1rh\1w>\1g to access the helpfiles.\n");
    cprintf("\n\1gPress \1w<\1rc\1w>\1g for help on commands.\n");
    cprintf("\1gPress \1w<\1r?\1w>\1g to get a list of commands.\n");
    cprintf("\1gPress \1w<\1rshift-Q\1w>\1g to ask a Question.\n\n");
    cprintf("\1gPress \1w<\1rq\1w>\1g to exit this menu.\n\n");
    cprintf("\1gChoice\1w:\1g ");

    cmd = get_single_quiet(" chqQ?");

    switch (cmd) {

	case 'h':
	    cprintf("\1f\1gHelpfiles.\1a\n");
	    nox = 1;
	    help_topics();
	    break;

	case 'c':
	    cprintf("\1f\1gCommand Helpfiles.\1a\n");
	    nox = 1;
	    help_commands();
	    break;

	case ' ':
	case 'q':
	    cprintf("\1f\1gQuit.\1a\n");
	    break;

	case 'Q':
	    cprintf("\1f\1gAsk a Question.\1a\n");
	    nox = 1;
	    express(1);
	    break;

	case '?':
	    cprintf("\1f\1gHelp!\1a\n");
	    more(MENUDIR "/menu_main", 1);
	    break;
    }
}

/*************************************************
* kickout_user()
*
* kicks a user from the bbs, only useable by
* Sysops.
*************************************************/
void
kickout_user()
{

    char *name;
    btmp_t *b;
    pid_t kick_pid;

    cprintf("\1f\1rUser to KICK OUT of the BBS: \1g");
    name = get_name(2);
    if (strlen(name) == 0)
	return;
    b = mono_read_btmp(name);
    if (b == NULL) {
	cprintf("\1w%s is not on Monolith.\n", name);
	return;
    }
    kick_pid = b->pid;
    if ((b->priv >= PRIV_SYSOP) && (usersupp->priv < PRIV_WIZARD)) {
	cprintf("Nono. You cannot kick a Sysop /" WIZARDTITLE ".\n");
	log_sysop_action("tried to kickout '%s' ;)", name);
	xfree(b);
	return;
    }
    xfree(b);
    cprintf("\1y\1fAre you sure you want to kick \1g%s \1yfrom the BBS??? "
	    ,name);
    if (yesno() == NO) {
	return;
    }
    log_sysop_action("kicked %s from the BBS!!!", name);
    kill(kick_pid, SIGABRT);
    cprintf("\1rUser \1g'%s' \1rhas been kicked from the BBS.\n", name);
    return;
}

/*************************************************
* toggle_away()
*************************************************/
void
toggle_away(void)
{

    btmp_t *user;
    char away[100];

    mono_change_online(usersupp->username, " ", 13);	/* toggle away */
    cmdflags ^= C_AWAY;
    user = mono_read_btmp(username);	/* find out what's the toggle is */

    if (user == NULL)
	return;

    if (user->flags & B_AWAY) {
	cprintf("\1f\1wYou are now marked as \1gAWAY\1w.\n");
	if (strlen(usersupp->awaymsg))
	    cprintf("\1gDefault away message: \1y%s\1a\n", usersupp->awaymsg);
	cprintf("\1f\1gNew Away message: \1c");
	getline(away, 99, TRUE);
	if (strlen(away))
	    strcpy(usersupp->awaymsg, away);
	mono_change_online(usersupp->username, usersupp->awaymsg, 16);
	change_atho(-1);
    } else {
	cprintf("\1f\1wYou are \1rno longer\1w marked as \1gAWAY\1w.\1a");
	mono_change_online(usersupp->username, "", 16);
    }
    xfree(user);
    return;
}

/*************************************************
* toggle_beeps()
*************************************************/
void
toggle_beeps(void)
{
    usersupp->flags ^= US_BEEP;
    cprintf("Your %sBeeps are now %sabled.", config.express, usersupp->flags & US_BEEP ? "\1gen" : "\1rdis");
    writeuser(usersupp, 0);
    return;
}

/************************************************
* toggle_interbbs()
************************************************/
void
toggle_interbbs(void)
{
    usersupp->flags ^= US_NOINTERXS;
    cprintf("\1f\1wYour InterBBS access is now %sabled\1w.\1a", usersupp->flags & US_NOINTERXS ? "\1rdis" : "\1gen");
    writeuser(usersupp, 0);
    return;
}

/*************************************************
* change_host
*************************************************/

void
change_host(user_t * user)
{

    char newb[17];

    nox = TRUE;
    cprintf("\1gDo you really want to change your location? (y/n) ");
    if (yesno() == YES) {
	cprintf("New location: ");
	getline(newb, 16, 0);
	strremcol(newb);
	mono_change_online(user->username, newb, 4);
    }
    if (usersupp->priv & PRIV_WIZARD) {
	cprintf("\1gDo you also want to change the 'online from' ? (y/n) ");
	if (yesno() == YES) {
	    cprintf("New lasthost: ");
	    getline(newb, 19, 0);
	    strcpy(user->lasthost, newb);
	}
    }
    return;
}

void
show_online(int type)
{

    char *p;

    p = wholist(type, usersupp);

#ifdef CLIENTSRC
    if (type == 1) {
	putchar(IAC);
	putchar(WHO_S);
	nox = 1;
    }
#endif

    more_string(p);
    xfree(p);

#ifdef CLIENTSRC
    if (type == 1) {
	putchar(IAC);
	putchar(WHO_E);
    }
#endif

}

int
change_alias()
{
    char aliasstr[L_USERNAME + 1];

    if (strlen(usersupp->alias)) {
	cprintf("\1f\1gYour current alias is \1y`%s'\1g. Do you want to change it? \1w(\1gy\1w/\1gn\1w) \1c", usersupp->alias);
	if (yesno() == NO) {
	    cprintf("\1f\1gAlias unchanged.\1a\n");
	    return 0;
	}
    }
    cprintf("\1f\1gEnter the new alias\1w: \1c");

    getline(aliasstr, L_USERNAME, 0);

    if (aliasstr[0] == '\0') {
	cprintf("\1f\1gAlias \1rnot\1g set.\1a\n");
	strcpy(usersupp->alias, "");
	return 0;
    } else {
	if (check_user(aliasstr) == TRUE) {
	    if (EQ(usersupp->username, aliasstr)) {
		strcpy(usersupp->alias, aliasstr);
		cprintf("\1f\1gYour alias is set to \1y`%s'\1g.\n", usersupp->alias);
		return 0;
	    } else {
		cprintf("\1f\1rThat is an existing user!\1a\n");
		return 1;
	    }
	}
    }
    strcpy(usersupp->alias, aliasstr);
    cprintf("\1f\1gYour alias is set to \1y`%s'\1g.\n", usersupp->alias);
    return 0;
}

void
mono_show_config(unsigned int num)
{

    unsigned int i;
    char temp_configname[30];
    MENU_DECLARE;

    MENU_INIT;
    the_menu_format.gen_0_idx = 1;	/* we want an index beginning w/ 0 */
    the_menu_format.auto_columnize = 1;

    for (i = 0; i < MAXCONFIGS; i++) {
	if (strlen(shm->config[i].bbsname) != 0)
	    strncpy(temp_configname, shm->config[i].bbsname, 28);
	else
	    strcpy(temp_configname, "-");

	if (temp_configname[0] == '-')	/* blank (dummy) entry */
	    continue;
	if (strlen(temp_configname) > 27)
	    temp_configname[27] = '\0';
	MENU_ADDITEM(do_nothing, 0, 0, "", "tv", temp_configname, (i == num) ? "1" : "0");
    }
    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(0);
    MENU_DESTROY;

    return;

}

/*  config options statics: */
void 
set_usersupp_config_flag(const unsigned int mask, const long foo,
			 const char *flagname)
{
    usersupp->config_flags ^= mask;
    cprintf("\n\1f\1c%s is now %s.\n", flagname, (usersupp->config_flags & mask) ?
	    "\1gon" : "\1roff");
}

/* if reverse is true, it means we're setting a disable flag..  
 * which means we have to display "off" when the flag's really on
 * doesn't effect actual flag, just what the user sees. 
 */
void 
set_usersupp_flag(const unsigned int mask, const long reverse,
		  const char *flagname)
{
    usersupp->flags ^= mask;
    if (reverse)
	cprintf("\n\1f\1c%s is now %s.\n", flagname, (usersupp->flags & mask) ?
		"\1roff" : "\1gon");
    else
	cprintf("\n\1f\1c%s is now %s.\n", flagname, (usersupp->flags & mask) ?
		"\1gon" : "\1roff");
}

void
hidden_info_menu(const unsigned int a, const long b, const char *c)
{
    cprintf("\1f\1cToggle hidden info.\1a\n");
    toggle_hidden_info(usersupp);
}

void
_set_screenlength(const unsigned int x, const long y, const char *z)
{
    char tempstr[10];
    int a;

    cprintf("\1f\1gEnter your screen length \1w[\1r%d\1w]:\1c ", usersupp->screenlength);
    getline(tempstr, 3, 1);
    if (tempstr[0] != 0) {
	a = atoi(tempstr);
	usersupp->screenlength = a;
	status_bar_on();
    }
}

void
_tgl_status_bar(const unsigned int a, const long b, const char *c)
{
    status_bar_off();
    cprintf("\1f\1cToggle statusbar %s\1c.\1a\n", (usersupp->flags & US_STATUSBAR) ? "\1roff" : "\1gon");
    if (!(usersupp->flags & US_STATUSBAR)) {
	cprintf("\n\1f\1gIf your screenlength is not set correctly, this will not work, and it might \n ");
	cprintf("mess up your screen so you can't read anything. If this happens, turn it off \n ");
	cprintf("again by pressing \1w<\1gc\1w\1w><\1go\1w><\1ge\1w>\1g, then \1w<\1gctrl-r\1w>\n");
	cprintf("\1rNow, are you absolutely sure you wish to turn your status bar on? \1w(\1gy\1w/\1gn\1w) ");
	if (yesno() == YES) {
	    usersupp->flags ^= US_STATUSBAR;
	    status_bar_on();
	}
    } else
	usersupp->flags ^= US_STATUSBAR;
}

void
_tgl_use_alias(const unsigned int x, const long y, const char *z)
{
    if (strlen(usersupp->alias)) {
	cprintf("\1f\1cToggle usage of default alias %s\1c.\1a\n", (usersupp->config_flags & CO_USEALIAS) ? "\1roff" : "\1gon");
	usersupp->config_flags ^= CO_USEALIAS;
    } else {
	cprintf("\1f\1rYou have not set an alias yet!\1a\n");
	usersupp->config_flags &= ~CO_USEALIAS;
    }
}

void
_tgl_silc(const unsigned int x, const long y, const char *z)
{
    IFSYSOP
    {
	cprintf("\1f\1cToggle SILC.\1a\n");
	cmdflags ^= C_NOSILC;
    }
}
