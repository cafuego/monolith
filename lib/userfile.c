/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <build-defs.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "monolith.h"

#define extern
#include "userfile.h"
#undef extern

#include "btmp.h"
#include "friends.h"
#include "log.h"
#include "registration.h"
#include "routines.h"
#include "sql_userforum.h"
#include "sql_usertopic.h"
#include "sql_user.h"
#include "sql_useruser.h"


/*************************************************
* check_user()
* DEPRICATED: uses mono_sql_u_check_user()
*************************************************/

int
check_user(const char *name)
{
    char work[61], *p;

    if (name == NULL)
	return FALSE;
    p = getuserdir(name);
    strncpy(work, p, 60);
    return fexists(work);
}

#ifdef USED
/* returns TRUE is password is correct, FALSE otherwise */
int
check_password(const user_t * user, const char *password)
{
    char salt[3];

    if (!user || !password || !strlen(password))
	return FALSE;

    strncpy( salt, user->password, 2 );
    salt[2] = '\0';

#ifdef DEBUG
    printf( "debug: salt: %s", salt ); 
#endif

    if (strcmp(crypt(password, salt), user->password) == 0)
	return TRUE;
    else
	return FALSE;
}
#endif

/* accepts usre and plaintext passwd and stores it encrypted */
/* returns -1 on error */
int
set_password(user_t * user, const char *password)
{
    if (user == NULL || password == NULL || password[0] == '\0')
	return -1;
    strcpy(user->password, crypt(password, CRYPTKEY));
    return 0;
}

/*************************************************
* readuser()
* on error NULL is returned. 
* error is logged 
*************************************************/

#define BUF_SIZE 3000
#define DELIM "|"

user_t *
readuser(const char *name)
{
    FILE *fp;
    char *p, type[30], work[BUF_SIZE + 1];
    user_t *user;
    int i;

    user = (user_t *) xcalloc(1, sizeof(user_t));

    (void) sprintf(work, "%s/save", getuserdir(name));
    (void) name2file(work);

    if ((fp = xfopen(work, "r", FALSE)) == NULL) {
	fprintf(stderr, "can't open userfile: %s\n", strerror(errno));
	xfree(user);
	return NULL;
    }
    /* okay..now read one line at a time, and parse it. */
    while (fgets(work, BUF_SIZE, fp) != NULL) {
	work[strlen(work) - 1] = '\0';	/* remove the \n at the end */
	/* quick fix to stop empty fields from crashing */
	/* warning! does not work with multiple fields! last argument 
	 * null, means whole string isn't parsed */
	if (work[strlen(work) - 1] == '|')
	    continue;

	p = strtok(work, DELIM);
	if (!p)
	    continue;

	strncpy(type, p, 29);
	type[29] = '\0';

	if (strcmp("username", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->username, p);
	} else if (strcmp("password", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->password, p);
        } else if (strcmp("lang", type ) == 0 ) {
            p = strtok(NULL, DELIM);
            if (p)
                strcpy(user->lang, p );
	} else if (strcmp("usernum", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->usernum = atoi(p);
	} else if (strcmp("priv", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->priv = atoi(p);
	} else if (strcmp("lastseen", type) == 0) {
	    for (i = 0; i < MAXQUADS; i++) {
		p = strtok(NULL, DELIM);
		if (p)
		    user->lastseen[i] = atoi(p);
	    }
	} else if (strcmp("generation", type) == 0) {
	    for (i = 0; i < MAXQUADS; i++) {
		p = strtok(NULL, DELIM);
		if (p)
		    user->generation[i] = atoi(p);
	    }
	} else if (strcmp("forget", type) == 0) {
	    for (i = 0; i < MAXQUADS; i++) {
		p = strtok(NULL, DELIM);
		if (p)
		    user->forget[i] = atoi(p);
	    }
	} else if (strcmp("roominfo", type) == 0) {
	    for (i = 0; i < MAXQUADS; i++) {
		p = strtok(NULL, DELIM);
		if (p)
		    user->roominfo[i] = atoi(p);
	    }
	} else if (strcmp("mailnum", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->mailnum = atol(p);
	    if (user->mailnum > 30000)
		user->mailnum = 0;

	    /* letop! werkt nog niet! */

	} else if (strcmp("flags", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->flags = atoi(p);
	} else if (strcmp("chat", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->chat = atoi(p);
	} else if (strcmp("config_flags", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->config_flags = atoi(p);
	} else if (strcmp("configuration", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->configuration = atoi(p);
	} else if (strcmp("screenlength", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->screenlength = atoi(p);
	} else if (strcmp("alias", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->alias, p);
	} else if (strcmp("alarm_clock", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->alarm_clock = atoi(p);
	} else if (strcmp("timescalled", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->timescalled = atol(p);
	} else if (strcmp("posted", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->posted = atoi(p);
	} else if (strcmp("RA_rooms", type) == 0) {
	    for (i = 0; i < 5; i++) {
		p = strtok(NULL, DELIM);
		if (p)
		    user->RA_rooms[i] = atoi(p);
	    }
	} else if (strcmp("validation_key", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->validation_key = atol(p);
	} else if (strcmp("x_s", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->x_s = atol(p);
	} else if (strcmp("hidden_info", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->hidden_info = atoi(p);
	} else if (strcmp("firstcall", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->firstcall = atol(p);
	} else if (strcmp("birthday_day", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->birthday.day = atoi(p);
	} else if (strcmp("birthday_month", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->birthday.month = atoi(p);
	} else if (strcmp("birthday_year", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->birthday.year = atoi(p);
	} else if (strcmp("laston_from", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->laston_from = atol(p);
	} else if (strcmp("laston_to", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->laston_to = atol(p);
	} else if (strcmp("online", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		user->online = atol(p);
#ifndef STUPID_BUG
	} else if (strcmp("lasthost", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->lasthost, p);
#endif
	} else if (strcmp("doing", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->doing, p);
	} else if (strcmp("timezone", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->timezone, p);
	} else if (strcmp("awaymsg", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->awaymsg, p);
	} else if (strcmp("xtrapflag", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->xtrapflag, p);
	} else if (strcmp("aideline", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->aideline, p);
	} else if (strcmp("RGname", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strncpy(user->RGname, p, RGnameLEN);
	    user->RGname[RGnameLEN] = '\0';
	} else if (strcmp("RGaddr", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strncpy(user->RGaddr, p, RGaddrLEN);
	    user->RGaddr[RGaddrLEN] = '\0';
	} else if (strcmp("RGcity", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strncpy(user->RGcity, p, RGcityLEN);
	    user->RGcity[RGcityLEN] = '\0';
	} else if (strcmp("RGstate", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->RGstate, p);
	} else if (strcmp("RGcountry", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->RGcountry, p);
	} else if (strcmp("RGzip", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->RGzip, p);
	} else if (strcmp("RGphone", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strncpy(user->RGphone, p, RGphoneLEN);
	    user->RGphone[RGphoneLEN] = '\0';
	} else if (strcmp("RGemail", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->RGemail, p);
	} else if (strcmp("RGurl", type) == 0) {
	    p = strtok(NULL, DELIM);
	    if (p)
		strcpy(user->RGurl, p);
	}
    }

    /* done reading */
    (void) fclose(fp);

    if (!(user->configuration))
	user->configuration = 0;

#ifdef DEBUG
    printf("Successfully read user: %s\n\r", user->username);
    fflush(stdout);
#endif

    return user;
}


/*************************************************
* int new_writeuser( user_t *user, int update )
*
* This save the users datafile. 
*
* if update==1 then the user should be updated in
* the wholist
*************************************************/

int
writeuser(user_t * user, int update)
{

    FILE *fp;
    char work[61], *p;
    char work2[66];
    int i, ret;

    if (user == NULL)
	return -1;

    p = getuserdir(user->username);
    strncpy(work, p, 60);

    if (fexists(work) == FALSE) {
	/* non-existant directory, -> create */
	ret = mkdir(work, 0700);
	if (ret == -1) {
	    (void) log_it("errors", "Cannot create directory `%s', mode 0700, err=%s", work, strerror(errno));
	    (void) fprintf(stderr, "error: couldn't create directory: %s.\n", work);
	    return -1;
	}
	(void) sprintf(work2, "%s/mail", work);
	ret = mkdir(work2, 0700);
	if (ret == -1) {
	    (void) log_it("errors", "Cannot create directory `%s', mode 0700, err=%s", work2, strerror(errno));
	    (void) fprintf(stderr, "error: couldn't create directory %s.\n", work2);
	    return -1;
	}
    }
    strcat(work, "/save");
    if ((fp = xfopen(work, "w", FALSE)) == NULL) {
	/* xfopen() does the logging for this */
	(void) fprintf(stderr, "error: couldn't open userfile: %s for writing.\n", work);
	mono_errno = E_NOPERM;
	return -1;
    }
    /* if a usernumber has not been assigned yet, do so now */
    if (user->usernum <= 0)
	get_new_usernum(user->username, &user->usernum);

    /* okay..now the saving starts... */

    (void) fprintf(fp, "username|%s\n", user->username);
    (void) fprintf(fp, "password|%s\n", user->password);
    (void) fprintf(fp, "usernum|%lu\n", (unsigned long) user->usernum);
    (void) fprintf(fp, "priv|%u\n", user->priv);

    (void) fprintf(fp, "lastseen");
    for (i = 0; i < MAXQUADS; i++)
	(void) fprintf(fp, "|%ld", user->lastseen[i]);
    (void) fprintf(fp, "\n");

    (void) fprintf(fp, "generation");
    for (i = 0; i < MAXQUADS; i++)
	(void) fprintf(fp, "|%d", user->generation[i]);
    (void) fprintf(fp, "\n");

    (void) fprintf(fp, "forget");
    for (i = 0; i < MAXQUADS; i++)
	(void) fprintf(fp, "|%d", user->forget[i]);
    (void) fprintf(fp, "\n");

    (void) fprintf(fp, "roominfo");
    for (i = 0; i < MAXQUADS; i++)
	(void) fprintf(fp, "|%d", user->roominfo[i]);
    (void) fprintf(fp, "\n");

    (void) fprintf(fp, "mailnum|%ld\n", user->mailnum);

    (void) fprintf(fp, "flags|%u\n", user->flags);
    (void) fprintf(fp, "chat|%u\n", user->chat);
    (void) fprintf(fp, "config_flags|%u\n", user->config_flags);
    (void) fprintf(fp, "lang|%s\n", user->lang);
    (void) fprintf(fp, "alias|%s\n", user->alias);
    (void) fprintf(fp, "configuration|%u\n", user->configuration);
    (void) fprintf(fp, "screenlength|%u\n", user->screenlength);
    (void) fprintf(fp, "alarm_clock|%d\n", user->alarm_clock);
    (void) fprintf(fp, "timescalled|%u\n", user->timescalled);
    (void) fprintf(fp, "posted|%d\n", user->posted);

    (void) fprintf(fp, "RA_rooms");
    for (i = 0; i < 5; i++)
	(void) fprintf(fp, "|%d", user->RA_rooms[i]);
    (void) fprintf(fp, "\n");

    (void) fprintf(fp, "validation_key|%ld\n", user->validation_key);
    (void) fprintf(fp, "x_s|%lu\n", user->x_s);
    (void) fprintf(fp, "hidden_info|%d\n", user->hidden_info);
    (void) fprintf(fp, "firstcall|%ld\n", user->firstcall);
    (void) fprintf(fp, "birthday_day|%d\n", user->birthday.day);
    (void) fprintf(fp, "birthday_month|%d\n", user->birthday.month);
    (void) fprintf(fp, "birthday_year|%d\n", user->birthday.year);
    (void) fprintf(fp, "laston_from|%ld\n", user->laston_from);
    (void) fprintf(fp, "laston_to|%ld\n", user->laston_to);
    (void) fprintf(fp, "online|%ld\n", user->online);
    (void) fprintf(fp, "doing|%s\n", user->doing);
    (void) fprintf(fp, "timezone|%s\n", user->timezone );
    (void) fprintf(fp, "awaymsg|%s\n", user->awaymsg);
    (void) fprintf(fp, "lasthost|%s\n", user->lasthost);
    (void) fprintf(fp, "xtrapflag|%s\n", user->xtrapflag);
    (void) fprintf(fp, "aideline|%s\n", user->aideline);
    (void) fprintf(fp, "RGname|%s\n", user->RGname);
    (void) fprintf(fp, "RGaddr|%s\n", user->RGaddr);
    (void) fprintf(fp, "RGcity|%s\n", user->RGcity);
    (void) fprintf(fp, "RGstate|%s\n", user->RGstate);
    (void) fprintf(fp, "RGcountry|%s\n", user->RGcountry);
    (void) fprintf(fp, "RGzip|%s\n", user->RGzip);
    (void) fprintf(fp, "RGphone|%s\n", user->RGphone);
    (void) fprintf(fp, "RGemail|%s\n", user->RGemail);
    (void) fprintf(fp, "RGurl|%s\n", user->RGurl);
    /* done saving */

    (void) fclose(fp);

    return 0;
}

int
del_user(const char *name)
{
    char work2[61];
    unsigned int user_id;
    int ret;

    /* do this first, delete commands won't matter if the table
       does not exit */
    ret = mono_sql_u_name2id(name, &user_id);
    if ( ret == 0 )
       del_sql_user(user_id);

    if (!check_user(name)) {
	mono_errno = E_NOUSER;
	return -1;
    }
    (void) sprintf(work2, "rm -rf %s", getuserdir(name));

    if (system(work2) == 0)
	return 0;
    return -1;
}

int
del_sql_user(unsigned int user_id)
{
    mono_sql_uf_kill_user(user_id);
    mono_sql_uu_kill_user(user_id);
    mono_sql_ut_kill_user(user_id);
    mono_sql_u_kill_user(user_id);
    return 0;
}

int
rename_user(const char *from, const char *to)
{
    user_t *user;

    if (mono_sql_u_check_user(from) == FALSE) {
	mono_errno = E_NOUSER;
	return -1;
    }
    if (mono_sql_u_check_user(to) == TRUE)
	return -1;

    user = readuser(from);
    if (user == NULL)
	return -1;
    strcpy(user->username, to);
    if (writeuser(user, 0))
	return -1;
    return del_user(from);
}

int
write_profile(const char *name, char *profile)
{
    FILE *fp;
    char work[61];

    if (!mono_sql_u_check_user(name)) {
	mono_errno = E_NOUSER;
	return -1;
    }
    (void) sprintf(work, "%s/profile", getuserdir(name));

    fp = xfopen(work, "w", FALSE);
    if (fp == NULL) {
	mono_errno = E_NOPERM;
	return -1;
    }
    (void) fprintf(fp, "%s", profile);
    (void) fclose(fp);
    return 0;
}


char *
read_profile(const char *name)
{
    int fd;
    char *p, work[61];
    struct stat buf;

    if (!mono_sql_u_check_user(name)) {
	mono_errno = E_NOUSER;
	return NULL;
    }

    (void) sprintf(work, "%s/profile", getuserdir(name));

    fd = open(work, O_RDONLY);
    if (fd < 0) {
	mono_errno = E_NOUSER;
	return NULL;
    }
    if (fstat(fd, &buf) == -1)
	return "";

    p = (char *) xmalloc((unsigned)buf.st_size);
    /* needs more error checking */
    read(fd, p, (unsigned)buf.st_size);
    (void) close(fd);
    return p;
}

char *
name2file(char *name)
{
    char *p;

    if (name == NULL)
	return NULL;

    p = name;
    while (*p != '\0') {
	if (*p == ' ')
	    *p = '_';
	else
	    *p = tolower(*p);
	p++;
    }
    return name;
}

/*************************************************
* isbad()
*
* returns 1 if user is xlogged
*************************************************/

int
isbad(const char *user_name)
{
    user_t *tmpuser;
    int amibad;

    if ((tmpuser = readuser(user_name)) == NULL) {
	mono_errno = E_NOUSER;
	return 0;
    }
    amibad = (tmpuser->flags & US_IAMBAD) ? TRUE : FALSE;
    xfree(tmpuser);
    return amibad;
}

/*************************************************
* dis_regis( user_t *userdata, int override )
*
* userdate: the user we are looking at.
* override: TRUE if the user can see the hidden
*           address, else FALSE
*************************************************/
char *
read_regis(const user_t * user, int override)
{
    char *p, line[80];
    char name[80];
    char address[80];
    char zip[80];
    char city[80];
    char state[80];
    char country[80];
    char phone[80];
    char email[80];
    char url[100];

    if (user == NULL)
	return NULL;

    p = (char *) xmalloc(60 * 80 * sizeof(char));
    strcpy(p, "\n");

    /* this is the old hide system, for backwards compatibility */
    if ((!(user->flags & US_NOHIDE)) && !override) {
	strcpy(p, "[info hidden]\n");
	return p;
    }

    mono_sql_u_get_registration( user->usernum, name, address, zip, city, state, country, phone );
    mono_sql_u_get_email( user->usernum, email );
    mono_sql_u_get_url( user->usernum, url );

    strcpy(p, "\1a\1g");

    if (strlen(name) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_REALNAME))
	    (void) sprintf(line, "\1f%s\n", name); 
	else if (override == TRUE)
	    (void) sprintf(line, "(%s)\n", name);
	strcat(p, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(address) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_ADDRESS))
	    (void) sprintf(line, "\1f%s\n", address);
	else if (override == TRUE)
	    (void) sprintf(line, "(%s)\n", address);
	strcat(p, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(zip) > 0 || strlen(city) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_CITY))
	    (void) sprintf(line, "\1f%s %s, ", zip, city);
	else if (override == TRUE)
	    (void) sprintf(line, "(%s %s), ", zip, city);
	strcat(line, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(state) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_COUNTRY))
	    (void) sprintf(line, "\1f%s, ", state);
	else if (override == TRUE)
	    (void) sprintf(line, "(%s), ", state);
	strcat(line, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(country) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_COUNTRY))
	    (void) sprintf(line, "\1f%s\n", country);
	else if (override == TRUE)
	    (void) sprintf(line, "(%s)\n", country);
	strcat(line, "\1a\1g");
	strcat(p, line);
    } else
	strcat(p, "\n");

    if (strlen(phone) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_PHONE))
	    (void) sprintf(line, "\1fPhone:\1g %s  ", phone);
	else if (override == TRUE)
	    (void) sprintf(line, "Phone:\1g (%s)  ", phone);
	strcat(line, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(email) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_EMAIL)) {
	    (void) sprintf(line, "\1fEmail:\1g %s\n", email);
        } else if (override == TRUE) {
	    (void) sprintf(line, "Email:\1g (%s)\n", email);
        }
	strcat(line, "\1a\1g");
	strcat(p, line);
    }
    if (strlen(url) > 0) {
	strcpy(line, "");
	if (!(user->hidden_info & H_URL)) {
	    (void) sprintf(line, "\1fURL  :\1g %s\n", url);
	} else if (override == TRUE) {
	    (void) sprintf(line, "URL  :\1g (%s)\n", url);
        }
	strcat(line, "\1a\1g");
	strcat(p, line);
    }
    strcat(line, "\1a");
    return p;
}

char *
getuserdir(const char *name)
{
    static char userdir[L_USERNAME + 50];

    (void) sprintf(userdir, "%s/%s", USERDIR, name);
    (void) name2file(userdir);
    return userdir;
}

#define USERNUM		BBSDIR "/etc/userkey"
#define LOCK		1
#define UNLOCK		0
#define S_BUSYUSERNUM	8	/* set status as busy */

unsigned int
get_new_usernum(const char *usernm, unsigned int *num)
{

    FILE *fp = NULL;
    char buf[10];
    unsigned long porqui = 0;

    /* michel does it the simple sql way */
    mono_sql_u_add_user(usernm);
    mono_sql_u_name2id(usernm, num);
    mono_sql_uf_new_user( *num );
    mono_sql_ut_new_user( *num );

    return *num;

    /* Set a lock to avoid nasty mess */
    if (mono_lock_usernum(LOCK) == -1) {
	log_it("errors", "Unable to get new usernumber: Can't get lock");
	return 0;
    }
    /*
     * Do not segfault if we can't open this file, but whine loud and hard.
     */
    fp = xfopen(USERNUM, "r", FALSE);
    if (fp == NULL) {
	mono_errno = E_NOFILE;
	log_it("errors", "Unable to get new usernumber");
	mono_lock_usernum(UNLOCK);
	fprintf(stderr, "ERROR: Cannot open the usernumber file! Fix asap!\n");
	fflush(stderr);
	return 0;
    }
    fgets(buf, 10, fp);
    buf[strlen(buf) - 1] = '\0';

    /*
     * make a log entry if we can't get the current usernum and return 0
     * to make sure the new user gets booted
     */
    if (sscanf(buf, "%lu", &porqui) != 1) {
	fclose(fp);
	log_it("errors", "Unable to get new usernumber: sscanf() failed");
	/* remove lock for next victim */
	mono_lock_usernum(UNLOCK);
	return 0;
    }
    fclose(fp);

    /*
     * since rewinding doesn't work, we just unlink the damned thing
     */
    unlink(USERNUM);

    /*
     * re-create file with incremented number
     */
    fp = xfopen(USERNUM, "a", TRUE);

    if (fp == NULL) {
	mono_errno = E_NOFILE;
	log_it("errors", "Unable to get new usernumber");
	mono_lock_usernum(UNLOCK);
	return 0;
    }
    porqui++;

    fprintf(fp, "%lu\n", porqui);
    fclose(fp);

    mono_lock_usernum(UNLOCK);

    return porqui;

}

int
mono_lock_usernum(int key)
{
    unsigned int a;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
/*
 * LOCK = 1
 * UNLOCK = 0
 */
    if (key == LOCK) {
	for (a = 0; (a < 50 && (shm->status & S_BUSYUSERNUM)); a++)
	    usleep(100000);

	shm->status |= S_BUSYUSERNUM;	/* set the lock-flag */
    } else {
	shm->status &= ~S_BUSYUSERNUM;	/* unset the lock-flag */
    }
    return 0;
}
/* eof */
