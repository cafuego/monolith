/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"

#include "input.h"
#include "menu.h"
#include "routines2.h"

void edit_config( void );

/*
 * Edit a configuration
 */
void
edit_config()
{

    config_t *config = NULL;
    int cmd, num = 0;
    char tempstr[23];
    MENU_DECLARE;

    /*
     * list configs and get number to edit
     */

    if( mono_sql_read_config(num, config) == FALSE ) {
        (void) sql_log_error( "Can't read config %d from table %s!", num, CONFIG_TABLE );
    }

    /*
     * do menu shit and allow editing
     */

    for (;;) {
        MENU_INIT;
        strcpy(the_menu_format.menu_title, "\n\1f\1w[\1gEdit BBS Configuration\1w]\n\n");

        MENU_ADDITEM("tiv", "BBS Name       ", "1", config->bbsname );
        MENU_ADDITEM("tiv", "Room Name      ", "2", config->forum );
        MENU_ADDITEM("tiv", "Room Name (pl) ", "3", config->forum_pl );
        MENU_ADDITEM("tiv", "Message        ", "4", config->message );
        MENU_ADDITEM("tiv", "Message (pl)   ", "5", config->message_pl );
        MENU_ADDITEM("tiv", "eXpress        ", "6", config->express );
        MENU_ADDITEM("tiv", "X Message      ", "9", config->x_message );
        MENU_ADDITEM("tiv", "X Message (pl) ", "8", config->x_message_pl );
        MENU_ADDITEM("tiv", "User           ", "9", config->user );
        MENU_ADDITEM("tiv", "User (pl)      ", "A", config->user_pl );
        MENU_ADDITEM("tiv", "Username       ", "B", config->username );
        MENU_ADDITEM("tiv", "Doing          ", "C", config->doing );
        MENU_ADDITEM("tiv", "Location       ", "D", config->location );
        MENU_ADDITEM("tiv", "Idle           ", "E", config->idle );
        MENU_ADDITEM("tiv", "Chat           ", "F", config->chatmode );
        MENU_ADDITEM("tiv", "Channel        ", "G", config->chatroom );
        MENU_ADDITEM("tiv", "Admin          ", "H", config->admin );
        MENU_ADDITEM("tiv", "Wizard         ", "I", config->wizard );
        MENU_ADDITEM("tiv", "Sysop          ", "J", config->sysop );
        MENU_ADDITEM("tiv", "Programmer     ", "K", config->programmer );
        MENU_ADDITEM("tiv", "Roomaide       ", "L", config->roomaide );
        MENU_ADDITEM("tiv", "Guide          ", "M", config->guide );

        MENU_DISPLAY(2);

        MENU_DESTROY;

        cprintf("\n\n  \1g\1fOption, or \1w<\1rreturn\1w>\1g to quit. \1c");
        cmd = get_single_quiet("123456789ABCDEFGHIJKLM\r\b\13");

        switch(cmd) {

            case '1':	/* bbs name */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yBBS\1g' name \1w(\1g%s\1w): \1c", config->bbsname);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->bbsname, tempstr);
                break;

            case '2':	/* room */
            case '3':	/* room (pl) */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yroom\1g' name \1w(\1g%s\1w): \1c", config->forum);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->forum, tempstr);
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", config->forum_pl);
                xgetline(tempstr,22,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->forum_pl, tempstr);
                break;

            case '4':	/* message */
            case '5':	/* message (pl) */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ymessage\1g' name \1w(\1g%s\1w): \1c", config->message);
                xgetline(tempstr,12,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->message, tempstr);
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", config->message_pl);
                xgetline(tempstr,14,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->message_pl, tempstr);
                break;

            case '6':	/* eXpress */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yeXpress\1g' name \1w(\1g%s\1w): \1c", config->express);
                xgetline(tempstr,12,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->express, tempstr);
                break;

            case '7':	/* X message */
            case '8':	/* X message (pl) */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yX message\1g' name \1w(\1g%s\1w): \1c", config->x_message);
                xgetline(tempstr,12,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->x_message, tempstr);
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", config->x_message_pl);
                xgetline(tempstr,14,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->x_message_pl, tempstr);
                break;

            case '9':	/* user */
            case 'A':	/* user (pl) */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yuser\1g' name \1w(\1g%s\1w): \1c", config->user);
                xgetline(tempstr,12,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->user, tempstr);
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter the plural \1w(\1g%s\1w): \1c", config->user_pl);
                xgetline(tempstr,14,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->user_pl, tempstr);
                break;

            case 'B':	/* username */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yusername\1g' name \1w(\1g%s\1w): \1c", config->username);
                xgetline(tempstr,12,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->username, tempstr);
                break;

            case 'C':	/* doing */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ydoing\1g' name \1w(\1g%s\1w): \1c", config->doing);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->doing, tempstr);
                break;

            case 'D':	/* location */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ylocation\1g' name \1w(\1g%s\1w): \1c", config->location);
                xgetline(tempstr,16,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->location, tempstr);
                break;

            case 'E':	/* idle - allow colours */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yidle\1g' message \1w(\1g%s\1w): \1c", config->idle);
                xgetline(tempstr,20,TRUE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->idle, tempstr);
                break;

            case 'F':	/* chat */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ychat\1g' name \1w(\1g%s\1w): \1c", config->chatmode);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->chatmode, tempstr);
                break;

            case 'G':	/* channel */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ychannel\1g' name \1w(\1g%s\1w): \1c", config->chatroom);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->chatroom, tempstr);
                break;

            case 'H':	/* admin */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yadmin\1g' name \1w(%s): \1c", config->admin);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->admin, tempstr);
                break;

            case 'I':	/* wiz */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ywizard\1g' name \1w(%s): \1c", config->wizard);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->wizard, tempstr);
                break;

            case 'J':	/* sysop */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1ysysop\1g' name \1w(\1p%s\1w): \1c", config->sysop);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->sysop, tempstr);
                break;

            case 'K':	/* progger */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yprogrammer\1g' name \1w(\1b%s\1w): \1c", config->programmer);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->programmer, tempstr);
                break;

            case 'L':	/* ql */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yroomaide\1g' name \1w(\1r%s\1w): \1c", config->roomaide);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->roomaide, tempstr);
                break;

            case 'M':	/* guide */
                strcpy(tempstr, "");
                cprintf("\1f\1gEnter a new `\1yguide\1g' name \1w(\1c%s\1w): \1c", config->guide);
                xgetline(tempstr,20,FALSE);
                if( strlen(tempstr) < 1 )
                    strcpy(config->guide, tempstr);
                break;

            default:	/*quit */
                cprintf("\1f\1gSave changes? \1w(\1gY\1w/\1gn\1w) \1c");
                if( yesno_default(YES) == YES ) {
                    if( mono_sql_save_config( num, config ) == FALSE ) {
                        (void) sql_log_error( "Can't save config %d back into %s table!", num, CONFIG_TABLE );
                        cprintf("\1f\1rArgh! Couldn't save the config!\n");
                    }
                } else {
                    cprintf("\1f\1rOK, not saving...\n");
                }
                return;
        } /* switch(cmd) */
    } /* for(;;) */
}
/* eof */
