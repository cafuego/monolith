/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"

int
main(int argc, char *argv[])
{
    user_t *userstr;
    int a;

    set_invocation_name(argv[0]);
    mono_setuid("sysop");

    if (argc < 1) {
	fprintf(stderr, "usage: %s <username>\n", argv[0]);
	exit(1);
    }
    if (!check_user(argv[1])) {
	printf("No such user.\n");
	exit(1);
    }
    printf("Changing user '%s' to Emperor, ok? (y/n) ", argv[1]);
    if ((a = getc(stdin)) != 'y') {
	printf("Ok, quitting right now.\n\n");
	exit(1);
    }
    if ((userstr = readuser(argv[1])) == NULL) {
	fprintf(stderr, "Can't load userfile\n");
	exit(1);
    }
    userstr->priv |= PRIV_WIZARD;
    writeuser(userstr, 0);
    xfree(userstr);
    return 0;
}
