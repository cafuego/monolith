/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <ctype.h>
#include <time.h>
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
#include "routines.h"
#include "ext.h" 
#include "setup.h"

#define extern
#include "help.h"
#undef extern

#include "input.h"
#include "routines2.h"
#include "menu.h"

static int help_important(void);
static int help_swedish(void);
static int help_choice(void);
static int help_policy(void);
static int help_communication(void);
static int help_species(void);
static int help_misc(void);
static int help_utils(void);
static int help_admin(void);
static int eat_menu(void);
static int food_menu(void);
static int drinks_menu(void);
static int help_fun(void);
static int idiocy_test(void);
static void shake_comedies(void);
static void shake_tragedies(void);
static void shake_poetry(void);
static void shake_histories(void);
static void other_literature(void);
static int express_commands_menu(void);
static int reading_commands_menu(void);
static int posting_commands_menu(void);
static int config_commands_menu(void);
static int helpish_commands_menu(void);
static int utility_commands_menu(void);

/*-----------------------------------------*/
/*
 * context:
 * l = long prompt
 * s = short prompt
 * c = config menu
 * o = config-options menu
 * e = enemies menu
 * f = friends menu
 * i = misc menu
 * m = main config menu
 * g = general config menu
 * p = message config menu
 * x = express message config menu
 * a = personal information config menu
 * 
 * B = basic
 * I = intermediate
 * A = advanced
 * R = reading
 * P = posting
 * X = message related
 * C = configuration related
 * H = help related
 * U = utility related
 * 
 * NOTE:  If adding helpfiles, keep them in alphabetical order by COMMAND KEY.
 * makes it easier to maintain this.  Config menu stuff might well be indexed
 * separately, to make it easier to keep that up to date.  
 * 
 */
#define M_FILENAME filename=(char *)xmalloc(sizeof(char)*200);\
		   filename[0]='\0';sprintf(filename,"%s%s"

void
online_help_wrapper(unsigned int a, long b, void *string)
{
    char *context;

    context = (char *) string;
    if (strlen(context))
        online_help(context[0]);
}


void
online_help(char context)
{

    MENU_DECLARE;
    char *filename = NULL, mesname[20], tempstr[100];

    strcpy(mesname, "");
    sprintf(mesname, "%s",config.message);
    mesname[0] = toupper(mesname[0]);

    for (;;) {
	MENU_INIT;

	strcpy(the_menu_format.menu_title,
	       "\n\n\1f\1w[\1r Online Help\1w: \1g");
	switch (context) {
	    case 'l':
		strcat(the_menu_format.menu_title, "Long Prompt HELP ");
		break;
	    case 's':
		strcat(the_menu_format.menu_title, "Short Prompt HELP ");
		break;
	    case 'm':
		strcat(the_menu_format.menu_title, "\1w<\1rC\1w>\1gonfig Menu HELP ");
		break;
	    case 'o':
		strcat(the_menu_format.menu_title, "\1w<\1rC\1w>\1gonfig \1w<\1ro\1w>\1gptions Menu ");
		break;
	    case 'g':
		strcat(the_menu_format.menu_title, "General Config HELP ");
		break;
	    case 'p':
		strcat(the_menu_format.menu_title, "Message System Config HELP ");
		break;
	    case 'x':
		strcat(the_menu_format.menu_title, "Express Message Config HELP ");
		break;
	    case 'a':
		strcat(the_menu_format.menu_title, "Personal Info Config HELP ");
		break;
	    case 'e':
		strcat(the_menu_format.menu_title, "Enemies Menu HELP ");
		break;
	    case 'f':
		strcat(the_menu_format.menu_title, "Friends Menu HELP ");
		break;
	    case 'i':
		strcat(the_menu_format.menu_title, "Misc Menu HELP ");
		break;
	    case 'B':
		strcat(the_menu_format.menu_title, "Basic HELP ");
		break;
	    case 'I':
		strcat(the_menu_format.menu_title, "Intermediate HELP ");
		break;
	    case 'A':
		strcat(the_menu_format.menu_title, "Advanced HELP ");
		break;
	    case 'P':
		strcat(the_menu_format.menu_title, "Posting HELP ");
		break;
	    case 'R':
		strcat(the_menu_format.menu_title, "Reading HELP ");
		break;
	    case 'X':
                sprintf(tempstr, "%s %s HELP ", config.express, config.x_message);
		strcat(the_menu_format.menu_title, tempstr);
		break;
	    case 'C':
		strcat(the_menu_format.menu_title, "Account Configuration HELP ");
		break;
	    case 'H':
		strcat(the_menu_format.menu_title, "Help HELP ");
		break;
	    case 'U':
		strcat(the_menu_format.menu_title, "Utility HELP ");
		break;
	}
	strcat(the_menu_format.menu_title, "Commands \1w]\n");
	strcpy(the_menu_format.menu_prompt,
	       "\1f\1rHELP:  \1gPress \1w<\1rkey\1w> \1gfor help \1w--");
/*  Config menu  m */

	if (strchr("m", context)) {
	    M_FILENAME, HELPDIR, "commands/gen_config");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "General Configuration", "G");
	}
	if (strchr("m", context)) {
	    M_FILENAME, HELPDIR, "commands/mes_config");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Message System Configuration", "M");
	}
	if (strchr("m", context)) {
	    M_FILENAME, HELPDIR, "commands/express_config");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Express Message Configuration", "X");
	}
	if (strchr("m", context)) {
	    M_FILENAME, HELPDIR, "commands/info_config");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Personal Info Configuration", "P");
	}

/*  general account config  g */

	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_ansi");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "ANSI Colors Enabled", "1");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_flash");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Flashing text enabled", "2");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_bold");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Bold text enabled", "3");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_showip");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Show hostnames in the wholist", "4");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_statusbar");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Status Bar", "5");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/Config_Llama");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set BBS Appearance", "6");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_screenlen");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set screen length", "7");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_expert");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Expert User Mode", "8");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/co_disable_cmdlist");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Extra command help enabled", "9");
	}
	if (strchr("g", context)) {
	    M_FILENAME, HELPDIR, "commands/CLient");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Configure Client", "C");
	}
/*  message system config  p */

	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/co_lastold");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "See last old post", "0");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/co_promptafter");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Prompt after each post", "1");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/co_pauseafter");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Pause after each screenful", "2");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/co_std_alias");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Use a default alias", "3");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/alias");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set an Alias", "4");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/neatmessages");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Empty lines around messages", "5");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/expand_header");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Expanded message headers", "6");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/date_len");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display short/long date", "7");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/show_deleted");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display deleted messages", "8");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/mono_header");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Monolith-style message headers", "9");
	}
	if (strchr("p", context)) {
	    M_FILENAME, HELPDIR, "commands/date_format");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "American/European date format", "A");
	}

/*  express message config  x */

	if (strchr("x", context)) {
            strcpy(tempstr,"");
            sprintf(tempstr, "Disabled %s at login", config.express);
	    M_FILENAME, HELPDIR, "commands/co_disx");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", tempstr, "1");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/co_frnd_notify");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Receive logon/off notifies", "2");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/co_shix");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "SHIX filter enabled", "3");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/co_interbbs");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "InterBBS access enabled", "4");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/co_show_fr");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Show friends online at login", "5");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/co_all_logons");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Receive all logon events", "6");
	}
	if (strchr("xX", context)) {
	    M_FILENAME, HELPDIR, "commands/friendsls");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit friendslist", "7");
	}
	if (strchr("xX", context)) {
	    M_FILENAME, HELPDIR, "commands/enemylst");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit enemylist", "8");
	}
	if (strchr("x", context)) {
	    M_FILENAME, HELPDIR, "commands/chatconf");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Chat config \1w[\1rmenu\1w]", "9");
	}

/*  personal info config  a */


	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/address_change");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Address Change \1w<\1ry\1w>\1gell", "A");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/birthday");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set birthday", "B");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/password");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set Password", "D");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/co_hideinfo");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Hidden info menu", "H");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/profile");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit Profile", "i");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/edit_profile");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Editor-Edit profile", "I");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/planet");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit Location", "L");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/timezone");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set Timezone menu", "T");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/validation");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Validation Menu", "V");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/url");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set Homepage URL", "W");
	}
	if (strchr("a", context)) {
	    M_FILENAME, HELPDIR, "commands/flying");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit Flying", "Y");
	}

/* end config help */

	if (strchr("lsIX", context)) {
            strcpy(tempstr,"");
            sprintf(tempstr, "Send quick %s", config.express);
	    M_FILENAME, HELPDIR, "commands/quick_x");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", tempstr, "9");
	}
	if (strchr("lRI", context)) {
	    M_FILENAME, HELPDIR, "commands/again");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Read post again", "a");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/add_friend");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Add user your Friends List", "a");
	}
	if (strchr("e", context)) {
	    M_FILENAME, HELPDIR, "commands/add_enemy");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Add user to Enemies List", "a");
	}
	if (strchr("lsIUX", context)) {
	    M_FILENAME, HELPDIR, "commands/away");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Change away status", "A");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/add_quick_xfriend");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Add user to Quick Friends List", "A");
	}
	if (strchr("lRB", context)) {
	    M_FILENAME, HELPDIR, "commands/backwards");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Toggle read direction", "b");
	}
	if (strchr("sXI", context)) {
	    M_FILENAME, HELPDIR, "commands/xbeep");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Toggle X beeps", "B");
	}
	if (strchr("lsXB", context)) {
            strcpy(tempstr,"");
            sprintf(tempstr, "Send Chat %s", config.express);
	    M_FILENAME, HELPDIR, "commands/chatmsg");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", tempstr, "c");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/clear_friends");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Clear Friends List", "c");
	}
	if (strchr("e", context)) {
	    M_FILENAME, HELPDIR, "commands/clear_enemies");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Clear Enemies List", "c");
	}
	if (strchr("sB", context)) {
	    M_FILENAME, HELPDIR, "commands/configmenu");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Configuration \1w[\1rmenu\1w]", "C");
	}
	if (strchr("ls", context)) {
	    M_FILENAME, HELPDIR, "commands/chatconf");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Chat config \1w[\1rmenu\1w]", "d");
	}
	if (strchr("lPI", context)) {
	    M_FILENAME, HELPDIR, "commands/delete");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Delete post", "D");
	}
	if (strchr("o", context)) {
	    M_FILENAME, HELPDIR, "commands/co_hidehost");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Hide your hostname", "D");
	}
	if (strchr("lsPB", context)) {
	    M_FILENAME, HELPDIR, "commands/enter");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Enter a post", "e");
	}			/* post a flame */
	if (strchr("lsPB", context)) {
	    M_FILENAME, HELPDIR, "commands/Editor_post");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Editor-Enter post", "E");
	}
	if (strchr("lRB", context)) {
	    M_FILENAME, HELPDIR, "commands/forward");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Read forward", "f");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/goto");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Goto next Quad", "g");
	}
	if (strchr("lsRPB", context)) {
	    M_FILENAME, HELPDIR, "commands/quadinfo");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Read Quad Rules", "i");
	}
	if (strchr("lsAX", context)) {
	    M_FILENAME, HELPDIR, "commands/intBBS_toggle");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "InterBBS access", "I");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/jump");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Jump to Quad", "j");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/Jump");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Jump through Quad", "J");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/knownlist");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "List your Quads", "k");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/K_unreadrooms");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "List ALL Quads", "K");
	}
	if (strchr("lsBU", context)) {
	    M_FILENAME, HELPDIR, "commands/logoff_mono");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Exit BBS session", "l");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/list_friends");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display Friends List", "l");
	}
	if (strchr("e", context)) {
	    M_FILENAME, HELPDIR, "commands/list_enemies");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display Enemies List", "l");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_lock");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Lock Terminal", "l");
	}
	if (strchr("lsIR", context)) {
	    M_FILENAME, HELPDIR, "commands/unread_rooms");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Unread Quad list", "L");
	}
	if (strchr("lsIU", context)) {
	    M_FILENAME, HELPDIR, "commands/misc");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Misc \1w[\1rmenu\1w]", "m");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_mail");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Mail", "m");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_markall");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Mark all posts as Read", "M");
	}
	if (strchr("lsRB", context)) {
            strcpy(tempstr,"");
            sprintf(tempstr, "Read next %s", config.message);
	    M_FILENAME, HELPDIR, "commands/next");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", tempstr, "n");
	}
	if (strchr("lsRA", context)) {
            strcpy(tempstr,"");
            sprintf(tempstr, "Read old %s", config.message_pl);
	    M_FILENAME, HELPDIR, "commands/old");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", tempstr, "o");
	}
	if (strchr("cCB", context)) {
	    M_FILENAME, HELPDIR, "commands/options");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Set options \1w[\1rmenu\1w]", "o");
	}
	if (strchr("lsB", context)) {
	    M_FILENAME, HELPDIR, "commands/profile_user");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Profile a user", "p");
	}
	if (strchr("lsBH", context)) {
	    M_FILENAME, HELPDIR, "commands/question");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Short Help \1w[\1rmenu\1w]", "q");
	}
	if (strchr("lsBH", context)) {
	    M_FILENAME, HELPDIR, "commands/guide_question");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Ask a Question", "Q");
	}
	if (strchr("lPI", context)) {
	    M_FILENAME, HELPDIR, "commands/reply");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Reply to post", "r");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/remove_friend");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Remove user from Friends List", "r");
	}
	if (strchr("e", context)) {
	    M_FILENAME, HELPDIR, "commands/remove_enemy");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Remove user from Enemies List", "r");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_roomlock");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Toggle Roomlock", "r");
	}
	if (strchr("sAX", context)) {
	    M_FILENAME, HELPDIR, "commands/intBBS_who");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "InterBBS Wholist", "R");
	}
	if (strchr("lRB", context)) {
	    M_FILENAME, HELPDIR, "commands/stop");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Stop read prompt", "s");
	}
	if (strchr("cC", context)) {
	    M_FILENAME, HELPDIR, "commands/chatconf");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Chat Setup", "s");
	}
	if (strchr("sPI", context)) {
	    M_FILENAME, HELPDIR, "commands/skip");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Skip Quad", "s");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_sysconfig");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display System Configuration", "s");
	}
	if (strchr("lsUA", context)) {
	    M_FILENAME, HELPDIR, "utils/search");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Search messages", "S");
	}
	if (strchr("cCA", context)) {
	    M_FILENAME, HELPDIR, "commands/termtype");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Change Term", "t");
	}
	if (strchr("f", context)) {
	    M_FILENAME, HELPDIR, "commands/toggle_f_notify");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Toggle Logon Notifies", "t");
	}
	if (strchr("lsUI", context)) {
	    M_FILENAME, HELPDIR, "commands/time");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Display the time", "t");
	}
	if (strchr("sRA", context)) {
	    M_FILENAME, HELPDIR, "commands/ungoto");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Un-goto (buggy)", "u");
	}
	if (strchr("lsXI", context)) {
	    M_FILENAME, HELPDIR, "commands/xreply");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Reply to X", "v");
	}
	if (strchr("lsB", context)) {
	    M_FILENAME, HELPDIR, "commands/who");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "See who is online", "w");
	}
	if (strchr("lsXB", context)) {
	    M_FILENAME, HELPDIR, "commands/xmessage");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Send X message", "x");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_reset_lastseen");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Reset Lastseen", "x");
	}
	if (strchr("lsXB", context)) {
	    M_FILENAME, HELPDIR, "commands/x_disable");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Toggle X status", "X");
	}
/*    if (strchr("", context)) {
 * M_FILENAME, HELPDIR, "commands/xlog");
 * MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "", "");
 * } */
	if (strchr("lsH", context)) {
	    M_FILENAME, HELPDIR, "commands/yell");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Yell for help", "y");
	}
	if (strchr("lsRB", context)) {
	    M_FILENAME, HELPDIR, "commands/zap");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Zap (forget) Quad", "z");
	}
	if (strchr("i", context)) {
	    M_FILENAME, HELPDIR, "commands/mi_zapall");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Zap all quadrants", "z");
	}
	if (strchr("lsXI", context)) {
	    M_FILENAME, HELPDIR, "commands/emote");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Send emote", "~");
	}
	if (strchr("lsXI", context)) {
	    M_FILENAME, HELPDIR, "commands/feeling");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Send feeling", "!");
	}
	if (strchr("lsXB", context)) {
	    M_FILENAME, HELPDIR, "commands/quote_xs");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Quote X messages", "\"");
	}
	if (strchr("lsXI", context)) {
	    M_FILENAME, HELPDIR, "commands/friendsls");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit friendslist", "<");
	}
	if (strchr("lsXI", context)) {
	    M_FILENAME, HELPDIR, "commands/enemylst");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Edit enemylist", ">");
	}
	if (strchr("lsRA", context)) {
	    M_FILENAME, HELPDIR, "commands/read_postnumber");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Read post by #", "#");
	}
	if (strchr("sRA", context)) {
	    M_FILENAME, HELPDIR, "commands/readlast_n");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Re-read", "-");
	}
	if (strchr("sRA", context)) {
	    M_FILENAME, HELPDIR, "commands/readlast_N");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Read from last post", "_");
	}
	// ctrl-n                          case 016:
	/*
	 * M_FILENAME, HELPDIR, "commands/upload");
	 * MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "", "");
	 * } */
	if (strchr("lsBH", context)) {
	    M_FILENAME, HELPDIR, "commands/short_helplist");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Online Help", "\?");
//	MENU_ADDITEM(online_help_wrapper, 1, 0, "s", "ti", "Online Help", "\?");
	}
	if (strchr("sBH", context)) {
	    M_FILENAME, HELPDIR, "commands/commands_longlist");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Command list (dated)", "/");
	}
	if (strchr("lsXB", context)) {
	    M_FILENAME, HELPDIR, "commands/chatwho");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Chat Wholist", ",");
	}
	if (strchr("sAU", context)) {
	    M_FILENAME, HELPDIR, "commands/quadcont");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Quad-Rating \1w[\1rmenu\1w]", "(");
	}
	if (strchr("sAU", context)) {
	    M_FILENAME, HELPDIR, "commands/clipboard");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Clipboard \1w[\1rmenu\1w]", "*");
	}
	if (strchr("l", context)) {
	    M_FILENAME, HELPDIR, "commands/clip");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "Clip post", "*");
	}

	if (!strchr("agmpx", context))
	    the_menu_format.auto_columnize = 1;
	else {
	    
	    MENU_ADDITEM(do_nothing, 0, 0, NULL, "ti", "-------------", "");

	    M_FILENAME, HELPDIR, "commands/short_helplist");
	    MENU_ADDITEM(more_wrapper, 1, 0, (char *) filename, "ti", "How to use Help", "\?");
	}

	MENU_PROCESS_INTERNALS;
        if (strchr("ag", context))
	    MENU_DISPLAY(2);
	else
	    MENU_DISPLAY(1);

	if (!MENU_EXEC_COMMAND) {
	    MENU_DESTROY;
	    break;
	}
	MENU_DESTROY;

	cprintf("\nPress a key when done..");
	inkey();
    }
}
#undef M_FILENAME


int express_commands_menu(void) {
    online_help('X');
    return 0;
}
int reading_commands_menu(void) {
    online_help('R');
    return 0;
}
int posting_commands_menu(void) {
    online_help('P');
    return 0;
}
int config_commands_menu(void) {
    online_help('C');
    return 0;
}
int helpish_commands_menu(void) {
    online_help('H');
    return 0;
}
int utility_commands_menu(void) {
    online_help('U');
    return 0;
}

/*************************************************
* help_topics()
* new help 'touch & go [tm]' put into seperate file.
*************************************************/

void
     help_topics() {

    register char cmd = '\0';
    int quit = 0;

    while (!quit) {

	more(HELPDIR "topics.menu", 1);
	if ((usersupp->flags & US_GUIDE) || (usersupp->flags & US_ROOMAIDE) || (usersupp->priv > PRIV_SYSOP)) {
	    cprintf("\1f\1w<\1r7\1w>\1r  Admin helpfiles.\n\1w     \1w  Information for Help Terminals and Quadrant Leaders\n");
	}
	cprintf("\nfw          [ gPress w<rSPACEw>g or w<rqw>g to quitw ]a\n");
	cprintf("\n\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q1234567c ");

	switch (cmd) {

	    case '1':
		cprintf("Important\n");
		quit = help_important();
		break;

	    case '2':
		cprintf("Communication\n");
		quit = help_communication();
		break;

	    case '3':
		cprintf("Species\n");
		quit = help_species();
		break;
	    case '4':
		cprintf("Miscellaneous\n");
		quit = help_misc();
		break;

	    case '5':
		cprintf("Utilities\n");
		quit = help_utils();
		break;

	    case '6':
		cprintf("Fun\n");
		quit = help_fun();
		break;

	    case 'c':
		cprintf("Commands\n");
		quit = help_commands();
		break;

	    case '7':
		if ((usersupp->flags & US_GUIDE) || (usersupp->flags & US_ROOMAIDE) || (usersupp->priv > PRIV_SYSOP)) {
		    cprintf("Admin\n");
		    quit = help_admin();
		    break;
		}
	    case 'q':
	    case ' ':
		quit = 1;
		cprintf("Quit\n\n\1f\1gAnd remember, you can always ask for help by pressing \1w<\1rshift-Q\1w>\1g\n");
		break;

	}
    }

}


/*************************************************
* help_important()
*
* Displays a list of important helpfiles.
*
*************************************************/

static int
    help_important() {

    register char cmd = '\0';
    int quit = 0;
    while (!quit) {
	more(HELPDIR "important/important.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12345 ");

	switch (cmd) {

	    case 'q':
		return 1;

	    case '1':
		cprintf("Choose Language\n");
		quit = help_choice();
		break;

	    case '2':
		more(HELPDIR "important/rules", 1);
		break;

	    case '3':
		more(HELPDIR "important/harrassment", 1);
		break;

	    case '4':
		more(HELPDIR "important/shix", 1);
		break;

	    case '5':
		more(HELPDIR "important/commands", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg     Press a key...\na");
	inkey();

    }
    return (quit == 1) ? 1 : 0;

}

/**************************************************************
*
* help_choice() helps you make a choice between swedish and
* normal policy files.
*
***************************************************************/

static int
    help_choice() {

    register char cmd = '\0';
    int quit = 0;

    while (!quit) {

	more(HELPDIR "important/policy/choice.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		cprintf("Policy in Swedish\n");
		quit = help_swedish();
		break;

	    case '2':
		cprintf("Policy\n");
		quit = help_policy();
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
    }
    return (quit == 1) ? 1 : 0;
}

/*************************************************
* help_swedish()
* Displays a list of swedish policy helpfiles.
*************************************************/

static int
    help_swedish(void) {

    register char cmd = '\0';

    more(HELPDIR "important/policy/policy.menu.swe", 1);
    cprintf("\1f\1g     Enter command\1w: \1c");
    cmd = get_single_quiet(" q123");

    for (;;) {
	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "important/policy/quadrants.swe", 1);
		break;

	    case '2':
		more(HELPDIR "important/policy/cursing.swe", 1);
		break;

	    case '3':
		more(HELPDIR "important/policy/aliens.swe", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 2;

	}
	cprintf("\nfg      Press a key...\na");
	inkey();

    }
}

/*************************************************
* help_policy
*************************************************/

static int
    help_policy(void) {

    register char cmd = '\0';

    for (;;) {

	more(HELPDIR "important/policy/policy.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet(" q123");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "important/policy/quadrants", 1);
		break;

	    case '2':
		more(HELPDIR "important/policy/cursing", 1);
		break;

	    case '3':
		more(HELPDIR "important/policy/aliens", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 2;

	}
	cprintf("\nfg      Press a key...\na");
	inkey();

    }
}

/*************************************************
* help_communication()
*************************************************/

static int
    help_communication() {

    register char cmd = '\0';
    for (;;) {
	more(HELPDIR "communication/communication.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q123456789 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "communication/express-messages", 1);
		break;

	    case '2':
		more(HELPDIR "communication/x-friends", 1);
		break;

	    case '3':
		more(HELPDIR "communication/x-enemies", 1);
		break;

	    case '4':
		more(HELPDIR "communication/who", 1);
		break;

	    case '5':
		more(HELPDIR "communication/quadrants", 1);
		break;

	    case '6':
		more(HELPDIR "communication/mail", 1);
		break;

	    case '7':
		more(HELPDIR "communication/posts", 1);
		break;

	    case '8':
		more(HELPDIR "communication/yells", 1);
		break;

	    case '9':
		more(HELPDIR "communication/interbbs", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg      Press a key...\na");
	inkey();

    }

}


/*************************************************
* help_species()
*
*
*
*************************************************/

static int
    help_species() {

    register char cmd = '\0';

    for (;;) {
	more(HELPDIR "species/species.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q123456 ");
	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "species/emperors", 1);
		break;

	    case '2':
		more(HELPDIR "species/fleetcommanders", 1);
		break;

	    case '3':
		more(HELPDIR "species/helpterminals", 1);
		break;

	    case '4':
		more(HELPDIR "species/sysanalists", 1);
		break;

	    case '5':
		more(HELPDIR "species/quadrantleaders", 1);
		break;

	    case '6':
		more(HELPDIR "species/aliens", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg      Press a key...\na");
	inkey();
    }

}


/*************************************************
* help_misc()
*************************************************/

static int
    help_misc() {

    register char cmd = '\0';

    for (;;) {
	more(HELPDIR "misc/misc.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q1234 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "misc/votingbooth", 1);
		break;

	    case '2':
		more(HELPDIR "misc/ghost", 1);
		break;

	    case '3':
		more(HELPDIR "misc/grolsch", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg      Press a key...\na");
	inkey();

    }

}


/*************************************************
* help_utils()
*************************************************/

static int
    help_utils() {

    register char cmd = '\0';

    for (;;) {
	more(HELPDIR "utils/utils.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12345 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "utils/colors", 1);
		break;

	    case '2':
		more(HELPDIR "utils/clipboard", 1);
		break;

	    case '3':
		more(HELPDIR "utils/pico-editor", 1);
		break;

	    case '4':
		more(HELPDIR "utils/client", 1);
		break;

	    case '5':
		more(HELPDIR "utils/search", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\n\1f\1g    Press a key...\n\1a");
	inkey();

    }

}

/****************************************
*
* help_fun
*
* some cool & fun crap
*
****************************************/

static int
    help_fun() {

    register char cmd = '\0';
    int quit = 0;

    while (!quit) {

	more(HELPDIR "fun/fun.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q1234 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "fun/dutch", 1);
		break;

	    case '2':
		quit = eat_menu();
		break;

	    case '4':
		more(HELPDIR "fun/pref", 1);
		break;

	    case '3':
		IFSYSOP
		    more(HELPDIR "fun/test/index", 1);
		quit = idiocy_test();
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;
	}
	cprintf("\nfg     Press a key...a");
	inkey();

    }
    return (quit == 1) ? 1 : 0;

}

/****************************************
* help_admin
****************************************/

static int
    help_admin() {

    register char cmd = '\0';

    for (;;) {
	cprintf("\n\nfw<r1w>g  Help for %ss.a\n     w-g Your rights and duties.a\n", GUIDE);
	cprintf("fw<r2w>g  Help for %ss.a\n     w-g Your rights and duties.a\n", ROOMAIDE);
	cprintf("\n\n     fw[g Press w<rSPACEw>g to go to the Main Menu or w<rqw>g to quit w]a\n");

	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12 ");

	switch (cmd) {

	    case '1':
		if ((usersupp->flags & US_GUIDE) || (usersupp->priv > PRIV_SYSOP))
		    more(HELPDIR "admin/help_terminal", 1);
		else
		    cprintf("\nfrYou're not a %s!a\n", GUIDE);
		break;

	    case '2':
		if ((usersupp->flags & US_ROOMAIDE) || (usersupp->priv > PRIV_SYSOP))
		    more(HELPDIR "admin/quadrant_leader", 1);
		else
		    cprintf("frYou're not a %s!a\n", ROOMAIDE);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	    case 'q':
		cprintf("Quit\n");
		return 1;

	}
    }
}

/****************************************
* eat_menu
* recipes for food & drinks.
****************************************/

static int
    eat_menu() {

    register char cmd = '\0';
    int quit = 0;

    while (!quit) {
	more(HELPDIR "fun/food/index", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q123 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		cprintf("Food\n");
		quit = food_menu();
		break;

	    case '2':
		cprintf("Drinks\n");
		quit = drinks_menu();
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;
	}
    }
    return (quit == 1) ? 1 : 0;
}


/*************************************************
*
*  food_menu()
*
* displays all sorts of nice recipes for food.
*
*************************************************/

static int
    food_menu() {

    register char cmd = '\0';

    for (;;) {
	more(HELPDIR "fun/food/food.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "fun/food/macaroni", 1);
		break;

	    case '2':
		more(HELPDIR "fun/food/pancakes", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg     Press a key...a\n");
	inkey();

    }

}


/*************************************************
*
*  drinks_menu()
*
* displays all sorts of nice recipes for drinks.
*
*************************************************/

static int
    drinks_menu() {

    register char cmd = '\0';

    for (;;) {
	more(HELPDIR "fun/food/drinks.menu", 1);
	cprintf("\1f\1g     Enter command\1w: \1c");
	cmd = get_single_quiet("q12345 ");

	switch (cmd) {

	    case 'q':
		cprintf("Quit\n");
		return 1;

	    case '1':
		more(HELPDIR "fun/food/sniper", 1);
		break;

	    case '2':
		more(HELPDIR "fun/food/woofer", 1);
		break;

	    case '3':
		more(HELPDIR "fun/food/spodee", 1);
		break;

	    case '4':
		more(HELPDIR "fun/food/whack", 1);
		break;

	    case '5':
		more(HELPDIR "fun/food/ouch", 1);
		break;

	    case ' ':
		cprintf("Back\n");
		return 0;

	}
	cprintf("\nfg     Press a key...a\n");
	inkey();
    }
}


/*************************************************
*
*  cool_test()
*
*   a crap test.
*
*************************************************/


static int
    idiocy_test() {

    register char cmd = '\0';
    int idiocy;

    idiocy = 0;

    cprintf("\nfg     Do you consider yourself an idiot? w(gy/nw)g  ");
    if (yesno() == NO) {
	idiocy++;
    }
    cprintf("fg     Do others consider you an idiot? w(gy/nw)g    ");
    if (yesno() == YES) {
	idiocy++;
    }
    cprintf("fg     How long did you sleep last night?a");
    cprintf("\nfw       <r1w>g Less than 3 hours.       w<r2w>g Between 3 and 8 hours.a");
    cprintf("\nfw       <r3w>g Between 5 and 8 hours.   w<r4w>g Over 8 hours.");
    cprintf("\nfg     Enter a value between 1 and 4 here...a");

    cmd = get_single_quiet("1234");
    switch (cmd) {

	case '1':
	    idiocy += 3;
	    break;
	case '2':
	    idiocy += 2;
	    break;
	case '3':
	    idiocy++;
	    break;
	case '4':
	    break;
	default:
	    cprintf("\n     frD'uh... something went wrong here...a\n");
	    break;
    }
    cprintf("\n\nfg     Saving idiocy index for alien y%sg.\n     Index isw: y%dg.", usersupp->username, idiocy);
    return 0;
}

/* ------------------------------------------------------------------------ */


int
    help_commands() {

    register char cmd = '\0';
    int quit = 0;

    while (!quit) {

	more(HELPDIR "commands/menu", 1);
	cprintf("\1f\1gPress: \1w<\1rnumber\1w>\1g of a menu, \1w<\1rq\1w>\1g to quit, \1w<\1rspace\1w>\1g to go up a level \1w: \1c");
	cmd = get_single_quiet("q123456 ");

	switch (cmd) {

	    case 'q':
		cprintf("\nQuitting help..\n");
		return 1;

	    case '1':
		cprintf("\nExpress Message related Commands\n");
		quit = express_commands_menu();
		break;

	    case '2':
		cprintf("\nReading related Commands\n");
		quit = reading_commands_menu();
		break;

	    case '3':
		cprintf("\nPosting related Commands\n");
		quit = posting_commands_menu();
		break;

	    case '4':
		cprintf("\nConfiguration related Commands\n");
		quit = config_commands_menu();
		break;

	    case '5':
		cprintf("\nHelp related Commands\n");
		quit = helpish_commands_menu();
		break;

	    case '6':
		cprintf("\nUtility related Commands\n");
		quit = utility_commands_menu();
		break;

	    case ' ':
		cprintf("\nBack a level\n");
		return 0;
	}
    }
    return (quit == 1) ? 1 : 0;

}

/* ------------------------------------------------------------------------ */
/****************************************************************************
*
*  Mjaaaahh.. I felt like doing some coding, so I added the complete works
*  of William Shakespeare (aka Bill) to the BBS code. Fun when you're bored
*  or boring when you're having fun. Whatever.
*
*  ps: If someone is going to whinge about the coffee tonight I will purge
*      their accounts. (check for Marsares / Kissaki).
*
*  May the force be with you!    - Peter.
*
****************************************************************************/

void
     literature_menu() {
    register char cmd = '\0';

    cprintf("fgLiterature Menu.\n");
    more(SHAKEDIR "index", 1);
    cprintf("fg\n    Choice: a");

    cmd = get_single_quiet("12345q ");
    switch (cmd) {

	case '1':
	    cprintf("gfComedies.\n");
	    shake_comedies();
	    return;
	case '2':
	    cprintf("fgHistories.\n");
	    shake_histories();
	    return;
	case '3':
	    cprintf("fgPoetry.\n");
	    shake_poetry();
	    return;
	case '4':
	    cprintf("fgTragedies.\n");
	    shake_tragedies();
	    return;
	case '5':
	    cprintf("fgOther.\n");
	    other_literature();
	    return;
	case 'q':
	case ' ':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    break;
    }
}
/* ------------------------------------------------------------------------ */
static void
     shake_comedies() {

    register char cmd = '\0';
    more(SHAKEDIR "comedies/index", 1);
    cprintf("fg\n    Choice: a");
    cmd = get_single_quiet("abcdefghijklmnopqr ");
    switch (cmd) {
	case 'a':
	    more(SHAKEDIR "comedies/asyoulikeit", 1);
	    break;
	case 'b':
	    more(SHAKEDIR "comedies/comedyoferrors", 1);
	    break;
	case 'c':
	    more(SHAKEDIR "comedies/allswellthatendswell", 1);
	    break;
	case 'd':
	    more(SHAKEDIR "comedies/cymbeline", 1);
	    break;
	case 'e':
	    more(SHAKEDIR "comedies/loveslabourslost", 1);
	    break;
	case 'f':
	    more(SHAKEDIR "comedies/measureformeasure", 1);
	    break;
	case 'g':
	    more(SHAKEDIR "comedies/merchantofvenice", 1);
	    break;
	case 'h':
	    more(SHAKEDIR "comedies/merrywivesofwindsor", 1);
	    break;
	case 'i':
	    more(SHAKEDIR "comedies/midsummersnightsdream", 1);
	    break;
	case 'j':
	    more(SHAKEDIR "comedies/muchadoaboutnothing", 1);
	    break;
	case 'k':
	    more(SHAKEDIR "comedies/periclesprinceoftyre", 1);
	    break;
	case 'l':
	    more(SHAKEDIR "comedies/tamingoftheshrew", 1);
	    break;
	case 'm':
	    more(SHAKEDIR "comedies/tempest", 1);
	    break;
	case 'n':
	    more(SHAKEDIR "comedies/troilusandcressida", 1);
	    break;
	case 'o':
	    more(SHAKEDIR "comedies/twelfthnight", 1);
	    break;
	case 'p':
	    more(SHAKEDIR "comedies/twogentlemenofverona", 1);
	    break;
	case ' ':
	case 'q':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    return;
	case 'r':
	    more(SHAKEDIR "comedies/winterstale", 1);
	    break;
    }
}
/* ------------------------------------------------------------------------ */
static void
     shake_histories() {

    register char cmd = '\0';
    more(SHAKEDIR "histories/index", 1);
    cprintf("fg\n    Choice: a");
    cmd = get_single_quiet("abcdefghijq ");
    switch (cmd) {
	case 'a':
	    more(SHAKEDIR "histories/1kinghenryiv", 1);
	    break;
	case 'b':
	    more(SHAKEDIR "histories/2kinghenryiv", 1);
	    break;
	case 'c':
	    more(SHAKEDIR "histories/3kinghenryvi", 1);
	    break;
	case 'd':
	    more(SHAKEDIR "histories/kinghenryviii", 1);
	    break;
	case 'e':
	    more(SHAKEDIR "histories/kingrichardii", 1);
	    break;
	case 'f':
	    more(SHAKEDIR "histories/1kinghenryvi", 1);
	    break;
	case 'g':
	    more(SHAKEDIR "histories/2kinghenryvi", 1);
	    break;
	case 'h':
	    more(SHAKEDIR "histories/kinghenryv", 1);
	    break;
	case 'i':
	    more(SHAKEDIR "histories/kingjohn", 1);
	    break;
	case 'j':
	    more(SHAKEDIR "histories/kingrichardiii", 1);
	    break;
	case ' ':
	case 'q':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    return;
    }
}
/* ------------------------------------------------------------------------ */
static void
     shake_poetry() {

    register char cmd = '\0';
    more(SHAKEDIR "poetry/index", 1);
    cprintf("fg\n    Choice: a");
    cmd = get_single_quiet("abcdeq ");
    switch (cmd) {
	case 'a':
	    more(SHAKEDIR "poetry/loverscomplaint", 1);
	    break;
	case 'b':
	    more(SHAKEDIR "poetry/sonnets", 1);
	    break;
	case 'c':
	    more(SHAKEDIR "poetry/venusandadonis", 1);
	    break;
	case 'd':
	    more(SHAKEDIR "poetry/rapeoflucrece", 1);
	    break;
	case 'e':
	    more(SHAKEDIR "poetry/various", 1);
	    break;
	case ' ':
	case 'q':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    return;
    }
}
/* ------------------------------------------------------------------------ */
static void
     shake_tragedies() {

    register char cmd = '\0';
    more(SHAKEDIR "tragedies/index", 1);
    cprintf("fg\n    Choice: a");
    cmd = get_single_quiet("abcdefghijq ");
    switch (cmd) {
	case 'a':
	    more(SHAKEDIR "tragedies/antonyandcleopatra", 1);
	    break;
	case 'b':
	    more(SHAKEDIR "tragedies/juliusceasar", 1);
	    break;
	case 'c':
	    more(SHAKEDIR "tragedies/othello", 1);
	    break;
	case 'd':
	    more(SHAKEDIR "tragedies/titusandronicus", 1);
	    break;
	case 'e':
	    more(SHAKEDIR "tragedies/coriolanus", 1);
	    break;
	case 'f':
	    more(SHAKEDIR "tragedies/kinglear", 1);
	    break;
	case 'g':
	    more(SHAKEDIR "tragedies/romeoandjuliet", 1);
	    break;
	case 'h':
	    more(SHAKEDIR "tragedies/hamlet", 1);
	    break;
	case 'i':
	    more(SHAKEDIR "tragedies/macbeth", 1);
	    break;
	case 'j':
	    more(SHAKEDIR "tragedies/timonofathens", 1);
	    break;
	case ' ':
	case 'q':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    return;
    }
}
/* ------------------------------------------------------------------------ */
static void
     other_literature() {

    register char cmd = '\0';
    more(LITDIR "index", 1);
    cprintf("fg\n    Choice: a");
    cmd = get_single_quiet("abcdq ");
    switch (cmd) {
	case 'a':
	    more(LITDIR "hyperion", 1);
	    break;
	case 'b':
	    more(LITDIR "max_havelaar", 1);
	    break;
	case 'c':
	    more(LITDIR "thepitandthependulum", 1);
	    break;
	case 'd':
	    more(LITDIR "poem", 1);
	    break;
	case ' ':
	case 'q':
	    cprintf("fgQuit.a");
	    cprintf("\n");
	    return;
    }
}

/* EOF */
