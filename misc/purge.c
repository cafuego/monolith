/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>

#include "monolith.h"
#include "libmono.h"

#define USERTMPFILE BBSDIR "tmp/purge.tmp"

void purge_users(void);
void main(int argc, char **argv);

/*************************************************
* main()
*************************************************/

void
main(int argc, char **argv)
{
    set_invocation_name(argv[0]);
    mono_setuid("sysop");

    purge_users();
    exit(0);
}

/*************************************************
* purge_users()
*************************************************/

void
purge_users()
{
    DIR *userdir;
    struct dirent *tmpdirent;
    long now, absenttime;
    user_type *tmpuser;
    FILE *fp;
    char name[L_USERNAME + 1], work[100];

    sprintf(work, "%s/%s", LOGDIR, "purgelog");
    if ((fp = fopen(work, "a")) == NULL) {
	fprintf(stderr, "Could not open the purgelog: %s.\n", work);
	exit(1);
    }
    fprintf(fp, "Monolith User Purge: %s\n", date());

    userdir = opendir(USERDIR);
    if (userdir == NULL) {
	printf("opendir() problems!\n");
	exit(0);
    }
    while ((tmpdirent = readdir(userdir)) != NULL) {
	if (tmpdirent->d_name[0] == '.')	/* ignore . files */
	    continue;

	strcpy(name, tmpdirent->d_name);
	name2file(name);

	tmpuser = readuser(name);

	if (tmpuser == NULL || strcasecmp(name, name2file(tmpuser->username)) != 0) {
	    fprintf(stderr, "purge: problems with %s!\n", name);
	    fprintf(fp, "purge: problems with %s!\n", name);
	    if (tmpuser != NULL)
		free(tmpuser);
	    continue;
	}
	time(&now);
	absenttime = now - tmpuser->laston_to;


	if (tmpuser->priv & PRIV_DELETED) {
	    del_user(tmpuser->username);
	    printf("deleted %s\n", tmpuser->username);
	    fprintf(fp, "deleted %s\n", tmpuser->username);
	}
	if (
	       (((absenttime >= 30 * 24 * 60 * 60) &&	/* 30 days & < 10logins */
		 (tmpuser->timescalled < 5))
		||
		(tmpuser->priv & PRIV_DELETED)	/* or delmarked */
		||
		(absenttime >= 90 * 24 * 60 * 60))	/* 90 days */
	       &&
	       ((tmpuser->flags & US_PERM) == 0)	/* not US_PERM */
	       &&
	       (tmpuser->timescalled < 1000)
	    ) {
	    del_user(tmpuser->username);
	    printf("deleted %s\n", tmpuser->username);
	    fprintf(fp, "deleted %s\n", tmpuser->username);




	}
	free(tmpuser);
    }
    fclose(fp);
    closedir(userdir);
    return;
}

/* eof */
