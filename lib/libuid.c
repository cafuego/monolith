/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

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

#define extern
#include "libuid.h"
#undef extern

int
mono_setuid(const char *newuid)
{
    strcpy(mono_uid, newuid);
    return 0;
}

char *
mono_getuid()
{
    return mono_uid;
}

int
set_invocation_name(const char *name)
{
    char *p;

    program_invocation_name = strdup(name);
    if (program_invocation_name == NULL)
	return -1;

    p = strrchr(name, '/');
    if (p != NULL) {
	p++;
	program_invocation_short_name = strdup(p);
    } else {
	program_invocation_short_name = strdup(name);
    }
    if (program_invocation_short_name == NULL)
	return -1;

    return 0;
}

/* eof */
