/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>		/* added by michel */
#include <string.h>		/* dito */
#include <stdlib.h>		/* too */
#include <sys/file.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"
#include "setup.h"

#include "display_message.h"
#include "fun.h"
#include "input.h"
#include "messages.h"
#include "uadmin.h"
#include "menu.h"
#include "qc.h"
#include "read_menu.h"  // display_short_prompt()
#include "commands.h"
#include "routines2.h"
#include "enter_message.h"

#define extern
#include "rooms.h"
#undef extern

int need_rewrite;

void menu_hostedit_add(const unsigned int, const long, const char *);
void menu_hostedit_remove(const unsigned int, const long, const char *);
void menu_hostedit_list(const unsigned int, const long, const char *);
void menu_hostedit_kill(const unsigned int, const long, const char *);

static void show_qls(void);
static int print_hosts_simple( unsigned int forum_id );
static void show_room_aides(void);  /* sql ql's version */

static char *fish[] =
{
    "Admin", "Personal", "BBS Issues", "Art and Culture",
    "Discussion", "Computers and Internet", "Support", "Languages",
    "Recreation - Chatting", "Recreation - Silly", "Recreation"
};

#define CATEGORIES	11	/* number of entries in array fish[] */

/* WARNING! this is similar to read_quad() but not the same. This also
 * takes care of mail stuff */
room_t
readquad(int num)
{
    room_t scratch;
    scratch = read_quad(num);
    if (num == 1) {
	scratch.highest = usersupp->mailnum;
	scratch.lowest = 0;
	scratch.maxmsg = 30;
    }
    return scratch;
}

int
i_may_read_forum(const unsigned int forum)
{
    room_t quad;

    quad = read_quad(forum);

    if (forum == DOCKING_BAY_FORUM)
	return 1;

    if (forum >= MAXQUADS)	/* Uhh-ohh.. */
	return 0;

    if (usersupp->priv & PRIV_DELETED)
	return 0;

    if (usersupp->priv & PRIV_TWIT) {
	if (forum == CURSE_FORUM || forum == MAIL_FORUM) 
	    return 1;
	else
	    return 0;
    }

    if (usersupp->priv >= PRIV_WIZARD)
	return 1;		/* emps can read anywhere */

    else if (forum == GARBAGE_FORUM || forum == EMPEROR_FORUM)
	return 0;		/* only emps in Garbage & Emps */

    else if (usersupp->priv >= PRIV_SYSOP)
	return 1;

    else if (forum == YELL_FORUM || forum == SYSOP_FORUM)
	return 0;		/* Only Sysops and higher */

    /* don't allow guest in mail */
    if (forum == MAIL_FORUM && EQ(usersupp->username, "Guest"))
	return 0;

    else if ((forum == QL_FORUM) && !(usersupp->flags & US_ROOMAIDE))
	return 0;

    else if ((forum == HT_FORUM) && !(usersupp->flags & US_GUIDE))
	return 0;

    else if ((forum == LOWER_ADMIN_FORUM) && 
		!(usersupp->flags & (US_ROOMAIDE | US_GUIDE)))
	return 0;

    else if (!(quad.flags & QR_INUSE))
	return 0;

    else if (usersupp->generation[forum] == (-5))		/* kicked */
	return 0;

    else if (forum == CURSE_FORUM && (!(usersupp->priv & PRIV_TWIT)))
	return 0;

    else if ((quad.flags & QR_PRIVATE) && quad.generation != 
		usersupp->generation[forum])
	return 0;

    return 1;
}

int
i_may_write_forum(const unsigned int forum)
{
    room_t quad;

    quad = read_quad(forum);

    if (forum >= MAXQUADS)  /* uhh-ohh */
	return 0;

    /* only tech's and higher in the docking bay */
    if (forum == DOCKING_BAY_FORUM && usersupp->priv < PRIV_TECHNICIAN)
	return 0;

    /* everyone in Yells> and Garbage> */
    if (forum == GARBAGE_FORUM || forum == YELL_FORUM)
	return 1;

    /* only sysops and ql's in readonly quad */
    if ((quad.flags & QR_READONLY) && (usersupp->priv < PRIV_SYSOP) &&
	is_ql(who_am_i(NULL), quad)) {
	return 0;
    } else {
	return i_may_read_forum(forum);
    }
}				


void
show_known_rooms(int param)
{
    char *p;

    switch (param) {
	case 1:
	    p = known_rooms_list(usersupp, 0);
	    break;
	case 2:
	    p = unread_rooms_list(usersupp);
	    break;
	default:
	    p = known_rooms_list(usersupp, 1);
	    break;
    }

    nox = 1;
    more_string(p);
    xfree(p);
    return;
}

/*************************************************
* known_rooms_list()
*************************************************/

char *
known_rooms_list(const user_t * user, int long_k_list)
{

    int quad_num, i;
    int unseen_ctr = 0, zapped_ctr = 0;
    char room_type[8], line[200];
    room_t scratch;
    char *textPtr;
    


    curr_line = 2;
    line_total = -1;		/* don't want any percentages at the -- more -- */

    textPtr = (char *) xmalloc(200 * (MAXQUADS + CATEGORIES));
    strcpy(textPtr, "");

    for (i = 0; i < CATEGORIES; i++) {

	sprintf(line, "\n%s%s%s%s:\n\n", BOLD, YELLOW, fish[i], WHITE);
	strcat(textPtr, line);

	for (quad_num = 0; quad_num < MAXQUADS; quad_num++) {

	    scratch = readquad(quad_num);

	    /* match all these conditions no matter what function
	     */
	    if (!i_may_read_forum(quad_num))
		continue;
	    if (!(scratch.flags & QR_INUSE))
		continue;
	    if (!long_k_list)
		if (is_zapped(quad_num, &scratch))
		    continue;

	    if (EQ(scratch.category, fish[i])) {

		if (((scratch.flags & QR_ANONONLY) || (scratch.flags & QR_ANON2))
		    && (scratch.flags & QR_PRIVATE)) {
		    sprintf(room_type, "%sA%sI", ANON_COL, PRIVATE_COL);
		} else if (scratch.flags & (QR_ANONONLY | QR_ANON2)) {
		    sprintf(room_type, "%sA ", ANON_COL);
		} else if (scratch.flags & QR_PRIVATE) {
		    sprintf(room_type, " %sI", PRIVATE_COL);
		} else {
		    sprintf(room_type, "  ");
		}

		sprintf(line, "    \1w%3d %s \1g%-36s", quad_num, room_type, scratch.name);
		strcat(textPtr, line);

		if (user->forget[quad_num] == scratch.generation) {
		    sprintf(line, "  \1w[\1rZapped\1w]\n");
		    zapped_ctr++;
		} else {
		    if (user->lastseen[quad_num] == scratch.highest)
			sprintf(line, "  \1w[\1c%ld \1g%s\1w]\n", scratch.highest - scratch.lowest, config.message_pl);
		    else {
			sprintf(line, "  \1w[\1c%ld \1g%s, \1c%ld \1gunread\1w]\n", scratch.highest, config.message_pl, scratch.highest - user->lastseen[quad_num]);
			unseen_ctr++;
		    }
		}
		strcat(textPtr, line);
	    }
	}
    }
    sprintf(line, "\n\1f\1g           You have \1c%d \1g%s with unread %s", unseen_ctr,
     (unseen_ctr == 1) ? config.forum : config.forum_pl, config.message_pl);

    if (!long_k_list)
	sprintf(line, ".\n");
    else
	sprintf(line, " and \1c%d \1gzapped %s.\n", zapped_ctr, (zapped_ctr == 1) ? config.forum : config.forum_pl);

    strcat(textPtr, line);
    return textPtr;
}

/*******************************************/
char *
unread_rooms_list(const user_t * user)
{

    int i, unseen_ctr = 0, skipped_ctr = 0;
    char room_type[8], line[200];
    room_t scratch;
    char *textptr;

    textptr = (char *) xmalloc(200 * MAXQUADS);
    strcpy(textptr, "\n");

    for (i = 0; i < MAXQUADS; i++) {
	scratch = readquad(i);

	if (!i_may_read_forum(i))
	    continue;
	if (!(scratch.flags & QR_INUSE))
	    continue;
	if (is_zapped(i, &scratch))
	    continue;
	if (user->lastseen[i] == scratch.highest)
	    continue;

	if (((scratch.flags & QR_ANONONLY) || (scratch.flags & QR_ANON2))
	    && (scratch.flags & QR_PRIVATE))
	    sprintf(room_type, "%s", PRIVATE_COL);
	else if (scratch.flags & (QR_ANONONLY | QR_ANON2))
	    sprintf(room_type, "%s", ANON_COL);
	else if (scratch.flags & QR_PRIVATE)
	    sprintf(room_type, "%s", PRIVATE_COL);
	else
	    sprintf(room_type, "\1g");

	sprintf(line, "\1w%3d. %s%-40s", i, room_type, scratch.name);
	strcat(textptr, line);

	if (skipping[i]) {
	    sprintf(line, "\1w[ \1r**\1yskipped\1w]\n");
	    skipped_ctr++;
	    unseen_ctr++;
	} else {
	    sprintf(line, "\1w[\1c%3ld \1gunread\1w]\n", scratch.highest - user->lastseen[i]);
	    unseen_ctr++;
	}
	strcat(textptr, line);
    }

    sprintf(line, "\n\1f\1gYou have \1c%d \1g%s with unread %s and \1c%d \1gskipped %s.\n", unseen_ctr, (unseen_ctr == 1) ? config.forum : config.forum_pl, config.message_pl, skipped_ctr, (skipped_ctr == 1) ? config.forum : config.forum_pl);
    strcat(textptr, line);
    return textptr;
}

/*
 * do_invite()
 */

void
do_invite()
{
    char *the_user;
    unsigned int user_id;

    cprintf("\1f\1r\nUser to invite to %s\1w>\1r: \1a", quickroom.name);
    nox = 1;
    the_user = get_name(2);

    if (strlen(the_user) == 0)
	return;

    if (check_user(the_user) == FALSE) {
	cprintf("\1f\1rNo such user.\1a\n");
	return;
    }
    if (invite(the_user, curr_rm) == -1) {
	cprintf("\1f\1rSomething went wrong inviting that user.\n\1a");
	return;
    }
    mono_sql_u_name2id(the_user, &user_id);
    mono_sql_uf_add_invited(user_id, curr_rm);

    log_sysop_action("invited %s into %s>.", the_user, quickroom.name);

    return;
}

/*
 * do_kickout()
 * queries for a user to kick out of current room 
 */

void
do_kickout()
{

    char *the_user = NULL;
    user_t *userP;		/* changed varname: PR */
    room_t scratch;
    unsigned int user_id;
    int not_done;

    scratch = read_quad(curr_rm);

    cprintf("\n");
    if (!(scratch.flags & QR_PRIVATE))
	cprintf("\1f\1rRemember that this is a \1gPublic \1r%s!\n\n", config.forum);

    cprintf("\1f\1rAlien to kickout from \1g%s\1w>: \1c", scratch.name);
    nox = 1;
    the_user = get_name(2);
    if (the_user[0] == 0)
	return;

    if (check_user(the_user) == FALSE) {
	cprintf("\n\1f\1rNo such user.\1a\n");
	return;
    }
    userP = readuser(the_user);
    if (userP->priv & PRIV_WIZARD) {
	cprintf("\n\1f\1rYou can't kickout " WIZARDTITLE "s\1r!!!\1a\n\n");
	log_sysop_action("tried to kick %s out of %s>", userP->username, scratch.name);
	xfree(userP);
	return;
    }
    if (kickout(the_user, curr_rm) == -1) {
	cprintf("\n\1f\1rSomething went wrong kicking that user.\n\1a");
	return;
    }
    mono_sql_u_name2id(the_user, &user_id);
    mono_sql_uf_add_kicked(user_id, curr_rm);

    log_sysop_action("kicked %s out of %s>.", the_user, quickroom.name);

    cprintf("\n\1f\1rYou now have to write a %s \1w(\1ryell\1w)\1r to the %s%s\1a\n", config.message, ADMINCOL, config.admin);
    cprintf("\1f\1rabout the kickout: who, why, where, for how long.%s",		      "\n\n(Kickout Notification)\1a\n");

    not_done = enter_message(YELL_FORUM, EDIT_NORMAL, FORCED_BANNER, NULL);

    while (not_done) {
	cprintf("\n\1rYou're required to enter this yell.\1a\n");
        not_done = enter_message(YELL_FORUM, EDIT_NORMAL, FORCED_BANNER, NULL);
    }
    xfree(userP);
    return;
    
}

/* zap all quadrants */

void
zap_all()
{
    int i;
    room_t room;

    cprintf("\1f\1gAre you \1rsure \1gyou want to zap (forget) \1rALL\1g %s? (y/N) \1c ", config.forum_pl);
    if (yesno_default(NO) == NO)
	return;
    

    for (i = 0; i < MAXQUADS; i++) {
	if (i == 1)
	    continue;		/* mail */
	room = readquad(i);
	if (i_may_read_forum(i) && !(room.flags & QR_NOZAP)) {
	    usersupp->forget[i] = room.generation;
	    usersupp->generation[i] = -1;
	}
    }
    cprintf("\1g\1fZapped all quadrants.\n");
    writeuser(usersupp, 0);
    return;
}

/*************************************************
* forget() / zap()
* forget current room 
*************************************************/

void
forget()
{
    if (quickroom.flags & QR_NOZAP) {
	cprintf("\1f\1rCan not zap (forget) this %s.\1a\n", config.forum);
	return;
    }
    cprintf("\1f\1gAre you \1rsure \1gyou want to zap (forget) this %s?\1c ", config.forum);
    if (yesno() == NO)
	return;

    /* the actual zapping: 
     * backup the old generation number, so we get noticed if the room
     * changes to something completely new 
     */

    usersupp->forget[curr_rm] = quickroom.generation;
    usersupp->generation[curr_rm] = (-1);
    writeuser(usersupp, 0);
    gotonext();
}

/*
 * killroom()
 * <a>dmin <r>oom <z>ap command: delete the current room
 */

void
killroom()
{

    int i;
    char temprmname[50];

    quickroom = readquad(curr_rm);

    if ((curr_rm < 11) && (usersupp->priv < PRIV_WIZARD)) {
	cprintf("\1gOnly an " WIZARDTITLE "\1g can delete this %s.\n", config.forum);
	return;
    }
    cprintf("\n\1f\1rAre you \1w!SURE!\1r you want to Delete this %s??? (y/n)", config.forum);
    if (yesno() == NO)
	return;

    quickroom.flags = 0;
    strcpy(temprmname, quickroom.name);
    strcpy(quickroom.name, "");
    if (quickroom.flags & QR_INUSE)
	quickroom.flags ^= QR_INUSE;

    cprintf("\1rClear \1w(\1rerase\1w)\1r all messages? (recommended) (y/n) ");
    if (yesno() == YES) {
	erase_all_messages(quickroom.highest);
	quickroom.highest = quickroom.lowest = 0;
    }

    for (i = 0; i < NO_OF_QLS; i++)
	strcpy(quickroom.qls[i], "");

    cprintf("\n\n\1f\1wResetting quickroom... \1a");
    write_quad(quickroom, curr_rm);

    log_sysop_action("deleted %s: %s>", config.forum, temprmname);

    _sql_qc_zero_forum_categories(curr_rm);

    curr_rm = 0;
    gotocurr();
}

/*
 * create_room()
 */

void
create_room()
{
    int number;
    char command, *quad_name;

    for (number = 0; number < MAXQUADS; number++)
	if (!(readquad(number).flags & QR_INUSE)) {
	    cprintf("%s%d%s%s",
		    "\n\1f\1gFound unused quadrant at quad #\1w",
		     number,
	            "\n\1rStop\1g searching for an unused quad?",
		    " \1w(\1ry\1w/\1rn\1w): \1c");
	    if (yesno() == YES) {
		cprintf("%s%d%s",
			"\n\1gInstall at quad number ",
			number, 
			"? \1w(\1ry\1w/\1rn\1w): \1c");
		if (yesno() == NO)
		    return;
		else
		    break;
	    } else
		continue;
	}
    if (number >= MAXQUADS) {
	cprintf("\1rNo unused %s was found.\n", config.forum);
	return;
    }
    cprintf("%s%s%s",
	"\1f\1rPlease keep the length of the name under 36 characters.\n",
    	"                      |------------------------------------|\n",
        "\1f\1gName for new quadrant\1w: \1c");
    quad_name = get_name(3);

    if (strlen(quad_name) == 0)
	return;
    if (get_room_number(quad_name) != -1) {
	cprintf("\1r'%s' already exists.\n", quad_name);
	return;
    }
    curr_rm = number;
    quickroom = readquad(curr_rm);

    strcpy(quickroom.name, quad_name);
    strcpy(quickroom.qls[0], "Sysop");
    quickroom.generation++;
    quickroom.lowest = ++quickroom.highest;
    quickroom.maxmsg = 100;
    quickroom.flags = QR_INUSE;

    cprintf("\1f\1g%s%s ",
	    config.forum,
	    "Type: \1w<\1y1\1w> \1gPublic \1w<\1y2\1w> \1rPrivate : \1c");

    command = get_single("12");

    if (command == '1') {	/* public room */
	cprintf("\1rThe %s will be PUBLIC.\n\n", config.forum);
	quickroom.flags &= ~QR_PRIVATE;
    } else {	/* invite -- just private flag, guessname off */
	cprintf("\1rThe %s will be INVITE ONLY.\n\n", config.forum);
	quickroom.flags |= QR_PRIVATE;
	if (quickroom.generation >= 100)
	    quickroom.generation = 10;
    }

    write_quad(quickroom, curr_rm);
    gotocurr();	  /* takes care of self invite.. no matter, sysop anyways */
    log_sysop_action("created %s: %s> ", config.forum, quickroom.name);

    editroom();
    change_forum_info();
    qc_edit_room();
}

void
erase_all_messages(long highest)
{
    long i;
    char filename[L_FILENAME + 1];

    cprintf("\n\1f\1rErasing\1w");

    for (i = 0; i <= highest; i++) {
	message_header_filename(filename, curr_rm, i);
	if (fexists(filename)) {
	    unlink(filename);
	    message_filename(filename, curr_rm, i);
	    if (fexists(filename))
		unlink(filename);
	}
	if (!(i % 10))		/* show progress.. */
	    cprintf(".");
    }
    cprintf("\n\1rdone.\1a");
}

/*************************************************
 * gotocurr()
 * goto room currently in global variable curr_rm
*************************************************/

void
gotocurr()
{
    static int prev_rm = -1;

    quickroom = readquad(curr_rm);

    if (prev_rm != curr_rm)
	mono_change_online(who_am_i(NULL), quickroom.name, 5);
    prev_rm = curr_rm;

    return;
}

/*************************************************
* whoknows()
*************************************************/

void
whoknows()
{
    char fname[70];

    sprintf(fname, FORUMDIR "/%d/whoknows", curr_rm);
    cprintf("\n");
    more(fname, 1);
    cprintf("\n");
}


/*
 * show roomaides with slot numbers for roomediting
 */
void
show_qls()
{
    int i;
    for (i = 0; i < NO_OF_QLS; i++) {
	cprintf("  \1f\1w[\1r%d\1w]\1r %s\1a\n", i + 1, quickroom.qls[i]);
    }
}

/*************************************************
* print_type()
*************************************************/

void
print_type()
{
    quickroom = readquad(curr_rm);

    cprintf("\1f\1y    %s\1g, %s #%d.", quickroom.name, config.forum, curr_rm);
    cprintf("\n\n\1g\1f");
    cprintf("    Current highest %s number: \1c%ld\1g.\n",
		config.message, quickroom.highest);
    cprintf("    Current lowest %s number: \1c%ld\1g.\n",
		config.message, quickroom.lowest);
    cprintf("    Maximum number of %s: \1c%d\1g.\n",
		config.message_pl, quickroom.maxmsg);
    show_room_flags();
    cprintf("    ");
    show_room_aides();
}

/*************************************************
* show_room_flags()
*************************************************/

void
show_room_flags()
{
    cprintf("\1f\1g    This is currently a");

    if ((quickroom.flags & QR_PRIVATE) == 0)
	cprintf(" \1gpublic");

    if (quickroom.flags & QR_PRIVATE)
	cprintf(" \1rprivate");

    cprintf(" \1g%s.\n", config.forum);

    if (quickroom.flags & QR_READONLY)
	cprintf("  \1rRead only %s.\n", config.forum);

    cprintf("  \1g  Subjectlines are ");
    if ((quickroom.flags & QR_SUBJECTLINE) == 0)
	cprintf("\1rnot ");
    cprintf("\1gallowed.\n");

    cprintf("  \1g  %s described status ", config.forum);
    if ((quickroom.flags & QR_DESCRIBED) == 0)
	cprintf("\1rnot ");
    cprintf("\1gset.\n\1a");

    if (quickroom.flags & QR_ANONONLY)
	cprintf("\1f\1p    Anonymous-only %s.\1a\n", config.forum);
    if (quickroom.flags & QR_ANON2)
	cprintf("\1f\1p    Anonymous-option %s.\1a\n", config.forum);
}

void
editroom()
{
    char command;
    int done;

    /* don't mung Lobby>, Mail> or Yells> if you're 'only' Sysop */
    if ((curr_rm <= 5) && (usersupp->priv < PRIV_WIZARD)) {
	cprintf("\1f\1rNope. Only %ss can edit this %s.\1a\n",
		config.wizard, config.forum);
	return;
    }
    need_rewrite = done = FALSE;
    quickroom = readquad(curr_rm);	/* read in current settings */

    while (!done) {
	/* display room information */
	cprintf("\1w\1f------------------------------------------------------------------\n");
	cprintf("\1g%s #%d\1w: \1g%-40.40s\n", config.forum, curr_rm, quickroom.name);

	cprintf("\n\1w[\1r1\1w] \1gEdit %s Name\1w : \1%c%s\1g.", config.forum,
		(quickroom.flags & QR_PRIVATE) ? 'r' :
		(quickroom.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'g',
		quickroom.name);

	cprintf("\n\1w[\1r2\1w] \1gEdit %s(s)", config.roomaide);
	cprintf("\n\1w[\1r3\1w] \1gEdit %s Type\1w: ", config.forum);
	cprintf("%s", (quickroom.flags & QR_PRIVATE) ? "\1rPRIVATE " : "\1gPUBLIC ");
	cprintf("\1g %s.", config.forum);

	cprintf("\n\1w[\1r4\1w] \1gEdit Message Type\1w: ");
	cprintf("%s %s", (quickroom.flags & QR_ANONONLY) ? "\1bANONYMOUS ONLY" :
	((quickroom.flags & QR_ANON2) ? "\1bANONYMOUS OPTION" : "\1gNORMAL"),
	    (quickroom.flags & QR_ALIASNAME) ? "\1w(\1bALIASNAME\1w)" : "");

	cprintf("\n\1w[\1r5\1w] \1gSubjectLines \1w: ");
	cprintf("%s", (quickroom.flags & QR_SUBJECTLINE) ? "\1gYEP." : "\1rNOOOOOOOOO!");

	cprintf("\n\1w[\1r6\1w] \1gDescribed    \1w: ");
	cprintf("%s", (quickroom.flags & QR_DESCRIBED) ? "\1gYEP." : "\1rNOOOOOOOOO!");

	IFSYSOP {
	    cprintf("\n\1r[\1w7\1r] \1gReadOnly     \1w: ");
	    cprintf("%s", (quickroom.flags & QR_READONLY) ? "\1gYEP." : "\1rNOOOOOOOOO!");

	    cprintf("\n\1r[\1w8\1r] \1gNoZap        \1w: ");
	    cprintf("%s", (quickroom.flags & QR_NOZAP) ? "\1gYEP." : "\1rNOOOOOOOOO!");

	    cprintf("\n\1r[\1w9\1r] \1gIN-USE       \1w: ");
	    cprintf("%s", (quickroom.flags & QR_INUSE) ? "\1gYEP." : "\1rNOOOOOOOOO!");

	    cprintf("\n\1r[\1wA\1r] \1gMaximum number of %s\1w: %d.",
		    config.message_pl, quickroom.maxmsg);
	    cprintf("\n\1r[\1wB\1r] \1gHighest %s number\1w: %ld.",
		    config.message, quickroom.highest);
	    cprintf("\n\1r[\1wC\1r] \1gLowest %s number \1w: %ld.",
		    config.message, quickroom.lowest);
	    cprintf("\n\1r[\1wD\1r] \1g%s category \1w: \1y%s.", config.forum, quickroom.category);

	    cprintf("\n\n\1wDon't fiddle with A, B, and C unless you KNOW what you're doing!\n");
	}

	cprintf("\1rField to change \1w:\n");
	cprintf("\1w<\1renter\1w>\1g to exit. ");
	command = get_single("123456789ABCD\n\r");

	if (command == '\n' || command == '\r')
	    done = TRUE;
	else
	    edit_room_field(&quickroom, curr_rm, command );
    }				/* end while */

    if (need_rewrite == TRUE) {
	cprintf("\1rSave changes \1w(\1ry/n\1w)\1r? ");
	if (yesno() == NO) {
	    /* reload old quickroom and return */
	    quickroom = readquad(curr_rm);
	} else {
	    write_quad(quickroom, curr_rm);
	    log_sysop_action("roomedited %s>", quickroom.name);
	}
    }
}

/*
 * edit_room_field()
 *
 * edit roomfields via the <a><r><e> menu
 */

void
edit_room_field(room_t * QRedit, unsigned int forum_id, int fieldnum )
{
    int loop, slot, i, j = 0;
    char *quad_name, cmd, name[L_USERNAME + 1];

    switch (fieldnum) {

	case '1':
	    cprintf("%s%s%s%s",
		"\1rPlease keep length of name to under 36 characters.\n",
	        "\1w                        ",
		"|------------------------------------|\n",
	    	"\1rEnter new quadrant name\1w: \1c");
	    quad_name = get_name(3);
	    if (strlen(quad_name) < 1) {
		cprintf("\1gOk, name not changed.\n");
		break;
	    }
	    if (get_room_number(quad_name) != -1) {
		cprintf("\1rThat name is already taken, sorry.\n");
	    } else {
		log_sysop_action("changed %ss name into %s.",
				QRedit->name, quad_name);
		strcpy(QRedit->name, quad_name);
                mono_sql_f_rename_forum( forum_id, quad_name );
		need_rewrite = TRUE;
	    }
	    break;

/*--------------------------------------------------------------------*/

	case '2':
	    IFSYSOP {
		show_qls();
	    }
	    else {
		cprintf("\1rYou're not allowed to do that.\n");
		break;
	    }

	    cprintf("\n\1f\1gSlot to edit \1w(<\1rEnter\1w> \1gto quit\1w): ");
	    cmd = get_single("12345 \rq\n");

	    if (cmd == '\r' || cmd == '\n' || cmd == ' ' || cmd == 'q')
		break;

	    slot = cmd - '1';	/* coz ql[0] is slot 1 ... duh */
	    cprintf("\1gUsername\1w: \1c");
	    strcpy(name, get_name(2));

	/* empty field for QL[slot] -> remove QL */

	    if (strlen(name) == 0) {
		if (check_user(QRedit->qls[slot]) == FALSE)
		    cprintf("\1rRemoving non-existant user as QL.\n");
		else if (change_QL(curr_rm, QRedit->qls[slot], -1) == -1) {
		    cprintf("\1rCould not remove %s as QL.\n", 
				QRedit->qls[slot]);
		    break;
		}
		strcpy(QRedit->qls[slot], "");
		need_rewrite = TRUE;
		break;
	    }
	    if (check_user(name) == FALSE) {
		cprintf("\1rNo such user.\n");
		break;
	    }
	    if (change_QL(curr_rm, name, 1) == -1) {
		cprintf("\1rCould not set %s as QL in #%d.\n", 
			name, curr_rm);
		break;
	    }
	    strcpy(QRedit->qls[slot], name);
	    show_qls();
	    need_rewrite = TRUE;
	    break;

/*-------------------------------------------------------------------*/

	case '3':		/* room type */

	    cprintf("\1g%s Type: \1w<\1r1\1w> \1gPublic \1w<\1r2\1w> \1rPrivate \1w: \1c", config.forum);
	    loop = get_single("12");

	    if (loop == '1') {
		cprintf("\1g%s set to PUBLIC.\n\n", config.forum);
		QRedit->flags &= ~QR_PRIVATE;
	    } else if (loop == '2') {
		cprintf("\1r%s set to INVITE ONLY.\n\n", config.forum);
		QRedit->flags |= QR_PRIVATE;
		cprintf("Kick out all users? ");
		if (yesno() == YES) {
		    QRedit->generation++;
		    if (QRedit->generation == 100)
			QRedit->generation = 10;
		    if (invite(who_am_i(NULL), curr_rm) == -1)
			cprintf("Error inviting yourself.\n");
		}
	    }
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '4':
	    cprintf("\1gPostType: \1w<\1y1\1w> \1gNormal \1w<\1y2\1w>\1pAnon-only \1w<\1y3\1w>\1pAnon-option \1w: \1c");

	    loop = get_single("123");

	    if (loop == '1') {	/* normal room, remove flags */
		QRedit->flags &= ~QR_ANONONLY;
		QRedit->flags &= ~QR_ANON2;
	    }
	    if (loop == '2') {	/* anononly room, set & unset proper flags */
		QRedit->flags |= QR_ANONONLY;
		QRedit->flags &= ~QR_ANON2;
	    }
	    if (loop == '3') {	/* anonopt room */
		QRedit->flags |= QR_ANON2;
		QRedit->flags &= ~QR_ANONONLY;
	    }
	    if (QRedit->flags & (QR_ANONONLY | QR_ANON2)) {
		cprintf("\1gShould the users be able to use Aliasnames? (y/n) ");
		if (yesno() == YES)
		    QRedit->flags |= QR_ALIASNAME;
		else
		    QRedit->flags &= ~QR_ALIASNAME;
	    } else
		QRedit->flags &= ~QR_ALIASNAME;

	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '5':
	    QRedit->flags ^= QR_SUBJECTLINE;
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '6':
	    QRedit->flags ^= QR_DESCRIBED;
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '7':
	    QRedit->flags ^= QR_READONLY;
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '8':
	    QRedit->flags ^= QR_NOZAP;
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case '9':
	    QRedit->flags ^= QR_INUSE;
	    need_rewrite = TRUE;
	    break;

/*--------------------------------------------------------------------*/

	case 'a':
	case 'A':
	    cprintf("\1rEnter the new maximum number of %s\1w(\1r%d\1w): \1c",
			config.message_pl, QRedit->maxmsg);
	    j = qc_get_pos_int('\0', 4);
	    if (j < 10) {
		cprintf("\1rDon't be daft! 10 %s is the minimum.\n",
			config.message_pl);
		break;
	    } else if (j > 1000) {
		cprintf("\1rDon't be daft! 1000 %s is the maximum.\n",
			config.message_pl);
		break;
	    } else {
		QRedit->maxmsg = j;
		need_rewrite = TRUE;
		break;
	    }

/*--------------------------------------------------------------------*/
	case 'b':
	case 'B':
	    cprintf("\1rEnter the new highest %s\1w: \1c", config.message);
	    j = qc_get_pos_int('\0', 7);
	    cprintf("\1f\1gAre you \1rSURE\1g you want to set the highest for `\1y%s\1g` to \1w%ld\1g? \1c", QRedit->name, j);
	    if (yesno() == YES) {
		QRedit->highest = j;
		cprintf("\1gIf you say so... done.\n");
		need_rewrite = TRUE;
	    }
	    break;
/*--------------------------------------------------------------------*/
	case 'c':
	case 'C':
	    cprintf("\1rEnter the new lowest %s\1w: \1c", config.message);
	    j = qc_get_pos_int('\0', 7);
	    cprintf("\1f\1gAre you \1rSURE\1g you want to set the lowest for `\1y%s\1g` to \1w%ld\1g? \1c", QRedit->name, j);
	    if (yesno() == YES) {
		QRedit->lowest = j;
		cprintf("\1gIf you say so... done.\n");
		need_rewrite = TRUE;
	    }
	    break;
/*--------------------------------------------------------------------*/
	case 'd':
	case 'D':

	    for (i = 0; i < CATEGORIES; i++)
		cprintf("\1w<\1r%2d\1w>\1g %s\n", i, fish[i]);

	    cprintf("\1gEnter the new category number\1w: \1c");

	    if ((i = qc_get_pos_int('\0', 2)) < 0)
		break;
	    else {
		if (i >= CATEGORIES) {
		    cprintf("\n\1r+++ OUT OF CHEESE ERROR - REDO FROM START +++\n\n");
		    break;
		} else {
		    strcpy(QRedit->category, fish[i]);
		    need_rewrite = TRUE;
		    break;
		}
		cprintf("\1r\1nUm, what?\n\n");
	    }
	    break;
/*--------------------------------------------------------------------*/

	default:
	    break;

    }				/* end switch */
}


/*************************************************
*
* look_into_quickroom()
*
* degree: 1 -> Sysop-look: show everything
*	  2 -> RA-look:    show just a little
*
*************************************************/

void
look_into_quickroom(int degree)
{
    int round, total = 0, blank, a, z;
    room_t scratch;
    user_t *tmpuser;
    time_t curtime;

    time(&curtime);

    curr_line = 5;
    line_count = 0;
    line_total = MAXQUADS;


    cprintf("\n\1f\1w Types: [\1pA\1w]nonymous, anonymous[\1pO\1w]ption, [\1rI\1w]nviteonly, [\1rP\1w]assworded,\n        [\1rG\1w]uessname, [\1bR\1w]eadonly, [\1cS\1w]ubjectline, [\1bN\1w]o roominfo.\n\n\1w\1f## \1yRoomName                       \1rPosts \1gType  \1cRoomAide       ");

    if (degree == 1)
	cprintf("\1gAbsence");

    cprintf("\n\1w----------------------------------------------------------------------------\n");

    for (round = 0; round < MAXQUADS; round++) {

	scratch = readquad(round);

	if ((scratch.highest >= 0) && (scratch.flags & QR_INUSE) &&
	    (i_may_read_forum(round) || degree == 1)) {
	    blank = 0;
	    cprintf("\1w%3d \1y%-30.30s \1r%6ld ", round, scratch.name,
		    scratch.highest);
	    total = total + scratch.highest;

	    if (scratch.flags & QR_PRIVATE)
		cprintf("\1rI");
	    else
		blank++;

	    if (scratch.flags & QR_READONLY)
		cprintf("\1bR");
	    else
		blank++;

	    if (scratch.flags & QR_SUBJECTLINE)
		cprintf("\1cS");
	    else
		blank++;

	    if ((scratch.flags & QR_DESCRIBED) == 0)
		cprintf("\1bN");
	    else
		blank++;

	    if (scratch.flags & QR_ANONONLY)
		cprintf("\1pA");
	    else if (scratch.flags & QR_ANON2)
		cprintf("\1pO");
	    else
		blank++;

	    for (a = 0; a < blank; a++)
		putchar(' ');

	    for (z = 0; z < NO_OF_QLS; z++) {
		if (strlen(scratch.qls[z]) > 0)
		    cprintf("\1c %-20s \1g", scratch.qls[z]);

		if (degree == 1) {
/*                  if (check_user(scratch.qls[z]) != 1)
 * cprintf("\1r[ABSENT]"); */
		} else {
		    tmpuser = readuser(scratch.qls[z]);
		    cprintf("%2ld days", ((curtime - tmpuser->laston_to) / 60 / 60 / 24));
		    xfree(tmpuser);
		}
	    }
	    cprintf("\n");
	    if (increment(0) == 1)
		break;
	}
    }

    if (degree == 1)
	cprintf("\nA total number of %d %s have been posted to %s.\n", total, config.message_pl, config.bbsname);

}

/*************************************************
*
* reset_room()
*
* this function resets a room (from qrfixer.c)
*
*************************************************/

void
reset_room()
{
    int roomnum;
    char *quad_name;
    room_t scratch;

    memset(&scratch, 0, sizeof(room_t));

    cprintf("Number of room to reset: ");
    quad_name = get_name(3);

    if (strlen(quad_name) < 1)
	return;

    roomnum = atoi(quad_name);

    scratch = readquad(roomnum);

    scratch.highest = 0;
    cprintf("Are you really sure you want to reset %s>?\n", scratch.name);
    if (yesno() == YES) {
	write_quad(scratch, roomnum);
	log_sysop_action("reset %s %d.%s>", config.forum, roomnum, scratch.name);
    } else {
	cprintf("Ok then. No resetting done this time.\n\n");
    }
    return;
}

void
move_rooms()
{
    cprintf("\nThis function has been Disabled.. makes too big a mess.\n");
    return;
}


/**************************************************************************
* gotonext()  :  handles both ways of goto-ing a quad with unreads.  the 
* first is a wrapper for unread_room().  if there are no "normally" unread
* quads, checks for skipped quads, and calls goto_next_skipped().
**************************************************************************/

void
gotonext()
{
    int i, old_rm, num_skipped = 0;

    /* goto next quad that's !skipped, !zapped, and can_read */
    old_rm = curr_rm;
    i = unread_room();
    curr_rm = i;
    gotocurr();

    if (old_rm != curr_rm) {	/* new messages? */
	if (curr_rm == 0)
	    if (usersupp->lastseen[0] != readquad(0).highest) {
		usergoto(curr_rm, old_rm, 1);	/* new docking bay post */
		return;
	    } else {		/* don't "pass through" docking bay or collect $200 */
		gotonext();
		return;
	} else {		/* new messages */
	    usergoto(curr_rm, old_rm, 1);
	    return;
	}
    } 

    if (last_skipped_rm > -1)	/* only count if we've skipped something */
	for (i = 0; i < MAXQUADS; i++)
	    if (skipping[i] == 1)
		num_skipped++;
    if (num_skipped == 0) {
	random_goto();
	return;
    } else {
	curr_rm = goto_next_skipped(num_skipped);
	cprintf("\1f\1r\n\n***\1yYou have \1g%d \1yskipped %s left\1r***\n", num_skipped, num_skipped != 1 ? config.forum_pl : config.forum);
    }
    usergoto(curr_rm, old_rm, 1);
    return;
}

/**********************************************************************
 * int goto_next_skipped returns the quad number of the next (desired)
 * skipped quad.  Handles resetting of global int last_skipped_quad
 *********************************************************************/

int
goto_next_skipped(const int num_skipped)
{
    int i, skip_ctr = 0;

    for (i = 0; i < MAXQUADS; i++)
	if (skipping[i] == 1) {
	    if (num_skipped == 1) {
		last_skipped_rm = -1; /* reset skip marker to "none skipped" */
		return i;
	    }
	    if ((i <= last_skipped_rm) && (++skip_ctr < num_skipped))
		continue;
	    if ((skip_ctr < num_skipped) || (i != last_skipped_rm))
		break;
	    else {		/* "wrap" loop */
		i = skip_ctr = last_skipped_rm = 0;  /* reset loop & counter */
		continue;	/* stuff, loop will then stop at lowest skipped */
	    }
	}
    last_skipped_rm = i;
    return i;
}

/*******************************************************************/
/* returns:
 *           -1: not a room
 *           -2: no input
 *  0..MAXQUADS: room number 
 */

int
get_room_name(const char *quad_name)
{
    int i;
    room_t scratch;

    nox = 1;
    if (strlen(quad_name) == 0)
	return -2;

    if (atoi(quad_name)) {
	i = atoi(quad_name);
	if (i < 0 || i >= MAXQUADS)
	    return -1;
	scratch = readquad(i);
	if (i_may_read_forum(i))
	    return i;
    }
    for (i = 0; i < MAXQUADS; i++) {
	scratch = readquad(i);
	if (strstr(scratch.name, quad_name) != NULL) {
	    if (i_may_read_forum(i)) {
		return i;
	    }
	}
    }
    return -1;
}

/* this is the jump function */
int
jump(int destructive_jump)
{
    int i;
    char *quad_name;

    cmdflags &= ~C_ROOMLOCK;

    quad_name = get_name(3);
    i = get_room_name(quad_name);

    if (i == -2)
	return 0;
    else if (i == -1) {
	cprintf("\1f\1rNo %s called `\1y%s\1r'.\1a\n", config.forum, quad_name);
	return 0;
    }
    usergoto(i, curr_rm, destructive_jump);

    return 1;
}

/*************************************************
* usergoto()
*************************************************/

void
usergoto(int new_rm, int old_rm, int destructive_jump)
{				/* used by <G>oto and <j>ump - actually does the work */
    room_t here;

    curr_rm = new_rm;
    here = readquad(curr_rm);
    gotocurr();

    if (old_rm != curr_rm) {
	if (here.highest <= usersupp->lastseen[curr_rm]) {
	    skipping[curr_rm] = 0;
	    usersupp->lastseen[curr_rm] = here.highest;
	    display_short_prompt();
	    cprintf("\1f\1w(\1g%ld %s\1w)\1a\n", here.highest - here.lowest, config.message_pl);
	} else {
	    if (usersupp->lastseen[curr_rm] < here.lowest)
		usersupp->lastseen[curr_rm] = here.lowest;
	    if (curr_rm && !skipping[curr_rm])	/* looks ugly at Docking Bay, don't bother */
		cprintf("\1f\1gRead next %s with unread %s.\n", config.forum, config.message_pl);
	    display_short_prompt();
	    if (skipping[curr_rm]) {
		cprintf("\1f\1w (\1gpreviously skipped\1w)");
		skipping[curr_rm] = 0;
	    }
	    cprintf("\n\1f\1w(\1g%ld %s, %ld unread\1w)\1g  Read new %s.\1a\n", here.highest - here.lowest, config.message_pl, here.highest - usersupp->lastseen[curr_rm], config.message_pl);
	}
	if (new_quadinfo()) {
	    cprintf("\1f\1b*** \1gThe %s\1w<\1rI\1w>\1gnfo has changed\1b ***\1a\n", config.forum);
	    writeuser(usersupp, 0);
	}
    }
    if ((skipping[old_rm] != 1) && (destructive_jump))
	mark_as_read(old_rm);

    usersupp->forget[curr_rm] = (-1);

    if (usersupp->generation[curr_rm] != quickroom.generation) {
	/* print the desc if the room has regenerated. */
	display_message(curr_rm, 0, DISPLAY_INFO);
	usersupp->generation[curr_rm] = quickroom.generation;
    }
    unseen = here.maxmsg - (here.highest - usersupp->lastseen[curr_rm]);

    if (here.highest == usersupp->lastseen[curr_rm])
	skipping[curr_rm] = 0;
    return;

}

/* this skips the current room */
void
skiproom()
{
    if (curr_rm > 1) {		/* no skipping in Docking Bay or Mail */
	if (usersupp->lastseen[curr_rm] < readquad(curr_rm).highest) {
	    last_skipped_rm = curr_rm;
	    skipping[curr_rm] = 1;
	}
	gotonext();
    } else if (curr_rm == 1)
	cprintf("\n\1f\1cSorry, can't skip Mail.\n\1a");	/* just in case. */
    return;
}

void
room_debug_info()
{
    room_t lizard;

    lizard = readquad(curr_rm);
    cprintf("Room debug info for: %s.\n", lizard.name);
    cprintf("Highest msg:\t%ld.\n", lizard.highest);
    cprintf("Lowest msg:\t%ld.\n", lizard.lowest);
    cprintf("Maximum: %d.   Current: %ld.\n", lizard.maxmsg, lizard.highest - lizard.lowest);
    cprintf("Gen: %c.   Flags: %d.   Rinfo: %d.\n", lizard.generation, lizard.flags, lizard.roominfo);
    cprintf("lastseen: %ld\n", usersupp->lastseen[curr_rm]);
}

/*************************************************
* check_generation()
*
* A room's generation number changes each time it
* is recycled. Users are kept out of private rooms
* or forgot rooms by matching the generation
* numbers. To avoid an accidental matchup,
* unmatched numbers are set to -1 here.
*
*************************************************/ void
check_generation()
{
    int i;

    for (i = 0; i < MAXQUADS; i++) {
	quickroom = readquad(i);
	if (usersupp->generation[i] != (-5))
	    if (usersupp->generation[i] != quickroom.generation)
		usersupp->generation[i] = (-1);
	if (usersupp->forget[i] != quickroom.generation)
	    usersupp->forget[i] = (-1);
    }
    return;
}

/*************************************************
* check_messages()
*************************************************/

int
check_messages(room_t room, int which)
{
    int i = room.highest - usersupp->lastseen[which];
    return (i < 0) ? 0 : i;
}

int
is_zapped(int room, const room_t *strukt )
{
    return (usersupp->forget[room] == strukt->generation);
}

/*
 * mark_as_read()
 *
 * Used to mark all messages in room as read. Now it marks the messages
 * up to and including lastseen as read. --  caf.
 */

void
mark_as_read(int room)
{
    usersupp->lastseen[room] = readquad(room).highest;
    return;
}

/* 
 * leave_n_unread_posts()
 *
 * marks all but n_unread messages in a quad as read
 */

void
leave_n_unread_posts(int room, int n_unread)
{
    if (readquad(room).highest - n_unread > readquad(room).lowest)
	usersupp->lastseen[room] = (readquad(room).highest - n_unread);
    return;
}


void
print_userlist_list(userlist_t * p)
{
    unsigned int j = 0;
    char line[100], *q;

    q = (char *) xmalloc(100 * 100);
    strcpy(q, "");

    sprintf(q, "\n\1g\1f");
    while (p) {
	sprintf(line, " \1g%-20s", p->name);
	p = p->next;
	if ((j++ % 3) == 2)
	    strcat(line, "\n");
	strcat(q, line);
    }
    strcat(q, "\n");
    if (j % 3 != 0)
	strcat(q, "\n");

    more_string(q);
    xfree(q);
    return;
}

void
print_forumlist_list(forumlist_t *p)
{
    char line[100], *q;

    q = (char *) xmalloc(100 * 100);
    strcpy(q, "");

    sprintf(q, "\1g\1f");
    while (p) {
	sprintf(line, "\1w%3u.\1g%s\n", p->forum_id, p->name);
	p = p->next;
        strcat(q, line);
    }

    more_string(q);
    xfree(q);
    return;
}

int
print_hosts_simple( unsigned int forum_id )
{
    userlist_t *p;
    int ret;

    ret = mono_sql_uf_list_hosts_by_forum(forum_id, &p);
    
    if ( ret == -1 ) {
       cprintf( "Error, could not get list of hosts.\n" );
       return -1;
    }

    cprintf( "\1g\1f" );

    while (p) {
        cprintf( "%s", p->name);
        if ( p->next ) cprintf( ", " );
	p = p->next;
    }

    cprintf( "\1n" );
    dest_userlist(p);
    return 0;
}

void
kickout_menu()
{
    userlist_t *p = NULL;

    cprintf("\1f\1rExperimental SQL KICKOUT menu.\1a\n");
    mono_sql_uf_list_kicked_by_forum(curr_rm, &p);
    print_userlist_list(p);
    dest_userlist(p);
    return;
}

void
invite_menu()
{
    userlist_t *p = NULL;

    cprintf("\1f\1rExperimental SQL INVITE menu.\1a\n");
    mono_sql_uf_list_invited_by_forum(curr_rm, &p);
    print_userlist_list(p);
    dest_userlist(p);
    return;
}

void
edithosts_menu()
{
    MENU_DECLARE;

    MENU_INIT;
    sprintf(the_menu_format.menu_title, "\n\n\1f\1w[\1gEdit hosts for Forum #%d\1w]\n\n", curr_rm);

    MENU_ADDITEM(menu_hostedit_add, 1, 0, "", "ti", "Add host", "a");
    MENU_ADDITEM(menu_hostedit_remove, 1, 0, "", "ti", "Remove host", "r");
    MENU_ADDITEM(menu_hostedit_list, 1, 0, "", "ti", "List hosts", "l");
    MENU_ADDITEM(menu_hostedit_kill, 1, 0, "", "ti", "Remove all hosts", "k");

        MENU_PROCESS_INTERNALS;

    for (;;) {
	MENU_DISPLAY(2);

	if (!MENU_EXEC_COMMAND)
	    break;

    }
	MENU_DESTROY;
    return;
}

void
menu_hostedit_add(const unsigned int ZZ, const long ZZZ, const char *ZZZZ)
{
    char *name;
    unsigned int id2;

    nox = 1;
    cprintf("\1f\1gEnter username\1w: \1c");

    name = get_name(1);

    if (strlen(name) == 0)
        return; 

    if (mono_sql_u_name2id(name, &id2) == -1) {
        cprintf("\1f\1rNo such user.\n");
        return; 
    }

    if (mono_sql_uf_is_host( id2, curr_rm ) == TRUE) {
        cprintf("\1f\1y%s \1gis already a host in #%d.\n", name, curr_rm );
        return; 
    }

    mono_sql_uf_add_host( id2, curr_rm );

    cprintf("\1f\1y%s \1ghas been added as a host to #%d.\n", name, curr_rm );

    return;
}

void
menu_hostedit_remove(const unsigned int ZZ, const long ZZZ, const char *ZZZZ)
{
    char *name;
    unsigned int id2;

    nox = 1;

    cprintf("\1f\1gEnter username\1w: \1c");
    name = get_name(5);

    if (strlen(name) == 0)
        return;

    mono_sql_u_name2id(name, &id2);

    if (mono_sql_uf_is_host( id2, curr_rm ) == FALSE) {
        cprintf("\1f\1y%s \1gis not a host in forum #%d.\n", name, curr_rm );
        return;
    }
    mono_sql_uf_remove_host( id2, curr_rm );
    cprintf("\1f\1y%s \1ghas been removed as a host from forum #%d.\n", name, curr_rm );
    return;

    return;
}

void
menu_hostedit_kill(const unsigned int ZZ, const long ZZZ, const char *ZZZZ)
{
    printf("Removing all hosts!\n");
    printf("Not implemented yet!\n");

/*    mono_sql_hosts_add( username,  */
    return;
}

void
menu_hostedit_list(const unsigned int ZZ, const long ZZZ, const char *ZZZZ)
{
    userlist_t *p = NULL;

    mono_sql_uf_list_hosts_by_forum(curr_rm, &p);
    print_userlist_list(p);
    dest_userlist(p);
    return;
}

void
show_room_aides()
{
    cprintf("\1f\1g%s\1w: ", config.roomaide);
    print_hosts_simple( curr_rm);
    cprintf( "\n" );
    cprintf("\1f\1g%s category\1w: \1y%s\1g.\n", config.forum, quickroom.category);
}

int
we_can_post(const unsigned int forum)
{
    /* priv check for guests and cursed handled in validate_read_command() */
    /* if we are in the docking bay, the user must be Sysop (or Technician) */
    if (forum == 0) {
	cprintf("\1f\1r\nOpen the door please, HAL.");
	fflush(stdout);
	sleep(1);
	if (usersupp->priv < PRIV_TECHNICIAN) {
	    cprintf("\n\1f\1rI'm sorry Dave, but I can't do that.\n");
	    return FALSE;
	} else {
	    cprintf("\n\1f\1rAre you sure you want me to open the \1yDocking Bay\1w>\1r doors, Dave? \1a");
	    if (yesno_default(FALSE) == FALSE) {
		cprintf("\1f\1r\nGood-bye, Dave.\n\1a");
		return FALSE;
	    }
	}
    }

    if (!i_may_write_forum(forum)) {
	cprintf("\n\1f\1gYou are not allowed to post here.\1a\n");
	return FALSE;
    }
    return TRUE;
}

/*************************************************
* unread_room()
* this functions finds a room with unread messages
* and it returns its number
*************************************************/
int
unread_room()
{

    int i;
    room_t scratch;

    for (i = 0; i < MAXQUADS; i++) {
        scratch = readquad(i);
        if (i_may_read_forum(i)
            && (skipping[i] == 0)
            && (!is_zapped(i, &scratch))) {
            if (check_messages(scratch, i) > 0) {
                return i;
            }
        }
    }
    return 0;
}


int
new_quadinfo()
{
    if ((quickroom.roominfo != usersupp->roominfo[curr_rm]) && (curr_rm != 2 || curr_rm != 5)) {
	usersupp->roominfo[curr_rm] = quickroom.roominfo;
	return TRUE;
    }
    return FALSE;
}
/* eof */
