/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#ifdef LINUX
#include <linux/kernel.h>
#include <linux/sys.h>
#endif

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "version.h"

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
#include "telnet.h"

#include "commands.h"
#include "express.h"
#include "main.h"
#include "input.h"
#include "inter.h"
#include "usertools.h"
#include "wholist.h"

#define extern
#include "routines2.h"
#include "sql_message.h"
#undef extern

#ifdef CLIENTSRC
static void _restore_colour(void);
static void _set_colour(char key);
static void _log_attrib(int flag);
static int attrib_log;
static colour_t attrib;
#endif

/*************************************************
* more()
*
* Put a file out to the screen, stopping every 24
* lines.  Like the unix more utility. 
*
* Useable commands from the -- more (25%) --:
*
*  <space>     scroll forward one page
*  <b>         scroll back one page
*  <return>    scroll forward one line
*  <backspace> scroll back one line
*  <q>         quit
*************************************************/

int
more(const char *filename, int color)
{

    FILE *f;
    unsigned int count;
    int inc, fs_res, a;
    int screenlength;
    char work[121];

    if (!usersupp)
	screenlength = 24;
    else
	screenlength = usersupp->screenlength;

    cprintf("\n");

    if ((f = xfopen(filename, "r", FALSE)) == NULL) {
	return -1;
    }
    line_total = file_line_len(f);
    line_count = 0;
    curr_line = 2;

    while (!feof(f)) {
	if (fgets(work, 120, f))
	    cprintf("%s", work);
	else
	    continue;

	inc = increment(0);

	if (inc == 1) {
	    cprintf("\n");
	    (void) fclose(f);
	    return 1;
	}
	if (inc == -1 || inc == -2) {
	    count = 0;
	    fs_res = 0;

	    while (fs_res == 0 &&
		   ((inc == -1 && count < 2 * screenlength - 3) ||
		    (inc == -2 && count < screenlength))) {
		fs_res = fseek(f, -2, SEEK_CUR);	/*
							 * seek 2 back 
							 */
		a = getc(f);

		if (fs_res != 0) {
		    line_count = 0;
		    fseek(f, 0, SEEK_SET);
		    (void) putchar('\007');
		}
		if (a == '\n')
		    count++;
	    }
	}
    }

    (void) fflush(stdout);
    (void) fclose(f);
    return 1;
}

void
more_wrapper(const unsigned int col, const long keypress, void *filename)
{
    more(filename, 1);
    if (keypress) {
	cprintf("\1f\1g\n -- Press a key when done.");
	inkey();
    }
    return;
}

char *
m_strcat(char *string1, const char *string2)
{

    string1 =
	(char *) realloc(string1, strlen(string1) + strlen(string2) + 1);
    strcat(string1, string2);
    return (string1);
}


int
more_string(char *const string)
{

    int inc;
    char c, *p, *q;

    if (string == NULL)
	return -1;

    line_total = 0;
    p = q = string;

    while (p) {
	p = strchr(q, '\n');
	line_total++;
	if (!p || string + strlen(string) - 1 == p)
	    break;
	q = ++p;
    }

    line_count = 0;
    curr_line = 2;

    q = string;

    while (1) {
	p = strchr(q, '\n');
	if (p == NULL) {
	    cprintf("%s", q);
	    break;
	}
	c = *p;
	*p = 0;
	cprintf("%s\n", q);
	*p = c;
	q = p + 1;

	inc = increment(0);

	if (inc == 1) {
	    cprintf("\n");
	    return 1;
	}
    }
    return 1;
}

/*
 * print_system_config()
 *
 * Shows system info in <m><s> menu.
 */

#ifdef LINUX
  /*
   * proto  (see man sysinfo)
   */
int sysinfo(struct sysinfo *info);
#endif

void
print_system_config()
{
#ifdef LINUX
    struct sysinfo *info = NULL;
#endif
#ifdef HAVE_SYS_UTSNAME_H
    struct utsname buf2;
    char *domain_name = NULL;
#endif
#ifdef HAVE_ANIMAL
    char *kernel_name = NULL;
#endif
    char *loadavg = NULL;
    float l1 = 0, l2 = 0, l3 = 0;
    int fd = -1, aproc = 0, iproc = 0;
    long len = 0;
    struct stat buf;

#ifdef USE_MYSQL
    /*
     * Show MySQL server info if present.
     */
    MYSQL mysql;
    int total = 0, mine = 0;
    float percent = 0;
    (void) mysql_get_server_info(&mysql);
#endif

#ifdef HAVE_SYS_UTSNAME_H
    /*
     * Kernel info.
     */
    if ((uname(&buf2)) == -1) {
	cprintf("\1f\1rError getting current kernel info!\n");
	return;
    }

    /*
     * 65 is defined in sys/utsname.h, so why not eh?
     * I can't be bothered looking up the RFC...
     */
    domain_name = (char *) xmalloc(65 * sizeof(char));
    strcpy(domain_name, "");
    if ((getdomainname(domain_name, 64)) == -1) {
	cprintf("\1f\1rError getting domain name!\n");
	(void) xfree(domain_name);
	return;
    }
#endif

#ifdef LINUX
    /*
     * System info (mem/disk)
     * load gets done right out of /proc/loadavg
     */
    info = xmalloc(sizeof(struct sysinfo));
    if ((sysinfo(info)) == -1) {
	cprintf("\1f\1rError getting current system info!\n");
	(void) xfree(info);
	return;
    }
#endif

#ifdef HAVE_ANIMAL
    /*
     * If animal name is reported on /proc filesystem, read it
     * into mem as opposed to calling /bin/cat.
     */
    if ((fd = open("/proc/sys/kernel/name", O_RDONLY)) == -1) {
	cprintf("\1f\1rError getting kernel name!\n");
	return;
    }

    /*
     * Bit arbitrary, but better too big than too small ;)
     */
    kernel_name = (char *) xmalloc(65 * sizeof(char));
    strcpy(kernel_name, "");
    len = read(fd, kernel_name, 64);

    /*
     * Chop trailing \n
     */
    kernel_name[len - 1] = '\0';
    (void) close(fd);
#endif

    if ((fd = open("/proc/loadavg", O_RDONLY)) == -1) {
	cprintf("\1f\1rError getting system load!\n");
	return;
    }
    loadavg = (char *) xmalloc(42 * sizeof(char));
    strcpy(loadavg, "");
    len = read(fd, loadavg, 41);
    loadavg[len - 1] = '\0';
    (void) close(fd);
    sscanf(loadavg, "%f %f %f %d/%d", &l1, &l2, &l3, &aproc, &iproc);
    xfree(loadavg);

    /*
     * Compile times.
     */
#ifdef CLIENTSRC
    stat(BBSDIR "/bin/yawc_client", &buf);
#else
    stat(BBSDIR "/bin/yawc_port", &buf);
#endif

#ifdef HAVE_SYS_UTSNAME_H
    cprintf("\n\1f");
    cprintf("\1wCompiled on host              :\1g %s %s\n", buf2.nodename,
	    EQ(domain_name, "(none)") ? "" : domain_name);
    cprintf("\1wHost type                     :\1g %s %s %s\n", buf2.machine,
	    buf2.sysname, buf2.release);
    cprintf("\1wOS build version              :\1g %s\n", buf2.version);
    (void) xfree(domain_name);
#endif

#ifdef HAVE_ANIMAL
    cprintf("\1wOS version                    :\1g %s\n", kernel_name);
    (void) xfree(kernel_name);
#endif

#ifdef USE_MYSQL
    cprintf("\n\1wMySQL Server %-16s :\1g %s\n",
	    mono_mysql_server_info(), mono_mysql_host_info());
    (void) fflush(stdout);
    total = mono_sql_mes_count(0);
    mine = mono_sql_mes_count(usersupp->usernum);
    percent = ((float) mine / (float) total) * 100;
    cprintf("\1wMessages currently in database:\1g %d, %d posted by you ",
	    total, mine);
    printf("(%.2f%%)\n", percent);
#endif
    cprintf("\n\1wLast compiled                 :\1g %s\1f\n",
	    printdate(buf.st_mtime, 0));
    cprintf("\1wBuild directory               :\1g %s \1w(\1g#%d\1w)\n",
	    BUILD_DIR, COMPILE_NUM);
    cprintf("\1wMachine load                  :\1g ");
    printf("%.2f %.2f %.2f", l1, l2, l3);
    cprintf(" \1w(\1g%d\1w/\1g%d\1w)\n", aproc, iproc);

#ifdef LINUX
    cprintf("\1wTotal memory                  :\1g %ld Mb\n\r",
	    info->totalram / 1000000);
    cprintf("\1wTotal swap memory             :\1g %ld Mb\n\n\r",
	    info->totalswap / 1000000);
    xfree(info);
#endif

    cprintf("\1wTotal Logins                  : \1g%u since %s\1f\1g.\n",
	    shm->login_count, printdate(shm->boot_time, 0));
    cprintf("\1wMaximum Users Online          : \1g%d.\n", MAXUSERS);
    cprintf("\1wSleeping timeout              : \1g%d minutes.\n", TIMEOUT);
    cprintf("\1wUnused accounts purge after   : \1g%d days.\n", PURGEDAY);
    print_inter_hosts();
    return;
}

/*************************************************
* execute_unix_command()
*************************************************/

void
execute_unix_command(const char *command)
{

    int a, b, command_exit;

    nox = 1;

    a = fork();			/* split into two processes     */

    if (a == 0) {		/* * daughterprocess, does the work */
	restore_term();
	if (system(command) == -1) {
	    cprintf("Strange system() error has occured.\n ");
	    exit(EXIT_FAILURE);
	}
    } else if (a > 0)		/* motherprocess that waits     */
	do {
	    command_exit = -999;
	    b = wait(&command_exit);
	}
	while (((b != a) && (b >= 0))
	       || ((command_exit != 256) && (command_exit != 0)));

    sttybbs(0);
    return;

}

/*************************************************
* increment()
*
* Returns: 1 -> the reader wants to abort this
*               message.
*          0 -> wants to continue reading.
*         -1 -> the reader wants to read one page
*		backwards.
*	  -2 -> the reader wants to read one line
*		backwards.
*
* Attention: if (line_total) is negative, the
*	     percentage will not be shown at all.
*
* extraflag:	1 -> the wholist is shown: there-
*		     fore, don't allow <W> from
*		     here.
*************************************************/

int
increment(int extraflag)
{

    int c, old_lc, old_lt;

    line_count++;

    if (usersupp->flags & US_PAUSE
	&& (++curr_line >= usersupp->screenlength - 1)
	&& (usersupp->screenlength > 5))
	/* set minimum length */
    {

#ifdef CLIENTSRC
	cprintf("%c%c", IAC, MORE_M);
#endif

	c = 'W';

	while (c == 'W' || c == 'x') {

	    are_there_held_xs();	/* a held-xs's in the more prompt */

	    IFANSI cprintf("7");	/* store the colors and attributes      */

#ifdef CLIENTSRC
	    (void) _log_attrib(FALSE);
#endif
	    if (line_total > 0)
		cprintf("\01w\01f -- \01gmore \01w(\01g%i%%\01w) --\01a",
			(int) ((float) line_count * 100 / line_total));
	    else
		cprintf("\01f\01w -- \01gmore \01w--\01a");

	    (void) fflush(stdout);
	    c = get_single_quiet("0123456789GNQSWcvx \r\n\030");

	    IFTWIT {
		if (strchr("0123456789cvx\030", c)) {
		    more(TWITMSG, 1);
		    c = '\0';
		}
	    }
	    else
	    IFUNVALID {
		if (strchr("0123456789cvx\030", c)) {
		    more(UNVALIDMSG, 0);
		    c = '\0';
		}
	    }
	    else
	    IFDEGRADED {
		if (strchr("0123456789cvx\030", c)) {
		    more(DEGRADEDMSG, 0);
		    c = '\0';
		}
	    }
	    else
	    IFGUEST {
		if (strchr("0123456789vx\030", c)) {
		    more(GUESTMSG, 1);
		    c = '\0';
		}
	    }

	    cprintf("\r                     \r");

	    IFANSI {
		cprintf("8");	/* restore the colors and attributes    */
#ifdef CLIENTSRC
		(void) _restore_colour();
#endif
	    }

	    switch (c) {

		case '\n':
		case '\r':
		    curr_line--;
		    break;

		case 32:	/* want to read one page further        */
		    curr_line = 1;
		    break;

		case 'c':
		    nox = 1;
		    cprintf
			("\1gPress \1w<\1rshift-c\1w>\1g to access the config menu.\n");
		    express(3);
		    break;

		case 'x':	/* send eXpress                         */
		    nox = 1;
		    express(0);
		    break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		    nox = 1;
		    express(c - 38);
		    break;

		case '\030':	/* read X-Log                            */
		    cprintf(_("\n\01f\01gRead X-Log.\01a\n"));
		    old_express();
		    break;

		case 'v':	/* send x to the last sender            */
		    nox = TRUE;
		    express(-1);
		    break;

		case 'W':	/* look at wholist                      */
		    if (extraflag != 1) {
			old_lc = line_count;
			old_lt = line_total;
			show_online(1);
			line_count = old_lc;
			line_total = old_lt;
		    } else
			curr_line = 1;
		    break;

		default:	/* want to quit reading                 */
		    curr_line = 1;
#ifdef CLIENTSRC
		    cprintf("%c%c", IAC, MORE_M);
#endif
		    return (1);
	    }
	}

#ifdef CLIENTSRC
	cprintf("%c%c", IAC, MORE_M);
#endif

    }
    return 0;
}


/*
 * get_single()
 */
char
get_single(const char *valid_string)
{
    register int c;

    c = get_single_quiet(valid_string);
    cprintf("%c\n", c);
    return c;
}

/*************************************************
* get_single_quiet()
*************************************************/

char
get_single_quiet(const char *valid_string)
{
    register int c;

    (void) fflush(stdout);	/* usually to make sure the previous text is visible */
    for (;;) {
	c = inkey();
	/* * First check it in the case given */
	if (strchr(valid_string, c))
	    break;
	/* * If not, if we're lower case, try upper case */
	if (islower(c)) {
	    c -= 32;
	    if (strchr(valid_string, c))
		break;
	}
    }
    return c;
}

/*************************************************
* yesno()
*************************************************/

int
yesno()
{				/* Returns 1 for yes, 0 for no */
    fflush(stdout);
    while (1) {
	switch (inkey()) {
	    case 'Y':
	    case 'y':
		cprintf("\1f\1cYes\n");
		return YES;
	    case 'N':
	    case 'n':
		cprintf("\1f\1cNo\n");
		return NO;
	}
    }
}

int
yesno_default(int def)
{				/* Returns 1 for yes, 0 for no */
    fflush(stdout);
    switch (inkey()) {
	case 'Y':
	case 'y':
	    cprintf("\1f\1cYes\n");
	    return YES;
	case 'N':
	case 'n':
	    cprintf("\1f\1cNo\n");
	    return NO;
	default:
	    if (def == YES)
		cprintf("\1f\1cYes\n");
	    else
		cprintf("\1f\1cNo\n");
	    return def;
    }
}

/* birthday functions, by michel */
/* we'll use a date_t to store birthdays, (Y2K, etc) */

/* bday.day = 1 - 31 */
/* bday.mon = 0 - 11 */
/* bday.year = years after 1900 */
/* if year = 2000 -> no birthyear given */

int
print_birthday(date_t bday)
/* convert to time_t and user strftime() to print it */
{
    struct tm t;
    char text[50];

    if (bday.day == 0)
	return -1;

    t.tm_sec = t.tm_min = 0;
    t.tm_hour = 12;
    t.tm_mday = bday.day;
    t.tm_mon = bday.month;
    t.tm_year = bday.year;

    mktime(&t);

    if (bday.year == 100) {
	strftime(text, 50, "%d %B", &t);
    } else
	strftime(text, 50, "%d %B %Y", &t);

    cprintf("%s", text);
    fflush(stdout);
    return 0;
}

/* sets or modifies ones birthday */
void
modify_birthday(date_t * bday)
{
    char birthday[11];
    date_t dag;
    struct tm t;

    IFNSYSOP if (bday->day != 0)
    {
	cprintf("\1f\1rYour birthday is already set.\n ");
	return;
    }
    cprintf("Enter the birthday in the form `dd-mm-yyyy' or `dd-mm'\n");

    while (1) {
	t.tm_sec = t.tm_min = 0;
	t.tm_hour = 12;
	t.tm_mday = t.tm_mon = t.tm_year = 0;

	cprintf("Birthday: \1c");
	getline(birthday, 11, TRUE);
	strremcol(birthday);

	if (strlen(birthday) == 0) {
	    cprintf("Okay, you can do it later.\n ");
	    break;
	} else
	    if (sscanf
		(birthday, "%d-%d-%d", &t.tm_mday, &t.tm_mon, &t.tm_year)
		== 3) {
	    t.tm_year -= 1900;
	    t.tm_mon--;
	    if (mktime(&t) != -1) {
		dag.day = t.tm_mday;
		dag.month = t.tm_mon;
		dag.year = t.tm_year;
		cprintf("\1gSet your birthdate to ");
		print_birthday(dag);
		cprintf("? (y/n) \1c");
		if (yesno() == YES) {
		    *bday = dag;
		    break;
		} else {
		    continue;
		}
	    } else {
		cprintf("Try again\n");
	    }
	} else if (sscanf(birthday, "%d-%d", &t.tm_mday, &t.tm_mon) == 2) {
	    t.tm_mon--;
	    t.tm_year = 100;
	    if (mktime(&t) != -1) {
		dag.day = t.tm_mday;
		dag.month = t.tm_mon;
		dag.year = 100;
		cprintf("\1gSet your birthday to ");
		print_birthday(dag);
		cprintf("? (y/n) \1c");
		if (yesno() == YES) {
		    *bday = dag;
		    break;
		} else {
		    continue;

		}
	    } else {
		cprintf("Try again\n");
	    }
	} else {
	    cprintf("Can't parse date, try again\n");
	}
    }
    return;
}

void
configure_sql()
{
    mysql_t *_sql;
    unsigned int flag = 0;

    _sql = (mysql_t *) xmalloc(sizeof(mysql_t));
    memset(_sql, 0, sizeof(mysql_t));

    // Loop this one until we have a config the user is happy with *and* that works.
    //
    do {
	// Loop until we have data and the user is happy with that data.
	//
	do {

	    cprintf("\n\1f\1gServer \1w(\1y%s\1w)\1w: \1c",
		    (strlen(_sql->host) == 0) ? shm->mysql.host : _sql->host);
	    getline(_sql->host, L_USERNAME, FALSE);
	    strremcol(_sql->host);
	    if( strlen(_sql->host) == 0)
		    strncpy(_sql->host, shm->mysql.host, L_USERNAME);

	    cprintf("\1f\1gUsername \1w(\1y%s\1w)\1w: \1c",
		    (strlen(_sql->user) == 0) ? shm->mysql.user : _sql->user);
	    getline(_sql->user, L_USERNAME, FALSE);
	    strremcol(_sql->user);
	    if( strlen(_sql->user) == 0)
		    strncpy(_sql->user, shm->mysql.user, L_USERNAME);

	    cprintf("\1f\1gPassword \1w(\1y%s\1w)\1w: \1c", shm->mysql.pass);
	    getline(_sql->pass, L_USERNAME, FALSE);
	    strremcol(_sql->pass);

	    cprintf("\1f\1gDatabase \1w(\1y%s\1w)\1w: \1c",
		    (strlen(_sql->base) == 0) ? shm->mysql.base : _sql->base);
	    getline(_sql->base, L_USERNAME, FALSE);
	    strremcol(_sql->base);
	    if( strlen(_sql->base) == 0)
		    strncpy(_sql->base, shm->mysql.base, L_USERNAME);

	    cprintf("\1f\1gSocket \1w(\1y%s\1w)\1w: \1c",
		    (strlen(_sql->sock) == 0) ? shm->mysql.sock : _sql->sock);
	    getline(_sql->sock, L_USERNAME, FALSE);
	    strremcol(_sql->sock);
	    if( strlen(_sql->sock) == 0)
		    strncpy(_sql->sock, shm->mysql.sock, L_USERNAME);

	    cprintf("\n\1f\1rTest this configuration? \1w(\1rY\1w/\1rn\1w)\1c ");
	    if(yesno_default(YES) == NO) {
		    cprintf("\n\1f\1rQuit configuration? \1w(\1rY\1w/\1rn\1w)\1c ");
		    if(yesno_default(YES) == YES) {
			    xfree(_sql);
			    return;
		    }
		    flag = 0;
	    } else {
		    flag = 1;
	    }

	} while (!flag);

	// User is happy with values; test them.
	//
	if (! (flag = mono_sql_test_connection(_sql)) ) {
	    cprintf("\1f\1rNew configuration cannot connect.\n\n");
	}

    } while ( !flag );

    // Config works!
    //
    cprintf("\1f\1gConnection OK.\n\1rSave this new configuration as default? \1w(\1ry\1w/\1rN\1w)\1c ");
    if( yesno_default(NO) == NO) {
	    xfree(_sql);
	    return;
    }

    // user wants to save new conn. as default. do so adn then switch all users.
    
    xfree(_sql);
    return;
}


/*************************************************************************/
/* moved this here from quadcont.c so it could be used in general..
 * enjoy.  -russ */

/* qc_get_pos_int(const char first, int digits)  
 * can be passed either the first char of an int input if the input is
 * started in another function, or passed a '\0' if input is to take place
 * entirely in this function.  parses single key char input and returns it
 * as a positive or zero ingeger, or -1 if no input was entered.
 * there is no backspace support at the moment for this function.
 */

int
qc_get_pos_int(const char first, const int places)
{
    int i, pos_int = 0, ctr = 0;
    char input = '\0', st[10];
    int digits;
    digits = places;
    st[0] = '\0';
    if (first != '\0')
	input = first;
    else
	input = get_single_quiet("1234567890\n\r");
    while (input != '\n' && input != '\r' && input != ' ') {
	switch (input) {
	    case '\b':
		if (ctr > 0) {
		    st[--ctr] = '\0';
		    cprintf("\b \b");
		}
		break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		st[ctr] = input;
		st[++ctr] = '\0';
		break;
	}

	if (input != '\b')
	    cprintf("%c", input);
	if (ctr <= digits)
	    input = get_single_quiet("1234567890\r\n\b");
	else
	    break;
    }
    if (!strlen(st) || st[0] == '\n' || st[0] == '\r' || st[0] == ' ')
	return -1;
    for (;;)			/* kill leading 0's */
	if (st[0] != '0' || (st[0] == 0 && st[1] == '\0'))
	    break;
	else
	    for (i = 1; st[i - 1] != '\0'; i++)
		st[i - 1] = st[i];
    if (ctr > digits)		/* trunctuate if too many digits (knob at keyboard) */
	st[ctr - 1] = '\0';
    pos_int = atoi(st);
    return pos_int;
}

#ifdef CLIENTSRC
static void
_log_attrib(int flag)
{
    attrib_log = flag;
    return;
}

void
save_colour(int key)
{
    if (!attrib_log)
	return;
    switch (key) {
	case 'a':
	    attrib.fg_colour = key;
	    attrib.bg_colour = 0;
	    attrib.attrib |= ATTRIB_NONE;
	    break;
	case 'b':
	case 'c':
	case 'd':
	case 'g':
	case 'p':
	case 'y':
	case 'w':
	    attrib.fg_colour = key;
	    break;
	case 'B':
	case 'C':
	case 'D':
	case 'G':
	case 'P':
	case 'Y':
	case 'W':
	    attrib.bg_colour = key;
	    break;
	case 'e':
	    attrib.attrib |= ATTRIB_BOLD;
	    break;
	case 'f':
	    attrib.attrib |= ATTRIB_FLASH;
	    break;
	case 'h':
	    attrib.attrib |= ATTRIB_HIDDEN;
	    break;
	case 'i':
	    attrib.attrib |= ATTRIB_INVERSE;
	    break;
	case 'u':
	    attrib.attrib |= ATTRIB_UNDERLINE;
	    break;
	default:
	    break;
    }
    return;
}

static void
_restore_colour()
{
    if (attrib.attrib & ATTRIB_BOLD)
	_set_colour('f');
    if (attrib.attrib & ATTRIB_FLASH)
	_set_colour('e');
    if (attrib.attrib & ATTRIB_HIDDEN)
	_set_colour('h');
    if (attrib.attrib & ATTRIB_INVERSE)
	_set_colour('i');
    if (attrib.attrib & ATTRIB_UNDERLINE)
	_set_colour('u');
    _set_colour(attrib.bg_colour);
    _set_colour(attrib.fg_colour);
    (void) _log_attrib(TRUE);
    return;
}

static void
_set_colour(char key)
{
    if (usersupp->flags & US_ANSI) {
	(void) save_colour(key);
	switch (key) {
	    case 'd':
		cprintf("[30m");
		break;		/* dark   textcolor       */
	    case 'r':
		cprintf("[31m");
		break;		/* red    textcolor       */
	    case 'g':
		cprintf("[32m");
		break;		/* green  textcolor       */
	    case 'y':
		cprintf("[33m");
		break;		/* yellow textcolor       */
	    case 'b':
		cprintf("[34m");
		break;		/* blue   textcolor       */
	    case 'p':
		cprintf("[35m");
		break;		/* purple textcolor       */
	    case 'c':
		cprintf("[36m");
		break;		/* cyan   textcolor       */
	    case 'w':
		cprintf("[37m");
		break;		/* white  textcolor       */
	    case 'D':
		cprintf("[40m");
		break;		/* dark   backgroundcolor */
	    case 'R':
		cprintf("[41m");
		break;		/* red    backgroundcolor */
	    case 'G':
		cprintf("[42m");
		break;		/* green  backgroundcolor */
	    case 'Y':
		cprintf("[43m");
		break;		/* yellow backgroundcolor */
	    case 'B':
		cprintf("[44m");
		break;		/* blue   backgroundcolor */
	    case 'P':
		cprintf("[45m");
		break;		/* purple backgroundcolor */
	    case 'C':
		cprintf("[46m");
		break;		/* cyan   backgroundcolor */
	    case 'W':
		cprintf("[47m");
		break;		/* white  backgroundcolor */
	    case 'a':
		cprintf("[0m");
		break;		/* RESET   attribute      */
	    case 'f':
		if ((usersupp->flags & US_NOBOLDCOLORS) == 0)
		    cprintf("[1m");
		break;		/* BOLD    attribute      */
	    case 'u':
		cprintf("[4m");
		break;		/* UNDERLINED attribute   */
	    case 'e':
		if ((usersupp->flags & US_NOFLASH) == 0)
		    cprintf("[5m");
		break;		/* FLASH   attribute      */
	    case 'i':
		cprintf("[7m");
		break;		/* INVERSE attribute      */
	    case 'h':
		cprintf("[8m");
		break;		/* HIDDEN  attribute      */
	    default:
		break;
	}
    }
    return;
}
#endif

/* eof */
