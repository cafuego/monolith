
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
#define ENVNAME_PASS "PASS"

int
main(int argc, char *argv[])
{
    char *p;
    char *username, *pass;
    user_t *user;

    p = getenv(ENVNAME_USER);

    if (p && strlen(p))
	username = strdup(p);
    else
	return -1;

    if ( EQ( username, "guest" ) ) return 0;

    p = getenv(ENVNAME_PASS);

    if (p && strlen(p))
	pass = strdup(p);
    else 
	return -1;

    user = readuser(username);

    if (user == NULL)
	return -1;

    if (check_password(user, pass) == FALSE)
	return -1;

    exit( 0 );

}
