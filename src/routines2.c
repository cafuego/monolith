/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

#ifdef LINUX
#include <linux/kernel.h>
#include <linux/sys.h>
#endif

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
#include "telnet.h"

#define extern
#include "routines2.h"
#undef extern

#include "commands.h"
#include "express.h"
#include "main.h"
#include "input.h"
#include "inter.h"
#include "usertools.h"
#include "wholist.h"


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

int
more_string(char *string)
{

    int inc;
    char c, *p, *q;

    line_total = 0;
    p = q = string;

    if (string == NULL)
	return -1;

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

/*************************************************
* print_system_config()
*************************************************/

#ifdef LINUX
  /* proto  (see man sysinfo) */
  int sysinfo(struct sysinfo *info);
#endif

void
print_system_config()
{
    struct stat buf;

#ifdef LINUX
    struct sysinfo *info;
#endif
  
    cprintf("\n\1f");

#ifdef CLIENTSRC
    stat(BBSDIR "/bin/yawc_client", &buf);
#else
    stat(BBSDIR "/bin/yawc_port", &buf);
#endif

    cprintf("\1wCompiled on host              :\1g monolith.\n");
    cprintf("\1wHost type                     :\1g ");

    (void) fflush(stdout);
    (void) system("/bin/echo `/bin/uname` `/bin/arch`");
#ifndef CLIENTSRC
    cprintf("\r");
#endif
    cprintf("\1wOS version                    :\1g ");
    (void) fflush(stdout);
    (void) system("/bin/cat /proc/sys/kernel/name");
    cprintf("\n\1wLast compiled                 : %s\n\r\1f", printdate(buf.st_mtime, 0));
    (void) fflush(stdout);

    /* Get system info via the proper channels */
#ifdef LINUX
    info =  xmalloc(sizeof(struct sysinfo));
    if( (sysinfo (info)) == -1 ) {
        cprintf("\1f\1rError getting current system info!\n");
    } else {
        cprintf ("\1wMachine load                  :\1g %ld %ld %ld\n\r", info->loads[0], info->loads[1], info->loads[2]);
        cprintf ("\1wTotal memory                  :\1g %ld Mb\n\r", info->totalram / 1000000);
        cprintf ("\1wTotal swap memory             :\1g %ld Mb\n\n\r", info->totalswap / 1000000);
    }
    xfree(info);
#endif 

    cprintf("\1wMaximum Users Online          : \1g%d.\n", MAXUSERS);
    cprintf("\1wSleeping timeout              : \1g%d min.\n", TIMEOUT);
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
	while (((b != a) && (b >= 0)) || ((command_exit != 256) && (command_exit != 0)));

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

    if (usersupp->flags & US_PAUSE && (++curr_line >= usersupp->screenlength - 1)
	&& (usersupp->screenlength > 5))
	/* set minimum length */
    {

#ifdef CLIENTSRC
	cprintf("%c%c", IAC, MORE_M);
#endif

	c = 'W';

	while (c == 'W' || c == 'x') {

	    are_there_held_xs();	/* a held-xs's in the more prompt */

	    IFANSI
		cprintf("7");	/* store the colors and attributes      */

	    if (line_total > 0)
		cprintf("\01w\01f -- \01gmore \01w(\01g%i%%\01w) --\01a", (int) ((float) line_count * 100 / line_total));
	    else
		cprintf("\01f\01w -- \01gmore \01w--\01a");

	    (void) fflush(stdout);
	    c = get_single_quiet("GNQSWcvx \r\n\030");

	    IFTWIT {
		if (strchr("cvx\030", c)) {
		    more(TWITMSG, 1);
		    c = '\0';
		}
	    }
	    else
	    IFUNVALID {
		if (strchr("cvx\030", c)) {
		    more(UNVALIDMSG, 0);
		    c = '\0';
		}
	    }
	    else
	    IFDEGRADED {
		if (strchr("cvx\030", c)) {
		    more(DEGRADEDMSG, 0);
		    c = '\0';
		}
	    }
	    else
	    IFGUEST {
		if (strchr("vx\030", c)) {
		    more(GUESTMSG, 1);
		    c = '\0';
		}
	    }

	    cprintf("\r                     \r");

	    IFANSI
		cprintf("8");	/* restore the colors and attributes    */

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
                cprintf("\1gPress \1w<\1rshift-c\1w>\1g to access the config menu.\n");
                express(3);
                break;

		case 'x':	/* send eXpress                         */
		    nox = 1;
		    express(0);
		    break;

		case '\030':	/* read X-Log                            */
		    cprintf("\n\01f\01gRead X-Log.\01a\n");
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

/*************************************************
* fmout2()
* fff:	1 -> copy to clipboard-file
*	7 -> copy to tempfile (tmpfile)
*************************************************/

void
fmout2(FILE * fp, char fff)
{
    unsigned int count;
    int a, inc, fs_res;
    FILE *f = NULL;
    char aaa[200], *p;
    int chr_cnt = 0;		/* -lj fix for long lines? */

    strcpy(aaa, "");
    p = aaa;

    if (fp == NULL)
	return;

    if (fff == 1)		/* if ClipBoardCopy */
	f = xfopen(CLIPFILE, "a", TRUE);
    else if (fff == 7)
	f = xfopen(tmpname, "w", TRUE);

    for (;;) {
	a = getc(fp);

	if (a <= 0)
	    break;

	if (a > 126)
	    continue;

	if (a >= 32 || a == 1 || a == 9 || a == 10 || a == 13) {
	    *p++ = a;
	    *p++ = 0;
	    p--;
	    chr_cnt++;		/* -lj */
	}
	if ((a == 13) || (a == 10) || (chr_cnt > 198)) {	/* -lj */
	    if (fff == 1 || fff == 7)
		fprintf(f, "%s", aaa);	/* Clip it to the ClipBoard/tmpfile */
	    else {
		cprintf("%s", aaa);

		inc = increment(0);
		if (inc == 1)
		    break;

		if (inc == -1 || inc == -2) {
		    /* scrollback one page or one line */
		    count = 0;
		    fs_res = 0;

		    while (fs_res == 0 &&
		    ((inc == -1 && count < 2 * usersupp->screenlength - 3) ||
		     (inc == -2 && count < usersupp->screenlength))) {
			fs_res = fseek(fp, -2, SEEK_CUR);	/* seek 2 back */
			a = getc(fp);

			if (a == 0) {
			    fs_res = 1;
			    line_count = 0;
			    (void) fseek(fp, 1, SEEK_CUR);
			    (void) putchar('\007');	/* beep; beginning */
			}
			if (a == '\n')
			    count++;
		    }
		}
	    }
	    strcpy(aaa, "");
	    p = aaa;
	    chr_cnt = 0;
	}
    }
    if (f != NULL)
	(void) fclose(f);

    return;

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
    register int  c;

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

    IFNSYSOP
	if (bday->day != 0) {
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
	} else if (sscanf(birthday, "%d-%d-%d", &t.tm_mday, &t.tm_mon, &t.tm_year) == 3) {
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

/*************************************************************************/
/* moved this here from quadcont.c so it could be used in general..
   enjoy.  -russ */

/* qc_get_pos_int(const char first, int digits)  
 * can be passed either the first char of an int input if the input is
 * started in another function, or passed a '\0' if input is to take place
 * entirely in this function.  parses single key char input and returns it
 * as a positive or zero ingeger, or -1 if no input was entered.
 * there is no backspace support at the moment for this function.
 */

int
qc_get_pos_int(const char first, int digits)
{
    int i, pos_int = 0;
    char input = '\0';

    if (first != '\0')
        input = first;
    else
        input = get_single_quiet("1234567890\n\r");
    for (;;) {
        switch (input) {
            case '\n':
            case '\r':
                return -1;
            case '0':
                digits--;
                break;
            case '1':
                pos_int += (int) (1 * (pow(10, --digits)));
                break;
            case '2':
                pos_int += (int) (2 * (pow(10, --digits)));
                break;
            case '3':
                pos_int += (int) (3 * (pow(10, --digits)));
                break;
            case '4':
                pos_int += (int) (4 * (pow(10, --digits)));
                break;
            case '5':
                pos_int += (int) (5 * (pow(10, --digits)));
                break;
            case '6':
                pos_int += (int) (6 * (pow(10, --digits)));
                break;
            case '7':
                pos_int += (int) (7 * (pow(10, --digits)));
                break;
            case '8':
                pos_int += (int) (8 * (pow(10, --digits)));
                break;
            case '9':
                pos_int += (int) (9 * (pow(10, --digits)));
                break;
        }
        cprintf("\1f\1c%c\1a", input);
        if (digits > 0) {
            input = '\0';
            input = get_single_quiet("1234567890\r\n");
            if ((input == '\r') || (input == '\n')) {
                for (i = 0; i < digits; i++)
                    pos_int /= 10;
                cprintf("\n\1a");
                return pos_int;
            }
        } else
            break;
    }                           /* for(;;) */
    cprintf("\n\1a");
    return pos_int;
}
/* eof */
