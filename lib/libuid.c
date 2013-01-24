
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "monolith.h"
#include "libuid.h"
#include "routines.h"

static char mono_uid[L_USERNAME+1];
static char *program_invocation_name = NULL;
static char *program_invocation_short_name = NULL;

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
    char *p, *q;

    q = strdup(name);
    if ( q == NULL ) 
	return -1;

    xfree( program_invocation_name );

    program_invocation_name = q;

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

char
*get_invocation_short_name()
{
    return program_invocation_short_name;
}

/* eof */
