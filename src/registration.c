/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "ext.h"

#define extern
#include "registration.h"
#undef extern

#include "main.h"
#include "input.h"
#include "log.h"
#include "routines.h"
#include "routines2.h"
#include "userfile.h"

void
enter_reginfo()
{
    IFNEXPERT
	more(REGISTER, 0);
    cprintf("This is your current personal information:\n");
    dis_regis(usersupp, TRUE);
    cprintf("\n\1wDo you want to change this ? (y/n) ");
    if (yesno() == TRUE) {
	log_it("intlog", "%s changed registration info", usersupp->username);
	change_info(usersupp, FALSE);
	toggle_hidden_info(usersupp);
	writeuser(usersupp, 0);
    }
}

/*************************************************
* dis_regis( user_t *userdata, int override )
* 
* userdate: the user we are looking at.
* override: TRUE if the user can see the hidden
*           address, else FALSE
*************************************************/

void
dis_regis(const user_t * user, int override)
{
    char *p = NULL;
    p = read_regis(user, override);
    more_string(p);
    xfree(p);
    return;
}

/*************************************************
* change_info()
* register a user with name and address 
* override means a user can change everything
* including his/her name.
*************************************************/
void
change_info(user_t * user, int override)
{
    char string[100];
    int done = FALSE;

    while (!done) {
	cprintf("\nPress enter to accept values in brackets.\n");

	if (override || (user->priv & PRIV_DEGRADED)) {
	    cprintf("REAL name: [%s] ", user->RGname);
	    getline(string, RGnameLEN, 1);
	    if (strlen(string) != 0)
		strcpy(user->RGname, string);
	}
	cprintf("Address [%s]: ", user->RGaddr);
	getline(string, RGaddrLEN, 1);
	if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGaddr, string);
        }

	cprintf("City/town [%s]: ", user->RGcity);
	getline(string, RGcityLEN, 1);
	if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGcity, string);
        }

	cprintf("ZIP code [%s]: ", user->RGzip);
	getline(string, RGzipLEN, 1);
	if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGzip, string);
        }

	cprintf("State [%s]: ", user->RGstate);
	getline(string, RGstateLEN, 1);
	if (strcmp(string, "none") == 0)
	    strcpy(user->RGstate, "");
	else if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGstate, string);
        }

	cprintf("Country [%s]: ", user->RGcountry);
	getline(string, RGcountryLEN, 1);
	if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGcountry, string);
        }

	cprintf("Phone number [%s]: ", user->RGphone);
	getline(string, RGphoneLEN, 1);
	if (strcmp(string, "none") == 0)
	    strcpy(user->RGphone, "");
	else if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGphone, string);
        }

	cprintf("Email address [%s]: ", user->RGemail);
	getline(string, RGemailLEN, 1);
	if (strcmp(string, "none") == 0)
	    strcpy(user->RGemail, "");
	else if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGemail, string);
        }

	cprintf("URL [%s]: ", user->RGurl);
	getline(string, RGurlLEN, 1);
	if (strcmp(string, "none") == 0)
	    strcpy(user->RGurl, "");
	else if (strlen(string) != 0) {
            strremcol( string );
	    strcpy(user->RGurl, string);
        }
	cprintf("\n");

	cprintf("*** You have entered the following:\n");
	dis_regis(user, TRUE);

	cprintf("\nIs this correct (y/n)? ");
	if (yesno() == YES)
	    done = TRUE;
    }
    return;
}

void
toggle_hidden_info(user_t * user)
{

    register char cmd;

    if (!(user->flags & US_NOHIDE)) {
	user->hidden_info = H_REALNAME | H_ADDRESS | H_CITY | H_COUNTRY
	    | H_PHONE | H_EMAIL | H_URL | H_BIRTHDAY;
	user->flags ^= US_NOHIDE;
    }
    do {
        time_t online_for;

        online_for = (time(0) - user->laston_from) / 60;

	cprintf("\1f\1c---- This is your info as it is visible for other users:\n");
    
	cprintf("\1f\1y%s\1g\n", user->username);
	dis_regis(user, FALSE);

        cprintf("\1r\1fONLINE \1cfor \1y%ld:%2.2ld \1c",
		 online_for / 60, online_for % 60);

        if (!(usersupp->flags & US_HIDDENHOST))
            cprintf("from \1r%s\1c", user->lasthost);

	cprintf("\n\n\1f\1c---- This is your full info:\n");

	cprintf("\1f\1y%s\1g\n", user->username);
	dis_regis(user, TRUE);
        cprintf("\1r\1fONLINE \1cfor \1y%ld:%2.2ld \1c",
		 online_for / 60, online_for % 60);
        cprintf("from \1r%s\1c\n", user->lasthost);

	more(MENUDIR "/menu_hide_info", 1);
	cmd = get_single_quiet("123456789anq? ");

	switch (cmd) {
	    case '1':
		user->hidden_info ^= H_REALNAME;
		break;
	    case '2':
		user->hidden_info ^= H_ADDRESS;
		break;
	    case '3':
		user->hidden_info ^= H_CITY;
		break;
	    case '4':
		user->hidden_info ^= H_COUNTRY;
		break;
	    case '5':
		user->hidden_info ^= H_PHONE;
		break;
	    case '6':
		user->hidden_info ^= H_EMAIL;
		break;
	    case '7':
		user->hidden_info ^= H_URL;
		break;
	    case '8':
		user->hidden_info ^= H_BIRTHDAY;
		break;
	    case '9':
		user->flags ^= US_HIDDENHOST;
		break;
	    case 'a':
		user->hidden_info = H_REALNAME | H_ADDRESS | H_CITY | H_COUNTRY
		    | H_PHONE | H_EMAIL | H_URL | H_BIRTHDAY;
		user->flags |= US_HIDDENHOST;
		break;
	    case 'n':
		user->hidden_info = 0;
		break;
	    case '?':
		more(MENUDIR "/menu_hide_info", 1);
		break;

	}
    }
    while (cmd != 'q' && cmd != ' ');
    return;
}

int
is_allowed_email(const char *email)
{

    FILE *fp;
    char buf[80];

    fp = xfopen( BBSDIR "etc/banned_email", "r", FALSE );
    if ( fp == NULL )  return TRUE;

    while (fgets(buf, 79, fp) != NULL) {
	if (buf[0] == '#')
	    continue;
	buf[(strlen(buf))] = '\0';
	if (EQ(email, buf)) {
	    more( BBSDIR "share/newuser/prohibemail", 0);
	    fclose(fp);
	    logoff(0);
	    return FALSE;
	}
	if (strstr(email, buf) != NULL) {
	    more( BBSDIR "share/newuser/email_not_accepted", 0);
	    fclose(fp);
	    return FALSE;
	}
    }
    fclose(fp);
    return TRUE;
}
/* eof */
