/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

#include "monolith.h"
#include "userfile.h"

int
main( int argc, char *argv[] )
{
    struct quickroom allquick[MAXROOMS];
    struct userinfo *start;	/* to mark beginning of malloc'ed RAM */
    long ucount = 0;

    chdir(BBSDIR);

    char name[L_USERNAME + 1];
    int i;
    struct userinfo *current, *uinfo;
    user_type *tmpuser;
    long unbr;
    DIR *userdir;
    struct dirent *tmpdirent;

    printf("1. Reading the users.\n");

    userdir = opendir(USERDIR);
    if (userdir == NULL) {
	printf("opendir() problems!\n");
	exit(0);
    }
    while ((tmpdirent = readdir(userdir)) != NULL) {
	if (tmpdirent->d_name[0] != '.') {
	    user = readuser(tmpdirent->d_name[0]);
	    /* make userlist */

	}
    }
    closedir(USERDIR);
    return 0;
}
