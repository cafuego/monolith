/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* define SQL_CONFIGS */

#define HAVE_FUN_C 1

#include <ctype.h>
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
#include "ext.h"

#include "bbsconfig.h"
#include "input.h"
#include "key.h"  
#include "msg_file.h"
#include "quadcont.h"
#include "registration.h"
#include "read_menu.h"
#include "rooms.h"
#include "setup.h"
#include "telnet.h"

#define extern
#include "commands.h"
#undef extern


void
main_menu()
{
    new_message_system();

}

void
sysop_menu()
{

    register char cmd = '\0';
    int i = 0;

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

            case 'A':
                cprintf("Dump all posts into SQL.\n");
                for (i = 0; i <= 75; i++)
                    bingle(i);
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
		char filename[L_FILENAME + 1];

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

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("\1f\1w(\1rRoom Options\1w)\1a\n");
	    more(MENUDIR "/menu_room", 1);
	    display_short_prompt();
	    cprintf("\1f\1pAdmin cmd: \1rRoom cmd: \1a");
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
		cprintf("\1f\1rKickout %s from %s.\1a\n", config.user, config.forum);
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
	display_short_prompt();
	cprintf("\1f\1wAdmin cmd: \1rQuadrant cmd: \1a");
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
		userlist_menu();
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
		cprintf("\1f\1wEdit %s Description.\1a\n", config.forum);
		change_forum_info();
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

	cmd = get_single_quiet("bBFrRP*\r\b ?");

	if (strchr("bBDEUPp", cmd))
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

	    case 'R':
		cprintf("\1f\1bReset a %s.  \1rTemporarily Disabled.\n%s", config.forum,
			"Use <a>dmin <r>oom, and set lowest and highest to 0.\n\1a");
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


/*************************************************
* config_menu()
*************************************************/

void
config_menu()
{

    char newb[40], mesname[20];
    register char cmd = '\0';
    int i = 1;

    strcpy(mesname, "");
    sprintf(mesname, "%s",config.message);
    mesname[0] = toupper(mesname[0]);

    while ((cmd != SP) && (cmd != 13) && (cmd != 'Q')) {
	IFNEXPERT
	{
	    cprintf("(Config Options)\n");
	    more(MENUDIR "/menu_config", 1);
	    display_short_prompt();
	    cprintf("\1gConfig: \1w\1f");
	}

	cmd = get_single_quiet("AabBCcDEFHiIKLMOPsSTWY\r\b ?/");

	IFTWIT
	    if (strchr("acDFHIKLMW", cmd)) {
	    more(TWITMSG, 1);
	    continue;
	}
	if (strchr("AbaCcEFIDKMPTWY?", cmd))
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

	    case 'M':
		cprintf("\1f\1g%s Menu.\n", mesname);
		menu_message();
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
		cprintf("\1f\1gChange WWW url.\n");
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
	    display_short_prompt();
	cprintf("\1gConfig: \1w\1f");
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

	cmd = get_single_quiet("lLmMrsxz\r\b ?/");

	switch (cmd) {
            case 'l':
            case 'L':
                nox = 1;
                cprintf("\1f\1gLock Terminal.\n");
                lock_terminal();
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
		reset_lastseen_message();
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

