/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
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
#include "telnet.h"
#include "setup.h"

#include "bbsconfig.h"
#include "chat.h"
#include "express.h"
#include "help.h"
#include "friends.h"
#include "input.h"
#include "inter.h"
#include "key.h"
#include "main.h"
#include "menu.h"
#include "messages.h"
#include "newuser.h"
#include "read_menu.h"
#include "registration.h"
#include "rooms.h"
#include "routines2.h"
#include "statusbar.h"
#include "uadmin.h"
#include "wholist.h"

#include "sql_user.h"
#include "sql_web.h"

#define extern
#include "usertools.h"
#undef extern

/* static vars */

static char *_locale[] =
{"European", "American"};

/* file wide static prototypes */

static void set_usersupp_flag(const unsigned int, const long, void *);
static void set_usersupp_config_flag(const unsigned int, const long, void *);
static unsigned int _get_locale(const unsigned long);
static float _get_monoholic_rating(const user_t * user);
static char *_get_monoholic_flag(const user_t * user);
static void _print_priv_flags(const user_t * user, forumlist_t * p);
static void _print_user_flags(const user_t * user);

#ifdef USE_ICQ
static void _change_icq_number(const unsigned int a, const long b, void *c);
static void _change_icq_password(const unsigned int a, const long b, void *c);
#endif


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
	cprintf(_("\1f\1gProfile user \1w(\1y%s\1w): \1c"), profile_default);
    else
	cprintf(_("\1g\1fProfile user\1w: \1c"));

    strncpy(p_name, get_name(5), L_BBSNAME + L_USERNAME - 2);
    p_name[L_BBSNAME + L_USERNAME - 1] = '\0';	/* no overflow, please */

    if (!strlen(p_name) && !strlen(profile_default))
	strcpy(p_name, usersupp->username);
    else if (!strlen(p_name))
	strcpy(p_name, profile_default);

    if (strchr(p_name, '@') != NULL) {
	if (parse_inter_address(p_name, ruser, rbbs) == 0) {
	    cprintf(_("\1r\1fNot a valid InterBBS name.\n"));
	    return;
	}
	dexi_profile(ruser, rbbs);
	sprintf(profile_default, "%s@%s", ruser, rbbs);
	return;
    }
    p_name[L_USERNAME - 1] = '\0';	/* no overflow, please */

    /*
     * This fails horribly if a user has an entry in the database, but
     * no userfile on disk. Using the old version instead.
     *
	* michel: but users should have an sql entry!
    if (check_user(p_name) == TRUE) {
*/
     if (mono_sql_u_check_user(p_name) == TRUE) {
	user = readuser(p_name);
	if (user == NULL) {
	    cprintf("Error reading userfile.\n");
	} else {
	    print_user_stats(user, usersupp);
	    xfree(user);
	    strcpy(profile_default, p_name);
	}
    } else
	cprintf(_("\1f\1rNo such user.\1a\n"));
    return;
}


/*************************************************
* config_menu()
*************************************************/

void
config_menu(void)
{

    static void _config_display_options(const unsigned int, const long, void *);
    static void _config_message_menu(const unsigned int, const long, void *);
    static void _config_express_menu(const unsigned int, const long, void *);
    static void _config_personal_info_menu(const unsigned int, const long, void *);

    char *context;
    MENU_DECLARE;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gMain Configuration Menu\1w]\n\n");
	MENU_ADDITEM(_config_display_options, 0, 0, NULL,
		     "ti", "General Configuration    \1w[\1rmenu\1w]", "G");
	MENU_ADDITEM(_config_message_menu, 0, 0, NULL,
		     "ti", "Message System Options   \1w[\1rmenu\1w]", "M");
	MENU_ADDITEM(_config_express_menu, 0, 0, NULL,
		     "ti", "Express Message Options  \1w[\1rmenu\1w]", "X");
	MENU_ADDITEM(_config_personal_info_menu, 0, 0, NULL,
		     "ti", "Personal Information     \1w[\1rmenu\1w]", "P");

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "ti", "-----------", "");

	context = (char *) xmalloc(sizeof(char) * 2);
	strcpy(context, "m");

	MENU_ADDITEM(online_help_wrapper, 0, 0, (char *) context,
		     "ti", "Online Help:  This Menu", "?");

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(1);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
}


void
_config_display_options(const unsigned int a, const long b, void *c)
{
    static void _bbs_appearance_wrapper(const unsigned int, const long, void *);
    static void _client_local_config(const unsigned int, const long, void *);
    static void _set_screenlength(const unsigned int, const long, void *);
    static void _tgl_status_bar(const unsigned int, const long, void *);

    char *tmpstr;
    MENU_DECLARE;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gGeneral Configuration Menu\1w]\n\n");
	MK_TMPSTR("ANSI Colour Display");
	MENU_ADDITEM(set_usersupp_flag, US_ANSI, 0, (char *) tmpstr,
		     "tiv", "Display ANSI Colours", "1",
		     (usersupp->flags & US_ANSI) ? "\1yYes" : " No");
	MK_TMPSTR("Display of flashing text");
	MENU_ADDITEM(set_usersupp_flag, US_NOFLASH, 1,
		     (char *) tmpstr,
		     "tiv", "Display Flashing Text", "2",
		     (usersupp->flags & US_NOFLASH) ? " No" : "\1yYes");
	MK_TMPSTR("Display of Bold Text");
	MENU_ADDITEM(set_usersupp_flag, US_NOBOLDCOLORS, 1,
		     (char *) tmpstr,
		     "tiv", "Display Bold Text", "3",
		     (usersupp->flags & US_NOBOLDCOLORS) ? " No" : "\1yYes");
	MK_TMPSTR("Display Hostnames in the Wholist");
	MENU_ADDITEM(set_usersupp_flag, US_IPWHOLIST, 0,
		     (char *) tmpstr,
		     "tiv", "Show hostnames in the wholist", "4",
		     (usersupp->flags & US_IPWHOLIST) ? "\1yYes" : " No");
	MENU_ADDITEM(_tgl_status_bar, 0, 0, NULL, "tiv", "Display Status Bar", "5",
		     (usersupp->flags & US_STATUSBAR) ? "\1yYes" : " No");
	MENU_ADDITEM(_bbs_appearance_wrapper, 0, 0, NULL,
		     "tiv", "Set BBS Style  \1w[\1rmenu\1w]", "6", "---");
	MALLOC_TMPSTR(100);
	sprintf(tmpstr, "Screenlength: \1w[\1r%2d\1w] \1glines", usersupp->screenlength);
	MENU_ADDITEM(_set_screenlength, 0, 0, (char *) tmpstr, "tiv", tmpstr, "7", "---");
	MK_TMPSTR("Expert User");
	MENU_ADDITEM(set_usersupp_flag, US_EXPERT, 0, (char *) tmpstr,
		     "tiv", "Expert User Mode", "8",
		     (usersupp->flags & US_EXPERT) ? "\1yYes" : " No");
	MK_TMPSTR("Enable Extra Command Help");
	MENU_ADDITEM(set_usersupp_flag, US_NOCMDHELP, 1,
		     (char *) tmpstr,
		     "tiv", "Extra Command Help", "9",
		     (usersupp->flags & US_NOCMDHELP) ? " No" : "\1yYes");

#ifdef CLIENTSRC
	MENU_ADDITEM(_client_local_config, 0, 0, NULL,
		     "tiv", "Local Client Configuration", "C", "---");
#endif

	if ((usersupp->flags & US_ROOMAIDE) || (usersupp->priv >= PRIV_SYSOP)
	    || (usersupp->flags & US_GUIDE)) {
	    MK_TMPSTR("Enable Extra Admin Help");
	    MENU_ADDITEM(set_usersupp_flag, US_ADMINHELP, 0,
			 (char *) tmpstr,
			 "tiv", "Extra Admin Help", "H",
		       (usersupp->flags & US_ADMINHELP) ? "\1yYes" : " No");
	}
	MENU_ADDITEM(do_nothing, 0, 0, NULL, "tiv", "-----------", "", " ");

	MK_TMPSTR("g");
	MENU_ADDITEM(online_help_wrapper, 0, 0, (char *) tmpstr,
		     "tiv", "Online Help: This Menu", "?", "---");

	the_menu_format.auto_columnize = 1;
	the_menu_format.no_boolean_values = 1;

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
}


void
_config_message_menu(const unsigned int a, const long b, void *c)
{

    static void _change_alias(const unsigned int, const long, void *);
    static void _tgl_use_alias(const unsigned int, const long, void *);
    static void _set_locale(const unsigned int, const long, void *);
    static void _set_date_display(const unsigned int, const long, void *);

    char *tmpstr, tempstr[100];
    MENU_DECLARE;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gMessage System Configuration Menu\1w]\n\n");
	MK_TMPSTR("Viewing of last old post");
	MENU_ADDITEM(set_usersupp_flag, US_LASTOLD, 0,
		     (char *) tmpstr,
		     "tiv", "See last old post", "0",
		     (usersupp->flags & US_LASTOLD) ? "\1yYes" : " No");
	MK_TMPSTR("Prompting after each post");
	MENU_ADDITEM(set_usersupp_flag, US_NOPROMPT, 1,
		     (char *) tmpstr,
		     "tiv", "Prompt after each post", "1",
		     (usersupp->flags & US_NOPROMPT) ? " No" : "\1yYes");
	MK_TMPSTR("Pause after each screen");
	MENU_ADDITEM(set_usersupp_flag, US_PAUSE, 0, (char *) tmpstr,
		     "tiv", "Pause after each screenfull", "2",
		     (usersupp->flags & US_PAUSE) ? "\1yYes" : " No");
	MENU_ADDITEM(_tgl_use_alias, 0, 0, NULL,
		     "tiv", "Use a default alias", "3",
		 (usersupp->config_flags & CO_USEALIAS) ? "\1yYes" : " No");
	MENU_ADDITEM(_change_alias, 0, 0, NULL,
		     "tiv", "Change your default alias", "4", "---");

	sprintf(tempstr, "Empty lines around %s", config.message_pl);
	MK_TMPSTR(tempstr);
	MENU_ADDITEM(set_usersupp_config_flag, CO_NEATMESSAGES, 0, (char *) tmpstr,
		     "tiv", tempstr, "5",
	     (usersupp->config_flags & CO_NEATMESSAGES) ? "\1yYes" : " No");

	sprintf(tempstr, "Expanded %s headers", config.message);
	MK_TMPSTR(tempstr);
	MENU_ADDITEM(set_usersupp_config_flag, CO_EXPANDHEADER, 0, (char *) tmpstr,
		     "tiv", tempstr, "6",
	     (usersupp->config_flags & CO_EXPANDHEADER) ? "\1yYes" : " No");

	sprintf(tempstr, "Display \1w%s\1g date",
		(usersupp->config_flags & CO_LONGDATE) ? "long" : "short");
	MENU_ADDITEM(_set_date_display, 0, 0, NULL,
		     "tiv", tempstr, "7", "---");

	if (usersupp->priv < PRIV_SYSOP)
	    sprintf(tempstr, "Notify on deleted %s", config.message_pl);
	else
	    sprintf(tempstr, "Display deleted %s", config.message_pl);
	MK_TMPSTR(tempstr);
	MENU_ADDITEM(set_usersupp_config_flag, CO_DELETEDINFO, 0, (char *) tmpstr,
		     "tiv", tempstr, "8",
	      (usersupp->config_flags & CO_DELETEDINFO) ? "\1yYes" : " No");

	sprintf(tempstr, "%s-Style %s headers", BBSNAME, config.message);
	MK_TMPSTR(tempstr);
	MENU_ADDITEM(set_usersupp_config_flag, CO_MONOHEADER, 0,
		     (char *) tmpstr, "tiv", tempstr, "9",
	       (usersupp->config_flags & CO_MONOHEADER) ? "\1yYes" : " No");

	sprintf(tempstr, "\1w%s\1g date format",
		_locale[_get_locale(usersupp->config_flags)]);
	MENU_ADDITEM(_set_locale, 0, 0, NULL,
		     "tiv", tempstr, "A", "---");

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "tiv", "-----------", "", "\1g--");

	MK_TMPSTR("p");
	MENU_ADDITEM(online_help_wrapper, 0, 0, (char *) tmpstr,
		     "ti", "Online Help:  This Menu", "?");

	the_menu_format.no_boolean_values = 1;
	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(1);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
}


void
_config_express_menu(const unsigned int a, const long b, void *c)
{
    static void _menu_friend_wrapper(const unsigned int, const long, void *);
    static void _chat_subscribe_wrapper(const unsigned int, const long, void *);
    static void _tgl_silc(const unsigned int, const long, void *);

    MENU_DECLARE;
    char *tmpstr;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title, "\n\1f\1w[\1gExpress Message Configuration Menu\1w]\n\n");

	MK_TMPSTR("X-message disable at login");
	MENU_ADDITEM(set_usersupp_flag, US_XOFF, 0,
		     (char *) tmpstr,
		     "tiv", "Disabled eXpress messages at login",
		     "1", (usersupp->flags & US_XOFF) ? "\1yYes" : " No");
	MK_TMPSTR("Receiving friendslist logon notices");
	MENU_ADDITEM(set_usersupp_flag, US_NOTIFY_FR, 0,
		     (char *) tmpstr,
		     "tiv", "Receive logon/off notifies",
		  "2", (usersupp->flags & US_NOTIFY_FR) ? "\1yYes" : " No");
	MK_TMPSTR("SHIX Filter");
	MENU_ADDITEM(set_usersupp_flag, US_SHIX, 0, (char *) tmpstr,
		     "tiv", "Use the SHIX Filter",
		     "3", (usersupp->flags & US_SHIX) ? "\1yYes" : " No");
	MK_TMPSTR("InterBBS access");
	MENU_ADDITEM(set_usersupp_flag, US_NOINTERXS, 1,
		     (char *) tmpstr,
		     "tiv", "InterBBS Access",
		  "4", (usersupp->flags & US_NOINTERXS) ? " No" : "\1yYes");
	MK_TMPSTR("Showing of friends online at login");
	MENU_ADDITEM(set_usersupp_config_flag, CO_SHOWFRIENDS, 0,
		     (char *) tmpstr,
		     "tiv", "Show friends online when you login",
	 "5", (usersupp->config_flags & CO_SHOWFRIENDS) ? "\1yYes" : " No");
	MK_TMPSTR("Display of all logon/off events");
	MENU_ADDITEM(set_usersupp_flag, US_NOTIFY_ALL, 0,
		     (char *) tmpstr,
		     "tiv", "Display all logon/off events",
		 "6", (usersupp->flags & US_NOTIFY_ALL) ? "\1yYes" : " No");
	MENU_ADDITEM(_menu_friend_wrapper, 0, FRIEND, NULL,
		     "tiv", "Friends List   \1w[\1rmenu\1w]",
		     "7", "---");
	MENU_ADDITEM(_menu_friend_wrapper, 0, ENEMY, NULL,
		     "tiv", "Enemies List   \1w[\1rmenu\1w]",
		     "8", "---");
	MENU_ADDITEM(_chat_subscribe_wrapper, 0, 0, NULL,
		     "tiv",
		     "Chat Channels:  subscribe/unsubscribe \1w[\1rmenu\1w]",
		     "9", "---");

	if (usersupp->priv >= PRIV_SYSOP)
	    MENU_ADDITEM(_tgl_silc, C_NOSILC, 0, NULL,
			 "tiv", "SILC Enabled",
			 "S", (cmdflags & C_NOSILC) ? " No" : "\1yYes");

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "tiv", "-----------", "", "\1g--");

	MK_TMPSTR("x");
	MENU_ADDITEM(online_help_wrapper, 0, 0, (char *) tmpstr,
		     "tiv", "Online Help:  This Menu",
		     "?", "---");

	the_menu_format.no_boolean_values = 1;
	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(1);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
    return;

}


void
_config_personal_info_menu(const unsigned int a, const long b, void *c)
{

    static void _hidden_info_menu(const unsigned int, const long, void *);
    static void _timezone_menu(const unsigned int, const long, void *);
    static void _change_address(const unsigned int, const long, void *);
    static void _change_birthday(const unsigned int, const long, void *);
    static void _change_flying(const unsigned int, const long, void *);
    static void _change_host(const unsigned int, const long, void *);
    static void _change_profile(const unsigned int, const long, void *);
    static void _edit_profile(const unsigned int, const long, void *);
    static void _key_menu_wrapper(const unsigned int, const long, void *);
    static void _change_password(const unsigned int, const long, void *);
    static void _change_url(const unsigned int, const long, void *);


    char *tmpstr, tempstr[100];
    MENU_DECLARE;

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gPersonal Info Configuration Menu\1w]\n\n");

	MENU_ADDITEM(_change_address, 0, 0, NULL,
		     "ti", "Change Address Information", "A");

	MENU_ADDITEM(_change_birthday, 0, 0, NULL,
		     "ti", "Update Birthday", "B");

	MENU_ADDITEM(_change_password, 0, 0, NULL,
		     "ti", "Change account password", "D");

	MENU_ADDITEM(_hidden_info_menu, 0, 0, NULL,
		     "ti", "Toggle Hidden Info \1w[\1rmenu\1w]", "H");

	MENU_ADDITEM(_change_profile, 0, 0, NULL,
		     "ti", "Change your profile", "i");

	MENU_ADDITEM(_edit_profile, 0, 0, NULL,
		     "ti", "Edit your profile", "I");

	sprintf(tempstr, "Change %s", LOCATION);
	MENU_ADDITEM(_change_host, 0, 0, NULL,
		     "ti", tempstr, "L");

	MENU_ADDITEM(_timezone_menu, 0, 0, NULL,
		     "ti", "Timezone Setup  \1w[\1rmenu\1w]", "T");

	MENU_ADDITEM(_key_menu_wrapper, 0, 0, NULL,
		     "ti", "Validation Key \1w[\1rmenu\1w]", "V");

	MENU_ADDITEM(_change_url, 0, 0, NULL,
		     "ti", "Change URL (homepage)", "W");

	sprintf(tempstr, "Change %s", config.doing);
	MENU_ADDITEM(_change_flying, 0, 0, NULL,
		     "ti", tempstr, "Y");

#ifdef USE_ICQ
IFSYSOP {

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "ti", "-----------", "");

	MENU_ADDITEM(_change_icq_number, 0, 0, NULL,
		     "ti", "Change ICQ Number", "u");

	MENU_ADDITEM(_change_icq_password, 0, 0, NULL,
		     "ti", "Change ICQ Password", "p");
}
#endif

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "ti", "-----------", "");

	MK_TMPSTR("a");
	MENU_ADDITEM(online_help_wrapper, 0, 0, (char *) tmpstr,
		     "ti", "Online Help:  This Menu", "?");

	the_menu_format.auto_columnize = 1;
	the_menu_format.no_boolean_values = 1;

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
}


void
_menu_friend_wrapper(const unsigned int a, const long which, void *c)
{
    if (which == ENEMY) {
	cprintf("\1f\1gChange your Enemylist.\1a\n\n");
	menu_friend(ENEMY);
    } else {
	cprintf("\1f\1gChange your Friendslist.\1a\n\n");
	menu_friend(FRIEND);
    }
}

void
_chat_subscribe_wrapper(const unsigned int a, const long b, void *c)
{
    cprintf(_("\1f\1pSubscribe to channel: \1a"));
    chat_subscribe();
}


void
_bbs_appearance_wrapper(const unsigned int a, const long b, void *c)
{
    cprintf("\1f\1gChange configuration.\n\n");
    cprintf("\1f\1gYour configuration is set to \1y`%s'\1g.\n", config.bbsname);
    cprintf("\1f\1gAre you sure you want to change this? \1w(\1gy\1w/\1gn\1w) \1c");
    if (yesno() == NO)
	return;

    usersupp->configuration = select_config("\n\n\1f\1gUse configuration\1w: \1c");
    mono_sql_read_config(usersupp->configuration, &config);

    cprintf("\1f\1gConfiguration set to \1y`%s'\1g.\n", config.bbsname);
}

void
_client_local_config(const unsigned int a, const long b, void *c)
{
#ifdef CLIENTSRC
    client_input(CONFIG, 0);
    while (netget(0) != '\n') ;
#else
    cprintf("%s%s%s%s%s%s",
    "\n\n\1f\1rThis option is disabled since you are not using the client.",
	    "\nThe client is available at:\n\n",
	    "  http://monolith.yawc.net/bbs/info/download_client.html",
    "\n\nFor information about obtaining the client, or for help installing",
       "\nthe client, ask in Monolith Client>  which is room #15, or ask a",
	    "\nSystem Guide by pressing <shift-Q>.\n\n");
#endif
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
    int control = 0, timescalled = 0, posted = 0, x_s = 0;
    time_t timecall, curtime;
    unsigned int a;
    btmp_t *record;
    forumlist_t *p = NULL;
    config_t frog;

    if ((EQ(user->username, "Guest")) && (usersupp->priv < PRIV_SYSOP)) {
	cprintf("\n\1f\1gThe Guest User.\n\1a");
	return;
    }
    curtime = time(0);
    timecall = ((curtime - user->laston_from) / 60);

    if (EQ(viewing_user->username, user->username))
	control = 1;

    mono_sql_uf_list_hosts_by_user(user->usernum, &p);

    /*
     * Time to rearrange flags a wee bit...
     * print monoholic/vanify flags right behind username and
     * move actual flags to a line of their own (Except newbie flag).
     *
     * First off, print actual priv flags (if they exist)
     */
    (void) _print_priv_flags(user, p);

    /*
     * print username.
     */
    cprintf("\1f\1y%s\1g ", user->username);

    /*
     * Now, print monoholic flag, customisable flags, user status flags...
     */
    (void) _print_user_flags(user);

    /*
     * Continue with rest of profile...
     */
    dis_regis(user, ((viewing_user->priv >= PRIV_SYSOP) || control));

    cprintf("\1f\1cFlying: %s\1a\n", user->doing);
    if (user->timezone)
	if (strlen(user->timezone)) {
	    struct tm *tp;
	    time_t now;
	    char str[40], datestr[60];

	    strcpy(str, user->timezone);
	    set_timezone(str);
	    time(&now);
	    tp = localtime(&now);
	    strftime(datestr, sizeof(datestr) - 1,
		     _("\1f\1cUser's localtime:\1g %A \1w(\1g%H:%M\1w) (%Z)\1a\n"), tp);
	    cprintf("%s", datestr);
	    strcpy(str, usersupp->timezone);
	    set_timezone(str);
	}
    if ((viewing_user->priv >= PRIV_SYSOP) && (strlen(user->aideline) > 0))
	cprintf(_("\1f\1yAideline:%s\n"), user->aideline);
#ifdef OLD
    if (user->flags & US_ROOMAIDE) {
	for (a = 0; a < 5; a++)
	    if (user->RA_rooms[a] >= 0) {
		if (found_RA_rooms == 0) {
		    cprintf("\1c\1f%s in: \1g", ROOMAIDE);
		    found_RA_rooms = 1;
		}
		cprintf("%d ", user->RA_rooms[a]);
	    }
	cprintf("\1a\1f\1c\n");
    }
#endif
    {

	if (p != NULL) {
	    cprintf("\1c\1f%s in: \1g\n", config.roomaide);
	    print_forumlist_list(p);
	    dest_forumlist(p);
	}
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
	cprintf(_("\1f\1cLast on \1g%s "), printdate(user->laston_from, 1));
	cprintf(_("\1cto \1g%s \1p"), printdate(user->laston_to, 2));
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
	cprintf(_("Birthday: \1g"));
	print_birthday(user->birthday);
	cprintf("\1c ");
    }

    if (viewing_user->priv >= PRIV_SYSOP) {	/* a sysop profiles */
	(void) mono_sql_read_config(user->configuration, &frog);
        (void) mono_sql_u_get_login_count(user->usernum, &timescalled);
        (void) mono_sql_u_get_post_count(user->usernum, &posted);
        (void) mono_sql_u_get_x_count(user->usernum, &x_s);

	cprintf(_("\1f\1cConfiguration\1w: \1g%s \1cMonoholic Rating\1w: \1g"), frog.bbsname);
	printf("%.3f", _get_monoholic_rating(user));
	cprintf("\n");
	cprintf(_("\1f\1cUsernum\1w: \1g%d \1cFirst login\1w: \1g%s\n"), user->usernum, printdate(user->firstcall, 1));
	cprintf(_("\1f\1cLogins\1w: \1g%-5d \1cPosts: \1g%-5d \1cOnlinetime: \1g%2ld:%2.2ld   \1cPriv: \1g%5d  \1cX's: \1g%5ld \n"),
		timescalled, posted, user->online / 60, user->online % 60,
		user->priv, x_s);
    } else if (control) {	/* you profile yourself */
        (void) mono_sql_u_get_login_count(user->usernum, &timescalled);
        (void) mono_sql_u_get_post_count(user->usernum, &posted);
        (void) mono_sql_u_get_x_count(user->usernum, &x_s);
	cprintf(_("First login: \1g%s\1c\nLogins: \1g%-5d \1cPosts: \1g%-5d \1cOnlinetime: \1g%2ld:%2.2ld \1cX's: \1g%5ld \n"),
		printdate(user->firstcall, 1), timescalled, posted,
	     user->online / 60, user->online % 60, x_s);
    } else			/* other people profile */
	cprintf("\n");


    record = mono_read_btmp(user->username);
    if (record != NULL) {
	if ((user->flags & US_GUIDE) && (record->flags & B_GUIDEFLAGGED))
	    cprintf(_("\1g*** is available to help others ***\n"));
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

    if (file_line_len(fp) > (viewing_user->screenlength - 15)) {
	fclose(fp);
	cprintf(_("\n\1a\1f\1gPress \1w<\1rSPACE\1w>\1g to continue or \1w<\1rs\1w>\1g to skip... \1a"));
	cmd = get_single_quiet(" sq");
	switch (cmd) {
	    case ' ':
		more(work, TRUE);
		cprintf("\n");
		break;

	    case 's':
	    case 'q':

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
	cprintf(_("\1gYou're not a %s!"), GUIDE);
	return;
    }
    tmpbtmp = mono_read_btmp(who_am_i(NULL));

    if (tmpbtmp == NULL)
	return;

    if ((tmpbtmp->flags & B_GUIDEFLAGGED) && (i == 0 || i == -1)) {
	if (usersupp->flags & US_ADMINHELP)
	    more(HELPTERM "/toggle_off", 1);
	mono_change_online(who_am_i(NULL), "", -7);	/* deguideflagize */
	change = -1;
    } else if ((!(tmpbtmp->flags & B_GUIDEFLAGGED)) && (i == 0 || i == 1)) {
	if (usersupp->flags & US_ADMINHELP)
	    more(HELPTERM "/toggle_on", 1);
	mono_change_online(who_am_i(NULL), "", 7);	/* guideflagize */
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
    cprintf("%s%s%s%s%s%s",
	    "\n\nTest Paragraph 1:\n   Now, we're going to check and ",
	    "see if your terminal programme is\n   capable of ",
	    "displaying colours properly.  This paragraph is being\n",
	    "   displayed with the ANSI colours off.  This paragraph",
	    " should\n   look correct.\n\n",
	    "Press a key to continue.. ");
    inkey();
    user->flags ^= US_ANSI;
    cprintf("\n\n\1f\1cToggling ANSI colour mode configuration..");
    cprintf(" Currently set to: %s\1c.\1a\n",
	    (user->flags & US_ANSI) ? "\1rON" : "\1gOFF");
    cprintf("%s%s%s%s%s",
	    "\n\nTest Paragraph 2:\n\1f\1g   Now, we're going to check",
	    " and see if your terminal programme is\n   capable of ",
	    "displaying colours properly.  This paragraph is being\n",
	    "   displayed with the \1cA\1bN\1cS\1gI colours \1rON\1g.",
	    " This paragraph might\n   not look correct.\1a\n\n");

    user->flags ^= US_ANSI;

    cprintf("%s%s%s%s",
	    "-  In the second paragraph if you see things like  ",
	    "w^[[31;0m  and display is   -\n-  NOT in color, then ",
	    "I'm sorry, your terminal program can't handle colour.",
	    "  -\n\n");

    cprintf("Did the Test Paragraph 2 display properly? (y/n) ");
    if (yesno() == YES)
	user->flags ^= US_ANSI;
    cprintf("\n\n\1f\1gANSI color configuration set to: %s\1g.\1a\n",
	    (user->flags & US_ANSI) ? "\1rON" : "\1gOFF");
    return;
}


void
_edit_profile(const unsigned int a, const long b, void *c)
{
    FILE *fp = NULL;
    char filename[L_FILENAME], *profile = NULL;

    cprintf(_("\1f\1gAre you sure you want to edit your profile? \1w(\1gy\1w/\1gn\1w) \1c"));
    if (yesno() == NO) {
	return;
    }
    sprintf(filename, "%s/profile", getuserdir(who_am_i(NULL)));

    /*
     * Get profile data from SQL and store in file.
     */
    if ((fp = fopen(filename, "w")) == NULL) {
	log_it("errors", "Can't open profile: %s", filename);
	cprintf(_("\1f\1rUnable to fopen() profile file!\n"));
	return;
    }
    if ((mono_sql_read_profile(usersupp->usernum, &profile)) == -1) {
	xfree(profile);
	log_it("errors", "mono_sql_read_profile() returned -1");
	cprintf(_("\1f\1rUnble to fetch profile from database!\n"));
	return;
    }
    fprintf(fp, "%s", profile);
    fclose(fp);
    xfree(profile);

    /*
     * Edit file.
     */
    editor_edit(filename);

    /*
     * Dump profile data back into database.
     */
    if ((profile = map_file(filename)) == NULL) {
	xfree(profile);
	log_it("errors", "Can't edit profile: %s", filename);
	cprintf(_("\1f\1rUnable to mmap() profile file contents!\n"));
	return;
    }
    if ((mono_sql_write_profile(usersupp->usernum, profile)) == -1) {
	xfree(profile);
	log_it("errors", "mono_sql_write_profile() returned -1");
	cprintf(_("\1f\1rUnable to store profile in database!\n"));
	return;
    }
    xfree(profile);
    check_profile_updated();
    return;
}


void
_change_profile(const unsigned int a, const long b, void *c)
{
    int i;
    char work[560], *profile = NULL, filename[L_FILENAME];
#ifndef CLIENTSRC
    char tempstr[300];
#endif

    cprintf(_("Are you sure you want to change your profile? (y/n) "));
    if (yesno() == NO) {
	return;
    }
    cprintf(_("\1yYou now have seven lines to create a nice profile of yourself.\1g\n\n"));

#ifdef CLIENTSRC
    client_input(G_LINES, 2);

    for (i = 0; i < 560 && (i == 0 || work[i - 1]); i++)
	work[i] = netget(0);
    work[i - 3] = '\0';
#else
    strcpy(work, "");
    for (i = 0; i < 7 && (i == 0 || tempstr[0]); i++) {
	cprintf(">");
	get_string_wr(78, tempstr, i, 1);
	strcat(work, tempstr);
	strcat(work, "\n");	/* better within the brackets: PR */
    }
#endif

    if (write_profile(who_am_i(NULL), work) == -1) {
	cprintf("\1r\1fCould not save your profile.\n\1a");
	return;
    }
    sprintf(filename, "%s/profile", getuserdir(who_am_i(NULL)));
    if ((profile = map_file(filename)) == NULL) {
	xfree(profile);
	log_it("errors", "Can't save profile: %s", filename);
	cprintf(_("\1f\1rUnable to mmap() profile file contents!\n"));
	return;
    }
    if ((mono_sql_write_profile(usersupp->usernum, profile)) == -1) {
	xfree(profile);
	log_it("errors", "mono_sql_write_profile() returned -1");
	cprintf(_("\1f\1rUnable to store profile in database!\n"));
	return;
    }
    xfree(profile);
    check_profile_updated();
}


void
_change_birthday(const unsigned int a, const long b, void *c)
{
    cprintf(_("\1f\1gChange birthday/date.\n"));
    modify_birthday(&(usersupp->birthday));
}


void
_change_flying(const unsigned int a, const long b, void *c)
{
    cprintf(_("\1gYour current %s is: \1y`%s\1a'\n"), config.doing, usersupp->doing);
    cprintf(_("\1f\1gDo you want to change this? \1w(\1gy\1w/\1gn\1w)\1c "));
    if (yesno() == YES) {
	cprintf("\1gEnter the new %s\1w: \1f\1c", config.doing);
	getline(usersupp->doing, 28, 0);
	usersupp->doing[28] = '\0';
	writeuser(usersupp, 0);
	mono_sql_u_update_doing( usersupp->usernum, usersupp->doing );
	mono_change_online(who_am_i(NULL), usersupp->doing, 14);
	cprintf("\1a");
    }
    return;
}

void
_change_address(const unsigned int a, const long b, void *c)
{
    cprintf(_("\1f\1gChange address.\n"));
    if ((usersupp->priv & PRIV_DEGRADED) || (!(usersupp->priv & PRIV_VALIDATED))) {
	enter_reginfo();
    } else
	cprintf("\1y\1f%s%s",
	   "You can not change your address info once you are validated.\n",
	"\1y(You can \1w<\1ry\1w>\1yell to ask the sysops to change it.)\n");

}

void
_change_url(const unsigned int a, const long b, void *c)
{
    char p[RGurlLEN];

    cprintf(_("\1gYour current url is: \1y`%s'\n"), usersupp->RGurl);
    cprintf(_("\1gDo you want to change this? \1w(\1gy\1w/\1gn\1w)\1c "));
    if (yesno() == YES) {
	cprintf(_("\1f\1gEnter the new url\1w: \1chttp://"));
	getline(p, RGurlLEN - 7, TRUE);
	if (strlen(p) < 5) {
	    strcpy(usersupp->RGurl, "");
	    cprintf(_("\1f\1gURL not set.\n"));
	} else {
	    sprintf(usersupp->RGurl, "http://%s", p);
	    mono_sql_u_update_url(usersupp->usernum, usersupp->RGurl);
	}

	cprintf("\1a");
    }
}

void
_change_password(const unsigned int a, const long b, void *c)
{
    do_changepw();
}

/**********************************************
* q_menu()
**********************************************/

void
q_menu(void)
{

    register char cmd = '\0';

    nox = 1;			/* disable x's in q_menu */

    cprintf(_("\n\1f\1gPress \1w<\1rh\1w>\1g to access the helpfiles.\n"));
    cprintf(_("\n\1gPress \1w<\1rc\1w>\1g for help on commands.\n"));
    cprintf(_("\1gPress \1w<\1r?\1w>\1g to get a list of commands.\n"));
    cprintf(_("\1gPress \1w<\1rshift-Q\1w>\1g to ask a Question.\n\n"));
    cprintf(_("\1gPress \1w<\1rq\1w>\1g to exit this menu.\n\n"));
    cprintf(_("\1gChoice\1w:\1g "));

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
	cprintf("\1w%s is not on %s BBS.\n", BBSNAME, name);
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

    mono_change_online(who_am_i(NULL), " ", 13);	/* toggle away */
    cmdflags ^= C_AWAY;
    user = mono_read_btmp(username);	/* find out what's the toggle is */

    if (user == NULL)
	return;

    if (user->flags & B_AWAY) {
	cprintf(_("\1f\1wYou are now marked as \1gAWAY\1w.\n"));
	if (strlen(usersupp->awaymsg))
	    cprintf(_("\1gDefault away message: \1y%s\1a\n"), usersupp->awaymsg);
	cprintf(_("\1f\1gNew Away message: \1c"));
	getline(away, 99, TRUE);
	if (strlen(away)) {
	    strcpy(usersupp->awaymsg, away);
  	    mono_sql_u_update_awaymsg(usersupp->usernum, away );
        }
	mono_change_online(who_am_i(NULL), usersupp->awaymsg, 16);
	change_atho(-1);
    } else {
	cprintf(_("\1f\1wYou are \1rno longer\1w marked as \1gAWAY\1w.\1a"));
	mono_change_online(who_am_i(NULL), "", 16);
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
    cprintf("Your %sBeeps are now %sabled.", config.express,
	    usersupp->flags & US_BEEP ? "\1gen" : "\1rdis");
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
    cprintf("\1f\1wYour InterBBS access is now %sabled\1w.\1a",
	    usersupp->flags & US_NOINTERXS ? "\1rdis" : "\1gen");
    writeuser(usersupp, 0);
    return;
}


void
_key_menu_wrapper(const unsigned int a, const long b, void *c)
{
    cprintf("\1f\1gKey Menu.\n");
    key_menu();
}


void
_change_host(const unsigned int a, const long b, void *c)
{

    char newb[17];

    nox = TRUE;
    cprintf("\1gDo you really want to change your location? (y/n) ");
    if (yesno() == YES) {
	cprintf("New location: ");
	getline(newb, 16, 0);
	strremcol(newb);
	mono_change_online(who_am_i(NULL), newb, 4);
    }
    if (usersupp->priv & PRIV_WIZARD) {
	cprintf("\1gDo you also want to change the 'online from' ? (y/n) ");
	if (yesno() == YES) {
	    cprintf("New lasthost: ");
	    getline(newb, 19, 0);
	    strcpy(who_am_i(NULL), newb);
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

void
_change_alias(const unsigned int a, const long b, void *c)
{
    char aliasstr[L_USERNAME + 1];

    if (strlen(usersupp->alias)) {
	cprintf("\1f\1gYour current alias is \1y`%s'\1g. Do you want to change it? \1w(\1gy\1w/\1gn\1w) \1c", usersupp->alias);
	if (yesno() == NO) {
	    cprintf("\1f\1gAlias unchanged.\1a\n");
	    return;
	}
    }
    cprintf("\1f\1gEnter the new alias\1w: \1c");

    getline(aliasstr, L_USERNAME, 0);

    if (aliasstr[0] == '\0') {
	cprintf("\1f\1gAlias \1rnot\1g set.\1a\n");
	strcpy(usersupp->alias, "");
	return;
    } else {
	if (mono_sql_u_check_user(aliasstr) == TRUE) {
	    if (EQ(usersupp->username, aliasstr)) {
		strcpy(usersupp->alias, aliasstr);
		cprintf("\1f\1gYour alias is set to \1y`%s'\1g.\n", usersupp->alias);
		return;
	    } else {
		cprintf("\1f\1rThat is an existing user!\1a\n");
		return;
	    }
	}
    }
    strcpy(usersupp->alias, aliasstr);
    cprintf("\1f\1gYour alias is set to \1y`%s'\1g.\n", usersupp->alias);
}


/*  config options statics: */
void
set_usersupp_config_flag(const unsigned int mask, const long foo,
			 void *flagname)
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
set_usersupp_flag(const unsigned int mask, const long reverse, void *flagname)
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
_hidden_info_menu(const unsigned int a, const long b, void *c)
{
    cprintf("\1f\1cToggle hidden info.\1a\n");
    toggle_hidden_info(usersupp);
    mono_sql_u_update_hidden(usersupp->usernum, usersupp->hidden_info);
}

void
_set_screenlength(const unsigned int x, const long y, void *z)
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
_tgl_status_bar(const unsigned int a, const long b, void *c)
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
_tgl_use_alias(const unsigned int x, const long y, void *z)
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
_tgl_silc(const unsigned int x, const long y, void *z)
{
    IFSYSOP
    {
	cprintf("\1f\1cToggle SILC.\1a\n");
	cmdflags ^= C_NOSILC;
    }
}

/*
 * Get locale array id (ugly & nasty but it works for the moment.)
 * Must at some stage rewrite this lot to use proper system locales.
 */
static unsigned int
_get_locale(const unsigned long config_flags)
{
    if (config_flags & CO_EUROPEANDATE)
	return 0;
    return 1;
}

/*
 * Toggle date format.
 */
static void
_set_locale(const unsigned int frog, const long kiss, void *prince)
{
    usersupp->config_flags ^= CO_EUROPEANDATE;
    cprintf("\n\1f\1cUsing %s date format.", _locale[_get_locale(usersupp->config_flags)]);
    return;
}

/*
 * Toggle date display.
 */
static void
_set_date_display(const unsigned int bongo, const long bouncing, void *horse)
{
    usersupp->config_flags ^= CO_LONGDATE;
    cprintf("\n\1f\1cDate display set to %s\1c.", (usersupp->config_flags & CO_LONGDATE) ? "\1wlong" : "\1wshort");
    return;
}

static float
_get_monoholic_rating(const user_t * user)
{
    return ((float) user->posted / (float) user->timescalled) + ((float) user->x_s / (float) user->timescalled / 100) + ((float) user->timescalled / 4000) + ((float) user->priv / 20000);
}

static char *
_get_monoholic_flag(const user_t * user)
{
    float var = 0;

    var = _get_monoholic_rating(user);

    if ((var < 1.8) || (user->timescalled < 666))
	return "";
    if (var < 1.8)
	return "\1f\1w* \1yAspiring Monoholic \1w* ";
    if (var < 2)
	return "\1f\1w* \1yMonoholic \1w* ";
    if (var < 2.5)
	return "\1f\1w* \1gDetermined Monoholic \1w* ";
    if (var < 3)
	return "\1f\1w* \1gDedicated Monoholic \1w* ";
    if (var < 3.5)
	return "\1f\1w* \1gCommitted Monoholic \1w* ";
    if (var < 4)
	return "\1f\1w* \1gGreat Monoholic \1w* ";
    if (var < 5)
	return "\1f\1w* \1gGrand Monoholic \1w* ";
    if (var < 6)
	return "\1f\1w* \1gHigh Monoholic \1w* ";
    if (var < 7)
	return "\1f\1w* \1gSupreme Monoholic \1w* ";
    if (var < 8)
	return "\1f\1w* \1bGreat High Monoholic \1w* ";
    if (var < 9)
	return "\1f\1w* \1bGrand High Monoholic \1w* ";
    if (var < 10)
	return "\1f\1w* \1bSupreme High Monoholic \1w* ";
    if (var < 11)
	return "\1f\1w* \1bExalted Great Monoholic \1w* ";
    if (var < 12)
	return "\1f\1w* \1bExalted Grand Monoholic \1w* ";
    if (var < 13)
	return "\1f\1w* \1bExalted High Monoholic \1w* ";
    if (var < 14)
	return "\1f\1w* \1bExalted Supreme Monoholic \1w* ";
    if (var < 15)
	return "\1f\1w* \1pSupreme Exalted High Monoholic \1w* ";
    if (var < 16)
	return "\1f\1w* \1pSupreme Exalted Great Monoholic \1w* ";
    if (var < 17)
	return "\1f\1w* \1pSupreme Exalted Grand Monoholic \1w* ";
    if (var < 18)
	return "\1f\1w* \1pSupreme Exalted High Monoholic \1w* ";
    return "\1f\1w(* \1rPenultimate Monoholic \1w*)";

}

static void
_print_priv_flags(const user_t * user, forumlist_t * p)
{
    cprintf("\n");
    if (user->priv & PRIV_WIZARD)
	cprintf("\1f\1w(*** %s%s \1w***) ", WIZARDCOL, config.wizard);
    else if (user->priv & PRIV_SYSOP)
	cprintf("\1f\1w(** %s%s \1w**) ", SYSOPCOL, config.sysop);
    else if (user->priv & PRIV_TECHNICIAN)
	cprintf("1f\1w(* %s%s \1w*) ", PROGRAMMERCOL, config.programmer);
#ifndef OLD
    else if (p != NULL)
#else
    else if (user->flags & US_ROOMAIDE)
#endif
	cprintf("\1f\1w(* %s%s \1w*) ", ROOMAIDECOL, config.roomaide);
    if (user->flags & US_GUIDE)
	cprintf("\1f\1w( %s%s \1w) ", GUIDECOL, config.guide);
    cprintf("\1f");
    if (user->flags & US_COOL)
	cprintf("\1f\1w*COOL AS ICE* ");
    if (user->flags & US_DONATOR)
	cprintf("\1g$ DONATOR $ ");
    cprintf("\n\1f\1g");
    return;
}

static void
_print_user_flags(const user_t * user)
{

    int ret;
    char xtrapflag[80];

#ifndef NO_MONOHOLIC_FLAG
    cprintf("%s ", _get_monoholic_flag(user));
#endif

    if (user->priv & PRIV_NEWBIE)
	cprintf("\1w- \1yNEW \1w- ", GUIDECOL, config.guide);

    if (!(user->priv & PRIV_VALIDATED))
	cprintf("\1w- \1yUnvalidated \1w- ");

    if (user->priv & PRIV_DEGRADED)
	cprintf("\1w[ \1rDegraded \1w] ");

    if (user->priv & PRIV_TWIT)
	cprintf("\1w[ \1rCURSED \1w] ");

    if (user->priv & PRIV_DELETED)
	cprintf("\1w[ \1rDeleted \1w] ");

    IFSYSOP {
	if (user->flags & US_LAG)
	    cprintf("\1r[ \1wLAG \1r] ");
    }

    IFWIZARD {
	if (user->flags & US_IAMBAD)
	    cprintf("\1r[ \1wX-LOG\1r ] ");
    }

    ret = mono_sql_u_get_xtrapflag( user->usernum, xtrapflag );
    if ( ret == 0 ) cprintf("\1f%s", user->xtrapflag);

    cprintf("\n\1a\1g");
    return;
}


void
_timezone_menu(const unsigned int a, const long b, void *c)
{

    static void _tzones_africa(const unsigned int, const long, void *);
    static void _tzones_america(const unsigned int, const long, void *);
    static void _tzones_antarctica(const unsigned int, const long, void *);
    static void _tzones_asia(const unsigned int, const long, void *);
    static void _tzones_australlia(const unsigned int, const long, void *);
    static void _tzones_europe(const unsigned int, const long, void *);

    MENU_DECLARE;
    MENU_INIT;
    nox = 1;

    MENU_ADDITEM(_tzones_africa, 0, 0, NULL, "t", "Africa");
    MENU_ADDITEM(_tzones_america, 0, 0, NULL, "t", "America");
    MENU_ADDITEM(_tzones_antarctica, 0, 0, NULL, "t", "Antarctica");
    MENU_ADDITEM(_tzones_asia, 0, 0, NULL, "t", "Asia");
    MENU_ADDITEM(_tzones_australlia, 0, 0, NULL, "t", "Australia");
    MENU_ADDITEM(_tzones_europe, 0, 0, NULL, "t", "Europe");

    strcpy(the_menu_format.menu_title, "\1f\1g\nTimezone Setup Menu:  Continent\n\n");
    the_menu_format.gen_1_idx = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gSelect Continent: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;

    if (strlen(usersupp->timezone)) {
	char str[40];
	strcpy(str, usersupp->timezone);
	set_timezone(str);
	cprintf("\n\1f\1gTimezone set to \1w[\1y%s\1w]\1g\1c\n", usersupp->timezone);
    } else
	cprintf("\1f\1gTimezone not set, using %s timezone.", BBSNAME);
}

static void _tz2str(const unsigned int, const long, void *);

void
_tzones_africa(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":Africa/Abidjan");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Abidjan");
    MK_TMPSTR(":Africa/Accra");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Accra");
    MK_TMPSTR(":Africa/Addis_Ababa");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Addis Ababa");
    MK_TMPSTR(":Africa/Algiers");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Algiers");
    MK_TMPSTR(":Africa/Asmera");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Asmera");
    MK_TMPSTR(":Africa/Bamako");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bamako");
    MK_TMPSTR(":Africa/Bangui");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bangui");
    MK_TMPSTR(":Africa/Banjul");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Banjul");
    MK_TMPSTR(":Africa/Bissau");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bissau");
    MK_TMPSTR(":Africa/Blantyre");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Blantyre");
    MK_TMPSTR(":Africa/Brazzaville");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Brazzaville");
    MK_TMPSTR(":Africa/Bujumbura");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bujumbura");
    MK_TMPSTR(":Africa/Cairo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Cairo");
    MK_TMPSTR(":Africa/Casablanca");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Casablanca");
    MK_TMPSTR(":Africa/Ceuta");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ceuta");
    MK_TMPSTR(":Africa/Conakry");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Conakry");
    MK_TMPSTR(":Africa/Dakar");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dakar");
    MK_TMPSTR(":Africa/Dar_es_Salaam");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dar es Salaam");
    MK_TMPSTR(":Africa/Djibouti");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Djibouti");
    MK_TMPSTR(":Africa/Douala");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Douala");
    MK_TMPSTR(":Africa/El_Aaiun");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "El Aaiun");
    MK_TMPSTR(":Africa/Freetown");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Freetown");
    MK_TMPSTR(":Africa/Gaborone");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Gaborone");
    MK_TMPSTR(":Africa/Harare");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Harare");
    MK_TMPSTR(":Africa/Johannesburg");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Johannesburg");
    MK_TMPSTR(":Africa/Kampala");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kampala");
    MK_TMPSTR(":Africa/Khartoum");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Khartoum");
    MK_TMPSTR(":Africa/Kigali");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kigali");
    MK_TMPSTR(":Africa/Kinshasa");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kinshasa");
    MK_TMPSTR(":Africa/Lagos");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lagos");
    MK_TMPSTR(":Africa/Libreville");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Libreville");
    MK_TMPSTR(":Africa/Lome");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lome");
    MK_TMPSTR(":Africa/Luanda");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Luanda");
    MK_TMPSTR(":Africa/Lubumbashi");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lubumbashi");
    MK_TMPSTR(":Africa/Lusaka");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lusaka");
    MK_TMPSTR(":Africa/Malabo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Malabo");
    MK_TMPSTR(":Africa/Maputo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Maputo");
    MK_TMPSTR(":Africa/Maseru");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Maseru");
    MK_TMPSTR(":Africa/Mbabane");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Mbabane");
    MK_TMPSTR(":Africa/Mogadishu");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Mogadishu");
    MK_TMPSTR(":Africa/Monrovia");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Monrovia");
    MK_TMPSTR(":Africa/Nairobi");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nairobi");
    MK_TMPSTR(":Africa/Ndjamena");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ndjamena");
    MK_TMPSTR(":Africa/Niamey");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Niamey");
    MK_TMPSTR(":Africa/Nouakchott");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nouakchott");
    MK_TMPSTR(":Africa/Ouagadougou");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ouagadougou");
    MK_TMPSTR(":Africa/Porto-Novo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Porto-Novo");
    MK_TMPSTR(":Africa/Sao_Tome");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Sao Tome");
    MK_TMPSTR(":Africa/Timbuktu");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Timbuktu");
    MK_TMPSTR(":Africa/Tripoli");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tripoli");
    MK_TMPSTR(":Africa/Tunis");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tunis");
    MK_TMPSTR(":Africa/Windhoek");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Windhoek");

    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:  City/Country\n\n");
    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gCity/Country in your Timezone: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}

void
_tzones_america(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":America/Anchorage");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Anchorage");
    MK_TMPSTR(":America/Antigua");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Antigua");
    MK_TMPSTR(":America/Aruba");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Aruba");
    MK_TMPSTR(":America/Barbados");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Barbados");
    MK_TMPSTR(":America/Bogota");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bogota");
    MK_TMPSTR(":America/Buenos_Aires");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Buenos Aires");
    MK_TMPSTR(":America/Caracas");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Caracas");
    MK_TMPSTR(":America/Cayman");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Cayman");
    MK_TMPSTR(":America/Chicago");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Chicago");
    MK_TMPSTR(":America/Costa_Rica");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Costa Rica");
    MK_TMPSTR(":America/Dawson");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dawson");
    MK_TMPSTR(":America/Denver");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Denver");
    MK_TMPSTR(":America/Detroit");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Detroit");
    MK_TMPSTR(":America/Dominica");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dominica");
    MK_TMPSTR(":America/Edmonton");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Edmonton");
    MK_TMPSTR(":America/El_Salvador");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "El Salvador");
    MK_TMPSTR(":America/Grenada");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Grenada");
    MK_TMPSTR(":America/Guadeloupe");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Guadeloupe");
    MK_TMPSTR(":America/Guatemala");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Guatemala");
    MK_TMPSTR(":America/Guyana");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Guyana");
    MK_TMPSTR(":America/Halifax");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Halifax");
    MK_TMPSTR(":America/Havana");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Havana");
    MK_TMPSTR(":America/Indianapolis");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Indianapolis");
    MK_TMPSTR(":America/Jamaica");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Jamaica");
    MK_TMPSTR(":America/Juneau");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Juneau");
    MK_TMPSTR(":America/Lima");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lima");
    MK_TMPSTR(":America/Los_Angeles");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Los Angeles");
    MK_TMPSTR(":America/Louisville");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Louisville");
    MK_TMPSTR(":America/Martinique");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Martinique");
    MK_TMPSTR(":America/Mexico_City");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Mexico City");
    MK_TMPSTR(":America/Montreal");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Montreal");
    MK_TMPSTR(":America/Montserrat");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Montserrat");
    MK_TMPSTR(":America/Nassau");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nassau");
    MK_TMPSTR(":America/New_York");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "New York");
    MK_TMPSTR(":America/Nipigon");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nipigon");
    MK_TMPSTR(":America/Nome");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nome");
    MK_TMPSTR(":America/Panama");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Panama");
    MK_TMPSTR(":America/Phoenix");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Phoenix");
    MK_TMPSTR(":America/Puerto_Rico");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Puerto Rico");
    MK_TMPSTR(":America/Rainy_River");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Rainy River");
    MK_TMPSTR(":America/Regina");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Regina");
    MK_TMPSTR(":America/Sao_Paulo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Sao Paulo");
    MK_TMPSTR(":America/St_Johns");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "St Johns");
    MK_TMPSTR(":America/St_Thomas");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "St Thomas");
    MK_TMPSTR(":America/St_Vincent");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "St Vincent");
    MK_TMPSTR(":America/Thunder_Bay");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Thunder Bay");
    MK_TMPSTR(":America/Tijuana");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tijuana");
    MK_TMPSTR(":America/Thunder_Bay");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Toronto");
    MK_TMPSTR(":America/Vancouver");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vancouver");
    MK_TMPSTR(":America/Whitehorse");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Whitehorse");
    MK_TMPSTR(":America/Winnipeg");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Winnipeg");
    MK_TMPSTR(":America/Yellowknife");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Yellowknife");

    the_menu_format.gen_1_idx = 1;
    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:  City/Country\n\n");
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gCity/Country in your Timezone: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}

void
_tzones_antarctica(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":Antarctica/Casey");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Casey");
    MK_TMPSTR(":Antarctica/DumontDUrville");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "DumontDUrville");
    MK_TMPSTR(":Antarctica/Mawson");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Mawson");
    MK_TMPSTR(":Antarctica/McMurdo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "McMurdo");
    MK_TMPSTR(":Antarctica/Palmer");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Palmer");
    MK_TMPSTR(":Antarctica/South_Pole");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "South Pole");

    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:\n\n");
    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1cCool!\1g  Antarctica region: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}

void
_tzones_asia(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":Asia/Amman");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Amman");
    MK_TMPSTR(":Asia/Ashkhabad");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ashkhabad");
    MK_TMPSTR(":Asia/Baghdad");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Baghdad");
    MK_TMPSTR(":Asia/Bahrain");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bahrain");
    MK_TMPSTR(":Asia/Baku");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Baku");
    MK_TMPSTR(":Asia/Bangkok");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bangkok");
    MK_TMPSTR(":Asia/Beirut");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Beirut");
    MK_TMPSTR(":Asia/Brunei");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Brunei");
    MK_TMPSTR(":Asia/Calcutta");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Calcutta");
    MK_TMPSTR(":Asia/Chungking");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Chungking");
    MK_TMPSTR(":Asia/Dacca");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dacca");
    MK_TMPSTR(":Asia/Damascus");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Damascus");
    MK_TMPSTR(":Asia/Dushanbe");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dushanbe");
    MK_TMPSTR(":Asia/Gaza");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Gaza");
    MK_TMPSTR(":Asia/Hong_Kong");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Hong Kong");
    MK_TMPSTR(":Asia/Irkutsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Irkutsk");
    MK_TMPSTR(":Asia/Ishigaki");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ishigaki");
    MK_TMPSTR(":Asia/Istanbul");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Istanbul");
    MK_TMPSTR(":Asia/Jakarta");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Jakarta");
    MK_TMPSTR(":Asia/Jerusalem");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Jerusalem");
    MK_TMPSTR(":Asia/Kabul");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kabul");
    MK_TMPSTR(":Asia/Kamchatka");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kamchatka");
    MK_TMPSTR(":Asia/Karachi");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Karachi");
    MK_TMPSTR(":Asia/Kashgar");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kashgar");
    MK_TMPSTR(":Asia/Katmandu");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Katmandu");
    MK_TMPSTR(":Asia/Krasnoyarsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Krasnoyarsk");
    MK_TMPSTR(":Asia/Kuala_Lumpur");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kuala Lumpur");
    MK_TMPSTR(":Asia/Kuching");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kuching");
    MK_TMPSTR(":Asia/Kuwait");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kuwait");
    MK_TMPSTR(":Asia/Magadan");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Magadan");
    MK_TMPSTR(":Asia/Manila");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Manila");
    MK_TMPSTR(":Asia/Muscat");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Muscat");
    MK_TMPSTR(":Asia/Nicosia");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Nicosia");
    MK_TMPSTR(":Asia/Novosibirsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Novosibirsk");
    MK_TMPSTR(":Asia/Omsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Omsk");
    MK_TMPSTR(":Asia/Phnom_Penh");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Phnom Penh");
    MK_TMPSTR(":Asia/Pyongyang");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Pyongyang");
    MK_TMPSTR(":Asia/Qatar");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Qatar");
    MK_TMPSTR(":Asia/Rangoon");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Rangoon");
    MK_TMPSTR(":Asia/Riyadh");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Riyadh");
    MK_TMPSTR(":Asia/Saigon");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Saigon");
    MK_TMPSTR(":Asia/Seoul");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Seoul");
    MK_TMPSTR(":Asia/Shanghai");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Shanghai");
    MK_TMPSTR(":Asia/Singapore");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Singapore");
    MK_TMPSTR(":Asia/Taipei");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Taipei");
    MK_TMPSTR(":Asia/Tashkent");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tashkent");
    MK_TMPSTR(":Asia/Tbilisi");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tbilisi");
    MK_TMPSTR(":Asia/Tehran");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tehran");
    MK_TMPSTR(":Asia/Tel_Aviv");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tel Aviv");
    MK_TMPSTR(":Asia/Tokyo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tokyo");
    MK_TMPSTR(":Asia/Ulan_Bator");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ulan Bator");
    MK_TMPSTR(":Asia/Vientiane");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vientiane");
    MK_TMPSTR(":Asia/Vladivostok");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vladivostok");
    MK_TMPSTR(":Asia/Yakutsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Yakutsk");
    MK_TMPSTR(":Asia/Yekaterinburg");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Yekaterinburg");
    MK_TMPSTR(":Asia/Yerevan");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Yerevan");

    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:  City/Country\n\n");
    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gCity/Country in your Timezone: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}


void
_tzones_australlia(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":Australia/ACT");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "ACT");
    MK_TMPSTR(":Australia/Adelaide");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Adelaide");
    MK_TMPSTR(":Australia/Brisbane");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Brisbane");
    MK_TMPSTR(":Australia/Broken_Hill");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Broken Hill");
    MK_TMPSTR(":Australia/Canberra");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Canberra");
    MK_TMPSTR(":Australia/Darwin");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Darwin");
    MK_TMPSTR(":Australia/Hobart");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Hobart");
    MK_TMPSTR(":Australia/LHI");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "LHI");
    MK_TMPSTR(":Australia/Lindeman");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lindeman");
    MK_TMPSTR(":Australia/Lord_Howe");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lord Howe");
    MK_TMPSTR(":Australia/Melbourne");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Melbourne");
    MK_TMPSTR(":Australia/NSW");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "NSW");
    MK_TMPSTR(":Australia/North");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "North");
    MK_TMPSTR(":Australia/Perth");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Perth");
    MK_TMPSTR(":Australia/Queensland");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Queensland");
    MK_TMPSTR(":Australia/South");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "South");
    MK_TMPSTR(":Australia/Sydney");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Sydney");
    MK_TMPSTR(":Australia/Tasmania");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tasmania");
    MK_TMPSTR(":Australia/Victoria");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Victoria");
    MK_TMPSTR(":Australia/West");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "West");
    MK_TMPSTR(":Australia/Yancowinna");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Yancowinna");

    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:  City/Region\n\n");
    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gCity/Region in your Timezone: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}

void
_tzones_europe(const unsigned int a, const long b, void *c)
{
    char *tmpstr;
    MENU_DECLARE;
    MENU_INIT;

    MK_TMPSTR(":Europe/Amsterdam");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Amsterdam");
    MK_TMPSTR(":Europe/Andorra");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Andorra");
    MK_TMPSTR(":Europe/Athens");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Athens");
    MK_TMPSTR(":Europe/Belfast");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Belfast");
    MK_TMPSTR(":Europe/Belgrade");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Belgrade");
    MK_TMPSTR(":Europe/Berlin");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Berlin");
    MK_TMPSTR(":Europe/Bratislava");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bratislava");
    MK_TMPSTR(":Europe/Brussels");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Brussels");
    MK_TMPSTR(":Europe/Bucharest");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Bucharest");
    MK_TMPSTR(":Europe/Budapest");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Budapest");
    MK_TMPSTR(":Europe/Chisinau");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Chisinau");
    MK_TMPSTR(":Europe/Copenhagen");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Copenhagen");
    MK_TMPSTR(":Europe/Dublin");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Dublin");
    MK_TMPSTR(":Europe/Gibraltar");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Gibraltar");
    MK_TMPSTR(":Europe/Helsinki");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Helsinki");
    MK_TMPSTR(":Europe/Istanbul");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Istanbul");
    MK_TMPSTR(":Europe/Kaliningrad");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kaliningrad");
    MK_TMPSTR(":Europe/Kiev");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Kiev");
    MK_TMPSTR(":Europe/Lisbon");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Lisbon");
    MK_TMPSTR(":Europe/Ljubljana");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Ljubljana");
    MK_TMPSTR(":Europe/London");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "London");
    MK_TMPSTR(":Europe/Luxembourg");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Luxembourg");
    MK_TMPSTR(":Europe/Madrid");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Madrid");
    MK_TMPSTR(":Europe/Malta");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Malta");
    MK_TMPSTR(":Europe/Minsk");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Minsk");
    MK_TMPSTR(":Europe/Monaco");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Monaco");
    MK_TMPSTR(":Europe/Moscow");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Moscow");
    MK_TMPSTR(":Europe/Oslo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Oslo");
    MK_TMPSTR(":Europe/Paris");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Paris");
    MK_TMPSTR(":Europe/Prague");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Prague");
    MK_TMPSTR(":Europe/Riga");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Riga");
    MK_TMPSTR(":Europe/Rome");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Rome");
    MK_TMPSTR(":Europe/Samara");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Samara");
    MK_TMPSTR(":Europe/San_Marino");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "San Marino");
    MK_TMPSTR(":Europe/Sarajevo");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Sarajevo");
    MK_TMPSTR(":Europe/Simferopol");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Simferopol");
    MK_TMPSTR(":Europe/Skopje");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Skopje");
    MK_TMPSTR(":Europe/Sofia");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Sofia");
    MK_TMPSTR(":Europe/Stockholm");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Stockholm");
    MK_TMPSTR(":Europe/Tallinn");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tallinn");
    MK_TMPSTR(":Europe/Tirane");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Tirane");
    MK_TMPSTR(":Europe/Vaduz");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vaduz");
    MK_TMPSTR(":Europe/Vatican");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vatican");
    MK_TMPSTR(":Europe/Vienna");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vienna");
    MK_TMPSTR(":Europe/Vilnius");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Vilnius");
    MK_TMPSTR(":Europe/Warsaw");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Warsaw");
    MK_TMPSTR(":Europe/Zagreb");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Zagreb");
    MK_TMPSTR(":Europe/Zurich");
    MENU_ADDITEM(_tz2str, 0, 0, (char *) tmpstr, "t", "Zurich");

    strcpy(the_menu_format.menu_title, "\1f\1g\n\nTimezone Setup Menu:  City/Country\n\n");
    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_prompt,
	   "\1f\1gCity/Country in your Timezone: \1w--");

    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
}

void
_tz2str(const unsigned int a, const long b, void *tzstring)
{
    strcpy(usersupp->timezone, tzstring);
}

#ifdef USE_ICQ

static void
_change_icq_number(const unsigned int a, const long b, void *c)
{

    char tmpstr[16];
    unsigned long icq_number = 0;

    cprintf(_("\1f\1gEnter ICQ Number.\n"));
    if ((usersupp->priv & PRIV_DEGRADED) || (!(usersupp->priv & PRIV_VALIDATED))) {
        more(UNVALIDMSG, 0);
        return;
    }

    if( (icq_number = mono_sql_u_icq_get_number( usersupp->usernum )) <= 0) {
        cprintf(_("\1f\1gYou have not set an ICQ Number.\n"));
        cprintf(_("\1f\1gWould you like to set one now? \1w(\1gy\1w/\1gn\1w)\1c "));
    } else {
        cprintf(_("\1f\1gYour current ICQ Number is \1y%ul\1g\n"), icq_number);
        cprintf(_("\1f\1gDo you want to change this? \1w(\1gy\1w/\1gn\1w)\1c "));
    }
    if (yesno() == YES) {
        cprintf(_("\1gEnter your ICQ number\1w: \1f\1c"));
        strcpy(tmpstr, "");
        getline(tmpstr, 15, 0);
        if(strlen(tmpstr) == 0)
            return;
        cprintf("\1a");
        if((icq_number = atol(tmpstr)) == 0) {
            cprintf(_("\1f\1rSorry, that is not a valid ICQ number.\n"));
            return;
        }
        if( (mono_sql_u_set_icq_number(usersupp->usernum, icq_number)) == -1)
            cprintf(_("\1f\1rSomething went wrong saving your ICQ Number.\n"));
        else
            cprintf(_("\1f\1gYour ICQ Number was saved.\n"));
    }
    return;
}

static void
_change_icq_password(const unsigned int a, const long b, void *c)
{

    char *icq_pw = NULL;

    cprintf(_("\1f\1gEnter ICQ Password.\n"));
    if ((usersupp->priv & PRIV_DEGRADED) || (!(usersupp->priv & PRIV_VALIDATED))) {
        more(UNVALIDMSG, 0);
        return;
    }

    cprintf(_("\1f\1gDo you want to change your ICQ Password? \1w(\1gy\1w/\1gn\1w)\1c "));
    if (yesno() == NO)
        return;

    cprintf(_("\1f\1w(\1gThis password is stored in encrypted form\1w)\n\n"));
    icq_pw = (char *) xmalloc( 65 * sizeof(char) );
    strcpy(icq_pw, "");
    cprintf(_("\1f\1gPlease enter your ICQ Password\1w (\1gMax. 64 characters\1w): \1c"));
    getline(icq_pw, -64, 1);

    if (strlen(icq_pw) == 0) {
        xfree(icq_pw);
        return;
    }

    if( (mono_sql_u_set_icq_pass(usersupp->usernum, icq_pw)) == -1)
        cprintf(_("\1f\1rSomething went wrong saving your ICQ Password.\n"));
    else
        cprintf(_("\1f\1gYour ICQ Password was saved.\n"));

    xfree(icq_pw);
    return;
}

#endif
