/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"

#define extern
#include "log.h"
#undef extern

#include "routines.h"
#include "libuid.h"

/* this uses a fixed string of length 80. it shouldn't */
int
log_it(const char *type, const char *event,...)
{

    FILE *fp;
    time_t t;
    va_list ptr;
    char work[80];

    (void) sprintf(work, "%s/%s", LOGDIR, type);

    fp = fopen(work, "a");
    if (fp == NULL) {
	(void) fprintf(stderr, "Can't open logfile %s Please report the error yourself.\n", work);
	return -1;
    }
    t = time(0);
    strcpy(work, ctime(&t));
    /* remove year and trailing '\n' */
    work[strlen(work) - 6] = '\0';
    (void) fprintf(fp, "%s %s[%d](%s): ", work, program_invocation_short_name, getpid(), mono_uid);

    va_start(ptr, event);
    (void) vfprintf(fp, event, ptr);
    va_end(ptr);

    (void) fputc('\n', fp);
    (void) fclose(fp);
    return 0;
}

/* userlog, logs logins, logoffs, etc */

int
log_user(const user_t * user, const char *hostname, char offstat)
{
    char logline[80], outmsg[8];
    time_t atime, thiscall;

    /* set up login date & time */
    atime = time(NULL);

    /* how are they terminating? */
    switch (offstat) {
	case ULOG_NORMAL:
	    strcpy(outmsg, " ");
	    break;
	case ULOG_DROP:
	    strcpy(outmsg, "DROP");
	    break;
	case ULOG_SLEEP:
	    strcpy(outmsg, "SLEEP");
	    break;
	case ULOG_OFF:
	    strcpy(outmsg, "OFF");
	    break;
	case ULOG_PROBLEM:
	    strcpy(outmsg, "PROBLEM");
	    break;
	case ULOG_KICKED:
	    strcpy(outmsg, "KICKED");
	    break;
	case ULOG_DENIED:
	    strcpy(outmsg, "DENIED");
	    break;
	default:
	    strcpy(outmsg, "UNKNOWN");
	    break;
    }

    if (offstat != ULOG_OFF) {
	thiscall = (atime - user->laston_from) / 60;
	(void) sprintf(logline, "%-20.20s %-16.16s (%3ld): %s",
		       user->username, user->lasthost,
		       thiscall, outmsg);
    } else {
	(void) sprintf(logline, "%-20.20s %-16.16s: %s",
		       outmsg, hostname, outmsg);
    }

    return log_it("bbslog", logline);
}

/* eof */
