
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
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#include "version.h"
#include "telnet.h"

#define extern
#include "ext.h"
#include "main.h"
#undef extern

#include "commands.h"
#include "input.h"
#include "express.h"
#include "friends.h"
#include "fun.h"
#include "key.h"
#include "messages.h"
#include "qc.h"
#include "statusbar.h"
#include "newuser.h"
#include "sql_config.h"
#include "sql_login.h"
#include "sql_online.h"
#include "sql_useruser.h"
#include "read_menu.h"
#include "registration.h"
#include "rooms.h"
#include "routines2.h"
#include "wholist.h"

int connecting_flag;
int first_login = FALSE;


char previous_host[L_HOSTNAME + 1];

/*********************************************************************
* sleeping(), dropcarr(), kickoutmyself()
*********************************************************************/

/* The idle function. This function is called every minute to check
 * if someone is idle. It also takes care of the alarm clock. 
 * It works with help of the SIGALRM signal that is sent every 60 secs */

RETSIGTYPE
sleeping(int sig)
{
    char i;

    sig++;
    i = nox;
    nox = 1;
    statusbar(NULL);
    nox = i;

    if (idletime >= 0)
	idletime++;

    if (idletime == 2)
	(void) mono_change_online(who_am_i(NULL), "", 6);

    if (idletime == 5)		/* if been idle for 5 minutes, put in wholist */
	(void) mono_change_online(who_am_i(NULL), "", 2);

#ifndef CLIENTSRC

    if ((idletime % TIMEOUT) == 0) {
	IFSYSOP {
	    cprintf("\n\007\1f\1wBEEP! Idle Admin are bad for the BBS!\nLog out!\1a\n");
	}
	else {
	    cprintf("\n\007\1pIs there life behind this terminal?\n\1rYou will be docked out in 1 minute unless you start looking more alive!\n");
	}
	fflush(stdout);
    } else if (((idletime == TIMEOUT + 1) && (usersupp->priv < PRIV_TECHNICIAN)) ||
	       (idletime >= 5 && connecting_flag == 1))
#else
    if (idletime == TIMEOUTCLIENT) {
	cprintf("%c%c%c", IAC, WILL, TELOPT_LOGOUT);
/*      cprintf("\n\007\1pIs there life behind this terminal?\n\1rYou will be docked out in 1 minute unless you start looking more alive!\n"); */
	fflush(stdout);
    } else if ((idletime == (TIMEOUTCLIENT + 1) && usersupp->priv < PRIV_TECHNICIAN) ||
	       (idletime >= 5 && connecting_flag == 1))
#endif
    {
	cprintf("\n\n\007\1cYou have been logged out due to inactivity.\n");
	logoff(ULOG_SLEEP);

    }
    signal(SIGALRM, sleeping);
    alarm(60);
    return;
}

/*************************************************
* updateself()
* Called by the SIGUSR2-signal
*************************************************/

RETSIGTYPE
updateself(int sig)
{
    user_t *user = NULL;
    unsigned int i;

    sig++;
    user = readuser(usersupp->username);

    if (user == NULL) {
	if (writeuser(user, 0) == -1) {
	    cprintf("\1r\1fEEK! Can't load or save myself!\n ");
	    logoff(0);
	} else {
	    signal(SIGUSR2, updateself);
	    return;
	}
    }
    for (i = 0; i < MAXQUADS; i++) {
	user->forget[i] = usersupp->forget[i];
	user->lastseen[i] = usersupp->lastseen[i];
    }
    xfree(usersupp);
    usersupp = user;
    (void) mono_change_online(who_am_i(NULL), "", 1);
    start_user_cache(usersupp->usernum);
    signal(SIGUSR2, updateself);
    return;
}

RETSIGTYPE
segfault(int sig)
{
    sig++;
    sleep(1);
    cprintf(_("OH NO!!!!!!! A SEGFAULT!!!!!\n "));
    /*
     * Well, it _might_ work... if not, I'll fiddle with ye
     * code for cleaning up a previous login.
     */
    (void) mono_sql_detach();
    logoff(ULOG_PROBLEM);
    return;
}

RETSIGTYPE
dropcarr(int sig)
{
    sig++;
    logoff(ULOG_DROP);
    return;
}

RETSIGTYPE
kickoutmyself(int sig)
{				/* <auk>ick out user sends a signal to this funct. */
    sig++;
    logoff(ULOG_KICKED);
    return;
}

/*************************************************
* enter_passwd()
*************************************************/

#define MAX_PASS_LEN	20

int
enter_passwd(const char *username)
{

    static int failures;
    char *pwtest = NULL;
    user_t *user = NULL;

    pwtest = (char *) xmalloc(MAX_PASS_LEN * sizeof(char));
    strcpy(pwtest, "");

    if (!(mono_sql_u_check_user(username))) {
	cprintf(_("No such user.\n"));
	log_it("badpw", "Login as %s from %s, but user does not exist", username, hname);
	return FALSE;
    }
    user = readuser(username);
    cprintf("\r%s's Password: ", user->username);
    (void) getline(pwtest, -19, 1);

    if (strlen(pwtest) == 0) {
	xfree(pwtest);
	failures++;
	return FALSE;
    }
    if (mono_sql_u_check_passwd(user->usernum, pwtest) == TRUE) {
	xfree(pwtest);
	xfree(user);
	return TRUE;
    }
    cprintf(_("Incorrect login.\n"));
    log_it("badpw", "%s from %s", user->username, hname);

    xfree(pwtest);
    xfree(user);

    if (++failures >= 4) {
	cprintf("Too many failures.\n");
	logoff(-1);
    }
    return FALSE;
}

void
do_changepw()
{

    char pwtest[20];

    cprintf(_("\1f\1gPlease enter your \1rcurrent\1g password: "));
    getline(pwtest, -19, 1);

    if (strlen(pwtest) == 0)
	return;

    if (mono_sql_u_check_passwd(usersupp->usernum, pwtest) == TRUE) {
	change_passwd(usersupp);
	(void) writeuser(usersupp, 0);
	(void) cprintf(_("Password changed.\n"));
    } else
	(void) cprintf(_("Password unchanged.\n"));

    return;
}

/*************************************************
* change_passwd()
*************************************************/

void
change_passwd(user_t * user)
{

    char pwread[20], pwtest[20];	/* for password validation */
    int done = FALSE;

    IFNEXPERT
	more(CHANGEPW, 0);

    while (!done) {
	cprintf(_("Please enter a password: "));
	getline(pwread, -19, 1);

	if (strlen(pwread)) {
	    cprintf(_("Please enter it again: "));
	    getline(pwtest, -19, 1);

	    if (strcmp(pwtest, pwread) != 0)
		cprintf(_("The passwords you typed didn't match.  Please try again.\n"));
	    else
		done = TRUE;
	}
    }

    mono_sql_u_set_passwd(user->usernum, pwread);
    cprintf("\n");

}				/* end change_passwd */

/*************************************************
* enter_name()
*
* enter_name prompts the user for their bbs user-
* name, then returns it.
*************************************************/

void
enter_name(char **username)
{

    char pwordshit[20];

    for (;;) {			/* loop until we get a real username */
	strcpy(*username, "");
	cprintf(_("\nUsername: "));
	strcpy(*username, get_name(1));

	if (!strlen(*username)) {
	    cprintf(_("Enter \"Off\" to quit or \"New\" to enter as a new user.\n"));
	    continue;
	} else if (EQ(*username, "new"))
	    return;

	else if (EQ(*username, "off")
		 || EQ(*username, "logoff")
		 || *username[0] == EOF
	    ) {
	    logoff(ULOG_OFF);
	} else {
	    /* if this user does not exist, fake a password-entry */
	    if (mono_sql_u_check_user(*username) == FALSE) {
		cprintf(_("%s's Password: "), *username);
		(void) getline(pwordshit, -19, 1);
		cprintf(_("Incorrect login.\n"));
		continue;
	    } else
		return;
	}
    }
}				/* enter_name() */


/*********************************************************************
*** *********************** MAIN PROGRAM ************************* ***
*********************************************************************/

int
main(int argc, char *argv[])
{

    int a;			/* misc                         */
    int newbie;			/* newbieflag, TRUE if newuser  */
    int done = FALSE;		/* True when finished login     */
    char hellomsg[40], *my_name = NULL;
    time_t laston;
    pid_t pid;
    char *username = NULL;

/*
 * argv[1]    contains the hostname that the user is calling from
 * argv[2]    contains the username if the user logged in from the queue
 */

#ifdef DEBUG_MEMORY_LEAK
    allocated_ctr = allocated_total = 0;
#endif

    set_invocation_name(argv[0]);
    mono_setuid("guest");
    chdir(BBSDIR);

#ifdef ENABLE_NLS
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, BBSDIR "/share/locale");
    textdomain(PACKAGE);
#endif

    if (getuid() != BBSUID && getgid() != BBSGID) {
	cprintf("You need BBS uid to start this program.\n");
	exit(0);
    }
    if (argc < 2) {
	cprintf("Usage: %s <hostname> [<username>].\n\n", argv[0]);
	strcpy(hname, "localhost");
	exit(0);
    }
    strncpy(hname, argv[1], 79);
    hname[79] = '\0';
    if (hname[0] == '\0')	/* we're from localhost */
	if (gethostname(hname, 79) == -1)
	    strcpy(hname, "localhost");

    connecting_flag = 1;

    store_term();
    atexit(restore_term);
    sttybbs(0);			/* and install the new ones     */

    signal(SIGINT, SIG_IGN);
    signal(SIGSEGV, segfault);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, dropcarr);
    signal(SIGHUP, dropcarr);	/* Cleanup gracefully if dropcarr */
    signal(SIGPIPE, dropcarr);	/* Cleanup gracefully if dropcarr */
    signal(SIGABRT, kickoutmyself);	/* <auk>icked?                  */
    signal(SIGIO, single_catchx);	/* receives sent X at signal    */
    signal(SIGUSR1, multi_catchx);	/* receives BroadCasts and ILCs */
    signal(SIGUSR2, updateself);	/* signalled after <aue>        */
    signal(SIGALRM, sleeping);	/* run once every minute        */
#ifdef OLD
    signal(SIGWINCH, getwindowsize);	/* window size change           */
#endif

    alarm(60);			/* send the first SIGALRM-signal after 60 seconds. */

    cmdflags = 0;		/* no internal command at start       */
    nox = 1;

    sprintf(temp, BBSDIR "/tmp/%d.1", getpid());	/* make up a temp file name     */
    unlink(temp);		/* remove it if it exists       */

    sprintf(tmpname, BBSDIR "/tmp/%d.2", getpid());
    unlink(tmpname);

    mono_sql_connect();

    /* OKAY, get ourselves a basic user, we don't want to rely on savefiles */
    usersupp = (user_t *) xcalloc(1, sizeof(user_t));
    /* usersupp = readuser( "Guest" ); */
    usersupp->screenlength = 35;
    usersupp->priv = 0;

    /* add here the random hello messages */
    srand(time(NULL));
    (void) sprintf(hellomsg, HELLODIR "/hello%d", ((int) random() % 7) + 1);
    (void) more(hellomsg, 1);

    fflush(stdout);
    usersupp->screenlength = 24;
    done = newbie = FALSE;

    /* THE LOGIN PROCEDURE */
    if (argc > 2 && strlen(argv[2]) > 0) {	/* in case of a queue pre-login */
	xfree(usersupp);
	usersupp = readuser(argv[2]);
    } else {
	while (!done) {
	    username = (char *) xmalloc(sizeof(char) * (L_USERNAME + 1));
	    strcpy(username, "");
	    enter_name(&username);	/* find out who the user is     */
	    xfree(usersupp);	/* free and re-allocate this before */
	    usersupp = (user_t *) xcalloc(1, sizeof(user_t));
	    if (EQ(username, "new")) {	/* a new user?                  */
		new_user(hname);
		first_login = TRUE;
		usersupp = readuser(username);
		done = TRUE;
	    } else {
		if (strcasecmp(username, "Guest") != 0) {
		    if ((done = enter_passwd(username)) == TRUE) {
			usersupp = readuser(username);
                    }
		} else {	/* is guest */
		    if (check_user("Guest")) {
			usersupp = readuser("Guest");
			done = TRUE;
		    } else {
			log_it("errors", "Create a guest account!\n");
			cprintf("\1f\1rGuest account is not enabled, sorry\n");
			xfree(username);
			logoff(ULOG_PROBLEM);
		    }
		}
	    }
	}
	xfree(username);
    }

    my_name = (char *) xmalloc(sizeof(char) * (L_USERNAME + 1));
    strcpy(my_name, usersupp->username);

    set_timezone(usersupp->timezone);
    mono_setuid(my_name);
    connecting_flag = 0;

#ifdef ENABLE_NLS
    {
	char env[L_LANG + 10];
	int ret;
	extern int _nl_msg_cat_cntr;

	sprintf(env, "LANGUAGE=%s", usersupp->lang);
	ret = putenv(env);
	IFSYSOP cprintf("Locale: %s\n", env);
	if (ret == -1) {
	    cprintf("Could not set locale.\n");
	}
	++_nl_msg_cat_cntr;
    }
#endif

#ifdef OLD
    getwindowsize(0);
#endif

#ifdef CLIENTSRC
    send_update_to_client();
    usersupp->flags |= US_CLIENT;
#else
    usersupp->flags &= ~US_CLIENT;
#endif

    if (usersupp->priv & PRIV_DELETED) {
	more(DELETEDDENY, 1);
	logoff(ULOG_DENIED);
    }

    // username = (char *) xmalloc(sizeof(char) * (L_USERNAME + 1));
    // strcpy(username, "");
    // strcpy(username, usersupp->username);

    mono_connect_shm();

    if (usersupp->configuration == 0)
	usersupp->configuration++;
    mono_sql_read_config(usersupp->configuration, &config);

    IFGUEST;
    else
    if ((pid = mono_return_pid(my_name)) != -1) {
	kill(pid, SIGTERM);
	cprintf("\n\1f\1rCleaning up previous login: \1w");
	while ((pid = mono_return_pid(my_name)) != -1) {
	    sleep(1);
	    cprintf(".");
	}
	cprintf("  \1r\007Done.\n");
    }
    sprintf(CLIPFILE, "%s/clipboard", getuserdir(my_name));

    if (!fexists(CLIPFILE)) {
	(void) close(creat(CLIPFILE, 0660));	/* create the file */
    }
    laston = usersupp->laston_from;
    usersupp->laston_from = time(&login_time);
    mono_sql_u_increase_login_count(usersupp->usernum);
    strcpy(previous_host, usersupp->lasthost);
    strncpy(usersupp->lasthost, hname, L_HOSTNAME);
    usersupp->lasthost[L_HOSTNAME] = '\0';

    IFTWIT
    {
	usersupp->flags |= US_XOFF;
    }

    /* ask for the key, if they are not validated yet */
    if ((!(usersupp->priv & PRIV_VALIDATED)) && first_login == FALSE)
	enter_key();

    if (usersupp->priv & PRIV_VALIDATED)
	usersupp->flags |= US_REGIS;
    else
	usersupp->flags &= ~US_REGIS;


    /* ask for their address, if they are degraded */
    IFDEGRADED
    {
	usersupp->flags |= US_XOFF;
	cprintf(_("\n*** Registration is required. ***\n\n"));
	(void) more(REGISTER, 0);
	change_info(usersupp, FALSE);
	writeuser(usersupp, 0);
	cprintf(_("\1f\1yNow \1w<\1ry\1w>\1yell to the admin to be granted full access.\n\n"));
    }

    /* this resets the rooms for guests */
    IFGUEST
    {
	room_t thing;

	usersupp->flags |= US_XOFF;
	for (a = 0; a < MAXQUADS; a++) {
	    thing = readquad(a);
	    usersupp->lastseen[a] = thing.lowest;
	    usersupp->generation[a] = -1;
	    usersupp->forget[a] = -1;
	}
    }

    IFNEWBIE
    {
	if (usersupp->online > 60 * 6) {	/* six hours */
	    usersupp->priv &= ~PRIV_NEWBIE;	/* remove newbie flag */
	}
    }

    writeuser(usersupp, 0);

    mono_remove_ghosts();

    mono_add_loggedin(usersupp);	/* put user to into sharedmem */

#ifdef PORT
    mono_sql_onl_add(usersupp->usernum, "telnet", usersupp->doing);
#else
    mono_sql_onl_add(usersupp->usernum, "client", usersupp->doing);
#endif

    setup_express();		/* setup express stuff */

    xfree(my_name);		/* not needed, who_am_i() in btmp.c knows who i am now */

    /* FROM THIS POINT ON, THE USER IS LOGGED IN, AND IN THE WHOLIST. */
    /* -------------------------------------------------------------- */

    /* start the status bar, if wanted */
    status_bar_on();

    print_login_banner(laston);

    if (usersupp->flags & US_XOFF) {	/* wants to be disabled at login */
	change_express(-1);
    } else {
	change_express(1);
    }
    check_profile_updated();	/* set wholist flag? */
    nox = 0;

    /* yes! let's tell friends of mine that i'm logged on! */
    sendx(NULL, "Bing!", OR_LOGIN);

    mailcheck();
    check_generation();

    curr_rm = 0;
    gotocurr();
    last_skipped_rm = -1;	/* set to anything negative here */

    for (a = 0; a < MAXQUADS; a++)
	skipping[a] = 0;

    if (first_login == TRUE) {
#ifdef QC_ENABLE
	if (qc_user_menu(1) == -1)	/* qc is horked, or locked out */
#endif
	    newbie_mark_as_read(5);	/* marks all but 5 messages as read */

	mono_sql_uu_clear_list(usersupp->usernum);
    }
#ifndef CLIENTSRC
    more(BBSDIR "/share/try_client", TRUE);
#endif

    short_prompt();
    logoff(ULOG_NORMAL);
    return 0;
}

/*************************************************
* user_terminate()
*************************************************/

int
user_terminate()
{

    register char cmd = 0;
    char xer[L_USERNAME + 1];

    /* add stuff here to check if someone is x-ing you */
    if (mono_find_x_ing(who_am_i(NULL), xer) == 0) {
	cprintf(_("\n\n\1g\1f%s \1yis still sending you an %s %s.\n\1a"), xer, config.express, config.x_message);
	cprintf(_("\1f\1gHit any key to continue...\1a"));
	inkey();
    }
    while (cmd != 'n' || cmd != 'y' || cmd != 'm') {

	cprintf(_("\n\1f\1gAre you sure you want to leave %s? \1w(\1gy\1w/\1gn\1w/\1gm\1w/\1g?\1w)\1g "), BBSNAME);
	cmd = get_single_quiet("ynm?");

	switch (cmd) {
	    case '?':
		cprintf(_("\1f\1gHelp.\1a\n"));
		(void) more(MENUDIR "/menu_logout_help_argh_roulette", 1);
		break;

	    case 'y':
		cprintf(_("\1f\1gYes.\n"));
		return 1;

	    case 'n':
		cprintf(_("\1f\1gNo.\n"));
		return -1;

	    case 'm':
		nox = 1;
		if (roulette() == 1) {
		    sleep(1);
		    return 1;
		} else {
		    sleep(1);
		    are_there_held_xs();
		    return -1;
		}

	    default:
		cprintf("\f\1rYow! What was that?\n");
		break;

	}

    }
    return -1;
}

/*************************************************
* logoff( int code )
* code should be zero for a normal logoff
*      should be one for a kickoff
*************************************************/

void
logoff(int code)
{

    char quote[40];

#ifdef MAIL_QUOTA
    if (!code)
	purge_mail_quad();
#endif

    if (connecting_flag) {
	(void) mono_detach_shm();
	(void) mono_sql_detach();
	(void) exit(0);
    }
#ifdef PORT
    (void) mono_sql_onl_remove(usersupp->usernum, "telnet");	/* remove from sql */
#else
    (void) mono_sql_onl_remove(usersupp->usernum, "client");	/* remove from sql */
#endif

    if (shm) {
	(void) dump_quickroom();
	(void) log_user(usersupp, hname, code);
	(void) mono_sql_log_logout(usersupp->usernum, login_time, time(0), hname, code);
	(void) mono_remove_loggedin(who_am_i(NULL));	/* remove user from wholist     */

	/* say goodbye */
	if (code == ULOG_NORMAL) {
	    sendx(NULL, "Womble", OR_LOGOUT);
	} else
	    sendx(NULL, "PWRout", OR_KICKOFF);

	(void) mono_detach_shm();
	(void) mono_sql_detach();
    }
    /* change online time stuff */
    usersupp->laston_to = time(0);
    usersupp->online += ((usersupp->laston_to - usersupp->laston_from) / 60);
    (void) writeuser(usersupp, 0);

    status_bar_off();

    /* Peter added the random quote on logging off */
    sprintf(quote, "%s/quote%d", QUOTEDIR, ((int) random() % MAXQUOTE) + 1);
    (void) more(quote, 1);
    /* on the CLient sometimes you don't see it though, so... */
    (void) fflush(stdout);

    (void) unlink(temp);	/* remove temporary files       */
    (void) unlink(tmpname);

    (void) cprintf("\1a\n");	/* K: this should remove all colors */
    remove_express();
    (void) fflush(stdout);
    exit(0);
}

/*************************************************
* print_login_banner()
*************************************************/

void
print_login_banner(time_t laston)
{

    char filename[50];
    int timescalled = 0;
    int ret;

    ret = mono_sql_u_get_login_count(usersupp->usernum, &timescalled);

    (void) cprintf(_("\n\1a\1f\1gWelcome to %s, \1g%s! \1gThis is your \1w#%d \1glogin.\n")
		   ,BBSNAME, usersupp->username, timescalled);

    if ((strncmp(previous_host, "none", 4)) != 0)
	(void) cprintf(_("\1f\1gLast login: %s \1g\1ffrom \1r%-16.16s\n"),
		       printdate(laston, 0), previous_host);

    /* motd */
    sprintf(filename, "%s/etc/motd", BBSDIR);

    if (fexists(filename)) {
	(void) more(filename, TRUE);
    }
    (void) cprintf("\1f\1gThere %s \1y%d\1g %s online right now at %s\n"
		   ,(shm->user_count == 1) ? "is" : "are", shm->user_count
	    ,(shm->user_count == 1) ? config.user : config.user_pl, date());

/*----------- Personal loginfiles ----------------------------------- */
    sprintf(filename, "%s/loginmsg", getuserdir(who_am_i(NULL)));

    if (fexists(filename)) {
	(void) more(filename, TRUE);
    }
    if (usersupp->config_flags & CO_SHOWFRIENDS) {
	cprintf(_("\1f\1gYour friends online\1w:\1a\n"));
	(void) fflush(stdout);
	friends_online();
    }
    return;
}

/*
 * mailcheck()
 * initial login-check for new mail
 */
void
mailcheck()
{

    int b, count;
    float percent;

    if EQ
	(who_am_i(NULL), "Guest")
	    return;

    count = count_mail_messages();

    percent = ((100.0 * count) / (100.0 * MAX_USER_MAILS));
    b = (int) (100 * percent);

    if (count > 0)
	cprintf("\n\1f\1gYour Mail quad is at %s%d \1gpercent capacity.\n",
		((b < 80) ? "" : ((b < 95) ? "\1y" : "\1r")), b);

    b = usersupp->mailnum - usersupp->lastseen[1];

    if (b < 0) {
	usersupp->lastseen[1] = usersupp->mailnum;
    }
    cprintf("\n");

    if (b == 1)
	cprintf(_("\1f\1gYou have a new private %s in \1pMail>\n"), config.message);
    else if (b > 1)
	cprintf(_("\1f\1gYou have \1r%d \1gnew private %s in \1pMail>\n"), b, config.message_pl);
    else
	cprintf(_("\1f\1gYou have \1rno\1g new Mail %s.\1a\n"), config.message_pl);
}

/*************************************************
* this gets the windowsize
*************************************************/

#ifdef OLD
void
getwindowsize(int sig)
{
#ifdef TIOCGWINSZ
    struct winsize ws;

    if (ioctl(0, TIOCGWINSZ, (char *) &ws) < 0)
	return;
    else if ((ws.ws_row) < 5 || ws.ws_row > 100)
	return;
    else
	usersupp->screenlength = ws.ws_row;

    if (sig == SIGWINCH)	/* if called by kill() */
	signal(SIGWINCH, getwindowsize);

    return;
#else
    return;
#endif
}
#endif

/*
 * Set wholist flag if profile updated within past 2 days
 */
void
check_profile_updated()
{
    char work[L_USERNAME + strlen(USERDIR) + 10];
    time_t timenow;
    struct stat buf;

    sprintf(work, "%s/profile", getuserdir(usersupp->username));
    work[strlen(work)] = '\0';

    if (!(fexists(work)))
	return;

    time(&timenow);
    stat(work, &buf);

    if ((timenow - buf.st_mtime) < (2 * 24 * 60 * 60))
	mono_change_online(usersupp->username, NULL, 12);
    else
	mono_change_online(usersupp->username, NULL, -12);

    return;
}

/* eof */
