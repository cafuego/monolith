/*
 * $Id$
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>

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
#include "input.h"

#include "main.h"
#include "messages.h"
#include "rooms.h"
#include "routines2.h"
#include "display_message.h"
#include "uadmin.h"

#define extern
#include "enter_message.h"
#undef extern


static void get_subject(message_header_t *);
static void get_reply_info(message_header_t *, const unsigned);
static int get_content(int);
static void get_anon_banner(unsigned int, message_header_t *);
static void get_custom_banner(message_header_t *);
static int save_new_message(message_header_t *, unsigned int);
static int save_new_mail(message_header_t *, userlist_t **);
static void clear_wholist_posting_flag(unsigned long);
static void set_wholist_posting_flag(unsigned long);
static int get_mail_names(const int, userlist_t **, message_header_t *);
static void get_autobanner_info(message_header_t *);
static int get_automail_info(message_header_t *, userlist_t **);
static void your_forum_is_boring_zzzz_form_letter(FILE *, char *, int);

int
enter_message(unsigned int forum, int mode, unsigned long banner_flag, const char *automail_name)
{
    room_t quad;
    message_header_t *header;
    userlist_t *recipient_list = NULL;
    int reply, abort = FALSE;

    nox = TRUE;
    reply = (mode == REPLY_MODE) ? 1 : 0;

    header = (message_header_t *) xmalloc(sizeof(message_header_t));
    init_message_header(header);
    quad = read_quad(forum);

    strcpy(header->author, usersupp->username);
    strcpy(header->forum_name, quad.name);
    header->f_id = forum;
    header->date = time(NULL);
    header->quad_flags = quad.flags;
    header->banner_type = banner_flag;


    if (banner_flag & AUTOBANNER) {
	get_autobanner_info(header);

	if (banner_flag & AUTOMAIL_BANNER) {	/* note: only single user mail supported! */
	    strcpy(header->recipient, automail_name);
	    get_automail_info(header, &recipient_list);
	    if (!recipient_list)
		abort = TRUE;
	}
	if ((banner_flag & FILE_POST_BANNER) && (header->banner_type & MAIL_BANNER)) {
	    get_mail_names(reply, &recipient_list, header);
	    if (!recipient_list)
		abort = TRUE;
	}
	forum = header->f_id;

    } else if (!we_can_post(forum))
	abort = TRUE;

    if (forum == MAIL_FORUM && !(header->banner_type & AUTOBANNER)) {
	get_mail_names(reply, &recipient_list, header);
	if (!recipient_list)
	    abort = TRUE;
    }
    if (abort) {
	dest_userlist(recipient_list);
	xfree(header);
	if (fexists(temp))
	    unlink(temp);
	return -1;
    }
    if (!(header->banner_type & AUTOBANNER)) {
	if (!banner_flag && (quad.flags & (QR_ANONONLY | QR_ANON2 | QR_ALIASNAME)))
	    get_anon_banner(quad.flags, header);

	if (banner_flag & CUSTOM_BANNER)
	    get_custom_banner(header);

	if (quad.flags & QR_SUBJECTLINE || reply) {
	    if (reply) {    /* yells are saved via save_new_mail */
		if (forum == YELL_FORUM) {  
		    get_mail_names(reply, &recipient_list, header);
		    header->banner_type |= (ADMIN_BANNER | YELL_REPLY_BANNER | MAIL_BANNER);
		} else if (quad.flags & QR_SUBJECTLINE && reply)
		    get_reply_info(header, quad.flags);
	    } else {
		get_subject(header);
            }
        }
    }
    if ( ! (mode == EDIT_NOEDIT && banner_flag & AUTOBANNER)) {
	size_t filesize;

	set_wholist_posting_flag(header->banner_type);
	display_message_header(header);
	abort = get_content(mode);
	clear_wholist_posting_flag(header->banner_type);
	filesize = get_filesize(temp);

	if (abort < 0 || !filesize) {
	    size_t filesize;
	    filesize = get_filesize(temp);

	    if (abort == -1 || !filesize)
		cprintf("\1f\1rEmpty %s not saved!\n", config.message_pl);
	    else if (abort == -2)
		cprintf("\1f\1r%s aborted.\n", config.message);
	    xfree(header);
	    dest_userlist(recipient_list);
	    if (fexists(temp))
		unlink(temp);
	    return -1;
	}
    }

    if (header->banner_type & YELL_REPLY_BANNER)
	message_move(YELL_FORUM, HANDLED_YELL_FORUM, message_reply_id(0), "");

    if (header->banner_type & (MAIL_BANNER | YELL_BANNER) &&
	!(header->banner_type & INFO_BANNER))  
	save_new_mail(header, &recipient_list);
    else
	save_new_message(header, forum);

    xfree(header);
    dest_userlist(recipient_list);
    if (fexists(temp))
	unlink(temp);

    return 0;
}

void
set_wholist_posting_flag(unsigned long banner_flag)
{
    if (!(banner_flag & ANON_BANNER))
	mono_change_online(who_am_i(NULL), "", 8);
}

void
clear_wholist_posting_flag(unsigned long banner_flag)
{
    if (!(banner_flag & ANON_BANNER))
	mono_change_online(who_am_i(NULL), "", -8);
}

int
save_new_message(message_header_t * header, unsigned int forum)
{
    char filename[L_FILENAME];

    if (header->banner_type & INFO_BANNER) {	/* quadinfo */

	info_header_filename(filename, forum);
	write_message_header(filename, header);
	info_filename(filename, forum);
	if (fexists(filename))
	    unlink(filename);
	copy(temp, filename);
    } else {
        time_function(TIME_START);
	header->m_id = get_new_message_id(forum);
	if (header->m_id == 0)
	    cprintf("\n\1f\1rget_new_message_id() returned 0.  aborting\n\1a");
	else {
	    header->f_id = forum;
	    message_header_filename(filename, forum, header->m_id);
	    write_message_header(filename, header);
	    copy(temp, message_filename(filename, forum, header->m_id));
        }
	if (usersupp->priv >= PRIV_TECHNICIAN)
	    cprintf("\n\1a\1wFilesystem time elapsed: %f", 
		    time_function(TIME_STOP));
	else
	    time_function(TIME_STOP);
    }
    
    time_function(TIME_START);
    save_to_sql(header, temp);
    if (usersupp->priv >= PRIV_TECHNICIAN)
        cprintf("\n\1a\1wSQL time elapsed: %f\n", time_function(TIME_STOP));
    else
	time_function(TIME_STOP);

    usersupp->posted++;

    return 0;
}

int
save_new_mail(message_header_t * header, userlist_t ** recipient_list)
{
    char filename[L_FILENAME];
    userlist_t *p;

    if (header->banner_type & YELL_REPLY_BANNER)
	save_new_message(header, HANDLED_YELL_FORUM);
    header->f_id = MAIL_FORUM;

    if (!(header->banner_type & AUTOMAIL_NO_PERSONAL_COPY)) {

	header->m_id = get_new_mail_number(who_am_i(NULL));
	mail_header_filename(filename, who_am_i(NULL), header->m_id);
	if ((write_message_header(filename, header)) == -1)
	    cprintf("\1f\1rError saving your copy of the mail.\1a\n");
	else {
	    mail_filename(filename, who_am_i(NULL), header->m_id);
	    if ((copy(temp, filename)) == -1) {
		cprintf("\1f\1rError saving your copy of the mail.\1a\n");
		mail_header_filename(filename, who_am_i(NULL), header->m_id);
		if (fexists(filename))
		    unlink(filename);
	    }
	}
    }
    for (p = *recipient_list; p; p = p->next) {
	if (strcmp(p->name, "Sysop") == 0) {	/* yell, save in YELL_FORUM */
	    unsigned long temp_banner_type;
	    temp_banner_type = header->banner_type;
	    header->banner_type = YELL_BANNER;	/* yell banner is exclusive ! */
	    save_new_message(header, YELL_FORUM);
	    header->banner_type = temp_banner_type;
	} else {
	    header->m_id = get_new_mail_number(p->name);
	    header->f_id = MAIL_FORUM;
	    mail_header_filename(filename, p->name, header->m_id);
	    if ((write_message_header(filename, header)) == -1)
		cprintf("\1f\1rError saving your mail to %s.\1a\n", p->name);
	    else {
		mail_filename(filename, p->name, header->m_id);
		if ((copy(temp, filename)) == -1) {	/* clean up mess */
		    cprintf("\1f\1rError saving your mail to %s.\1a\n", p->name);
		    mail_header_filename(filename, p->name, header->m_id);
		    if (fexists(filename))
			unlink(filename);
		}
	    }
	}
    }

    return 0;
}


void
get_anon_banner(unsigned int anon_type, message_header_t * header)
{
    if (anon_type & QR_ANON2) {
	cprintf("\1f\1gAnonymous \1w(\1rY/N\1w)\1g?\1a ");
	if (yesno() == 0) {
	    header->banner_type = NO_BANNER;
	    return;
	}
    }
    header->banner_type = ANON_BANNER;
    strcpy(header->alias, "");

    if (!(anon_type & QR_ALIASNAME))
	return;

    cprintf("\1f\1gDo you want to add an aliasname to this %s? \1w(\1rY/N\1w) \1c",
	    config.message);
    if (yesno() == YES) {
	if ((usersupp->config_flags & CO_USEALIAS) && strlen(usersupp->alias)) {
	    strcpy(header->alias, usersupp->alias);
	} else {
	    cprintf("\1f\1gAlias\1w: \1c");
	    getline(header->alias, L_USERNAME, 1);
	}
    }
}

void
get_subject(message_header_t * header)
{
    cprintf("\1f\1gSubject\1w: \1y");
    getline(header->subject, L_SUBJECT - 1, 1);
}

void
get_reply_info(message_header_t * header, const unsigned quad_flags)
{
    message_header_t reply_header;
    int read_success = 0;
    char filename[L_FILENAME];

    if (curr_rm == MAIL_FORUM) {
	mail_header_filename(filename, who_am_i(NULL), message_reply_id(0));
	read_success = read_message_header(filename, &reply_header);
    } else {
	message_header_filename(filename, header->f_id, message_reply_id(0));
	read_success = read_message_header(filename, &reply_header);
    }

    if (read_success == -1)
	return;

    header->reply_m_id = reply_header.m_id;
    header->reply_f_id = reply_header.f_id;
    header->reply_t_id = reply_header.t_id;

    if (quad_flags & QR_SUBJECTLINE) {
        if (!strlen(reply_header.subject))
	    sprintf(header->subject, "\1a\1w[\1cNo Subject\1w]\1a");
        else
	    snprintf(header->subject, L_SUBJECT, "%s", reply_header.subject);
	snprintf(header->reply_to_author, L_USERNAME, "%s",
		 (reply_header.banner_type & ANON_BANNER) ? "Anonymous" :
		 reply_header.author);
	header->subject[L_SUBJECT] = header->reply_to_author[L_USERNAME] = '\0';
    }
}


void
get_autobanner_info(message_header_t * header)
{

    static void quad_lizard(message_header_t *);

    if (header->banner_type & INFO_BANNER)
	return;

    if (header->banner_type & FILE_POST_BANNER) {
	if (header->f_id == MAIL_FORUM)
	    header->banner_type |= MAIL_BANNER;
	return;
    }

	/* note:  quad_lizard() runs enter_message recursively */
    if (header->banner_type & QUADLIZARD_BANNER)
	quad_lizard(header);

    if (header->banner_type & QUADLIZARD_MAIL_BANNER) {
	header->banner_type |= SYSTEM_BANNER;
	strcpy(header->author, "Quad Lizard\1a");
    }

    if (header->banner_type & QUADCONT_MAIL_BANNER) {
	header->banner_type |= SYSTEM_BANNER;
	strcpy(header->author, "HAL");
	strcpy(header->subject, "\1f\1yQuadrant evaluation results.\1a");
    }

    if (header->banner_type & DELETE_MAIL_BANNER) {
	header->banner_type |= QL_BANNER;
	strcpy(header->author, usersupp->username);
	strcpy(header->subject, "\1f\1rDeleted Message\1a");
    }
    if (header->banner_type & AUTOMAIL_BANNER) {
	header->f_id = MAIL_FORUM;
	header->banner_type |= MAIL_BANNER;
    }
    if (header->banner_type & X_2_MAIL_BANNER) {
	strcpy(header->author, usersupp->username);
	strcpy(header->subject, "\1f\1gSaved X message!\1a");
    }
    if (header->banner_type & YELL_BANNER)
	header->banner_type = YELL_BANNER;	/* no bitwise.. yell banner is exclusive */

    return;
}


int
get_automail_info(message_header_t * header, userlist_t ** recipient_list)
{
    userlist_t element;

    if (header->banner_type & YELL_BANNER)
	get_subject(header);
    strcpy(element.name, header->recipient);
    add_to_userlist(element, recipient_list);
    return 0;
}

void
get_custom_banner(message_header_t * header)
{
    cprintf("\n\1f\1wBanner: ");
    getline(header->banner, L_BANNER - 1, 0);
}

/*
 * mode = 1: normal message.
 * mode = 2: upload message.
 * mode = 3: editor message.
 */

int
get_content(int mode)
{
    FILE *fp;
    int lines = 0;

    fp = xfopen(temp, "a", TRUE);

    if (usersupp->config_flags & CO_NEATMESSAGES)
	cprintf("\n\1a\1c");

    switch (get_buffer(fp, mode, &lines)) {

	case 's':
	case 'S':
	    fclose(fp);
	    return 1;
	    break;

	case 'a':
	case 'A':
	default:
	    fclose(fp);
	    unlink(temp);
	    return -2;
    }

    return 0;
}

int
get_mail_names(const int reply, userlist_t ** name_list, message_header_t * header)
{
    char mailname[L_USERNAME + 1];
    userlist_t element, *p;
    int duplicate;

    header->banner_type |= MAIL_BANNER;

    if ((usersupp->priv & PRIV_ALLUNVALIDATED)) {
	strcpy(element.name, "Sysop");
	strcpy(header->recipient, "Sysop");
	add_to_userlist(element, name_list);
	return 0;
    }
    for (;;) {
	if (reply) {
	    strcpy(mailname, message_reply_name(NULL));
	} else {
	    if (!name_list) {
		cprintf("\1f\1yRecipient:\1c ");
	    } else {
		cprintf("\1f\1gPress \1w<\1renter\1w>\1g to finish.\n");
		cprintf("\1f\1yNext recipient\1w:\1c ");
	    }
	    strcpy(mailname, get_name(2));
	}

	if (!strlen(mailname))
	    break;
	if (strcmp(mailname, "Sysop"))
	    if (check_user(mailname) != TRUE || mailname[0] == '.') {
		cprintf("\1f\1rNo such user.\1a\n");
		if (reply)
		    break;
		else
		    continue;
	    }
	if (is_enemy(mailname, who_am_i(NULL))) {
	    cprintf("\1f\1rYou are not allowed to mail \1g%s\1a\n", mailname);
	    if (usersupp->priv >= PRIV_SYSOP) {
		cprintf("\n\1f\1p%s has you enemy-listed. Override? (y/n)", mailname);
		if (yesno() == YES)
		    log_sysop_action("Overrode the X-Enemy-list and Mailed %s.", mailname);
		else
		    continue;
	    } else
		continue;
	}
				/* check for duplicates */
	duplicate = FALSE;
	for (p = *name_list; p; p = p->next) 
	    if (strcmp(p->name, mailname) == 0) {
		duplicate = TRUE;
		break;
	    }
	
	if (duplicate)
	    continue;

	strcpy(element.name, mailname);
	add_to_userlist(element, name_list);

	if (reply) 
	    break;
    }  

    strcpy(header->recipient, "");
    for (p = *name_list; p; p = p->next) {
	strcat(header->recipient, p->name);
	if (p->next)
	   strcat(header->recipient, ", ");
	if (strlen(header->recipient) >= L_RECIPIENT - (L_USERNAME + 3)) {
	    header->recipient[L_RECIPIENT - 12] = '\0';
	    strcat(header->recipient, " CC: More");
	    break;
	}
    }
    return 0;
}

void
enter_admin_message(void)
{
    if (usersupp->priv & (PRIV_TECHNICIAN | PRIV_SYSOP | PRIV_WIZARD)) {
	cprintf("\1f\1rEnter %s with title.\1a\n", config.message);
	fflush(stdout);
	if (usersupp->priv & (PRIV_WIZARD | PRIV_SYSOP))
	    enter_message(curr_rm, EDIT_NORMAL, ADMIN_BANNER, NULL);
	else if (usersupp->priv & (PRIV_TECHNICIAN))
	    enter_message(curr_rm, EDIT_NORMAL, TECH_BANNER, NULL);
    }
    return;
}

void
enter_yell()
{
    more(YELLINFO, 1);

    cprintf("\1f\1gDo you really want to \1w<\1rY\1w>\1gell to the Administrators?\1a ");
    if (yesno() == NO) {
	return;
    }
    enter_message(YELL_FORUM, EDIT_NORMAL, YELL_BANNER, "Sysop");
}

void
change_forum_info(void)
{
    char filename[L_FILENAME + 1];
    char cmd;
    room_t quad;

    if (!display_message(curr_rm, 0, DISPLAY_INFO))
	cprintf("\1f\1rNo %s information file exists currently.\1a\n", config.forum);

    cprintf("\1f\1gChoice: \1w<\1re\1w>\1gnter normally, \1w<\1rE\1w>\1gditor-edit, \1w<\1rI\1w>\1gnsert ClipBoard, \1w<\1ru\1w>\1gpload \1w->\1a");
    cmd = get_single("eEIQU \r\n");

    if (strchr("Q \r\n", cmd))
	return;

    switch (cmd) {
	case 'e':
	    cprintf("\n\1f\1rEnter %s Info.\n", config.forum);
	    enter_message(curr_rm, EDIT_NORMAL, INFO_BANNER, NULL);
	    break;

	case 'E':{
		cprintf("\1f\1rEdit %s Info.\1a\n", config.forum);

		info_filename(filename, curr_rm);
		if (fexists(temp))
		    unlink(temp);
		if (fexists(filename))
		    copy(filename, temp);
		editor_edit(temp);
		enter_message(curr_rm, EDIT_NOEDIT, INFO_BANNER, NULL);
		break;
	    }

	case 'I':
	    if (fexists(temp))
		unlink(temp);
	    copy(filename, temp);
	    enter_message(curr_rm, EDIT_NOEDIT, INFO_BANNER, NULL);
	    break;

	case 'U':
	    cprintf("\n\1f\1rUpload:  Press <Control-d> when done.\n\1a");
	    enter_message(curr_rm, EDIT_CTRLD, INFO_BANNER, NULL);
    }

    quad = read_quad(curr_rm);
    quad.roominfo++;
    quad.flags |= QR_DESCRIBED;	/* set the flag */
    write_quad(quad, curr_rm);
    log_sysop_action("changed Forum Info for %s>", quad.name);
    return;
}

/*
 * NOTE:  uses both temp files!  (temp and tmpname) due to recursive nature
 * of quad_lizard's call to enter_message for each your_forum_is_boring_zzzz() 
 * (see static decl above) 
 */

void
quad_lizard(message_header_t * header)
{
    FILE *fp1, *fp2;
    char filename[L_FILENAME + 1];
    int i, ql_ctr, idle_for, mail_the_lamer_qls;
    long j;
    room_t quad;
    time_t timenow;
    user_t *qlPtr = NULL;
    message_header_t last_message;

    time(&timenow);
    log_it("quadlizard", "Starting up!");

    cprintf("\n\1f\1rMail idle %ss? \1w(\1ry\1w/\1rN\1w) \1c", config.roomaide);
    mail_the_lamer_qls = yesno_default(NO);

    sprintf(header->author, "%s Lizard", config.forum);
    sprintf(header->subject, "\1rList of %s 'issues'", config.forum);
    header->f_id = SYSOP_FORUM;

    fp2 = xfopen(tmpname, "w", TRUE);

    for (i = 20; i < MAXQUADS; i++) {
	quad = read_quad(i);
	if (!((quad.flags & QR_INUSE) && (quad.highest)))
	    continue;

/* last post is old? */

	for (j = quad.highest; j > quad.lowest && j > 0; j--) {
	    strcpy(filename, "");
	    message_header_filename(filename, i, j);
	    if (!fexists(filename))
		continue;

	    if (read_message_header(filename, &last_message) == -1)
		continue;
	    else
		break;
	}

	idle_for = (int) ((timenow - last_message.date) / 86400);

#ifdef NO_POSTS_FOR_30_DAYS
	if (idle_for >= 30) {
#else /* 15 days */
	if (idle_for >= 15) {
#endif
	    fprintf(fp2,
		  "\1f\1w%d.\1g%s\1w: \1gLast post \1w%d\1g days ago.\1a\n",
		    i, quad.name, idle_for);

	    for (j = 0; j < NO_OF_QLS && mail_the_lamer_qls; j++) {
		if ((strlen(quad.qls[j]) <= 0) || (strcmp(quad.qls[j], "Sysop") == 0))
		    continue;
		else if (check_user(quad.qls[j])) {
		    char quadname[L_QUADNAME + 1];
		    char name[L_USERNAME + 1];

		    strcpy(quadname, quad.name);
		    strcpy(name, quad.qls[j]);

		    fp1 = xfopen(temp, "w", FALSE);
		    your_forum_is_boring_zzzz_form_letter(fp1, quad.name,
							  idle_for);
		    fclose(fp1);
		    enter_message(MAIL_FORUM, EDIT_NOEDIT, QUADLIZARD_MAIL_BANNER, name);

		    log_it("quadlizard",
			   "Mailed %s QL #%ld, %s no posts in %d days.",
			   quadname, j, name, idle_for);
		    fprintf(fp2, "    \1f\1gNotified QL %ld, \1y%s\1g, via mail.\1a\n",
			    j, name);
		}
	    }
	}
/* QL absentee, missing, screwed, etc. */

	if (strcmp(quad.category, "Admin") == 0)
	    continue;
	ql_ctr = 0;
	for (j = 0; j < NO_OF_QLS; j++) {
	    if ((strlen(quad.qls[j]) <= 0) || (strcmp(quad.qls[j], "Sysop") == 0))
		continue;
	    else if (check_user(quad.qls[j]) != 1) {
		fprintf(fp2, "\1f\1w%d.\1r%s\1w: \1rQL slot #\1w%ld: \1rUser \1r%s \1rdoesn't exist.  \1w(\1r!\1w)\1a\n", i, quad.name, j + 1, quad.qls[j]);
		continue;
	    } else {
		qlPtr = readuser(quad.qls[j]);
		ql_ctr++;
	    }
	    if (timenow - qlPtr->laston_to > 2592000) {
		fprintf(fp2, "\1f\1w%d.\1y%s\1w: \1gQL #\1w%ld\1y %s\1g absent \1w%d\1g days.\1a\n", i, quad.name, j + 1, quad.qls[j], (int) (timenow - qlPtr->laston_to) / 86400);
	    }
	    if ((!(qlPtr->flags & US_ROOMAIDE)) && (qlPtr->priv < PRIV_SYSOP)) {
		fprintf(fp2, "\1f\1w%d.\1r%s\1w: \1rQL #\1w%ld \1r%s\1r has no QL flag.  \1w(\1r!\1w)\1a\n", i, quad.name, j + 1, quad.qls[j]);
	    }
	    xfree(qlPtr);
	}
	if (!ql_ctr) {
	    fprintf(fp2, "\1f\1w%d.\1y%s\1w: \1yNo QL.\1a\n", i, quad.name);
	}
    }
    fclose(fp2);
    if (fexists(temp))
	unlink(temp);
    copy(tmpname, temp);
}

void
your_forum_is_boring_zzzz_form_letter(FILE * fp, char *quadname, int idle)
{
    fprintf(fp, "%s%s%s%s%d%s%s%s%s%s%s%s%s%s",
	    "\1a\1c\n",
	    "  Your Quadrant, \1f\1y",
	    quadname,
	    "\1a\1c, has not had any new posts for \1f\1r",
	    idle,
	    "\1a\1c days.\n\n",
	    "  (You should enter a post there..)\n\n",
     "  Some Quadrants are harder to generate interest in than others.\n\n",
	    "  A post entered by you:\n\n",
	    "  - Shows your interest in the Quadrant.\n",
	    "  - Serves as a reminder that the Quadrant is there. (and users do forget)\n",
	    "  - Gets the Quadrant scrolling again.\n",
	    "  - Usually generates enough interest to get the Quadrant's readers to post.\n\n",
	    "                      Good Luck!\n\n");

}

void
i_deleted_your_post_haha_mail(const unsigned int current_post)
{
    FILE* fp;
    char filename[L_FILENAME + 1], name[L_USERNAME + 1];
    message_header_t message;
    room_t quad;

    if (curr_rm == MAIL_FORUM)
	return;

    message_header_filename(filename, curr_rm, current_post);
    read_message_header(filename, &message);

    strcpy(name, message.author);
    if (check_user(name) == FALSE) {
	cprintf("\1f\1rOriginal author \1w`\1y%s\1w' \1rdoes not exist.\1a\n", name);
	return;
    }
    
    if (fexists(temp))
	unlink(temp);

    cprintf("\n\1f\1gWould you like to include a reason for the deletion? (y/n) \1c");
    if (yesno() == YES) {
	cprintf("\1gEnter text.  Deleted message will be appended to reason.\n\n\1a\1c");
	get_content(EDIT_NORMAL);
    }

    quad = read_quad(curr_rm);
    fp = xfopen(temp, "a", TRUE);

    if (fp != NULL) {
	fprintf(fp, "%s%s%s", 
		"\1a\1w\n-- The following message was deleted from \1f\1y",
		quad.name, "\1a\1w --\n");
	fclose(fp);
    }
    display_message(curr_rm, current_post, DISPLAY_2_TEMP_A);
    enter_message(curr_rm, EDIT_NOEDIT, DELETE_MAIL_BANNER, name);
}

void
mail_myself_quadcont_results(int ** eval)
{
    FILE* fp;
    room_t quad;
    int i, items_left = 0;

    fp = xfopen(temp, "w", FALSE);
    if (fp == NULL) {
	cprintf("\n\1f\1rTempfile write error.\1a");
	return;
    }

    for (i = 0; i < MAXQUADS; i++) {
	if (*eval[i] == -1) {
	    quad = readquad(i);
	    if (usersupp->forget[i] != quad.generation) {
		if (!items_left)
		    fprintf(fp, "\n\1f\1gSelection algorithm suggests zapping the following quads:\1a\n\n");
		fprintf(fp, "\1f\1y%d\1w> \1g%s\n", i, quad.name);
		items_left++;
	    }
	}
    }

    items_left = 0;
    for (i = 0; i < MAXQUADS; i++) {
	if (*eval[i] > 0) {
	    quad = readquad(i);
	    if (usersupp->forget[i] == quad.generation) {
		if (!items_left)
		    fprintf(fp, "\n\1f\1gSelection algorithm suggests unzapping the following quads:\1a\n\n");
		fprintf(fp, "\1f\1y%d\1w> \1g%s\n", i, quad.name);
		items_left++;
	    }
	}
    }

    fclose(fp);
    
    enter_message(MAIL_FORUM, EDIT_NOEDIT, QUADCONT_MAIL_BANNER, who_am_i(NULL));
}
