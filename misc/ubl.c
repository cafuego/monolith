/* $Id$ 
 *
 * this file should be run once every day using cron.
 * WKR list should show kicked people in non-invite only rooms 
 *
 * BUG: Users with long names don't seem to appear in the list.
 *
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
#include "msg_file.h"
#include "libmono.h"
#include "setup.h"

typedef struct userinfo {
    char username[L_USERNAME + 1];
    unsigned int timescalled;
    unsigned priv;
    long laston;
    unsigned flags;
    char generation[MAXQUADS];
    char forget[MAXQUADS];
} info_t;

/* global variables */
unsigned int usercount = 0;
info_t *all_users = NULL;
room_t *all_rooms = NULL;

int NewUser = 0, no_cursed = 0, no_deleted = 0, no_purged = 0, expert_users = 0,
    shix_users = 0;
/* ----------------------------- prototypes ------------------------- */

void read_all_users(void);
void read_all_rooms(void);
int main(int, char **);
void update_whoknows(void);
void update_admin_lists(void);

/* cleaning up the warnings behind peter */
int post_ubl_results(void);

int whotest(info_t *, int);
char *add_date(void);
int columnize(char *, char *);
int user_compare(const info_t *, const info_t *);

/* ----------------------------------------------------------------- */

int
main(int argc, char **argv)
{
    set_invocation_name(argv[0]);
    chdir(BBSDIR);

    /* read the necessary data */
    mono_connect_shm();
    read_all_users();
    mono_detach_shm();
    read_all_rooms();

    /* update lists */
    update_whoknows();
    update_admin_lists();
    printf("Posting results to Fleet headquarters...");
    fflush(stdout);
    if (post_ubl_results() == -1)
	log_it("ubl", "Couldn't post results to Fleet Headquarters");
    printf("done.\n");

    /* end */
    xfree(all_users);
    xfree(all_rooms);
    return 0;
};

/************************************************************
* columnize a character string by insert string appropriately
* if the string is long enough to send to file then return
* YES, else return NO
************************************************************/

int
columnize(char *name, char *output)
{
    strcat(output, " ");
    strcat(output, name);

    while (strlen(output) % 19)	/* columnize output */
	strcat(output, " ");
    if (strlen(output) > 63)	/* send to file when line is full */
	return 1;
    return 0;
}

/*
 */
void
read_all_users()
{
    char name[L_USERNAME + 1];
    unsigned int i, j;
    info_t *current;
    user_t *user;
    DIR *userdir;
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

    /* calculate how much memory is needed for all users */
    all_users = xmalloc(usercount * sizeof(info_t));
    current = all_users;

    if (all_users == NULL) {
	fprintf(stderr, "%s", "Unable to allocate memory!!!\n");
	closedir(userdir);
	exit(3);
    }
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
	    xfree(all_users);
	    exit(3);
	}
	user = readuser(name);
	if (user == NULL)
	    continue;
	if (EQ(user->username, "Sysop"))
	    continue;

	/* copy user_t to into_t */
	strcpy(current->username, user->username);
	current->timescalled = user->timescalled;
	current->laston = user->laston_to;
	current->priv = user->priv;
	current->flags = user->flags;
	for (j = 0; j < MAXQUADS; j++) {
	    current->generation[j] = user->generation[j];
	    current->forget[j] = user->forget[j];
	}

	if (current->flags & US_SHIX)
	    shix_users++;
	if (current->flags & US_EXPERT)
	    expert_users++;

	xfree(user);
	current++;
	i++;
	printf("%-5u users left to read.\r", usercount - i);	/* users left to READ */
	fflush(stdout);

    }				/* while */

    closedir(userdir);

    printf("\nsorting....");
    fflush(stdout);
    qsort(all_users, usercount, sizeof(info_t), user_compare);
    printf("done\n");
    fflush(stdout);

    return;
}

int
user_compare(const info_t * een, const info_t * twee)
{
    return strcasecmp(een->username, twee->username);
    /* need something better to do alphabetic sorting */
}

/*************************************************
*/
void
read_all_rooms()
{
    unsigned int i;

    mono_connect_shm();

    all_rooms = xmalloc(MAXQUADS * sizeof(room_t));

    for (i = 0; i < MAXQUADS; i++) {
	all_rooms[i] = read_quad(i);
    }
    mono_detach_shm();
    return;
};

/*************************************************
* add_date()				(10-04-95)
*************************************************/
char *
add_date()
{
    time_t t;
    struct tm *tp;
    static char str[30];

    time(&t);
    tp = localtime(&t);

    sprintf(str, "%.2d-%.2d-%.2d", tp->tm_mday, 1 + tp->tm_mon, tp->tm_year);
    return str;
}

/************************************************************
* update the who knows room list for all rooms
************************************************************/

void
update_whoknows()
{
    FILE *fp;
    char filestr[160];
    int lngth, rm_nbr;
    info_t *u;
    unsigned int i;

    for (rm_nbr = 0; rm_nbr < MAXQUADS; rm_nbr++) {
	u = all_users;

	if ((all_rooms[rm_nbr].flags & QR_INUSE) == 0)
	    continue;		/* don't update unused rooms */

	printf("Updating WKR file for room: %d.\r", rm_nbr);
	fflush(stdout);

	sprintf(filestr, FORUMDIR "/%u/whoknows", rm_nbr);

	fp = xfopen(filestr, "w", FALSE);
	if (fp == NULL) {
	    fprintf(stderr, "Couldn't open whoknows file for room %d\n", rm_nbr);
	    exit(1);
	}
	/* make a heading in the whoknows file for this room */
	sprintf(filestr, "\1f\1gWho knows \1w%2.2d.\1g%s\1w>\1g as of %s:\n", rm_nbr,
		all_rooms[rm_nbr].name, add_date());

	lngth = strlen(filestr);
	for (i = 0; i < lngth - 1; ++i)
	    strcat(filestr, "-");	/* underline heading */
	fprintf(fp, "\1y%s\1g\n", filestr);

	strcpy(filestr, "");

	for (i = 0; i < usercount; i++) {
	    if (strlen(u->username) > 0)
		if (whotest(u, rm_nbr)) {
		    if (columnize(u->username, filestr)) {
			fprintf(fp, "%s\n", filestr);
			*filestr = '\0';
		    }
		}
	    u++;
	}			/* end users for loop */

	if (strlen(filestr) < 54)
	    fprintf(fp, "%s\n", filestr);
	fclose(fp);

    }				/* end rooms for loop */

    return;
}



/*************************************************
* update_admin_lists()
*************************************************/

void
update_admin_lists()
{
    FILE *af, *ax, *au, *at, *as, *ad;
    unsigned int nameflag = 0, rm_nbr, no_donator = 0, no_ht = 0;
    char tmpstr[120];
    unsigned int i, z;
    info_t *u;
    int away;
    time_t currtime;

    af = xfopen(ADMINLIST, "w", FALSE);
    ax = xfopen(ADMINLISTSYS, "w", FALSE);
    au = xfopen(NEWUSERLIST, "w", FALSE);
    at = xfopen(TWITLIST, "w", FALSE);
    as = xfopen(PURGELIST, "w", FALSE);
    ad = xfopen(DONATORLIST, "w", FALSE);

    if (!(af && ax && au && at && ad)) {
	fprintf(stderr, "%s", "Couldn't open list file for output.\n");
	exit(1);
    }
    printf("Opened admin list files.\n");

    fprintf(af, "%s, %s, %s, %s\n\1f\1w================ \1g%s\1w )==============\n\n\1f\1gUsername              \1rAway \1yTitles\n\1w------------------------------------------------------------------------------", WIZARDTITLE, SYSOPTITLE, ROOMAIDETITLE, GUIDETITLE, add_date());
    fprintf(ax, "\1f%s, %s, %s, %s\n\1w=============( \1g%s\1w )============\n\n\1r*************************************\n******** \1wThe Unofficial List \1r********\n*************************************\n\n\1gUsername              \1rAway \1yTitles\n------------------------------------------------------------------------------", WIZARDTITLE, SYSOPTITLE, ROOMAIDETITLE, GUIDETITLE, add_date());
    fprintf(au, "The NewUser-List\n==( %s )==\n\nUsername              Logins   Absent\n-------------------------------------------\n", add_date());
    fprintf(at, "The TWIT&Deleted-List\n=====( %s )====\n\nUsername                       Absent\n-------------------------------------------\n", add_date());
    fprintf(as, "The Purge-List\n=( %s )=\n\nUsername               Logins  Absent\n-------------------------------------------\n", add_date());
    fprintf(ad, "\1f\1wThe Donator-List    \n==( \1g%s\1w )==\1a\n\n\1f\1rUsername              Logins   Absent\1a\n-------------------------------------------\n", add_date());

    currtime = time(NULL);

    printf("Making lists of %d users.\n", usercount);
    fflush(stdout);

    u = all_users;
    for (i = 0; i < usercount; i++) {

	if (!u)
	    break;

	if (strlen(u->username) < 1) {
	    u++;
	    continue;
	}
	/*printf( "%s..", u->username ); fflush( stdout ); */

	away = (currtime - u->laston) / (24 * 60 * 60);

	if (((u->flags & US_PERM) == 0)
	    && ((u->timescalled < 10 && away > 30) ||
		(away > 150) ||
		(u->priv & PRIV_DELETED)
	    )
	    ) {			/* Purge-Baits */
	    fprintf(as, "%-20s  %5d     %3d\n", u->username, u->timescalled, away);
	    no_purged++;
	}
	if (u->flags & US_DONATOR) {	/* no_donator-List */
	    fprintf(ad, "\1f\1g%-20s %5d       \1y%d\1a\n",
		    u->username, u->timescalled, away);
	    no_donator++;
	}
	if (u->timescalled < 10) {	/* NewUserList */
	    fprintf(au, "%-20s %5d       %d\n", u->username, u->timescalled, away);
	    NewUser++;
	}
	if (u->priv & (PRIV_TWIT | PRIV_DELETED)) {	/* TWIT/Deleted-List    */
	    fprintf(at, "%-20s %-7s    %3d\n",
		    u->username, ((u->priv & PRIV_DELETED) ? "DELETED" : "TWIT"), away);

	    if (u->priv & PRIV_TWIT)
		no_cursed++;
	    else if (u->priv & PRIV_DELETED)
		no_deleted++;
	}
	/* names don't get written to file unless they're aides, etc. */
	if ((u->priv & (PRIV_TECHNICIAN | PRIV_SYSOP | PRIV_WIZARD))
	    || (u->flags & US_GUIDE)
	    || (u->flags & US_ROOMAIDE)
	    ) {
	    nameflag = 1;

	    sprintf(tmpstr, "\n\1g%-20s  \1r%3d",
		    u->username, away);
	    while (strlen(tmpstr) % (40))
		strcat(tmpstr, " ");

	    if (u->priv & PRIV_WIZARD)
		strcat(tmpstr, WIZARDTITLE " ");
	    if (u->priv & PRIV_SYSOP)
		strcat(tmpstr, SYSOPTITLE " ");
	    if (u->priv & PRIV_TECHNICIAN)
		strcat(tmpstr, PROGRAMMERTITLE " ");
	    if (u->flags & US_ROOMAIDE)
		strcat(tmpstr, ROOMAIDETITLE " ");
	    if (u->flags & US_GUIDE) {
		strcat(tmpstr, GUIDETITLE " ");
		no_ht++;
	    }
	    fprintf(af, "%s\n", tmpstr);
	    fprintf(ax, "%s\n", tmpstr);
	    strcpy(tmpstr, "");
	}
	for (rm_nbr = 0; rm_nbr < MAXQUADS; rm_nbr++) {
	    if (u != NULL) {
		for (z = 0; z < NO_OF_QLS; z++) {
		    if ((EQ(u->username, all_rooms[rm_nbr].qls[z]))
			&& (all_rooms[rm_nbr].flags & QR_INUSE)) {

			if (nameflag == 0) {
			    nameflag = 1;
			    fprintf(af, "\n%s is RA in %d, but without RA-flag!\n", u->username, rm_nbr);
			    fprintf(ax, "\n%s is RA in %d, but without RA-flag!\n", u->username, rm_nbr);
			}
			if (!((all_rooms[rm_nbr].flags & QR_GUESSNAME)
			      || (rm_nbr <= 5 && rm_nbr >= 2)))
			    fprintf(af, "                                   \1w%d.\1y%s>\n",
				    rm_nbr, all_rooms[rm_nbr].name);
			fprintf(ax, "                                   \1w%d.\1y%s>\n",
				rm_nbr, all_rooms[rm_nbr].name);

		    }
		}
	    }
	}			/* end room loop */

	nameflag = 0;
	u++;

    }				/* end user loop */

    fprintf(af, "\nThere are %u Help Terminals\n", no_ht);
    fclose(af);

    fclose(ax);

    fprintf(au, "\n%u users are new.\n", NewUser);
    fclose(au);

    fprintf(ad, "\1f\1w\n%u users are Donators\1a\n", no_donator);
    fclose(ad);

    fprintf(at, "\n%u users are marked for Deletion, %u users are TWITted.\n"
	    ,no_deleted, no_cursed);
    fclose(at);

    fprintf(as, "\n%u users will be Deleted in the next purge.\n", no_purged);
    fclose(as);

    return;
}

/*************************************************
*
* who test
* returns YES (1) or NO (0) depending on whether
* user has joined room or not
*
* should probably use something in libquad.c
*
*************************************************/

int
whotest(info_t * u, int rm_nbr)
{
#ifndef OLDWHOTEST

    if (rm_nbr == 0)
        return 1;

    if (u->priv & PRIV_DELETED)
        return 0;

    if (u->priv & PRIV_TWIT) {
        if (rm_nbr == 13 || rm_nbr == 1)  /* mail / curseroom */
            return 1;
        else
            return 0;
    }

    if (u->priv >= PRIV_WIZARD)
        return 1;               /* Wizard are _always_ allowed */

    else if (rm_nbr == 5 || rm_nbr == 4)
        return 0;               /* only emps in Garbage & Emps */

    else if (u->priv >= PRIV_SYSOP)
        return 1;

    else if (rm_nbr == 2 || rm_nbr == 3)
        return 0;               /* Only Sysops and higher in 2>, 3>     */
    /* don't allow guest in mail */
    if (rm_nbr == 1 && EQ(u->username, "Guest"))
        return 0;

    else if ((rm_nbr == 6) && !(u->flags & (US_ROOMAIDE | US_GUIDE)))
        return 0;

    else if (!(all_rooms[rm_nbr].flags & QR_INUSE))
        return 0;

    else if (u->generation[rm_nbr] == (-5))        /* kicked */
        return 0;

    else if (rm_nbr == 13 && (!(u->priv & PRIV_TWIT)))
        return 0;

    else if ((all_rooms[rm_nbr].flags & QR_PRIVATE) && all_rooms[rm_nbr].generation != u->generation[rm_nbr])        return 0;

/* watch it, we're slightly chagning it fro mmay_read_room */

    return (u->forget[rm_nbr] != all_rooms[rm_nbr].generation);

#else
    if (u == NULL)
	return 0;

    if (((rm_nbr == 2) || (rm_nbr == 3)) && (u->priv < PRIV_SYSOP))
	return 0;

    else if (((rm_nbr == 4) || (rm_nbr == 5)) && (u->priv < PRIV_WIZARD))
	return 0;

    else if (u->priv & PRIV_WIZARD)
	return 1;		/* wizards are _always_ allowed */

    else if ((rm_nbr == 6) && !((u->flags & US_ROOMAIDE) || (u->flags & US_GUIDE) ||
				(u->priv >= PRIV_SYSOP)))
	return 0;

    else
	return (

	/* only bother if the room is in use */
		   (all_rooms[rm_nbr].flags & QR_INUSE)
	/* if the room is not guessname */
		   && ((!(all_rooms[rm_nbr].flags & QR_GUESSNAME))
	/* or user is an aide */
		       || (u->priv >= PRIV_SYSOP)
	/* or user already knows the room */
		 || (all_rooms[rm_nbr].generation == u->generation[rm_nbr]))
	/* is not a private room */
		   && ((!(all_rooms[rm_nbr].flags & QR_PRIVATE))
	/* or user is a Wizard */
		       || (u->priv >= PRIV_WIZARD)
	/* or if the room is guessname */
		       || (all_rooms[rm_nbr].flags & QR_GUESSNAME)
	/* or user already knows the room */
		 || (all_rooms[rm_nbr].generation == u->generation[rm_nbr]))
	/* user is not kicked out of the public room */
		   && (u->generation[rm_nbr] != (-5))
	    );			/* end of complex return */
#endif
}

int
post_ubl_results()
{

    FILE *fp;
    message_header_t header;
    char filename[L_FILENAME + 1];

    memset(&header, 0, sizeof(message_header_t));
    header.banner_type = SYSTEM_BANNER;
    strcpy(header.author, "Monolith UBL");
    strcpy(header.forum_name, "Fleet Headquarters");
    header.date = time(NULL);
    header.f_id = SYSOP_FORUM;
    strcpy(header.subject, "UBL results.");

    mono_connect_shm();
    header.m_id = get_new_message_id(SYSOP_FORUM);
    mono_detach_shm();

    sprintf(filename, FORUMDIR "/%u/%u.h", header.f_id, header.m_id);
    fp = xfopen(filename, "w", FALSE);
    if (fp == NULL)
	return -1;
    fwrite(&header, sizeof(message_header_t), 1, fp);
    fclose(fp);

    sprintf(filename, FORUMDIR "/%u/%u.t", header.f_id, header.m_id);
    fp = xfopen(filename, "w", FALSE);
    if (fp == NULL)
	return -1;

    /* the actual post content. */
    fprintf(fp, "UBL updated %d users.\n", usercount);
    fprintf(fp, "URL found %d new users.\n", NewUser);
    fprintf(fp, "UBL found %d cursed users.\n", no_cursed);
    fprintf(fp, "%d users were marked for deletion.\n", no_deleted);
    fprintf(fp, "%d users have Expert Status enabled. (%d%%)\n", expert_users, (expert_users * 100) / usercount);
    fprintf(fp, "%d users have SHIX enabled. (%d%%)\n", shix_users, (shix_users * 100) / usercount);

    fclose(fp);

    return 0;
}
