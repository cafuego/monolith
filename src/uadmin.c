/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

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
#include "friends.h"
#include "input.h"
#include "registration.h"
#include "routines2.h"
#include "usertools.h"

#define extern
#include "uadmin.h"
#undef extern

static void edit_field(user_t * userdata, int fieldnum);
static int mode_string(char *, user_t *);


static int uadmin_need_rewrite;

/*************************************************
* useradmin()
*************************************************/

void
useradmin( const char *username )
{
    char oldname[L_USERNAME + 1], f_bef[16], f_aft[16];
    char name[L_USERNAME + 1];
    int command = 0, isonline = 0;
    unsigned int oldpriv;
    user_t *user;

    /* if these were initalized up there, it doesn't work.. */
    uadmin_need_rewrite = FALSE;

    /* Get a name if nothing was passed */
    if( username == NULL ) {
        cprintf("User to edit: ");
        strcpy(name, get_name(2));

        if (strlen(name) == 0)
	    return;
    } else	/* name was passed to function */
        snprintf(name, L_USERNAME, "%s", username);

    if (mono_sql_u_check_user(name) == FALSE) {
	cprintf("\1rNo such user.\n");
	return;
    }
    user = readuser(name);

    if (user == NULL) {
	cprintf("\1r\1fCan not read user %s\n\1a", name);
	return;
    }
    if ((user->priv > usersupp->priv) && (usersupp->priv < PRIV_WIZARD)) {
	cprintf("\n\1rYou cannot edit a user whose priv are higher then yours.\n");
	xfree(user);
	return;
    }
    mode_string(f_bef, user);

    strcpy(oldname, user->username);
    oldpriv = user->priv;

    if (mono_return_pid(oldname) != -1)
	isonline = TRUE;
    else
	isonline = FALSE;

    while (command != '\r' && command != '\n' && command != ' ') {

	cprintf("\n\1a\1f\1yUser       \1w: \1g%-20s    \1w| \1r(\1g%c\1r) \1w[\1rC\1w]\1cool         \1r(\1g%c\1r) \1w[\1rL\1w]\1cag\n",
		user->username, (user->flags & US_COOL) ? '*' : ' ',
		(user->flags & US_LAG) ? '*' : ' ');

	cprintf("\1w[\1r#\1w] \1yUsernum\1w: \1y#%lu   \1w| \1r(\1g%c\1r) \1w[\1rD\1w]\1conator      \1r(\1g%c\1r) \1w[\1rH\1w]\1celpTerm\n",
		user->usernum, (user->flags & US_DONATOR) ? '*' : ' ',
		(user->flags & US_GUIDE) ? '*' : ' ');

	cprintf("\1yLast from  \1w: \1y%-23s \1w| \1r(\1g%c\1r) \1w[\1rB\1w]\1cad User     \1r(\1g%c\1r) \1w[\1rQ\1w]\1cuadleader\n",
		user->lasthost, (user->flags & US_IAMBAD) ? '*' : ' ',
		(user->flags & US_ROOMAIDE) ? '*' : ' ');

	cprintf("\1w-----------+-------------------------+--------+------------------------------\n");

	cprintf("    \1yName   \1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y9.Emperor",
		user->RGname, user->priv & PRIV_WIZARD ? '*' : ' ');

	if (user->flags & US_LAG)
	    cprintf("\1bL");
	cprintf("\n");

	cprintf("\1w[\1rI\1w] \1yAddress\1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y8.Systems Analyst\n",
		user->RGaddr, user->priv & PRIV_TECHNICIAN ? '*' : ' ');

	cprintf(" \1rn  \1yCity   \1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y7.Fleet Comm. %s\n",
		user->RGcity, user->priv & PRIV_SYSOP ? '*' : ' ', isonline ? "\1w\1e**********\1a\1f" : "");

	cprintf(" \1rf  \1yCountry\1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y6.Preferred   %s\n",
		user->RGstate, user->priv & PRIV_PREFERRED ? '*' : ' ', isonline ? "\1w\1e*        *\1a\1f" : "");

	cprintf(" \1ro  \1yZIPCode\1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y5.Newbie      %s\n",
		user->RGzip, user->priv & PRIV_NEWBIE ? '*' : ' ', isonline ? "\1w\1e* \1rONLINE \1w*\1a\1f" : "");

	cprintf("    \1yPhone  \1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y4.Validated   %s\n",
		user->RGphone, user->priv & PRIV_VALIDATED ? '*' : ' ', isonline ? "\1w\1e*        *\1a\1f" : "");

	cprintf("    \1yEmail  \1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y3.Degraded    %s\n",
		user->RGemail, (user->priv & PRIV_DEGRADED) ? '*' : ' ', isonline ? "\1w\1e**********\1a\1f" : "");

	cprintf("\1w-----------+----------------------------------+ \1r(\1g%c\1r) \1y2.Cursed\n",
		(user->priv & PRIV_TWIT) ? '*' : ' ');

	cprintf("\1yLast on    \1w: \1g%-30s   \1w| \1r(\1g%c\1r) \1y1.Deleted\n",
		printdate(user->laston_to, 1), (user->priv & PRIV_DELETED) ? '*' : ' ');

	cprintf("\1w[\1rF\1w]\1ylying   \1w: \1c%-30s\1a\1D\1f\n", user->doing);
	cprintf("\1w[\1rA\1w]\1yideline \1w: \1g%-60s\n", user->aideline);
	cprintf("\1w[\1rp\1w]\1yFlag    \1w: \1g%-60s\n\n", user->xtrapflag);

	cprintf("\1f\1w[\1rspace\1w] \1gAbort  \1w[\1rENTER\1w] \1gSave and quit.\n");
	cprintf("\1f\1w(space is not always guaranteed to work)\n\n\1pChoice\1w> ");

	command = get_single(" 123456789$%AbBCEfFHILpPQW\r\n");

	if (command != '\r' && command != '\n' && command != ' ')
	    edit_field(user, command);

    }
    if (uadmin_need_rewrite == FALSE) {		/* NO CHANGES */
	cprintf("\1gNo changes made.\n");
	return;
    } else if (command == ' ') {	/* ABORT */
	cprintf("\1rAborting edit.\n");
	return;
    } else {
	cprintf("\1rSaving...\n");

	mode_string(f_aft, user);

	if (writeuser(user, 999) == -1) {
	    cprintf("\1f\1rCould not save user %s\n\1a", user->username);
	    xfree(user);
	    return;
	}
	xfree(user);

	if (mono_return_pid(name) != -1) {
	    cprintf("%s is online; sending readsignal to him/her...\n", name);
	    if (kill(mono_return_pid(name), SIGUSR2) == -1)
		cprintf("\1f\1rCould not send update signal to %s.\n\1a", name);
	}
	if (strcmp(f_bef, f_aft) != 0)
	    log_sysop_action("edited %s: <%s> to <%s>", name, f_bef, f_aft);

    }
    return;
}

/*************************************************
* edit_field()
*************************************************/

static void
edit_field(user_t * user, int fieldnum)
{
    char ny[22];
    int n;

    switch (fieldnum) {

	case '$':
	    user->flags ^= US_DONATOR;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '%':
	    break;

	case 'A':
	    cprintf("Enter new aideline: ");
	    getline(user->aideline, 80, 0);
	    uadmin_need_rewrite = TRUE;
	    log_sysop_action("changed %s's aideline to %s"
			     ,user->username, user->aideline);
	    break;

	case 'b':
	    modify_birthday(&(user->birthday));
	    log_sysop_action("changed %s's birthday");
	    uadmin_need_rewrite = TRUE;
	    break;

	case 'B':
	    IFWIZARD
	    {
		user->flags ^= US_IAMBAD;
		uadmin_need_rewrite = TRUE;
	    }
	    break;

	case 'C':
	    IFWIZARD
	    {
		user->flags ^= US_COOL;
		uadmin_need_rewrite = TRUE;
	    }
	    break;
/*
 * case 'E':
 * change_friend_enemy_list(user, -1);
 * log_sysop_action("changed %s's friend list\n", user->username);
 * break;
 * 
 * case 'F':
 * change_friend_enemy_list(user, 1);
 * log_sysop_action("changed %s's enemy list\n", user->username);
 * break;
 */
	case 'f':
	    cprintf("Enter new Flying: ");
	    getline(user->doing, 30, 0);
	    mono_change_online(user->username, user->doing, 14);
	    uadmin_need_rewrite = TRUE;
	    log_sysop_action("modified %s's flying to %s."
			     ,user->username, user->doing);
	    break;

	case 'H':
	    user->flags ^= US_GUIDE;
	    uadmin_need_rewrite = TRUE;
	    log_sysop_action("toggled Help Terminal status for %s."
			     ,user->username);
	    break;

	case 'I':
	    change_info(user, TRUE);
	    log_sysop_action("modified address info for %s."
			     ,user->username);
	    uadmin_need_rewrite = TRUE;
	    break;

	case 'L':
	    user->flags ^= US_LAG;
	    uadmin_need_rewrite = TRUE;
	    break;

	case 'p':
	    cprintf("Enter new profileflag (current is %s): ", user->xtrapflag);
	    getline(user->xtrapflag, 69, 0);
	    uadmin_need_rewrite = TRUE;
	    log_sysop_action("changed %s's profileflag to %s."
			     ,user->username, user->xtrapflag);
	    break;

	case 'P':
	    cprintf("New Password: ");
	    getline(ny, 18, 1);
            set_password( user, ny);
            mono_sql_u_set_passwd( user->usernum, ny );
	    uadmin_need_rewrite = TRUE;
	    log_sysop_action("changed %s's password."
			     ,user->username);
	    break;

	case 'Q':
	    user->flags ^= US_ROOMAIDE;
	    uadmin_need_rewrite = TRUE;
	    break;

	case 'U':
	    user->flags ^= US_REGIS;
	    uadmin_need_rewrite = TRUE;
	    break;

	case 'W':
	    if (usersupp->priv < PRIV_WIZARD)
		break;
	    cprintf("\1a\1f\1wEMPEROR-Options:\n-----------------\n\n");
	    cprintf("\1g Change User\1w[\1rN\1w]\1game.\n");
	    cprintf("\1g Change     \1w[\1rT\1w]\1gotalOnline\n");
	    cprintf("            \1w[\1r1\1w]\1g Permanent user. (%c)\n",
		    (user->flags & US_PERM) ? '*' : ' ');

	    cprintf("\n\1wEMPEROR \1pChoice\1w> \1a");

	    n = get_single_quiet("NT1 \r\n");
	    switch (n) {
		case 'N':
		    break;

		case 'T':
		    {
			char newb[6];
			cprintf("New TotalTime in hours: ");
			getline(newb, 5, 1);
			user->online = atol(newb) * 60;
		    }
		    uadmin_need_rewrite = TRUE;
		    break;

		case '1':
		    user->flags ^= US_PERM;
		    uadmin_need_rewrite = TRUE;
		    break;

	    }
	    break;

	case '9':
	    if (usersupp->priv < PRIV_WIZARD) {
		cprintf("Who do you think you are?\n");
		break;
	    }
	    user->priv ^= PRIV_WIZARD;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '8':
	    if (usersupp->priv < PRIV_WIZARD) {
		cprintf("Who do you think you are?\n");
		break;
	    }
	    user->priv ^= PRIV_TECHNICIAN;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '7':
	    if (usersupp->priv < PRIV_WIZARD) {
		cprintf("Who do you think you are?\n");
		break;
	    }
	    user->priv ^= PRIV_SYSOP;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '6':
	    user->priv ^= PRIV_PREFERRED;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '5':
	    user->priv ^= PRIV_NEWBIE;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '4':
	    user->priv ^= PRIV_VALIDATED;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '3':
	    user->priv ^= PRIV_DEGRADED;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '2':
	    user->priv ^= PRIV_TWIT;
	    uadmin_need_rewrite = TRUE;
	    break;

	case '1':
	    user->priv ^= PRIV_DELETED;
	    uadmin_need_rewrite = TRUE;
	    break;

	default:
	    cprintf("No such option.\n");
	    break;
    }

}

/*************************************************
* log_sysop_action()
*
* SysopLOG-log-function, made by Brisi, programmer
* at KarantaniaNet BBS.
*
* Changed by KHaglund: now you don't have to
* sprintf() before using this function. Use it as
* log_sysop_action("%d %s", number, name);
*************************************************/

int
log_sysop_action(const char *event,...)
{
    FILE *fp;
    time_t t;
    va_list ptr;
    char work[100];

    sprintf(work, "%s%s", SYSOPLOGDIR, usersupp->username);
    name2file(work);

    fp = xfopen(work, "a", FALSE);
    if (fp == NULL)
	return -1;

    t = time(0);
    strcpy(work, ctime(&t));
    /* remove year and trailing '\n' */
    work[strlen(work) - 6] = '\0';
    fprintf(fp, "%s: ", work);

    va_start(ptr, event);
    vfprintf(fp, event, ptr);
    va_end(ptr);

    (void) fputc('\n', fp);
    (void) fclose(fp);
    return 0;

}

static int
mode_string(char *string, user_t * user)
{

    sprintf(string, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
	    (user->flags & US_EXPERT) ? 'E' : ' ',
	    (user->flags & US_DONATOR) ? '$' : ' ',
	    (user->flags & US_ROOMAIDE) ? 'Q' : ' ',
	    (user->flags & US_GUIDE) ? 'H' : ' ',
	    (user->flags & US_COOL) ? 'C' : ' ',
	    (user->flags & US_LAG) ? 'L' : ' ',
	    (user->priv & PRIV_WIZARD) ? '9' : ' ',
	    (user->priv & PRIV_TECHNICIAN) ? '8' : ' ',
	    (user->priv & PRIV_SYSOP) ? '7' : ' ',
	    (user->priv & PRIV_PREFERRED) ? '6' : ' ',
	    (user->priv & PRIV_CHATMODE) ? '5' : ' ',
	    (user->priv & PRIV_VALIDATED) ? '4' : ' ',
	    (user->priv & PRIV_DEGRADED) ? '3' : ' ',
	    (user->priv & PRIV_TWIT) ? '2' : ' ',
	    (user->priv & PRIV_DELETED) ? '1' : ' ');
    return 0;
}

void
namechange(void)
{
    char newname[L_USERNAME + 1], oldname[L_USERNAME + 1];
    char command[200];
    unsigned int num;
    int i;
    user_t *new_userfile = NULL, *old_userfile = NULL;
    btmp_t *btmp_ptr;
    pid_t kick_pid;

    cprintf("\1f\1rRename a user.\n\nEnter user to rename: ");
    strcpy(oldname, get_name(2));

    cprintf("\1f\1r\n\1gEnter new name\1w[\1y%s\1w] \1c", oldname);
    strcpy(newname, get_name(2));
    if (!strlen(newname))
	return;

    if (mono_sql_u_check_user(newname) == TRUE) {
	cprintf("\1f\1rName already exists.\n\1a");
	return;
    }
    cprintf("\1f\1gChange \1w[\1y%s\1w]\1g to \1w[\1g%s\1w]\1g? ",
	    oldname, newname);
    if (yesno() == NO)
	return;

    if (mono_sql_u_name2id(oldname, &num) == -1) {
	cprintf("\n\1f\1rCouldn't get sql usernumber, aborting\n");
	return;
    } else if ((old_userfile = readuser(oldname)) == NULL) {
	cprintf("\n\1f\1rCouldn't read old userfile, aborting\n");
	return;
    }

    if (old_userfile->priv >= PRIV_SYSOP) {
	cprintf("\n\1f\1rSysops have to be renamed by hand.\n");
	xfree(old_userfile);
	return;
    }

    if ((kick_pid = mono_return_pid(oldname)) != -1) {
	kill(kick_pid, SIGTERM);
	cprintf("\n\1f\1r%s is online.  Kicking..", oldname);
	while ((kick_pid = mono_return_pid(oldname)) != -1) {
	    sleep(1);
	    cprintf(".");
	}
	cprintf("  Done.");
    }

    strcpy(command, "");
    sprintf(command, "cp -a %s ",
	    getuserdir(oldname));
    strcat(command,  getuserdir(newname));
    system(command);

    if ((new_userfile = readuser(newname)) == NULL) {
	xfree(old_userfile);
	cprintf("\n\1f\1rCouldn't read new userfile, aborting\n");
	return;
    }

    strcpy(new_userfile->username, newname);
    if (writeuser(new_userfile, 999) == -1) {
	cprintf("\n\1f\1rCouldn't write new userfile, this will need a fix.");
	xfree(old_userfile);
	xfree(new_userfile);
	return;
    }

    old_userfile->priv |= PRIV_DELETED;
    if (writeuser(old_userfile, 999) == -1) 
	cprintf("\n\1f\1rCouldn't write old userfile, flag it as deleted.");
    xfree(new_userfile);
    xfree(old_userfile);

    if (mono_sql_rename_user(num, newname) == -1) {
	cprintf("\n\1f\1rSQL update failed AFTER namechange!");
	return;
    }

    cprintf("\n\1r\1fSignalling all users to reload cache\1w.");
    for (i = shm->first; i != -1; i = btmp_ptr->next) {
	btmp_ptr = &(shm->wholist[i]);
	kill(btmp_ptr->pid, SIGUSR2);
	usleep(200000);
	cprintf(".");
	}

    strcpy(command, "");
    sprintf(command, "rm -r %s",
	    getuserdir(oldname));
    cprintf("\n\n\1f\1rRemove old user dir with command:\n\n\1y%s\1r",
	command); 
    cprintf("\n\n(y/n) :");
    if (yesno() == YES)
        system(command);
    else
	cprintf("\nOk..  leaving it.  Delete by hand.");
    cprintf("\1f\1g\n\nNamechange completed.\n");
}
/* eof */
