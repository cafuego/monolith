/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

/* define SQL_CONFIGS */

#define HAVE_FUN_C 1

#include <ctype.h>
#include <stdio.h>
#include <sys/signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
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
#include "ext.h"

#include "bbsconfig.h"
#include "clipboard.h"
#include "enter_message.h"
#include "express.h"
#include "friends.h"
#include "fun.h"
#include "help.h"
#include "input.h"
#include "main.h"
#include "messages.h"
#include "qc.h"
#include "registration.h"
#include "read_menu.h"
#include "routines2.h"
#include "rooms.h"
#include "telnet.h"
#include "usertools.h"
#include "uadmin.h"

#define extern
#include "commands.h"
#undef extern

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
	    display_short_prompt();
	    cprintf("\1f\1wAdmin cmd: \1a");
	}

	cmd = get_single_quiet("ABCEFgGKMNOPQRTU\r\b ?");

	if (strchr("ABCERFgOP", cmd))
	    nox = 1;		/* is busy, wants no x's */

	switch (cmd) {

#ifdef DUMP_ALL_POSTS_INTO_SQL
            case 'A':
                cprintf("Dump all posts into SQL.\n");
                for (i = 0; i < MAXQUADS; i++)
                    bingle(i);
                break;
#endif

            case 'A':
		/* michel test */
		cprintf( "This is a Euro symbol: ¤\n" );
		cprintf( "This is a Euro symbol too: €\n" );
		cprintf( "This is a Cent symbol: ¢\n" );
/*
		mono_sql_ut_update_lastseen( usersupp->usernum, curr_rm, 0 );
*/
 		break;


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

	    case 'F': {
		char filename[L_FILE + 1];

		cprintf("\1f\1wPost a file in this room. Specify absolute pathname.\n");
		cprintf("Filename:\1a ");
		fflush(stdout);
		getline(filename, 80, 1);
		if (!fexists(filename)) {
		    cprintf("\n\n\1f\1rCouldn't locate file %s", filename);
		    break;
		}
		if (fexists(temp))
		    unlink(temp);
		copy(filename, temp);
		enter_message(curr_rm, EDIT_NOEDIT, FILE_POST_BANNER, NULL);
		break;
	    }
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

	    case 'P':
		cprintf("\1f\1wPost as %s\n", config.sysop);
		enter_message(curr_rm, EDIT_NORMAL, SYSOP_BANNER, NULL);
		continue;

	    case 'R':
		cprintf("\1f\1r%s cmd: \1a", config.forum);
		sysoproom_menu();
		continue;

	    case 'U':
		cprintf("\1f\1rUser cmd: \1a");
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
	display_short_prompt();
	cprintf("\1f\1wAdmin cmd: \1a");
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
	    display_short_prompt();
            cprintf("\1f\1wAdmin cmd: \1pConfig cmd: \1c");
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
	display_short_prompt();
        cprintf("\1f\1wAdmin cmd: \1pConfig cmd: \1c");
    }
    return;
}

void
sysoproom_menu()
{

    register char cmd = '\0';

    while ((cmd != SP) && (cmd != 13)) {
	IFNEXPERT
	{
	    cprintf("\1f\1w(\1rRoom Options\1w)\1a\n");
	    more(MENUDIR "/menu_room", 1);
	    display_short_prompt();
	    cprintf("\1f\1pAdmin cmd: \1r%s cmd: \1a", config.forum );
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
		change_forum_info();
		break;

	    case 'E':
		cprintf("\1f\1rEdit %s.\1a\n", config.forum);
		editroom();
		break;

	    case 'G':
		cprintf("\1f\1rGenerate low-traffic %s post.\1a\n", config.forum);
		enter_message(SYSOP_FORUM, EDIT_NOEDIT, QUADLIZARD_BANNER, NULL);
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
		cprintf("\1f\1pList of %s.\1a\n", config.forum_pl);
		look_into_quickroom(1);
		break;

	    case 'N':
		cprintf("\1f\1rNoteBook-utility.\1a\n");
		notebook(1);
		break;

	    case 'P':
		cprintf("\1f\1rPost as " ROOMAIDETITLE ".\1a\n");
		enter_message(curr_rm, EDIT_NORMAL, QL_BANNER, NULL);
		break;

	    case 'Q':
#ifdef QC_ENABLE
		cprintf("\1f\1rQuadcont Menu.\1a\n");
		new_qc();
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
		back( 6 + strlen( config.forum ) );
		return;
	}
	display_short_prompt();
	cprintf("\1f\1wAdmin cmd: \1r%s cmd: \1a", config.forum );
    }
    return;
}


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
	    display_short_prompt();
	    cprintf("\1f\1wAdmin cmd: \1rUser cmd: \1a");
	}

	cmd = get_single_quiet("EDkKLnv\r\b ?");

	if (strchr("BEKLvn", cmd))
	    nox = 1;

	switch (cmd) {
	    case 'D': {
		char user[L_USERNAME+1];
		cprintf("\1f\1rDelete a user.\1a\n");
		cprintf("User to be deleted: ");
		getline( user, L_USERNAME, 1 );
		if ( strlen( user ) < 1 ) break;
		cprintf( "Are you sure you want to delete %s ? (y/n)", user );
		if ( yesno() == YES ) {
   	 		del_user( user );
		}
		break;
		}


	    case 'E':
		cprintf("\1f\1rEdit a user.\1a\n");
		useradmin( NULL );
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
		if (!mono_sql_u_check_user(p)) {
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
		userlist_menu();
		break;

	    case 'v':
		cprintf("\1f\1gVerify users' emailaddress.\n");
		cprintf("\1f\1rDisabled\n");
		break;

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
		back(10);
		return;
	}

	    display_short_prompt();
	cprintf("\1f\1wAdmin cmd: \1rUser cmd: \1a");

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
	    display_short_prompt();
	    cprintf(ROOMAIDETITLE " cmd: \1a");
	}
	cmd = get_single_quiet("DEiIkKNPRTW\r\b ?");

	if (strchr("DEIK", cmd))
	    nox = 1;

	switch (cmd) {

	    case 'D':
		cprintf(_("\1f\1wEdit %s Description.\1a\n"), config.forum);
		change_forum_info();
		break;

	    case 'E':
		cprintf(_("\1f\1wEdit %s.\1a\n"), config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/edit", 1);
		editroom();
		break;

	    case 'i':
		cprintf("\1f\1wInvite user to %s.\1a\n", config.forum);
		if (usersupp->flags & US_ADMINHELP)
		    more(QUADRANT "/invite", 1);
		do_invite();
		break;

            case 'I': 
		invite_menu();
                break;
            

	    case 'k':
		cprintf(_("\1f\1wKick user from %s.\1a\n"), config.forum);
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
		enter_message(curr_rm, EDIT_NORMAL, QL_BANNER, NULL);	
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
	    display_short_prompt();
	cprintf(ROOMAIDETITLE " cmd: \1a");

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
	    display_short_prompt();
	    cprintf("\1f\1wAdmin cmd: " WIZARDTITLE " cmd: \1a");
	}

	cmd = get_single_quiet("bBFrPQRS*\r\b ?");

	if (strchr("bBDEUPQSp", cmd))
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

	    case 'P':
		cprintf("\1f\1wPost as %s\n", config.wizard);
		enter_message(curr_rm, EDIT_NORMAL, EMP_BANNER, NULL);
		break;

           case 'Q':
                cprintf("\1f\1rFix quickroom info? (y\n) \1c");
                if( yesno() == YES )
                    (void) mono_sql_f_fix_quickroom();
 		break;

	    case 'R':
		cprintf("\1f\1bReset a %s.  \1rTemporarily Disabled.\n%s", config.forum,
			"Use <a>dmin <r>oom, and set lowest and highest to 0.\n\1a");
		break;

           case 'S':
                cprintf("\1f\1rConfigure SQL \1w(\1ry\1w/\1rN\1w) \1c");
                if( yesno_default(NO) == YES )
                    (void) configure_sql();
 		break;

	    case '?':
		cprintf("\1f\1p(" WIZARDTITLE " Options\1p)\1a\n");
		more(MENUDIR "/menu_emp", 1);
		break;

	    default:
		back(13);
		return;
	}

	    display_short_prompt();
	cprintf("\1f\1wAdmin cmd: Emperor cmd: \1a");
    }
}


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
	    display_short_prompt();
	    cprintf("\1gMisc: \1w");
	}

	cmd = get_single_quiet("lLmMkrsxz\r\b ?/");

	switch (cmd) {
            case 'l':
            case 'L':
                nox = 1;
                cprintf(_("\1f\1gLock Terminal.\n"));
                lock_terminal();
                break;

            case 'k':
		nox = 1;
		cprintf("\1f\1gChange Locale.\n");
#ifdef ENABLE_NLS
		cprintf("Current locale: %s\n", usersupp->lang );
		cprintf("\1f\1gChoose Language\1w: \1c" );
 		getline( usersupp->lang, L_LANG, 1 );
                { extern int  _nl_msg_cat_cntr;
                  setlocale( LC_MESSAGES, usersupp->lang );
                  ++_nl_msg_cat_cntr;
                }
#else
		cprintf("Locales not supported.\n");
#endif
                break;

	    case 'm':
		if (usersupp->priv & PRIV_ALLUNVALIDATED)
		    break;
	        if (!(usersupp->priv & PRIV_VALIDATED))
		    break;

		nox = 1;
		cprintf("\1f\1cMail Direct.\n");
		enter_message(MAIL_FORUM, EDIT_NORMAL, NO_BANNER, NULL);
		break;

	    case 'M':
		cprintf(_("\1f\1cMark messages as read.\n"));
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
		cprintf(_("\1f\1cSystem Configuration.\n"));
		print_system_config();
		break;

	    case 'x':
		cprintf("\1f\1cReset Lastseen in this %s.\n", config.forum);
		reset_lastseen_message();
		break;

	    case 'z':
		cprintf("\1f\1cZap all %ss.\n", config.forum);
		zap_all();
		break;


	    case '/':
		cprintf(_("\1f\1w(\1gMisc. Options\1w)\n"));
		more(MENUDIR "/menu_misc", 1);
		break;
 
	    case '?':
		online_help('i');
		break;

	    default:
		back(6);
		return;
	}
	    display_short_prompt();
	cprintf("\1f\1gMisc: \1c");

    }
}


void
userlist_menu()
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


void
lock_terminal(void)
{
    cmdflags ^= C_LOCK;
    mono_change_online(who_am_i(NULL), " ", 17);
    cprintf("c");
    fflush(stdout);
    cprintf("\n\1f\1w[ \1gTerminal locked for \1y%s@%s \1w]", usersupp->username, config.bbsname );
    fflush(stdout);
    inkey();
    unlock_terminal();
    cmdflags ^= C_LOCK;
    mono_change_online(who_am_i(NULL), " ", 17);
    return;
}

void
unlock_terminal(void)
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

void
yell_menu(void)
{
    register char cmd = '\0';

    static void check_passwd(void);

    cprintf("\n\1f\1gPress \1w<\1rd\1w>\1g to delete your account.\1a\n");
    cprintf("\1f\1gPress \1w<\1rY\1w>\1g to send a Yell to the Administrators.\1a\n");
    cprintf("\1f\1gPress \1w<\1rq\1w>\1g to quit...\1a\n\n");
    cprintf("\1f\1gChoice\1w: \1r");

    cmd = get_single_quiet("dqY ");
    switch (cmd) {

	case 'Y':
	    cprintf("\1f\1g1\1w: \1gYell.\1a\n");
	    enter_yell();
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
    if (mono_sql_u_check_passwd(usersupp->usernum, pwtest) == TRUE) {
	usersupp->priv ^= PRIV_DELETED;
	writeuser(usersupp, 0);
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

