/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"
#include "registration.h"
#include "cgi.h"

typedef struct {
    char name[128];
    char val[128];
} entry;

int
main(int argc, char *argv[])
{

    user_t *user;
    int m = 0, x = 0;
    entry entries[1000];
    char *cl;

    printf("Content-type: text/html\n\n");

    cl = getenv("QUERY_STRING");
    if (cl == NULL) {
	printf("No query information to decode.\n");
	fflush(stdout);
	exit(1);
    }
    for (x = 0; cl[0] != '\0'; x++) {
	m = x;
	getword(entries[x].val, cl, '&');
	plustospace(entries[x].val);
	unescape_url(entries[x].val);
	getword(entries[x].name, entries[x].val, '=');
    }
    user = readuser(entries[0].val);

    if (user == NULL) {
	printf("<H1>No such user.</H1>\n");
	return 1;
    }
    if (user->priv & PRIV_VALIDATED) {
	printf("<h3>Your account is already registered!</h3>\n");
	return 1;
    }
    if (user->validation_key != atoi(entries[1].val)) {
	printf("<h3>Incorrect registration key!</h3>\n");
	return 1;
    }
    user->priv |= PRIV_VALIDATED;
    writeuser(user, 0);
    printf("<html><head>\n");
    printf("<title>Registering %s</title>\n", user->username);
    printf("</head><body bgcolor=\"#FFFFFF\">\n");
    printf("<h3 align=\"center\">Registration accepted</h3>\n");
    printf("<hr><p align=\"left\">Welcome to the Monolith Community, %s!\n", user->username);
    printf("</body></html>\n");
    return 0;
}
