
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include "userfile.h"
#include "routines.h"
#include "sql_userforum.h"
#include "libshix.h"

char *
post_to_file(unsigned int quad, unsigned long number, const char *name)
{
    static char filename[100];	/* too much probably */

    if ((quad < 0) || (quad > MAXQUADS)) {
	mono_errno = E_NOQUAD;
	return NULL;
    }
    if (number < 0) {
	mono_errno = E_NOMESG;
	return NULL;
    }
    if (quad == 1)
	(void) snprintf(filename, 100, "%s/mail/%ld", getuserdir(name), number);
    else
	(void) snprintf(filename, 100, QUADDIR "/%d/%ld", quad, number);

    return filename;
}


/*************************************************
* save_essage()
* mtmp: temp file that containes message
*	- this file is removed when posted.
* rec : recipient if mail

* return value: 0 - okay
* return value non-0 - error
*************************************************/

int
save_message(const char *mtmp, unsigned int room, const char *rec)
{
    int tries = 0;
    char recip[L_USERNAME + 1];

    if (rec == NULL)
	strcpy(recip, "");
    else
	strcpy(recip, rec);

    if (!fexists(mtmp)) {
	mono_errno = E_NOFILE;
	return -1;
    }
    if (EQ(recip, "sysop")) {
	while (copy(mtmp, post_to_file(2, get_new_post_number(2), NULL)) != 0) {
	    if (++tries > 20) {
		(void) log_it("rooms", "error in saving post in yells");
		return -1;
	    }
	}
    } else if (room == 1) {	/* put in both mail dirs */
	while (copy(mtmp, post_to_file(1, get_new_mail_number(recip), recip)) != 0) {
	    if (++tries > 20) {
		(void) log_it("rooms", "error in saving post in %s's mail", recip);
		return -1;
	    }
	}
    } else {
	while (copy(mtmp, post_to_file(room, get_new_post_number(room), NULL)) != 0) {
	    if (++tries > 20) {
		(void) log_it("rooms", "error in saving post in #%d", room);
		return -1;
	    }
	}
    }
    return 0;
}


/* return values:
 *  0 - okay
 * -1 - error 
 */
int
copy_message(unsigned int from, unsigned long number, unsigned int to, const char *mailbox)
{
    char to_file[100], from_file[100];

    strcpy(from_file, post_to_file(from, number, mailbox));
    strcpy(to_file, post_to_file(to, get_new_post_number(to), mailbox));

    if (copy(from_file, to_file) != 0)
	return -1;		/* ERROR! */
    return 0;
}

int
move_message(unsigned int from, unsigned long number, unsigned int to, const char *mailbox)
{
    char to_file[100], from_file[100];
    int ret;

    strcpy(from_file, post_to_file(from, number, mailbox));
    strcpy(to_file, post_to_file(to, get_new_post_number(to), mailbox));

    ret = rename(from_file, to_file);
    return ret;
}

int
trash_message(unsigned int quad, unsigned long number, const char *mailbox)
{
    if (quad != 1) {		/* don't copy deleted mails to the   */
	(void) copy_message(quad, number, 5, NULL);	/* garbagequad.      */
	/* if cannot delete, then bad luck, and just delete */
    }
    return delete_message(quad, number, mailbox);
}


int
delete_message(unsigned int quad, unsigned long number, const char *mailbox)
{

    if (unlink(post_to_file(quad, number, mailbox)) != 0) {
	mono_errno = E_NOMESG;
	return -1;
    }
    return 0;
}

/*************************************************
* fpgetfield()
*************************************************/

void
fpgetfield(FILE * fp, char *string)
{
    int a = 0, b;

    strcpy(string, "");

    do {
	b = getc(fp);
	if (b < 1 || b == EOF) {
	    string[a] = '\0';
	    return;
	}
	string[a] = b;
	++a;
    }
    while (a < 510);		/* added the || a<510 -lj */

    string[a] = '\0';
}

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

    if ((quad_num < 0) || (quad_num > MAXQUADS)) {
	mono_errno = E_NOQUAD;
	return -1;
    }
    if (check_user(QLname) == FALSE && how == 1) {
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

int
may_read_room(user_t user, room_t room, int quad_num)
{
    if (quad_num == 0)
	return 1;

    if (user.priv & PRIV_DELETED)
	return 0;

    if (user.priv & PRIV_TWIT) {
	if (quad_num == 13 || quad_num == 1)	/* mail / curseroom */
	    return 1;
	else
	    return 0;
    }

    if (user.priv >= PRIV_WIZARD)
	return 1;		/* Wizard are _always_ allowed */

    else if (quad_num == 5 || quad_num == 4)
	return 0;		/* only emps in Garbage & Emps */

    else if (user.priv >= PRIV_SYSOP)
	return 1;

    else if (quad_num == 2 || quad_num == 3)
	return 0;		/* Only Sysops and higher in 2>, 3>     */

    /* don't allow guest in mail */
    if (quad_num == 1 && EQ(user.username, "Guest"))
	return 0;

    else if ((quad_num == 8) && !(user.flags & US_ROOMAIDE))
	return 0;

    else if ((quad_num == 6) && !(user.flags & (US_ROOMAIDE | US_GUIDE)))
	return 0;

    else if (!(room.flags & QR_INUSE))
	return 0;

    else if (user.generation[quad_num] == (-5))		/* kicked */
	return 0;

    else if (quad_num == 13 && (!(user.priv & PRIV_TWIT)))
	return 0;

    else if ((room.flags & QR_PRIVATE) && room.generation != user.generation[quad_num])
	return 0;

    return 1;
}

int
may_write_room(user_t user, room_t room, int a)
{

    /* only tech's and higher in the docking bay */
    if (a == 0 && user.priv < PRIV_TECHNICIAN)
	return 0;

    /* everyone in Yells> and Garbage> */
    if (a == 5 || a == 2)
	return 1;

    /* only sysops and ql's in readonly rooms */
    if ((room.flags & QR_READONLY) && (user.priv < PRIV_SYSOP) &&
	!is_ql(user.username, room)) {
	return 0;
    } else {
	return may_read_room(user, room, a);
    }
}				/* eof */

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


/* return the first free number to post */
unsigned long
get_new_post_number(unsigned int quadno)
{
    room_t *p;

    (void) mono_lock_rooms(1);

    /* for convenience */
    p = &shm->rooms[quadno];

    /* first step: increase highest postnumber */
    p->highest++;

    /* next, check if we should increase the lowest number */
    if ((p->highest - p->lowest) > p->maxmsg) {
	(void) delete_message(quadno, p->lowest, NULL);
	p->lowest++;
    }
    (void) mono_lock_rooms(0);

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

    if (check_user(user) == FALSE) {
	mono_errno = E_NOUSER;
	return -1;
    }
    if ((room < 0) || (room > MAXQUADS)) {
	mono_errno = E_NOQUAD;
	return -1;
    }
    scratch = read_quad(room);
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

    if (check_user(name) == FALSE)
	return -1;
    if ((room < 0) || (room > MAXQUADS))
	return -1;

    user = readuser(name);

    if (user == NULL)
	return -1;

    scratch = read_quad(room);

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


int
read_post_header(FILE * fp, post_t * post)
{
    int b;
    char bbb[550];

    if (fp == NULL)
	return -1;

    memset(post, 0, sizeof(post_t));

    (void) getc(fp);
    post->type = getc(fp);
    (void) getc(fp);		/* read dummy 0 */

    while (1) {
	b = getc(fp);

	if (b == EOF || b == 'M')
	    return 0;

	fpgetfield(fp, bbb);

	switch (b) {

	    case 'L':
		post->lines = atoi(bbb) + 1;
		break;

	    case 'I':
		post->num = atol(bbb);
		break;

	    case 'Y':
		post->ref = atol(bbb);
		break;

	    case 'D':
		strncpy(post->modifier, bbb, L_USERNAME);
		break;

	    case 'U':
		post->moddate = atol(bbb);
		break;

	    case 'S':
		strncpy(post->subject, bbb, L_SUBJECT);
		break;

		/* X identifies reference author (user whose post was replied to) */
	    case 'X':
		strncpy(post->refauthor, bbb, L_USERNAME);
		break;

	    case 'A':
		strncpy(post->author, bbb, L_USERNAME);
		break;

	    case 'O':
		strncpy(post->origroom, bbb, L_QUADNAME);
		break;

	    case 'R':
		strcpy(post->recipient, bbb);
		break;

	    case 'T':
		post->date = atol(bbb);
		break;

	    case 'C':
		strncpy(post->alias, bbb, L_USERNAME);
		break;

		/* post number that was replied to */
	    case 'Q':
		post->ref = atol(bbb);
		break;
	    case 'Z':
		post->reftype = atol(bbb);
		break;
	}
    }

    return 0;
}


/* this function puts the header of post in the open file descriptor fp */
int
write_post_header(FILE * fp, post_t mesg)
{
    if (fp == NULL)
	return -1;

    (void) fprintf(fp, "%c%c%c", 255, (char) mesg.type, 0);	/* type of message    */
    (void) fprintf(fp, "L000%c", 0);	/* #Lines in msg      */
    (void) fprintf(fp, "T%ld%c", mesg.date, 0);		/* date/time          */
    (void) fprintf(fp, "A%s%c", mesg.author, 0);	/* author     */

    if (mesg.refauthor && strlen(mesg.refauthor))
	(void) fprintf(fp, "X%s%c", mesg.refauthor, 0);
    if (mesg.ref)
	(void) fprintf(fp, "Q%ld%c", mesg.ref, 0);
    if (mesg.reftype)
	(void) fprintf(fp, "Z%ld%c", mesg.reftype, 0);
    if (mesg.alias && strlen(mesg.alias) != 0)
	(void) fprintf(fp, "C%s%c", mesg.alias, 0);	/* aliasname          */
    if (mesg.recipient && strlen(mesg.recipient) != 0)
	(void) fprintf(fp, "R%s%c", mesg.recipient, 0);		/* recipient if mail  */
    if (mesg.subject && strlen(mesg.subject) != 0 && mesg.type != MES_DESC)
	(void) fprintf(fp, "S%s%c", mesg.subject, 0);	/* subjectline        */

    if (!mesg.recipient && strlen(mesg.recipient) == 0)		/* do not save orig room whe */
	(void) fprintf(fp, "O%s%c", mesg.origroom, 0);	/* original room      */

    (void) putc('M', fp);	/* start of message   */
    return 0;
}

int
write_post_footer(FILE * filep, int lines)
{
    (void) putc(0, filep);
    if (fseek(filep, 4L, SEEK_SET) != 0)
	return -1;

    fprintf(filep, "%3.3d%c", lines, '\0');
    return 0;
}

int
make_auto_message(const char *tofile, const char *fromfile, post_t mesg)
{
    FILE *fp, *fp2;
    int b, lines;

    fp2 = xfopen(fromfile, "r", FALSE);
    if (fp2 == NULL)
	return -1;

    fp = xfopen(tofile, "w", FALSE);
    if (fp == NULL) {
	(void) fclose(fp2);
	return -1;
    }
    (void) write_post_header(fp, mesg);
    lines = 0;

    while (b = getc(fp2), b > 0) {
	(void) putc(b, fp);
	if (b == '\n')
	    lines++;
    }
    (void) fclose(fp2);
    (void) putc(0, fp);
    (void) write_post_footer(fp, lines);
    (void) fclose(fp);
    return 0;
}

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

    if (room == 1) {		/* seaching mail */
	bing = read_quad(room);
	/* malloc a shitload of memory! */
	p = (char *) xmalloc((bing.maxmsg) * LINE_LENGTH);
	start = user->mailnum - bing.maxmsg;
	if (start < 0)
	    start = 0;
    } else {			/* normal quad */
	bing = read_quad(room);
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
	    snprintf(work, 80, BBSDIR "save/quads/%d/%ld", room, i);

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
