/* $Id$ 
 * A modified UBL esp. for converting data
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#include MYSQL_HEADER

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
    return 0;
};

void
read_all_users()
{
    char name[L_USERNAME + 1];
    unsigned int i;
    user_t *user;
    DIR *userdir;
    MYSQL_RES *res;
    int ret;
    struct dirent *tmpdirent;

    printf("Reading the users.\n");

    userdir = opendir(USERDIR);

    if (userdir == NULL) {
	printf("opendir(%s) problems!\n", USERDIR);
	exit(2);
    }
    usercount = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {
	if (tmpdirent->d_name[0] != '.')
	    usercount++;
    }

    rewinddir(userdir);
    printf("\nFound %u users\n", usercount);

    i = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {

	if (tmpdirent->d_name[0] == '.')	/* ignore . files */
	    continue;
	strcpy(name, tmpdirent->d_name);

	if (i > usercount) {
	    printf("\nError, not enough memory allocated\n ");
	    closedir(userdir);
	    exit(3);
	}

        printf("\n%s", tmpdirent->d_name);

	user = readuser(name);
	if (user == NULL) {
	    continue;
	    printf("\nNull user.");
        } else
	   printf("%s\n", user->username);
	if (EQ(user->username, "Sysop") || EQ(user->username, "Guest"))
	    continue;


	mono_sql_u_update_url( user->usernum, user->RGurl );
	mono_sql_u_update_email(user->usernum, user->RGemail );
	mono_sql_u_update_hidden(user->usernum,user->hidden_info );

/****************************/
/* do something useful here */
/****************************/

#ifdef CONVERT_PASSWORDS_2_SQL
  	ret = mono_sql_query( &res, "UPDATE user SET password='%s' WHERE id=%u",
             user->password, user->usernum );
#endif

#define STRIP_ALL_PREFERRED_FLAGS

#ifdef STRIP_ALL_PREFERRED_FLAGS
	if (!(user->priv & PRIV_SYSOP || user->priv & PRIV_WIZARD))  
	    if (user->priv & PRIV_PREFERRED) {
	        user->priv -= PRIV_PREFERRED;
                writeuser(user, 0);
	    }

#endif

/*-x-x-x-x-x-x-x-x-x-x-x-x-*-x*/
/* done doing anything useful */
/*-x-x-x-x-x-x-x-x-x-x-x-x-*-x*/

	xfree(user);
	i++;
	printf("%-5u users left to read.\r", usercount - i);	/* users left to READ */
	fflush(stdout);

    }				/* while */

    closedir(userdir);
    return;
}

