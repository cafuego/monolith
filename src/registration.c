/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

#define extern
#include "registration.h"
#undef extern

#include "main.h"
#include "menu.h"
#include "input.h"
#include "routines2.h"

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
	mono_sql_u_update_registration(usersupp->usernum,
	       usersupp->RGname, usersupp->RGaddr, usersupp->RGzip, usersupp->RGcity,
		 usersupp->RGstate, usersupp->RGcountry, usersupp->RGphone);
	mono_sql_u_update_hidden(usersupp->usernum, usersupp->hidden_info);
	mono_sql_u_update_email(usersupp->usernum, usersupp->RGemail);
	mono_sql_u_update_url(usersupp->usernum, usersupp->RGurl);
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

#define ADDR_NAME	1
#define ADDR_ADDRESS	2
#define ADDR_CITY	4
#define ADDR_ZIP	8
#define ADDR_STATE	16
#define ADDR_COUNTRY	32
#define ADDR_PHONE	64
#define ADDR_EMAIL	128
#define ADDR_URL	256

    void _address_edit(const unsigned int, const long, void *);

    char tempstr[100];
    user_t *tempuser;
    MENU_DECLARE;

    tempuser = (user_t *) xmalloc(sizeof(user_t));

    for (;;) {
	MENU_INIT;
	/* 
	 * WARNING: since all menu object's void pointer function arg
	 * point at the same thing in memory, in this case, a pointer
	 * to a malloc'd user_t, we -DO NOT- want MENU_DESTROY to try
	 * and free one for each menu item!  instead we set 
	 * destroy_void_object in the menu format struct to off, and
	 * free() the object explicitly at the end of the function.
	 */
	the_menu_format.destroy_void_object = 0;

	memcpy(tempuser, user, sizeof(user_t));

	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gRegistration Information Menu\1w]\n\n");

	if (override || (user->priv & PRIV_DEGRADED)) {
	    sprintf(tempstr, "REAL name: %s", tempuser->RGname);
	    MENU_ADDITEM(_address_edit, ADDR_NAME, RGnameLEN,
			 (user_t *) tempuser, "t", tempstr);
	}
	sprintf(tempstr, "Address:   %s", tempuser->RGaddr);
	MENU_ADDITEM(_address_edit, ADDR_ADDRESS, RGaddrLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "City/town: %s", tempuser->RGcity);
	MENU_ADDITEM(_address_edit, ADDR_CITY, RGcityLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "ZIP code:  %s", tempuser->RGzip);
	MENU_ADDITEM(_address_edit, ADDR_ZIP, RGzipLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "State:     %s", tempuser->RGstate);
	MENU_ADDITEM(_address_edit, ADDR_STATE, RGstateLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "Country:   %s", tempuser->RGcountry);
	MENU_ADDITEM(_address_edit, ADDR_COUNTRY, RGcountryLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "Phone:     %s", tempuser->RGphone);
	MENU_ADDITEM(_address_edit, ADDR_PHONE, RGphoneLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "Email:     %s", tempuser->RGemail);
	MENU_ADDITEM(_address_edit, ADDR_EMAIL, RGemailLEN,
		     (user_t *) tempuser, "t", tempstr);

	sprintf(tempstr, "URL:       %s", tempuser->RGurl);
	MENU_ADDITEM(_address_edit, ADDR_URL, RGurlLEN,
		     (user_t *) tempuser, "t", tempstr);

	the_menu_format.gen_1_idx = 1;
	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(1);

	if (!MENU_EXEC_COMMAND)
	    break;

	memcpy(user, tempuser, sizeof(user_t));
	MENU_DESTROY;
    }
    MENU_DESTROY;
    xfree(tempuser);
}

void
_address_edit(const unsigned int fieldname, const long fieldlen, void *user)
{
    user_t *t_user;
    char string[200];

    t_user = (user_t *) user;

    switch (fieldname) {
	case ADDR_NAME:
	    cprintf("\1f\1g\nNew Name \1w[\1g%s\1w]:\1g ", t_user->RGname);
	    break;
	case ADDR_ADDRESS:
	    cprintf("\1f\1g\nNew Address \1w[\1g%s\1w]:\1g ", t_user->RGaddr);
	    break;
	case ADDR_CITY:
	    cprintf("\1f\1g\nNew City \1w[\1g%s\1w]:\1g ", t_user->RGcity);
	    break;
	case ADDR_ZIP:
	    cprintf("\1f\1g\nNew ZIP \1w[\1g%s\1w]:\1g ", t_user->RGzip);
	    break;
	case ADDR_STATE:
	    cprintf("\1f\1g\nNew State \1w[\1g%s\1w]:\1g ", t_user->RGstate);
	    break;
	case ADDR_COUNTRY:
	    cprintf("\1f\1g\nNew Country \1w[\1g%s\1w]:\1g ", t_user->RGcountry);
	    break;
	case ADDR_PHONE:
	    cprintf("\1f\1g\nNew Phone \1w[\1g%s\1w]:\1g ", t_user->RGphone);
	    break;
	case ADDR_EMAIL:
	    cprintf("\1f\1g\nNew Email \1w[\1g%s\1w]:\1g ", t_user->RGemail);
	    break;
	case ADDR_URL:
	    cprintf("\1f\1g\nNew URL \1w[\1g%s\1w]:\1g ", t_user->RGurl);
	    break;
    }

    string[0] = '\0';
    xgetline(string, fieldlen, 1);
    if (strlen(string))
	strremcol(string);
    else
	return;

    switch (fieldname) {
	case ADDR_NAME:
	    strcpy(t_user->RGname, string);
	    break;
	case ADDR_ADDRESS:
	    strcpy(t_user->RGaddr, string);
	    break;
	case ADDR_CITY:
	    strcpy(t_user->RGcity, string);
	    break;
	case ADDR_ZIP:
	    strcpy(t_user->RGzip, string);
	    break;
	case ADDR_STATE:
	    strcpy(t_user->RGstate, string);
	    break;
	case ADDR_COUNTRY:
	    strcpy(t_user->RGcountry, string);
	    break;
	case ADDR_PHONE:
	    strcpy(t_user->RGphone, string);
	    break;
	case ADDR_EMAIL:
	    strcpy(t_user->RGemail, string);
	    break;
	case ADDR_URL:
	    strcpy(t_user->RGurl, string);
	    break;
    }
}

void
toggle_hidden_info(user_t * user)
{

    register char cmd;

    if (!(user->flags & US_NOHIDE)) {
	user->hidden_info = H_REALNAME | H_ADDRESS | H_CITY | H_COUNTRY
	    | H_PHONE | H_EMAIL | H_URL | H_BIRTHDAY | H_ZIP;
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
	cmd = get_single_quiet("1234567890anq? ");

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
	    case '0':
		user->hidden_info ^= H_ZIP;
		break;
	    case 'a':
		user->hidden_info = H_REALNAME | H_ADDRESS | H_CITY | H_COUNTRY
		    | H_PHONE | H_EMAIL | H_URL | H_BIRTHDAY | H_ZIP;
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

    fp = xfopen(BBSDIR "/etc/banned_email", "r", FALSE);
    if (fp == NULL)
	return TRUE;

    while (fgets(buf, 79, fp) != NULL) {
	if (buf[0] == '#')
	    continue;
	buf[(strlen(buf))] = '\0';
	if (EQ(email, buf)) {
	    more(BBSDIR "/share/newuser/prohibemail", 0);
	    fclose(fp);
	    logoff(0);
	    return FALSE;
	}
	if (strstr(email, buf) != NULL) {
	    more(BBSDIR "/share/newuser/email_not_accepted", 0);
	    fclose(fp);
	    return FALSE;
	}
    }
    fclose(fp);
    return TRUE;
}
/* eof */
