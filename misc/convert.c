/* $Id$ 
 * A modified UBL esp. for converting data
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"
#include "setup.h"


/* global variables */
unsigned int usercount = 0;

void read_all_users(void);
int main(int, char **);

/* ----------------------------------------------------------------- */

int
main(int argc, char **argv)
{
    set_invocation_name(argv[0]);
    chdir(BBSDIR);

    /* read the necessary data */
    mono_sql_connect();
    mono_connect_shm();
    read_all_users();
    mono_detach_shm();
    mono_sql_detach();
};

void
read_all_users()
{
    char name[L_USERNAME + 1];
    unsigned int i, j;
    user_t *user;
    DIR *userdir;
    MYSQL_RES *res;
    int ret;
    struct dirent *tmpdirent;

    printf("Reading the users.\n");

    userdir = opendir(USERDIR);

    if (userdir == NULL) {
	fprintf(stderr, "opendir(%s) problems!\n", USERDIR);
	perror(program_invocation_short_name);
	exit(2);
    }
    usercount = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {
	if (tmpdirent->d_name[0] != '.')	/* && tmpdirent->d_type == DT_DIR ) peculiar.. */
	    usercount++;
    }
    rewinddir(userdir);

    printf("\nFound %u users\n", usercount);

    i = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {

#ifdef DT_DIR
	if (tmpdirent->d_type != DT_DIR)	/* ignore normal files */
	    continue;
#endif /* DT_DIR */

	if (tmpdirent->d_name[0] == '.')	/* ignore . files */
	    continue;

	strcpy(name, tmpdirent->d_name);

	if (i > usercount) {
	    fprintf(stderr, "%s", "Error, not enough memory allocated\n ");
	    closedir(userdir);
	    exit(3);
	}
	user = readuser(name);
	if (user == NULL)
	    continue;
	if (EQ(user->username, "Sysop"))
	    continue;

/****************************/
/* do something useful here */
/****************************/

  	ret = mono_sql_query( &res, "UPDATE user SET password='%s' WHERE id=%u",
             user->password, user->usernum );

	xfree(user);
	i++;
	printf("%-5u users left to read.\r", usercount - i);	/* users left to READ */
	fflush(stdout);

    }				/* while */

    closedir(userdir);
    return;
}

