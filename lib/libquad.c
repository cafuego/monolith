
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>		/* also */
#include <string.h>		/* too */
#include <sys/file.h>		/* for flock */
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>		/* added by kirth */
#include <time.h>

#include "monolith.h"

#define extern
#include "libquad.h"
#undef extern

#include "btmp.h"
#include "log.h"
#include "sql_topic.h"
#include "msg_file.h"
#include "userfile.h"
#include "routines.h"
#include "sql_userforum.h"
#include "sql_user.h"
#include "libshix.h"


/*************************************************
* change_QL
* how : 1 flags as QL, -1 removes the flag
* returns 0 on success
*************************************************/

int
change_QL(unsigned int quad_num, const char *QLname, int how)
{
    user_t *QL;
    int i, has_ql_flag;

    if ( quad_num > MAXQUADS) {
	mono_errno = E_NOQUAD;
	return -1;
    }
    if (mono_sql_u_check_user(QLname) == FALSE && how == 1) {
	mono_errno = E_NOUSER;
	return -1;
    }
    if ((QL = readuser(QLname)) == NULL)
	return -1;

    if (how > 0) {
	for (i = 0; i < 5; i++) {
	    /* if we're already marked */
	    if (QL->RA_rooms[i] == quad_num)
		break;
	    mono_sql_uf_add_host(QL->usernum, quad_num);
	    if (QL->RA_rooms[i] < 0) {
		QL->RA_rooms[i] = quad_num;
		break;
	    }
	}
	QL->flags |= US_ROOMAIDE;
    } else {
	has_ql_flag = 0;
	mono_sql_uf_remove_host(QL->usernum, quad_num);
	for (i = 0; i < 5; i++) {
	    if (QL->RA_rooms[i] == quad_num)
		QL->RA_rooms[i] = -1;
	    if (QL->RA_rooms[i] > 0)
		has_ql_flag++;
	}

	if (has_ql_flag == 0)
	    QL->flags &= ~US_ROOMAIDE;
	else
	    QL->flags |= US_ROOMAIDE;
    }

    if (writeuser(QL, 0) != 0) {
	xfree(QL);
	return -1;
    }
    xfree(QL);

    if (mono_return_pid(QLname) != -1)
	(void) kill(mono_return_pid(QLname), SIGUSR2);

    return 0;
}

int
is_ql(const char *name, room_t quad)
{
    unsigned int i;

    for (i = NO_OF_QLS; i; i--)
	if (EQ(name, quad.qls[i - 1]))
	    return TRUE;
    return FALSE;
}

/* this function returns the quickroom structure of room 'num' */
room_t
read_quad(unsigned int num)
{
    room_t scratch;
    if (!shm) {
	mono_errno = E_NOSHM;
	return scratch;
    }
    scratch = shm->rooms[num];
    return scratch;
}

int
read_forum( unsigned int no, room_t *forum ) 
{
    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    /* memcpy( forum, &shm->rooms[no], sizeof( room_t ) ); */
    *forum = shm->rooms[no];
    return 0;
}

/* this function writes the quickroom to shm */
/* still need to implement locking, and writing to file */
int
write_quad(room_t room, int num)
{
    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    (void) mono_lock_rooms(1);
    shm->rooms[num] = room;
    (void) mono_lock_rooms(0);
    return 0;
}

int
write_forum( unsigned int no, room_t * forum )
{
    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    (void) mono_lock_rooms(1);
    shm->rooms[no] = *forum;
    (void) mono_lock_rooms(0);
    return 0;
}

int
dump_quickroom()
{
    int fd, d;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    (void) mono_lock_rooms(1);
    fd = xopen(QUICKROOM, O_WRONLY | O_CREAT, FALSE);

    /* if fails, do not crash horribly, but save someplace else */

#ifdef FILE_LOCKING
    while (flock(fd, LOCK_EX | LOCK_NB) < 0) {
	(void) sleep(1);
	if (timeout++ > 20) {
	    (void) log_it("errors", "can't lock quickroom, for dumping");
	    return -1;
	}
    }
#endif /* FILE_LOCKING */

    d = write(fd, shm->rooms, MAXQUADS * sizeof(room_t));

#ifdef FILE_LOCKING
    (void) flock(fd, LOCK_UN);
#endif /* FILE_LOCKING */
    (void) close(fd);
    (void) mono_lock_rooms(0);

    if (d < 1)
	return -1;
    return 0;
}


/* note:  save_new_message -must- abort if this funx returns 0.  */
unsigned int
get_new_message_id(const unsigned int quadno)
{
    room_t *p;

    (void) mono_lock_rooms(1);

    /* for convenience */
    p = &shm->rooms[quadno];

    /* first step: increase highest postnumber */
    p->highest++;

    if (!p->highest) {  
	if (p->lowest > p->highest)    /* rollover */
	    log_it("forums", "forum %u has rolled over..  convert it!", quadno);
	return 0;
    }	

    /* next, check if we should increase the lowest number */
    if ((p->highest - p->lowest) > p->maxmsg) {
	message_delete(quadno, p->lowest);
	p->lowest++;
    }
    (void) mono_lock_rooms(0);

    mono_sql_t_updated_highest( quadno, p->highest );
    return p->highest;
}


unsigned long
get_new_mail_number(const char *name)
{
    long a;
    user_t *user;

    user = readuser(name);

    if (!user) {
	mono_errno = E_NOUSER;
	return -1;
    }
    user->mailnum++;
    a = user->mailnum;

    if (writeuser(user, 0) == -1) {
	xfree(user);
	return -1;
    }
    xfree(user);
    if (mono_return_pid(name) != -1) {
	if ((kill(mono_return_pid(name), SIGUSR2)) == -1) {
	    return -1;
	}
    }
    return a;
}

unsigned int
get_room_number(const char *name)
{
    int i;
    for (i = 0; i < MAXQUADS; i++)
	if (EQ(shm->rooms[i].name, name))
	    return i;
    return -1;
}

/*************************************************
* kickout()
* returns 1 on successfull kickout
*************************************************/
int
kickout(const char *user, unsigned int room)
{

    user_t *userP;
    room_t scratch;

    if (mono_sql_u_check_user(user) == FALSE) {
	mono_errno = E_NOUSER;
	return -1;
    }
    if ( room > MAXQUADS ) {
	mono_errno = E_NOQUAD;
	return -1;
    }
    read_forum( room, &scratch );
    userP = readuser(user);

    if (!userP) {
	mono_errno = E_NOUSER;
	return -1;
    }
    if (userP->priv & PRIV_WIZARD) {
	xfree(userP);
	return -1;
    }
    if (userP->generation[room] == (-5)) {
	xfree(userP);
	return -1;
    }
    userP->generation[room] = (-5);
    userP->forget[room] = scratch.generation;
    mono_sql_uf_add_kicked( room, userP->usernum );

    if (writeuser(userP, 0) != 0) {
	xfree(userP);
	return -1;
    }
    if (mono_return_pid(user) != -1)
	(void) kill(mono_return_pid(user), SIGUSR2);	/* READSELF-signal              */

    (void) log_it("rooms", "kicked %s out of %s>", userP->username, scratch.name);
    xfree(userP);
    return 1;
}


/*************************************************
* invite()
* return value:  0 succeeded
*               -1 error
*************************************************/

int
invite(const char *name, unsigned int room)
{

    user_t *user;
    room_t scratch;

    if (mono_sql_u_check_user(name) == FALSE)
	return -1;
    if ( (room > MAXQUADS) )
	return -1;

    user = readuser(name);

    if (user == NULL)
	return -1;

    read_forum( room, &scratch );

    if (user->generation[room] == scratch.generation) {
	xfree(user);
	return -1;
    }
    user->generation[room] = scratch.generation;
    user->forget[room] = -1;
    mono_sql_uf_add_invited( room, user->usernum );

    (void) writeuser(user, 0);

    if (mono_return_pid(name) != -1)
	(void) kill(mono_return_pid(name), SIGUSR2);

    (void) log_it("rooms", "someone invited %s to %s>", user->username,
		  scratch.name);

    xfree(user);
    return 0;
}


#ifdef SEARCH_CODE_DONE

#define LINE_LENGTH		(L_SUBJECT + L_USERNAME + 20)

char *
search_msgbase(char *string, unsigned int room, unsigned long start, user_t * user)
{

    int match = 0;
    int count = 0;
    int end = 0;
    long i = 0;
    post_t post;
    room_t bing;
    FILE *fp;
    char work[80], *p, line[LINE_LENGTH];
    regexp *exp = NULL;

    read_forum( room, &bing );

    if (room == 1) {		/* seaching mail */
	/* malloc a shitload of memory! */
	p = (char *) xmalloc((bing.maxmsg) * LINE_LENGTH);
	start = user->mailnum - bing.maxmsg;
	if (start < 0)
	    start = 0;
    } else {			/* normal quad */
	/* malloc a shitload of memory! */
	p = (char *) xmalloc((bing.highest - bing.lowest) * LINE_LENGTH);
	if (start < bing.lowest)
	    start = bing.lowest;
    }

    /* compile regular expression */
    exp = regcomp(string);
    strcpy(p, "");
    if (strstr(string, "[") == NULL && strstr(string, "?") == NULL)
	(void) snprintf(line, LINE_LENGTH, "\1f\1gCase insensitive substring search for `\1y%s\1g'.\n\n", string);
    else
	(void) snprintf(line, LINE_LENGTH, "\1f\1gCase insensitive regular expression search for `\1y%s\1g'.\n\n", string);
    strcat(p, line);

    if (room == 1)
	end = user->mailnum;
    else
	end = bing.highest;

    for (i = start; i <= end; i++) {

	/* open the message */
	if (room == 1)
	    snprintf(work, 80, "%s/mail/%ld", getuserdir(user->username), i);
	else
	    snprintf(work, 80, BBSDIR "/save/quads/%d/%ld", room, i);

	fp = fopen(work, "r");
	if (fp == NULL)
	    continue;

	/* read in the header and find a match */
	(void) read_post_header(fp, &post);

	/* close the post file */
	(void) fclose(fp);

	if (post.type == MES_ANON || post.type == MES_AN2) {
	    if (regexec(exp, post.alias) != 0)
		match = 1;
	} else {
	    if (regexec(exp, post.author) != 0)
		match = 2;
	}
	if ((regexec(exp, post.recipient) != 0) && room == 1)
	    match = 1;
	if ((regexec(exp, post.subject) != 0) && (match != 1))
	    match = 3;

	switch (match) {
	    case 1:
		if (room == 1)
		    (void) sprintf(line, "  \1f\1g#%ld\t\t\1y%-20s    \1w(\1g%s\1w)\n", i, post.recipient, (strlen(post.subject) == 0) ? "No subject" : post.subject);
		else
		    (void) sprintf(line, "  \1f\1g#%ld\t\t\1y%-20s    \1w(\1g%s\1w)\n", i, post.alias, (strlen(post.subject) == 0) ? "No subject" : post.subject);
		strcat(p, line);
		count++;
		break;
	    case 2:
		(void) sprintf(line, "  \1f\1g#%ld\t\t\1y%-20s    \1w(\1g%s\1w)\n", i, post.author, (strlen(post.subject) == 0) ? "No subject" : post.subject);
		strcat(p, line);
		count++;
		break;
	    case 3:
		if (strlen(post.subject) > 32)
		    (void) sprintf(line, "  \1f\1g#%ld\t\t\1g%-20s    \1w(\1y%.33s...\1w)\n", i, post.author, post.subject);
		else
		    (void) sprintf(line, "  \1f\1g#%ld\t\t\1g%-20s    \1w(\1y%s\1w)\n", i, post.author, (strlen(post.subject) == 0) ? "No subject" : post.subject);
		strcat(p, line);
		count++;
		break;
	}
	match = 0;
    }
    xfree(exp);			/* OI!!! */
    (void) sprintf(line, "\n\1f\1y      %d\1g match%s found.\n", count, (count == 1) ? "" : "es");
    strcat(p, line);
    return p;
}

#endif

