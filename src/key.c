/* $Id$ */

/*
 * I've made a lot of changes to the include files, so you'll probably
 * have to change that too. Further more, to store the validation key,  i
 * use the variable 'last_validation_key', which isn't used on monolith
 * anywya. I had to use an existing variable, because it needs to be saved
 * and if i change the format of the savefile, i'll have to convert all 
 * userfiles, etc..horrible.  I really need to get working on ascii
 * userfiles again..  so much about comments..for any questions, mail to 
 * m.oosterhof@student.utwente.nl
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"

#define extern
#include "key.h"
#undef extern

#include "input.h"
#include "routines2.h"

void
key_menu()
{

    char c;
    int i = 0;

    if (usersupp->priv & PRIV_VALIDATED) {
	cprintf("You are already validated.\n");
	return;
    }
    cprintf("\n\1f\1w<\1rs\1w>\1gend another key\n\1a");
    cprintf("\1f\1w<\1re\1w>\1gnter your key\n\1a");
    cprintf("\1f\1w<\1rq\1w>\1guit\n\n\1a");

    cprintf("\1f\1gConfig\1w:\1g Key\1w:\1g ");
    while ((c = get_single_quiet("segq ")) != 'q') {
	switch (c) {
	    case 's':
		cprintf("Send another validation key.\n");
		i = send_key(usersupp);
		switch (i) {
		    case 0:
			cprintf("\nYour validation key was sent to `%s'.\n", usersupp->RGemail);
			break;
		    case 1:
			cprintf("\nYour email address contains invalid characters. Please check whether\n");
			cprintf("it is correct or not and send the correct address to the admin via\n");
			cprintf("a <y>ell.\n");
			break;
		    default:
			cprintf("\nAn unknown error has occurred whilst trying to send your key. Please\n");
			cprintf("notify the admin via a <y>ell, so they can fix the problem.\n");
			break;
		}
		break;
	    case 'e':
		cprintf("Enter your key.\n");
		enter_key();
		break;
	    case ' ':
	    case 'q':
		return;
	}
	cprintf("Config\1w:\1g Key\1w:\1g ");
    }
    return;
}

void
enter_key()
{

    char key[5];

    cprintf("\nPlease enter your validation key, if you have received ");
    cprintf("Otherwise, press return.\n");

    cprintf("\nPlease enter your key: ");
    getline(key, 5, 0);

    if (atol(key) == usersupp->validation_key) {
	cprintf("The key is correct, you are now validated.\n\n");
	usersupp->priv |= PRIV_VALIDATED;
    } else {
	cprintf("Sorry, but the key was incorrect..\n\n");
    }
    return;

}

int
send_key(const user_type * user)
{

#define ACCEPTED "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@1234567890_%.-"

#ifdef HAVE_SENDMAIL
    FILE *sendmail = NULL;
    char *message = NULL;
#else
    char cmd[200];
#endif
    size_t accepted;

    accepted = strspn(user->RGemail, ACCEPTED);

    if (accepted < strlen(user->RGemail)) {
	log_it("email", "%s has invalid chars in the email address. Validation key not sent.", usersupp->username);
	return 1;
    }

#ifdef HAVE_SENDMAIL
    if( (sendmail = popen( SENDMAIL " -t", "w" )) == NULL ) {
	log_it("email", "Error trying to send validation key to %s.\nCan't popen(%s).", user->RGemail, SENDMAIL);
	return 2;
    }
    if ((message = map_file(VALIDATION_EMAIL)) == NULL) {
        xfree(message);
	log_it("email", "Error trying to send validation key to %s.\nCan't mmap %s.", user->RGemail, VALIDATION_EMAIL);
	return 2;
    }

    fprintf(sendmail, "To: %s <%s>\n", user->RGname, user->RGemail);
    fprintf(sendmail, "Subject: %s BBS validation key.\n", BBSNAME);
    fprintf(sendmail, "%s\n", message);
    fprintf(sendmail, "\n*** Your %s BBS validation key is: %lu ***\n", BBSNAME, user->validation_key);

    xfree(message);
    if( pclose(sendmail) == -1 ) {
	log_it("email", "Error trying to send validation key to %s.\nCan't pclose(%X).", user->RGemail, sendmail);
	return 2;
    }
#else
    sprintf(cmd, "/bin/mail -s \"%s BBS Validation Key: %ld\" \'%s\' < %s &"
	    ,BBSNAME, user->validation_key, user->RGemail, VALIDATION_EMAIL);

    if (system(cmd) == -1) {
	log_it("email", "Error trying to send validation key to %s.\nCommand: %s.", user->RGemail, cmd);
	return 2;
    }
#endif

    log_it("email", "Validation key sent to %s at %s.", usersupp->username, usersupp->RGemail);
    return 0;
}

void
generate_new_key(user_type * user)
{

    time_t bing;

    time(&bing);
    srandom(bing);
    user->validation_key = random() % 9999;
}
