/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
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

#include "newuser.h"

#include "input.h"
#include "key.h"
#include "main.h"
#include "usertools.h"
#include "registration.h"
#include "routines2.h"

static void newuser_getname(char * name);
static void newuser_getpasswd( user_id_t user_id);
static void newuser_registration(user_t * user);
static user_t *newuser_makesupp(void);
static int check_lockout(const char *hostname);

/*************************************************
* new_user()
*************************************************/

void
new_user(const char *hostname)
{
    int i = 0;
    unsigned int usernum;

    if( check_lockout(hostname) == FALSE )
        logoff(0);
    usersupp = newuser_makesupp();

    more(BBSDIR "/share/newuser/welcome_about", 0);
    cprintf("\1f\1gPress a key..");
    inkey();	
    more(BBSDIR "/share/newuser/welcome_commands", 0);
    cprintf("\1f\1gPress a key..");
    inkey();
    more(BBSDIR "/share/newuser/welcome_rules", 0);
    cprintf("\1f\1gPress a key..");
    inkey();
    more(BBSDIR "/share/newuser/welcome_username", 0);
 
    newuser_getname(usersupp->username);

    /* now we have a name, try to get a number */
    get_new_usernum( usersupp->username, &usernum );
    usersupp->usernum = usernum;

    newuser_getpasswd(usersupp->usernum);
    newuser_registration(usersupp);

    test_ansi_colours(usersupp);
    cprintf("\1f\1g");
    more(BBSDIR "/share/newuser/hideinfo", 0);
    usersupp->hidden_info = H_REALNAME | H_ADDRESS | H_CITY | H_COUNTRY
		    | H_PHONE | H_EMAIL | H_URL | H_BIRTHDAY;
    usersupp->flags |= US_HIDDENHOST;

    cprintf("\n\1f\1gPress a key to continue.. \1a");
    inkey();
    toggle_hidden_info(usersupp);

    config_menu();

    cprintf("\1f\1g\nA personal validation key is being generated and sent to the\n");
    cprintf("following email address: `%s'.\n\n", usersupp->RGemail);
    cprintf("Sending... ");
    fflush(stdout);

    generate_new_key(usersupp->usernum);

    i = send_key(usersupp->usernum );

    switch (i) {

	case 0:
	    more(BBSDIR "/share/newuser/key_done", 0);
	    break;

	case 1:
            more(BBSDIR "/share/newuser/key_invalid_chars", 0);
	    break;

	default:
            more(BBSDIR "/share/newuser/key_unknown_error", 0);
	    break;
    }

    fflush(stdout);
    sleep(1);

    /* final message to the new user */
    cprintf("Press a key to continue.. ");
    inkey();
    more(BBSDIR "/share/newuser/final", 1);

    log_it("newuserlog", "Created user '%s' from host '%s'", usersupp->username, hname);
    writeuser(usersupp, 0);
    mono_sql_u_update_registration( usersupp->usernum,
        usersupp->RGname, usersupp->RGaddr, usersupp->RGzip, usersupp->RGcity,
        usersupp->RGstate, usersupp->RGcountry, usersupp->RGphone );
    mono_sql_u_update_hidden( usersupp->usernum, usersupp->hidden_info );
    mono_sql_u_update_email( usersupp->usernum, usersupp->RGemail );
    mono_sql_u_update_url( usersupp->usernum, usersupp->RGurl );

    cprintf("Press a key to continue.. \1a");
    inkey();
    cprintf("\n");
    return;
}

static user_t *
newuser_makesupp()
{
    int a;
    long aa;
    user_t *user;

    user = (user_t *) xcalloc(1, sizeof(user_t));

    strcpy(user->username, "newbie");

    for (a = 0; a < MAXQUADS; a++) {
	user->lastseen[a] = 0L;
	user->generation[a] = -1;
	user->forget[a] = -1;
    }
    user->flags = (US_BEEP | US_NOHIDE | US_PAUSE | US_NOTIFY_FR);
    user->priv = PRIV_NEWBIE;
    user->chat = 16;
    user->screenlength = 24;

    for (a = 0; a < 5; a++)
	user->RA_rooms[a] = -1;

    strcpy(user->lasthost, "\1rnewbie.com");
    strcpy(user->doing, "\1Y\1bI'm a tea drinker!");

    time(&aa);
    user->laston_from = aa;
    user->laston_to = aa;
    user->firstcall = aa;
    return user;
}

static void
newuser_getname(char *name)
{

    for (;;) {
	cprintf("\1f\1gUsername: \1w");
	strcpy(name, get_name(2));

	if (strlen(name) < 1)
	    continue;

	if (mono_sql_u_check_user(name) == TRUE) {
	    cprintf("\1rSorry, that name is already taken; try another one.\n");
	    continue;
	}
	if (strlen(name) > 16) {
	    cprintf("\1rSorry, but that name is too long.\n");
	    continue;
	}
	if ((taboo(name)) || EQ(name, "new")) {
	    cprintf("\1rSorry, but we do not accept \1w\"\1y%s\1w\"\1r as name.\n", name);
	    continue;
	}
	cprintf("\1gThe username you entered is \1w'\1r%s\1w'\1g.\n", name);
	cprintf("Are you sure want to use this name?? ");

	if (yesno() == NO) {
	    cprintf("\1yOk, do you want to quit?");
	    if (yesno() == YES) {
		logoff(ULOG_OFF);
	    } else
		continue;
	}
	cprintf("\n");
	break;
    }
    return;
}

static void
newuser_getpasswd( user_id_t user_id)
{
    char pwread[20], pwtest[20];	/* for password validation */
    int done = FALSE;

    more(BBSDIR "/share/newuser/password", 1);

    while (!done) {
	cprintf("\1f\1gPlease enter a password: ");
	xgetline(pwread, -19, 1);

#ifdef USED
/*       have to comment this out. i don't know what it is for
		michel 20/09/1999 */
	if ((!strlen(pwread)) && (user->timescalled > 0))
	    return;
#endif

	if (strlen(pwread)) {
	    cprintf("\1f\1gPlease enter it again: ");
	    xgetline(pwtest, -19, 1);

	    if (strcmp(pwtest, pwread) != 0)
		cprintf("\1rThe passwords you typed didn't match.  Please try again.\n");
	    else
		done = TRUE;
	}
    }

    mono_sql_u_set_passwd( user_id, pwread );
    cprintf("\n");
    return;
}

static void
newuser_registration(user_t * user)
{
    char p[RGurlLEN];
    int ret;

    cprintf("\1f\1g");
    more(BBSDIR "/share/newuser/registration", 0);

    cprintf("Please enter your real name.\n");
    do {
	cprintf("\n* FULL REAL name: ");
	xgetline(user->RGname, RGnameLEN, 1);
    }
    while (strlen(user->RGname) <= 6);

    cprintf("\nNext, enter your address (street name, and number).\n\n");
    do {
	cprintf("* Address:        ");
	xgetline(user->RGaddr, RGaddrLEN, 1);
    } while (strlen(user->RGaddr) < 3);

    do {
	cprintf("* City/Suburb:    ");
	xgetline(user->RGcity, RGcityLEN, 1);
    } while (strlen(user->RGcity) < 2);

    do {
	cprintf("* Zip code:       ");
	xgetline(user->RGzip, RGzipLEN, 1);
    } while (strlen(user->RGzip) < 3);

	cprintf("  State/Province: ");
	strcpy( user->RGstate, "" ); /* initialise, or get garbage */
	xgetline(user->RGstate, RGstateLEN, 1);

    do {
	cprintf("* Country:        ");
	xgetline(user->RGcountry, RGcountryLEN, 1);
    } while (strlen(user->RGcountry) < 3);

	cprintf("  Phone number:   ");
	strcpy( user->RGphone, "" ); /* initialise or get garbage! */
	xgetline(user->RGphone, RGphoneLEN, 1);

    more(BBSDIR "/share/newuser/email", 0);

    cprintf("\n\1f\1gPlease enter your e-mail address in the form: \1wuser@host.domain.edu\n");
    fflush(stdout);
    sleep(3);

    do {
	cprintf("\n\n* Email address:  ");
	xgetline(user->RGemail, RGemailLEN, 1);
    }
    while ((strlen(user->RGemail) < 7)
	   || (is_allowed_email(user->RGemail) == FALSE));

    cprintf("\nYou can enter a homepage address (URL) if you wish.\n");
    cprintf("Enter it in the form: host.machine.edu/directory\n");
    cprintf("If you don't have a homepage, or don't understand this ");
    cprintf("just press return.\n\n");

    fflush(stdout);

    cprintf("\n\nURL:              http://");
    xgetline(p, RGurlLEN - 7, 1);
    if (strlen(p) < 5) {
	strcpy(user->RGurl, "");
	cprintf("URL left blank.\n");
    } else {
        strremcol( p );
	sprintf(user->RGurl, "http://%s", p);
    }

    ret = mono_sql_u_update_registration( usersupp->usernum,
        usersupp->RGname, usersupp->RGaddr, usersupp->RGzip, usersupp->RGcity,
        usersupp->RGstate, usersupp->RGcountry, usersupp->RGphone );
    if ( ret == -1 ) fprintf( stderr, "Problems saving registration info.\n" );
    ret = mono_sql_u_update_email( usersupp->usernum, usersupp->RGemail );
    if ( ret == -1 ) fprintf( stderr, "Problems saving email.\n" );
    ret = mono_sql_u_update_url( usersupp->usernum, usersupp->RGurl );
    if ( ret == -1 ) fprintf( stderr, "Problems saving URL.\n" );

    cprintf("\n\1f\1gYou have entered the following information:\n\n");
    dis_regis(user, TRUE);

    cprintf("\n\1wPlease check that it is correct!\n");
    fflush(stdout);
    sleep(3);

    cprintf("\1f\1g\n\nIs the information correct so far? \1w(\1ry\1w/\1rn\1w) ");
    if (yesno() == NO)
	change_info(user, TRUE);	/* so they can edit their name */
    return;
}

static int
check_lockout(const char *hostname) {

    char frog[85];
    FILE *fp = NULL;

    if (fexists(".nonew")) {
        more(NONEWMSG, 0);
        sleep(1);
        return 0;
    }

    fp = xfopen(LOCKOUTFILE, "r", FALSE);
    if (fp != NULL) {
        while (fgets(frog, 84, fp)) {
            if (frog[0] == '#' || strlen(frog) <= 2)
                continue;       /* #'s are like REM's */

            frog[strlen(frog)-1] = '\0';

            if (strstr(hostname, frog) != NULL) {
                fclose(fp);
                cprintf("\n\rYou are not welcome here. Your login attempt has been logged.\n\r");
                more(BBSDIR "/share/newuser/prohibnew", 0);
                log_it("prohiblog", "Failed new user login from %s", hostname);
                sleep(2);
                return 0;
            }
        }

    }
    fclose(fp);
    return 1;
}
/* eof */
