/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>		/* also */
#include <string.h>		/* too */
#include <sys/file.h>		/* for flock */
#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>		/* added by kirth */
#include <time.h>
#include <ctype.h>
#include <dirent.h>

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

#ifdef CLIENTSRC
#include "telnet.h"
#endif

#define extern
#include "messages.h"
#undef extern

#include "enter_message.h"
#include "input.h"
#include "rooms.h"
#include "msg_file.h"

#include "sql_message.h"
#include "sql_userforum.h"
#include "sql_rating.h"

void
search()
{

#ifdef SEARCH_CODE_DONE

    char *p, string[25];

    cprintf("\1f\1g\nYou can use wildcards and exclusions such as \1r?\1g and \1r[^St]\1g in your search.\n");
    cprintf("\1f\1gMatches are displayed in \1yyellow\1g.\n");
    cprintf("\1f\1g\nEnter a string to search for\1w:\1a\1c ");
    getline(string, 24, 0);
    if (strlen(string) == 0) {
	cprintf("\1f\1rI'm not going to look for that, that's nothing!\n");
	return;
    }
    cprintf("\1f\1gSearching...\1a");
    fflush(stdout);
    p = search_msgbase(string, curr_rm, 0, usersupp );
    cprintf("\n");
    if (strlen(p) < 110)
	cprintf("\1f\1rNothing found...\n");
    else
	more_string(p);
    xfree(p);

#endif

    return;
}


void
newbie_mark_as_read(int number_to_leave_unread)
{
    int i;
    for (i = 2; i < MAXQUADS; i++)
	leave_n_unread_posts(i, number_to_leave_unread);

    cprintf("\n\n\1f\1gAll but the last %d messages", number_to_leave_unread);
    cprintf(" in each quadrant have been marked as read.\n\n");

    writeuser(usersupp, 0);
    return;
}

void
reset_lastseen_message(void)
{
    room_t frog;

    cprintf("\1f\n\1y%s \1g#%d, Lastseen %s %ld.\n", config.forum, curr_rm,
	    config.message, usersupp->lastseen[curr_rm]);
    cprintf("\1gDo you want to reset this value? ");

    if (yesno() == YES) {
	frog = read_quad(curr_rm);
	if (frog.highest < frog.maxmsg)
	    usersupp->lastseen[curr_rm] = 0;
	else
	    usersupp->lastseen[curr_rm] = frog.highest - frog.maxmsg;
	writeuser(usersupp, 0);
    }
    return;
}

/* wrapper for copy and move, copies if parameter 'copy' is true, otherwise moves */
int
copy_message_wrapper(const unsigned int current_post, const int is_not_my_post, const int copy)
{
    int to_quad, fail, count;
    char tempstr[100], to_name[L_USERNAME + 1];
    room_t scratch;

    scratch = read_quad(curr_rm);

    if (!copy && !(!is_not_my_post
		  || is_ql(who_am_i(NULL), scratch)
		  || usersupp->priv >= PRIV_SYSOP || curr_rm == MAIL_FORUM))
	return -1;

    cprintf("\1f\1g%s %s.\n", (copy) ? "Copy" : "Move", config.message);
    cprintf("Enter destination %s name/number: \1c", config.forum);
    strcpy(tempstr, get_name(3));
    to_quad = get_room_name(tempstr);

    if (to_quad < 0) {
	if (to_quad == -1)
	    cprintf("\1f\1rNo such %s.\1a\n", config.forum);
	return -1;
    }

    scratch = read_quad(to_quad);

    if (usersupp->priv >= PRIV_SYSOP) {
	if (to_quad == YELL_FORUM) {
	    cprintf("Can't copy/move to YELLS, it'll mung stuff..\n");
	    return -1;
	} else if (!i_may_write_forum(to_quad)) { /* no fc's in emps */
	 cprintf("You're not allowed to %s the %s there.\n",
		(copy) ? "copy" : "move", config.message);
	    return -1;
	}

    } else if (!i_may_write_forum(to_quad) 
	|| to_quad == YELL_FORUM || to_quad == MAIL_FORUM) {
	cprintf("You're not allowed to %s the %s there.\n",
		(copy) ? "copy" : "move", config.message);
	return -1;
    }

    if (curr_rm != MAIL_FORUM) {
        char filename[L_FILENAME + 1];
        message_header_t message;

	message_header_filename(filename, curr_rm, current_post);
	read_message_header(filename, &message);

	if (message.banner_type & ANON_BANNER &&
		usersupp->priv < PRIV_SYSOP) {
	    cprintf("\1gYou can't copy anonymous posts.\n");
	    return -1;
	}
    }

    cprintf("\1f\1g%s this %s to `\1y%s\1g'? \1w(\1gy\1w/\1gn\1w)\1c ",
	    (copy) ? "Copy" : "Move", config.message, scratch.name);
    if (yesno() == NO) 
	return 0;

    for (count = 0;; count++) {

	if (to_quad == MAIL_FORUM) {

	    cprintf("\n\1f\1yRecipient: \1c");
	    strcpy(to_name, get_name(2));
	    if (!strlen(to_name)) {
		if (!copy && count)
		    message_delete(curr_rm, current_post);
		break;
	    } else if (check_user(to_name) == FALSE) {
		cprintf("\nNo such user..");
		continue;
	    }
	    fail = message_copy(curr_rm, to_quad, current_post, 
		to_name, (copy) ? MOD_COPY : MOD_MOVE);

	} else 
    	    if (copy)
	        fail = message_copy(curr_rm, to_quad, current_post, "", MOD_COPY);
	    else
	        fail = message_move(curr_rm, to_quad, current_post, "");
	if (fail == 0)
	    cprintf("\n\1f\1gThe %s has been %s.\1a\n\n", 
		config.message, (copy) ? "copied" : "moved");
	else
	    cprintf("\1f\1rOops, couldn't %s the %s.\1a\n",
		    config.message, (copy) ? "copy" : "move");
	if (to_quad != MAIL_FORUM)
	    break;
    }

    return 0;
}

int
delete_message_wrapper(const unsigned int current_post, const int is_not_my_post)
{
    int tempint;

    if ((!is_not_my_post)
	|| (is_ql(who_am_i(NULL), read_quad(curr_rm)))
	|| (usersupp->priv >= PRIV_SYSOP)
	|| (curr_rm == MAIL_FORUM)) {

	cprintf("\1f\1rDelete.\1a\n");
	cprintf("\1f\1gAre you sure you want to delete this %s? \1w(\1gy\1w/\1gn\1w)\1a", config.message);
	if (yesno() == FALSE) 
	    return 0;
	
	if (curr_rm == MAIL_FORUM)
	    tempint = message_delete(curr_rm, current_post);
	else if (is_not_my_post) {
	    i_deleted_your_post_haha_mail(current_post);
	    tempint = message_move(curr_rm, GARBAGE_FORUM, current_post, "");
	} else
	    tempint = message_move(curr_rm, GARBAGE_FORUM, current_post, "");

	if (tempint == 0)
	    cprintf("\1f\1g%s deleted.\1a\n", config.message);
	else
	    cprintf("\1f\1rOops, %s could'nt be deleted.\1a\n", config.message);

    } else
	cprintf("\1f\1rI'm sorry Dave, but I can't do that..\1a\n");
    return 0;
}

void
lookup_anon_author(const unsigned int current_post)
{
    char filename[L_FILENAME + 1];
    room_t scratch;
    message_header_t message;

    scratch = read_quad(curr_rm);

    if ((usersupp->priv >= PRIV_SYSOP)
	|| is_ql(who_am_i(NULL), scratch)) {

	message_header_filename(filename, curr_rm, current_post);
	read_message_header(filename, &message);

	if (message.banner_type & ANON_BANNER) {
	    scratch = read_quad(curr_rm);
	    cprintf("\n\1f\1w[ \1gPost was by\1w: \1y%s\1w ]\1a\n", message.author);
	    log_sysop_action("looked at anonymous post #%d by %s in %s>",
		 current_post, message.author, scratch.name);
	} else
	    cprintf("\1f\1wNot an anonymous post.\1a\n");
    } else
	cprintf("\n");
    return;
}

void
x_message_to_mail(const char *x, char *to_user)
{
    FILE *fp;

    if ((check_user(to_user)) == FALSE)
	return;

    cprintf("\1f\1gSave %s %s to \1y%s\1g's Mail? \1w(\1gy\1w/\1gn\1w) \1c", 
		config.express, config.x_message, to_user);

    if (yesno() == NO)
	return;

    fp = xfopen(temp, "w", TRUE);

    if (fp != NULL) {
        fprintf(fp, "%s%s%s",
	    "\n\1f\1b*** \1gYou logged off while I was sending you this", 
	    " eXpress message \1b***\1a\1c\n\n",
            x );

	fclose(fp);
    }
    enter_message(MAIL_FORUM, EDIT_NOEDIT, X_2_MAIL_BANNER, to_user);
}

void message_clip(const char *header_string)
{
    FILE *fp;

    fp = xfopen(CLIPFILE, "a", TRUE);

    if (fp != NULL) {
	fprintf(fp, "%s", header_string);
	fclose(fp);
    }
}

void message_2_temp(const char *header_string, char mode)
{
    FILE* fp;

    if (mode == 'a')
        fp = xfopen(temp, "a", TRUE);
    else
	fp = xfopen(temp, "w", TRUE);

    if (fp != NULL) {
	fprintf(fp, "%s", header_string);
	fclose(fp);
    }
}

void
purge_mail_quad(void)
{
    int i, mail_total;
    char filename[L_FILENAME + 1];

    mail_total = count_mail_messages();

    if (mail_total == -1)
	return;

    if (mail_total > MAX_USER_MAILS &&
	usersupp->priv < PRIV_TECHNICIAN) {

	for (i = 0; mail_total - MAX_USER_MAILS > 0; i++) {
	    mail_header_filename(filename, who_am_i(NULL), i);
	    if (fexists(filename)) {
		unlink(filename);
		mail_total--;
	    } else {
		if (i > usersupp->mailnum)  /* uhh-ohh.. */
		    break;
		continue;
	    }
	    mail_filename(filename, who_am_i(NULL), i);
	    if (fexists(filename))
		unlink(filename);
	}
    }
    return;
}

int
count_mail_messages(void)
{
    int count;
    char mail_dir[L_FILENAME + 1], name[L_USERNAME + 1];

    strcpy(name, usersupp->username);
  
    if (check_user(name) == FALSE || EQ(name, "Guest"))
	return -1; 
    sprintf(mail_dir, "%s/mail/.", getuserdir(who_am_i(NULL)));

    count = count_dir_files(mail_dir);

    return (count > 0) ? count / 2 : -1;
}


#ifdef USE_RATING

void
rate_message(message_t *message)
{
    char buf_str[3];
    int score = 0;

    if( (mono_sql_rat_check_rating(usersupp->usernum, message->m_id, message->f_id )) == -1 ) {
        cprintf("\1f\1rYou have already rated this %s.\1a\n", config.message);
        return;
    }
    cprintf("\n\1f\1gRatings vary from \1y-9\1g for a horrible %s to \1y9\1g for an excellent %s\n", config.message, config.message);
    cprintf("\1f\1gThe overall score is based on the average of all ratings for this %s.\n\n", config.message);
    cprintf("\1f\1gYour rating for this %s \1w(\1g-9 thru 9\1w): \1c", config.message);

    strcpy(buf_str, "");
    getline(&buf_str, 3, TRUE);

    if( (sscanf(buf_str, "%d", &score)) != 1) {
        cprintf("\1f\1rSorry, but `\1y%s\1r' is not a valid entry.\1a\n", buf_str);
        return;
    } else if( (score < -9) || (score > 9) ) {
        cprintf("\1f\1rSorry, but `\1y%d\1r' is not a valid entry.\1a\n", score);
        return;
   }

    cprintf("\1f\1gSave a rating of \1y%d\1g for this %s? \1w(\1gy\1w/\1gn\1w) \1c", score, config.message);
    if( yesno() == NO )
        return;

    if( (mono_sql_rat_add_rating(usersupp->usernum, message->m_id, message->f_id, score )) == -1 )
        cprintf("\1f\1rAn internal SQL error occurred.\1a\n");
    else
        cprintf("\1f\1gScore saved.\1a\n");

    return;
}
#endif
