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
#include <sys/time.h>
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
#include "routines2.h"
#include "uadmin.h"

#include "sql_message.h"
#include "sql_userforum.h"
#include "sql_rating.h"
#include "sql_llist.h"
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
    p = search_msgbase(string, curr_rm, 0, usersupp);
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
reset_lastseen_message()
{
    cprintf("\1f\n\1y%s \1g#%d, Lastseen %s %ld.\n", config.forum, curr_rm,
	    config.message, usersupp->lastseen[curr_rm]);
    cprintf("\1gDo you want to reset this value? ");

    if (yesno() == YES) {
        room_t frog;

        read_forum( curr_rm, &frog );

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

    read_forum( curr_rm, &scratch );

    if (!copy && !(!is_not_my_post
		   || is_ql(who_am_i(NULL), scratch)
		   || usersupp->priv >= PRIV_SYSOP || curr_rm == MAIL_FORUM))
	return -1;

    if (copy && curr_rm == MAIL_FORUM && usersupp->priv < PRIV_SYSOP) {
	cprintf("\n\1f\1gCC (forward) this Mail %s? \1w(\1gy\1w/\1gn\1w) \1c",
		 config.message);
	to_quad = MAIL_FORUM;
    } else {
        cprintf("\1f\1g%s %s.\n", (copy) ? "Copy" : "Move", config.message);
        cprintf("Enter destination %s name/number: \1c", config.forum);
        strcpy(tempstr, get_name(3));
        to_quad = get_room_name(tempstr);

        if (to_quad < 0) {
	    if (to_quad == -1)
	        cprintf("\1f\1rNo such %s.\1a\n", config.forum);
	    return -1;
        }
    }

    read_forum( to_quad, &scratch );

    if (usersupp->priv >= PRIV_SYSOP) {
	if (to_quad == YELL_FORUM) {
	    cprintf("Can't copy/move to YELLS, it'll mung stuff..\n");
	    return -1;
	} else if (!i_may_write_forum(to_quad)) {	/* no fc's in emps */
	    cprintf("You're not allowed to %s the %s there.\n",
		    (copy) ? "copy" : "move", config.message);
	    return -1;
	}
    } else if (!i_may_write_forum(to_quad)
		|| to_quad == YELL_FORUM
		|| (to_quad == MAIL_FORUM && !copy)
		|| (to_quad == MAIL_FORUM && curr_rm != MAIL_FORUM)) {
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
    if (!(to_quad == MAIL_FORUM && copy && usersupp->priv < PRIV_SYSOP))
        cprintf("\1f\1g%s this %s to `\1y%s\1g'? \1w(\1gy\1w/\1gn\1w)\1c ",
		(copy) ? "Copy" : "Move", config.message, scratch.name);
    if (yesno() == NO)
	return 0;

    for (count = 0;; count++) {

	if (to_quad == MAIL_FORUM) {

	    cprintf("\1f\1yRecipient: \1c");
	    strcpy(to_name, get_name(2));
	    if (!strlen(to_name)) {
		if (!copy && count)
		    message_delete(curr_rm, current_post);
		break;
	    } else if (check_user(to_name) == FALSE) {
		cprintf("\nNo such user..");
		continue;
	    } else if (is_enemy(to_name, who_am_i(NULL))) {
                cprintf("\1f\1rYou are not allowed to CC to \1g%s.\1a\n", to_name);
                if (usersupp->priv >= PRIV_SYSOP) {
                    cprintf("\n\1f\1p%s has you enemy-listed. Override? (y/n)", to_name);
                    if (yesno() == YES)
                        log_sysop_action("Overrode the X-Enemy-list and CC'd %s.",
					 to_name);
                    else
                        continue;
                } else
                    continue;
	    }

	    fail = message_copy(curr_rm, to_quad, current_post,
				to_name, (copy) ? MOD_COPY : MOD_MOVE);

	} else if (copy)
	    fail = message_copy(curr_rm, to_quad, current_post, "", MOD_COPY);
	else
	    fail = message_move(curr_rm, to_quad, current_post, "");
	if (fail == 0) {
	    if (to_quad != MAIL_FORUM)
	        cprintf("\n\1f\1gThe %s has been %s.\1a\n\n",
		    config.message, (copy) ? "copied" : "moved");
            else 
		cprintf("\1f\1gDone.. \1yNext ");
	} else
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
	|| (is_ql(who_am_i(NULL), readquad(curr_rm)))
	|| (usersupp->priv >= PRIV_SYSOP)
	|| (curr_rm == MAIL_FORUM)) {

	cprintf("\1f\1rDelete.\1a\n");
	cprintf("\1f\1gAre you sure you want to delete this %s? \1w(\1gy\1w/\1gn\1w) \1a", config.message);
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

    read_forum( curr_rm, &scratch );

    if ((usersupp->priv >= PRIV_SYSOP)
	|| is_ql(who_am_i(NULL), scratch)) {

	message_header_filename(filename, curr_rm, current_post);
	read_message_header(filename, &message);

	if (message.banner_type & ANON_BANNER) {
	    /* scratch = read_quad(curr_rm);  michel commented out. */
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
		x);

	fclose(fp);
    }
    enter_message(MAIL_FORUM, EDIT_NOEDIT, X_2_MAIL_BANNER, to_user);
}

void
message_clip(const char *header_string)
{
    FILE *fp;

    fp = xfopen(CLIPFILE, "a", TRUE);

    if (fp != NULL) {
	fprintf(fp, "%s", header_string);
	fclose(fp);
    }
}

void
message_2_temp(const char *header_string, char mode)
{
    FILE *fp;

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
		if (i > usersupp->mailnum)	/* uhh-ohh.. */
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
rate_message(message_t * message, unsigned int number, unsigned int forum)
{
    char buf_str[3];
    int score = 0;

    if(forum < 21) {
	cprintf("\1f\1rYou are not allowed to rate %s posted in this %s.\1a\n", config.message_pl, config.forum);
        return;
    }

    if ((mono_sql_rat_check_rating(usersupp->usernum, number, forum)) == -1) {
	cprintf("\1f\1rYou have already rated this %s.\1a\n", config.message);
	return;
    }
    cprintf("\n\1f\1gRatings vary from \1y-9\1g for a horrible %s to \1y9\1g for an excellent %s\n", config.message, config.message);
    cprintf("\1f\1gThe overall score is based on the average of all ratings for this %s.\n\n", config.message);
    cprintf("\1f\1gYour rating for this %s \1w(\1g-9 thru 9\1w): \1c", config.message);

    strcpy(buf_str, "");
    getline(buf_str, 3, TRUE);

    if ((sscanf(buf_str, "%d", &score)) != 1) {
	cprintf("\1f\1rSorry, but `\1y%s\1r' is not a valid entry.\1a\n", buf_str);
	return;
    } else if ((score < -9) || (score > 9)) {
	cprintf("\1f\1rSorry, but `\1y%d\1r' is not a valid entry.\1a\n", score);
	return;
    }
    cprintf("\1f\1gSave a rating of \1y%d\1g for this %s? \1w(\1gy\1w/\1gn\1w) \1c", score, config.message);
    if (yesno() == NO)
	return;

    if ((mono_sql_rat_add_rating(usersupp->usernum, number, forum, score)) == -1)
	cprintf("\1f\1rAn internal SQL error occurred.\1a\n");
    else
	cprintf("\1f\1gScore saved.\1a\n");

    return;
}
#endif

#define SEARCH_RES_LEN	1000

void
search_via_sql(unsigned int forum)
{
    int count = 0;
    sr_list_t *list = NULL;
    char needle[31], tempuser[2 * L_USERNAME + 1], *p = NULL, line[SEARCH_RES_LEN];
    struct timeval tv_start;
    struct timeval tv_end;
    struct timezone tz;
    long working = 0;

    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;

    IFSYSOP {
	cprintf("\1f\1gSearch only this %s? \1w(\1gY\1w/\1gn\1w) \1c", config.forum);
	if (yesno_default(YES) == NO) {
            cprintf("\1f\1rNote that searching %d %s will take approximately forever!\n", mono_sql_mes_count(), config.message_pl);
            cprintf("\1f\1rAre you REALLY ABSOLUTELY sure you want to do this? \1w(\1ry\1w/\1rN\1w) \1c");
            if(yesno_default(NO) == YES) {
	        forum = -1;
            } else {
                cprintf("\1f\1gOkay, searching only this %s.\n", config.forum);
            }
        }
    }
    if (forum == 1) {
	cprintf("\1f\1rCan't search Mail yet, sorry.\n");
	return;
    }
    cprintf("\n\1f\1gFind \1w(\1gmax 30 chars\1w): \1c");
    strcpy(needle, "");
    getline(needle, 30, FALSE);
    needle[strlen(needle)] = '\0';

    if (needle[0] == '*' || needle[0] == '?') {
	cprintf("\1f\1rIllegal character '%c' at start of search string.\1a\n", needle[0]);
	return;
    }
    if (!(strlen(needle))) {
	cprintf("\1f\1rCan't search for nothing...\1a\n");
	return;
    }
    cprintf("\1f\1r\1eSearching...");
    fflush(stdout);

    (void) gettimeofday(&tv_start, &tz);
    count = mono_sql_mes_search_forum(forum, needle, &list);
    (void) gettimeofday(&tv_end, &tz);

    working = ((1000000 * tv_end.tv_sec) + tv_end.tv_usec) - ((1000000 * tv_start.tv_sec) + tv_start.tv_usec);

    if (count == -1) {
	cprintf("\r\1a\1f\1rSearching... An error occurred after ");
	printf("%.4f", (float) working / 1000000);
	cprintf(" seconds.\n");
	mono_sql_ll_free_sr_list(list);
	return;
    } else if (count == 0) {
	cprintf("\r\1a\1f\1gSearching... nothing found.\n");
	mono_sql_ll_free_sr_list(list);
	return;
    } else {
	cprintf("\r\1a\1f\1gSearching... \1y%d\1g match%s.\nPress any key to display results...", count, (count != 1) ? "es" : "");
	fflush(stdout);
	inkey();
    }

    p = (char *) xmalloc(SEARCH_RES_LEN * (count + 5));
    strcpy(p, "");

    sprintf(line, "\n\1f\1g\nFound \1y%d \1gmatch%s in %.6f seconds. Listing by %s and %s number.\n\n", count, (count != 1) ? "es" : "", (float) working / 1000000, config.forum, config.message);
    strcat(p, line);

    sprintf(line, "\1f\1g%-18s     \1y%-25s      \1gId \1y%-22s\n\1w--------------------------------------------------------------------------------\n", config.user, config.forum, "Subject");
    strcat(p, line);

    while (list != NULL) {
	if (strlen(list->result->forum) > 25) {
	    list->result->forum[22] = '.';
	    list->result->forum[23] = '.';
	    list->result->forum[24] = '.';
	}
	if (strlen(list->result->subject) > 20) {
	    list->result->subject[19] = '.';
	    list->result->subject[20] = '.';
	    list->result->subject[21] = '.';
	}
	list->result->forum[25] = '\0';
	list->result->subject[22] = '\0';

	if (EQ(list->result->flag, "normal")) {
            if( (list->result->author == NULL) || (!strlen(list->result->author)) || (EQ(list->result->author,"(null)")) )
	        sprintf(line, "\1f\1rDeleted %-10s \1w%3d.\1y%-25s \1g%7d \1y%-22s\n", config.user, list->result->f_id, list->result->forum, list->result->m_id, ((list->result->subject == NULL) || (EQ(list->result->subject, "(null)"))) ? "[no subject]" : list->result->subject);
            else
	        sprintf(line, "\1f\1g%-18s \1w%3d.\1y%-25s \1g%7d \1y%-22s\n", list->result->author, list->result->f_id, list->result->forum, list->result->m_id, ((list->result->subject == NULL) || (EQ(list->result->subject, "(null)"))) ? "[no subject]" : list->result->subject);
	} else if ((EQ(list->result->flag, "anon") && (strlen(list->result->alias) > 6)) || EQ(list->result->flag, "alias")) {
	    sprintf(tempuser, "'%s'", list->result->alias);
	    sprintf(line, "\1f\1g%-18s \1w%3d.\1y%-25s \1g%7d \1y%-22s\n", tempuser, list->result->f_id, list->result->forum, list->result->m_id, ((list->result->subject == NULL) || (EQ(list->result->subject, "(null)"))) ? "[no subject]" : list->result->subject);
	} else {
	    sprintf(line, "\1f\1bAnonymous %-8s \1w%3d.\1y%-25s \1g%7d \1y%-22s\n", config.user, list->result->f_id, list->result->forum, list->result->m_id, ((list->result->subject == NULL) || (EQ(list->result->subject, "(null)"))) ? "[no subject]" : list->result->subject);
	}
	strcat(p, line);
	list = list->next;
    }
    sprintf(line, "\1f\1w--------------------------------------------------------------------------------\n\1gFound \1y%d \1gmatch%s in %.6f seconds. Listing by %s and %s number.\n", count, (count != 1) ? "es" : "", (float) working / 1000000, config.forum, config.message);
    strcat(p, line);
    mono_sql_ll_free_sr_list(list);

    more_string(p);
    xfree(p);

    return;
}
