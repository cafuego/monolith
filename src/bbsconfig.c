/*
 * $Id$
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include "bbsconfig.h"
#include "input.h"
#include "menu.h"

#include "monolith.h"
#include "libmono.h"
#include "ext.h"
#include "log.h"
#include "routines2.h"
#include "sql_config.h"

static config_t cfg;
static int num;

static void _config_set_bbsname(const unsigned int a, const long b, const char *string);
static void _config_set_room(const unsigned int a, const long b, const char *string);
static void _config_set_message(const unsigned int a, const long b, const char *string);
static void _config_set_xpress(const unsigned int a, const long b, const char *string);
static void _config_set_xmessage(const unsigned int a, const long b, const char *string);
static void _config_set_user(const unsigned int a, const long b, const char *string);
static void _config_set_username(const unsigned int a, const long b, const char *string);
static void _config_set_doing(const unsigned int a, const long b, const char *string);
static void _config_set_location(const unsigned int a, const long b, const char *string);
static void _config_set_idle(const unsigned int a, const long b, const char *string);
static void _config_set_chat(const unsigned int a, const long b, const char *string);
static void _config_set_channel(const unsigned int a, const long b, const char *string);
static void _config_set_admin(const unsigned int a, const long b, const char *string);
static void _config_set_wizard(const unsigned int a, const long b, const char *string);
static void _config_set_sysop(const unsigned int a, const long b, const char *string);
static void _config_set_progdude(const unsigned int a, const long b, const char *string);
static void _config_set_roomaide(const unsigned int a, const long b, const char *string);
static void _config_set_guide(const unsigned int a, const long b, const char *string);
static void _config_save(const unsigned int a, const long b, const char *string);
static int _config_ok( void );

/*
 * Create a configuration.
 */
void
create_config()
{

    char str_t[40];
    MENU_DECLARE;

    /*
     * Set to zero, so mono_sql_save_config knows it's a new one.
     */
    num = 0;
    
    /*
     * it's a static var, so we better clean it first!
     */
    (void) memset(&cfg, 0, sizeof(config_t));

    for (;;) {

	MENU_INIT;
        sprintf(the_menu_format.menu_title, "\n\1f\1w[\1gCreate BBS Configuration\1w: \1y%s\1w]\n\n", cfg.bbsname);

	sprintf(str_t, "BBS Name \1w(\1y%s\1a\1f\1w)", cfg.bbsname);
	MENU_ADDITEM(_config_set_bbsname, 1, 0, "", "tiv", str_t, "1", cfg.bbsname);
	sprintf(str_t, "Room Name \1w(\1y%s\1a\1f\1w)", cfg.forum);
	MENU_ADDITEM(_config_set_room, 0, 0, "", "tiv", str_t, "2", cfg.forum);
	sprintf(str_t, "Room Name (pl) \1w(\1y%s\1a\1f\1w)", cfg.forum_pl);
	MENU_ADDITEM(_config_set_room, 0, 0, "", "tiv", str_t, "3", cfg.forum_pl);
	sprintf(str_t, "Message \1w(\1y%s\1a\1f\1w)", cfg.message);
	MENU_ADDITEM(_config_set_message, 0, 0, "", "tiv", str_t, "4", cfg.message);
	sprintf(str_t, "Message (pl) \1w(\1y%s\1a\1f\1w)", cfg.message_pl);
	MENU_ADDITEM(_config_set_message, 0, 0, "", "tiv", str_t, "5", cfg.message_pl);
	sprintf(str_t, "eXpress \1w(\1y%s\1a\1f\1w)", cfg.express);
	MENU_ADDITEM(_config_set_xpress, 0, 0, "", "tiv", str_t, "6", cfg.express);
	sprintf(str_t, "X Message \1w(\1y%s\1a\1f\1w)", cfg.x_message);
	MENU_ADDITEM(_config_set_xmessage, 0, 0, "", "tiv", str_t, "7", cfg.x_message);
	sprintf(str_t, "X Message (pl) \1w(\1y%s\1a\1f\1w)", cfg.x_message_pl);
	MENU_ADDITEM(_config_set_xmessage, 0, 0, "", "tiv", str_t, "8", cfg.x_message_pl);
	sprintf(str_t, "User \1w(\1y%s\1a\1f\1w)", cfg.user);
	MENU_ADDITEM(_config_set_user, 0, 0, "", "tiv", str_t, "9", cfg.user);
	sprintf(str_t, "User (pl) \1w(\1y%s\1a\1f\1w)", cfg.user_pl);
	MENU_ADDITEM(_config_set_user, 0, 0, "", "tiv", str_t, "a", cfg.user_pl);
	sprintf(str_t, "Username \1w(\1y%s\1a\1f\1w)", cfg.username);
	MENU_ADDITEM(_config_set_username, 0, 0, "", "tiv", str_t, "b", cfg.username);
	sprintf(str_t, "Doing \1w(\1y%s\1a\1f\1w)", cfg.doing);
	MENU_ADDITEM(_config_set_doing, 0, 0, "", "tiv", str_t, "c", cfg.doing);
	sprintf(str_t, "Location \1w(\1y%s\1a\1f\1w)", cfg.location);
	MENU_ADDITEM(_config_set_location, 0, 0, "", "tiv", str_t, "d", cfg.location);
	sprintf(str_t, "Idle \1w(\1y%s\1a\1f\1w)", cfg.idle);
	MENU_ADDITEM(_config_set_idle, 0, 0, "", "tiv", str_t, "e", cfg.idle);
	sprintf(str_t, "Chat \1w(\1y%s\1a\1f\1w)", cfg.chatmode);
	MENU_ADDITEM(_config_set_chat, 0, 0, "", "tiv", str_t, "f", cfg.chatmode);
	sprintf(str_t, "Channel \1w(\1y%s\1a\1f\1w)", cfg.chatroom);
	MENU_ADDITEM(_config_set_channel, 0, 0, "", "tiv", str_t, "g", cfg.chatroom);
	sprintf(str_t, "Admin \1w(\1y%s\1a\1f\1w)", cfg.admin);
	MENU_ADDITEM(_config_set_admin, 0, 0, "", "tiv", str_t, "h", cfg.admin);
	sprintf(str_t, "Wizard \1w(\1y%s\1a\1f\1w)", cfg.wizard);
	MENU_ADDITEM(_config_set_wizard, 0, 0, "", "tiv", str_t, "i", cfg.wizard);
	sprintf(str_t, "Sysop \1w(\1y%s\1a\1f\1w)", cfg.sysop);
	MENU_ADDITEM(_config_set_sysop, 0, 0, "", "tiv", str_t, "j", cfg.sysop);
	sprintf(str_t, "Programmer \1w(\1y%s\1a\1f\1w)", cfg.programmer);
	MENU_ADDITEM(_config_set_progdude, 0, 0, "", "tiv", str_t, "k", cfg.programmer);
	sprintf(str_t, "Roomaide \1w(\1y%s\1a\1f\1w)", cfg.roomaide);
	MENU_ADDITEM(_config_set_roomaide, 0, 0, "", "tiv", str_t, "l", cfg.roomaide);
	sprintf(str_t, "Guide \1w(\1y%s\1a\1f\1w)", cfg.guide);
	MENU_ADDITEM(_config_set_guide, 0, 0, "", "tiv", str_t, "m", cfg.guide);

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);

        /*
         * Do weird shit to check if we're done editing and are allowed to save.
         */
	if (!MENU_EXEC_COMMAND) {
            if(_config_ok()) {
                _config_save(0, 0, "");
	        break;
            } else {
                cprintf("\1f\1rReally abort creating this config? \1w(\1ry\1w/\1rN\1w)\1c ");
                if( yesno_default(NO) == YES )
                    break;
            }
        }
        MENU_DESTROY;
    }	/* for(;;) */

    MENU_DESTROY;
    return;
}

/*
 * Edit a configuration
 */
void
edit_config()
{
    MENU_DECLARE;
    char str_t[40];

    if ((num = select_config("\n\1f\1gSelect configuration to edit\1w: \1c")) < 1)
	return;

    /*
     * it's a static var, so we better clean it first!
     */
    (void) memset(&cfg, 0, sizeof(config_t));

    if ((mono_sql_read_config(num, &cfg)) == -1) {
	(void) log_it("errors", "Can't read config %d from table %s!", num, CONFIG_TABLE);
	cprintf("\1f\1rUnable to read config %d from %s table!\n", num, CONFIG_TABLE);
	return;
    }
    for (;;) {

	MENU_INIT;
	sprintf(the_menu_format.menu_title, "\n\1f\1w[\1gEdit BBS Configuration\1w: \1y%s\1w]\n\n", cfg.bbsname);

	sprintf(str_t, "BBS Name \1w(\1y%s\1a\1f\1w)", cfg.bbsname);
	MENU_ADDITEM(_config_set_bbsname, 0, 0, "", "tiv", str_t, "1", cfg.bbsname);
	sprintf(str_t, "Room Name \1w(\1y%s\1a\1f\1w)", cfg.forum);
	MENU_ADDITEM(_config_set_room, 0, 0, "", "tiv", str_t, "2", cfg.forum);
	sprintf(str_t, "Room Name (pl) \1w(\1y%s\1a\1f\1w)", cfg.forum_pl);
	MENU_ADDITEM(_config_set_room, 0, 0, "", "tiv", str_t, "3", cfg.forum_pl);
	sprintf(str_t, "Message \1w(\1y%s\1a\1f\1w)", cfg.message);
	MENU_ADDITEM(_config_set_message, 0, 0, "", "tiv", str_t, "4", cfg.message);
	sprintf(str_t, "Message (pl) \1w(\1y%s\1a\1f\1w)", cfg.message_pl);
	MENU_ADDITEM(_config_set_message, 0, 0, "", "tiv", str_t, "5", cfg.message_pl);
	sprintf(str_t, "eXpress \1w(\1y%s\1a\1f\1w)", cfg.express);
	MENU_ADDITEM(_config_set_xpress, 0, 0, "", "tiv", str_t, "6", cfg.express);
	sprintf(str_t, "X Message \1w(\1y%s\1a\1f\1w)", cfg.x_message);
	MENU_ADDITEM(_config_set_xmessage, 0, 0, "", "tiv", str_t, "7", cfg.x_message);
	sprintf(str_t, "X Message (pl) \1w(\1y%s\1a\1f\1w)", cfg.x_message_pl);
	MENU_ADDITEM(_config_set_xmessage, 0, 0, "", "tiv", str_t, "8", cfg.x_message_pl);
	sprintf(str_t, "User \1w(\1y%s\1a\1f\1w)", cfg.user);
	MENU_ADDITEM(_config_set_user, 0, 0, "", "tiv", str_t, "9", cfg.user);
	sprintf(str_t, "User (pl) \1w(\1y%s\1a\1f\1w)", cfg.user_pl);
	MENU_ADDITEM(_config_set_user, 0, 0, "", "tiv", str_t, "a", cfg.user_pl);
	sprintf(str_t, "Username \1w(\1y%s\1a\1f\1w)", cfg.username);
	MENU_ADDITEM(_config_set_username, 0, 0, "", "tiv", str_t, "b", cfg.username);
	sprintf(str_t, "Doing \1w(\1y%s\1a\1f\1w)", cfg.doing);
	MENU_ADDITEM(_config_set_doing, 0, 0, "", "tiv", str_t, "c", cfg.doing);
	sprintf(str_t, "Location \1w(\1y%s\1a\1f\1w)", cfg.location);
	MENU_ADDITEM(_config_set_location, 0, 0, "", "tiv", str_t, "d", cfg.location);
	sprintf(str_t, "Idle \1w(\1y%s\1a\1f\1w)", cfg.idle);
	MENU_ADDITEM(_config_set_idle, 0, 0, "", "tiv", str_t, "e", cfg.idle);
	sprintf(str_t, "Chat \1w(\1y%s\1a\1f\1w)", cfg.chatmode);
	MENU_ADDITEM(_config_set_chat, 0, 0, "", "tiv", str_t, "f", cfg.chatmode);
	sprintf(str_t, "Channel \1w(\1y%s\1a\1f\1w)", cfg.chatroom);
	MENU_ADDITEM(_config_set_channel, 0, 0, "", "tiv", str_t, "g", cfg.chatroom);
	sprintf(str_t, "Admin \1w(\1y%s\1a\1f\1w)", cfg.admin);
	MENU_ADDITEM(_config_set_admin, 0, 0, "", "tiv", str_t, "h", cfg.admin);
	sprintf(str_t, "Wizard \1w(\1y%s\1a\1f\1w)", cfg.wizard);
	MENU_ADDITEM(_config_set_wizard, 0, 0, "", "tiv", str_t, "i", cfg.wizard);
	sprintf(str_t, "Sysop \1w(\1y%s\1a\1f\1w)", cfg.sysop);
	MENU_ADDITEM(_config_set_sysop, 0, 0, "", "tiv", str_t, "j", cfg.sysop);
	sprintf(str_t, "Programmer \1w(\1y%s\1a\1f\1w)", cfg.programmer);
	MENU_ADDITEM(_config_set_progdude, 0, 0, "", "tiv", str_t, "k", cfg.programmer);
	sprintf(str_t, "Roomaide \1w(\1y%s\1a\1f\1w)", cfg.roomaide);
	MENU_ADDITEM(_config_set_roomaide, 0, 0, "", "tiv", str_t, "l", cfg.roomaide);
	sprintf(str_t, "Guide \1w(\1y%s\1a\1f\1w)", cfg.guide);
	MENU_ADDITEM(_config_set_guide, 0, 0, "", "tiv", str_t, "m", cfg.guide);

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);
	if (!MENU_EXEC_COMMAND)
	    break;
        MENU_DESTROY;
    }				/* for(;;) */

    MENU_DESTROY;

    _config_save(0,0,"");
    return;
}

/*
 * Deletes a config.
 *
 * Except if it does that, the while() in select_config() fucks up
 * completely, so we'll have to do that differently. There's no deleting
 * until such time, just edit it.
 */
void
delete_config()
{
    int i = 0;
    config_t bing;

    i = select_config("\n\1f\1gWhich config do you want to delete? ");

    (void) mono_sql_read_config(i, &bing);
    cprintf("\n\1f\1gAre you absolutely certain you want to delete `\1y%s\1g'?\n", bing.bbsname);
    cprintf("There is currently \1rNO\1g way we can restore configs from back-up, so this is\n");
    cprintf("not undo-able and the config will be lost forever.\n");

    cprintf("\n\1f\1rDelete? \1w(\1ry\1w/\1rN\1w) \1c");
    if( yesno_default(NO) == YES ) {
        cprintf("\1f\1rConfig `\1y%s\1r' not deleted; permission denied.\n", bing.bbsname);
        cprintf("\1f\1rYou can edit it if you want, though.\n");
    } else {
        cprintf("\1f\1gConfig `\1y%s\1g' was not deleted.\n", bing.bbsname);
    }
    return;

}

/*
 * Displays a list of config names and returns the id of the
 * one the user chooses. (or the current config, if invalid)
 */
int
select_config(const char *prompt)
{

    unsigned int i = 1;
    int j = 0;
    config_t cfg;
    MENU_DECLARE;

    MENU_INIT;

    /*
     * build the menu
     */
    strcpy(the_menu_format.menu_title, "\n\1f\1w[\1gChoose BBS Configuration\1w]\1a\n\n");

    the_menu_format.gen_1_idx = 1;
    the_menu_format.auto_columnize = 1;

    /*
     * Keep reading until no more configs...
     */
    while((mono_sql_read_config(i, &cfg)) != -1) {
	MENU_ADDITEM(NULL, 0, 0, "", "tv", cfg.bbsname, (i == usersupp->configuration) ? "1" : "0");
        i++;
    }

    /*
     * Display menu and kill the thing.
     */
    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(0);
    MENU_DESTROY;

    /*
     * Print off the prompt.
     */
    cprintf("%s", prompt );
    fflush(stdout);

    /*
     * Check what config we chose and return
     * the number, unless something bad happened.
     */
    j = qc_get_pos_int('\0', 2);
    if (j == -1)
	j = usersupp->configuration;
    return j;

}

/*
 * A Zillion little functions!
 */
void
_config_set_bbsname(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yBBS\1g' name \1w(\1g%s\1w): \1c", cfg.bbsname);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1) {
        if(a) { /* new config, check if name is taken... */
            if( mono_sql_config_exists(tempstr) ) {
                cprintf("\n\1f\1rOi! Config '\1y%s\1r' already exists! You MUST choose a different name!\n", tempstr);
                return;
            }
        }
	strcpy(cfg.bbsname, tempstr);
    }
    return;
}

void
_config_set_room(const unsigned int a, const long b, const char *string)
{
    char tempstr[23];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yroom\1g' name \1w(\1g%s\1w): \1c", cfg.forum);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.forum, tempstr);
    strcpy(tempstr, "");
    cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", cfg.forum_pl);
    getline(tempstr, 22, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.forum_pl, tempstr);
    return;
}

void
_config_set_message(const unsigned int a, const long b, const char *string)
{

    char tempstr[15];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ymessage\1g' name \1w(\1g%s\1w): \1c", cfg.message);
    getline(tempstr, 12, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.message, tempstr);
    strcpy(tempstr, "");
    cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", cfg.message_pl);
    getline(tempstr, 14, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.message_pl, tempstr);
    return;
}

void
_config_set_xpress(const unsigned int a, const long b, const char *string)
{
    char tempstr[13];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yeXpress\1g' name \1w(\1g%s\1w): \1c", cfg.express);
    getline(tempstr, 12, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.express, tempstr);
    return;
}

void
_config_set_xmessage(const unsigned int a, const long b, const char *string)
{
    char tempstr[15];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yX message\1g' name \1w(\1g%s\1w): \1c", cfg.x_message);
    getline(tempstr, 12, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.x_message, tempstr);
    strcpy(tempstr, "");
    cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", cfg.x_message_pl);
    getline(tempstr, 14, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.x_message_pl, tempstr);
    return;
}

void
_config_set_user(const unsigned int a, const long b, const char *string)
{
    char tempstr[15];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yuser\1g' name \1w(\1g%s\1w): \1c", cfg.user);
    getline(tempstr, 12, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.user, tempstr);
    strcpy(tempstr, "");
    cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", cfg.user_pl);
    getline(tempstr, 14, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.user_pl, tempstr);
    return;
}

void
_config_set_username(const unsigned int a, const long b, const char *string)
{
    char tempstr[13];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yusername\1g' name \1w(\1g%s\1w): \1c", cfg.username);
    getline(tempstr, 12, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.username, tempstr);
    return;
}

void
_config_set_doing(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ydoing\1g' name \1w(\1g%s\1w): \1c", cfg.doing);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.doing, tempstr);
    return;
}

void
_config_set_location(const unsigned int a, const long b, const char *string)
{
    char tempstr[16];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ylocation\1g' name \1w(\1g%s\1w): \1c", cfg.location);
    getline(tempstr, 16, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.location, tempstr);
    return;
}

void
_config_set_idle(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yidle\1g' message \1w(\1g%s\1w): \1c", cfg.idle);
    getline(tempstr, 20, TRUE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.idle, tempstr);
    return;
}

void
_config_set_chat(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ychat\1g' name \1w(\1g%s\1w): \1c", cfg.chatmode);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.chatmode, tempstr);
    return;
}

void
_config_set_channel(const unsigned int a, const long b, const char *string)
{
    char tempstr[20];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ychannel\1g' name \1w(\1g%s\1w): \1c", cfg.chatroom);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.chatroom, tempstr);
    return;
}

void
_config_set_admin(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yadmin\1g' name \1w(%s): \1c", cfg.admin);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.admin, tempstr);
    return;
}
void
_config_set_wizard(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ywizard\1g' name \1w(%s): \1c", cfg.wizard);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.wizard, tempstr);
    return;
}

void
_config_set_sysop(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1ysysop\1g' name \1w(\1p%s\1w): \1c", cfg.sysop);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.sysop, tempstr);
    return;
}
void
_config_set_progdude(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yprogrammer\1g' name \1w(\1b%s\1w): \1c", cfg.programmer);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.programmer, tempstr);
    return;
}
void
_config_set_roomaide(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yroomaide\1g' name \1w(\1r%s\1w): \1c", cfg.roomaide);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.roomaide, tempstr);
    return;
}
void
_config_set_guide(const unsigned int a, const long b, const char *string)
{
    char tempstr[21];

    strcpy(tempstr, "");
    cprintf("\1f\1gEnter a new `\1yguide\1g' name \1w(\1c%s\1w): \1c", cfg.guide);
    getline(tempstr, 20, FALSE);
    if (strlen(tempstr) > 1)
	strcpy(cfg.guide, tempstr);
    return;
}
void
_config_save(const unsigned int a, const long b, const char *string)
{
    cprintf("\1f\1gSave changes? \1w(\1gY\1w/\1gn\1w) \1c");
    if (yesno_default(YES) == YES) {
	if (mono_sql_save_config(num, &cfg) == -1) {
	    cprintf("\1f\1rArgh! Couldn't save the config!\n");
	}
    } else {
	cprintf("\1f\1rOK, not saving...\n");
    }
    return;
}

int
_config_ok()
{
    if( strlen(cfg.bbsname) < 2 ) {
        cprintf("\n\1f\1rYou must change the BBS Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.forum) < 2 ) {
        cprintf("\n\1f\1rYou must change the Forum Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.forum_pl) < 2 ) {
        cprintf("\n\1f\1rYou must change the plural Forum Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.message) < 2 ) {
        cprintf("\n\1f\1rYou must change the Message Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.message_pl) < 2 ) {
        cprintf("\n\1f\1rYou must change the plural Message Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.express) < 2 ) {
        cprintf("\n\1f\1rYou must change the eXpress Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.x_message) < 2 ) {
        cprintf("\n\1f\1rYou must change the X Message Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.x_message_pl) < 2 ) {
        cprintf("\n\1f\1rYou must change the plural X Message Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.user) < 2 ) {
        cprintf("\n\1f\1rYou must change the User Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.user_pl) < 2 ) {
        cprintf("\n\1f\1rYou must change the plural User Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.username) < 2 ) {
        cprintf("\n\1f\1rYou must change the Username Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.doing) < 2 ) {
        cprintf("\n\1f\1rYou must change the Doing Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.location) < 2 ) {
        cprintf("\n\1f\1rYou must change the Location Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.chatmode) < 2 ) {
        cprintf("\n\1f\1rYou must change the Chatmode Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.chatroom) < 2 ) {
        cprintf("\n\1f\1rYou must change the Chatroom Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.admin) < 2 ) {
        cprintf("\n\1f\1rYou must change the Admin Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.wizard) < 2 ) {
        cprintf("\n\1f\1rYou must change the Wizard Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.sysop) < 2 ) {
        cprintf("\n\1f\1rYou must change the Sysop Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.programmer) < 2 ) {
        cprintf("\n\1f\1rYou must change the Programmer Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.roomaide) < 2 ) {
        cprintf("\n\1f\1rYou must change the Roomaide Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.guide) < 2 ) {
        cprintf("\n\1f\1rYou must change the Guide Name before you can save!\n");
        return FALSE;
    }
    if( strlen(cfg.idle) < 2 ) {
        cprintf("\n\1f\1rYou must change the Idle Name before you can save!\n");
        return FALSE;
    }

    return TRUE;
}
/* eof */
