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
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"

int main(int, char **);
void statuscheck(char *);

int 
main(int argc, char **argv)
{

    set_invocation_name(argv[0]);

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
    if (argc == 1)
	statuscheck("1234");
    else
	statuscheck(argv[1]);
    mono_detach_shm();

    return 0;
}

void 
statuscheck(char *username)
{

    printf("There %s %d user%s on the BBS."
	   ,(shm->user_count == 1) ? "is" : "are", shm->user_count
	   ,(shm->user_count == 1) ? "" : "s");
    if (strcasecmp(username, "1234") != 0)
	printf(" %s is%s logged in."
	       ,username
	       ,(mono_return_pid(username) == -1) ? " not" : "");
    printf("\n");
    return;
}
