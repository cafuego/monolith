/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "monolith.h"

#include "routines.h"
#include "userfile.h"

void purge_users(void);
int main(void);

/*************************************************
* main()
*************************************************/

int
main()
{
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
    user_t *tmpuser;
    FILE *fp;
    char name[L_USERNAME + 1], work[100];
    int i = 0;

    sprintf(work, BBSDIR "www/bbs/users/userlist.html");

    // throw away old userlist.
    unlink(work);

    if ((fp = fopen(work, "a")) == NULL) {
	fprintf(stderr, "Could not open the output file: %s.\n", work);
	exit(1);
    }
    userdir = opendir(USERDIR);
    if (userdir == NULL) {
	printf("opendir() problems!\n");
	exit(0);
    }
    fprintf(fp, "<html><head><title>List of all BBS USers</title></head>\n");
    fprintf(fp, "<!--#include virtual=\"/include/body-ul.html\" -->\n");
    fprintf(fp, "<center>\n");
    fprintf(fp, "<img src=\"/pix/banners/bbs_users_userlist.jpg\" height=\"32\" width=\"468\">\n");
    fprintf(fp, "</center><p><p>\n");
    fprintf(fp, "<center>\n");


    fprintf(fp, "<table border=0 cellspacing=3 cellpadding=1>\n");
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
	if (tmpuser->priv & PRIV_DELETED) {
	    free(tmpuser);
	    continue;
	}
	if ((i % 5) == 0)
	    fprintf(fp, "<tr>\n");

	fprintf(fp, "<td ");

	if (tmpuser->priv & PRIV_WIZARD) {
	    fprintf(fp, "bgcolor=\"#BBBBBB\">\n");
	} else if (tmpuser->priv & PRIV_SYSOP) {
	    fprintf(fp, "bgcolor=\"#BBBBBB\">\n");
	} else if (tmpuser->priv & PRIV_TECHNICIAN) {
	    fprintf(fp, "bgcolor=\"#000050\">\n");
	} else if (tmpuser->flags & US_ROOMAIDE) {
	    fprintf(fp, "bgcolor=\"#500000\">\n");
	} else if (tmpuser->flags & US_GUIDE) {
	    fprintf(fp, "bgcolor=\"#005050\">\n");
	} else {
	    fprintf(fp, "bgcolor=\"#005000\">\n");
	}

	fprintf(fp, "<a href=\"/cgi-bin/profile?name=%s\">%s</a>\n", tmpuser->username, tmpuser->username);
	fprintf(fp, "<br></td>\n");
	free(tmpuser);
	if ((i % 5) == 4)
	    fprintf(fp, "</tr>\n");
	i++;
    }
    fprintf(fp, "</table>\n");
    fprintf(fp, "</center>\n");
    fprintf(fp, "</body></html>\n");
    fclose(fp);
    closedir(userdir);
    return;
}

/* eof */
