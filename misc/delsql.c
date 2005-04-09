/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <time.h>

#include <mysql.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"

#define extern
#include "ext.h"
#undef extern


int main(int, char **);

int 
main(int argc, char **argv)
{

    if (argc > 3) {
	fprintf(stdout, "Usage: %s [username].\n", argv[0]);
	fflush(stdout);
	exit(1);
    }
    if (check_user(argv[1]) == -1) {
	fprintf(stdout, "Error: %s is not an existing user.\n", argv[1]);
	fflush(stdout);
	exit(1);
    }
    mono_connect_shm();
    mono_sql_connect();
    del_sql_user( 1188 );
    del_sql_user( 1191 );
    mono_sql_detach();
    mono_detach_shm();

    return 0;
}

