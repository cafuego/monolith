
/* www authenticate 
 * used by mod_external_auth in apache */
 
/* todo: free all allocated memory ;) */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"

#define ENVNAME_USER "USER"
#define ENVNAME_GROUP "GROUP"

int
main(int argc, char *argv[])
{
    char *p;
    char *username, *group;

    p = getenv(ENVNAME_USER);

    if (p && strlen(p))
	username = strdup(p);
    else
	return -1;

    p = getenv(ENVNAME_GROUP);

    if (p && strlen(p))
	group = strdup(p);
    else
	return -1;

    if ( strcasecmp( group, "bbs" )) return -1;

    if ( check_user( username ) == TRUE ) return 0;
    else return -1;

}
