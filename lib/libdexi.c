/* dexi_routines.c */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#include "monolith.h"

#include "routines.h"
#include "dexi.h"

FILE *_dexifp = NULL;

#define REMOTE_HOSTS BBSDIR "/etc/hosts"

void
mono_setdexient()
{
    if (_dexifp != NULL)
	rewind(_dexifp);
    return;
}

void
mono_enddexient()
{
    if (_dexifp == NULL)
	return;
    fclose(_dexifp);
    _dexifp = NULL;
}

rbbs_t *
mono_getdexient()
{

    static rbbs_t dexientry;
    char line[100];
    if (_dexifp == NULL) {
	_dexifp = xfopen(REMOTE_HOSTS, "r", FALSE);
	if (_dexifp == NULL)
	    return NULL;
    }
    while (1) {
	if (fgets(line, sizeof(line), _dexifp) == NULL)
	    return NULL;
	if (line[0] == '#')
	    continue;
	if (sscanf(line, "%s %s %ud", dexientry.name, dexientry.addr, &dexientry.port) != 3)
	    continue;
	return &dexientry;
    }

}
