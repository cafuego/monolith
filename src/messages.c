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

#include "chat.h"
#include "commands.h"
#include "clipboard.h"
#include "express.h"
#include "friends.h"
#include "help.h"
#include "input.h"
#include "rooms.h"
#include "uadmin.h"
#include "main.h"
#include "usertools.h"
#include "routines2.h"

#include "sql_userforum.h"

#define MODIFICATION_TIME

static void move_message_to_trash(void);

long reply_num;			/* msgnumber of the replied msg         */
char curr_post_is_mine = FALSE;

/*************************************************
* entmsg()
* type: MES_NORMAL	-> normal
*	MES_ROOMAIDE	-> RA-title
*	MES_SYSOP	-> SYSOP-title
*	MES_WIZARD	-> WIZARD-title
*	MES_REPLY	-> reply
* how:	1	-> normal entering (EDIT_NORMAL)
*	2	-> upload (Ctrl-D to end) (EDIT_CTRLD)
*	3	-> Editor (PICO or local) (EDIT_EDITOR)
*	5	-> reply
*	9	-> forced message
*************************************************/

   /* note: replystuff called here in entmsg() only for mail name stuff.
    * called for actual reply info in make_message().  dirty hack to get
    * around scope issues as multimail names are (potentially) realloc'd in
    * get_multimail_names().
    */

int
entmsg(int msgtype, int enterhow)
{
    char *names;
    int fail = 0, temp_mes_type;
    post_t *scratch_msgPtr;

    names = (char *) xmalloc(L_USERNAME + 2);
    strcpy(names, "");

    if (!we_can_post())
	return FALSE;
    curr_line = nox = 1;	/* disable receiving X's right now... */
    if ((usersupp->flags & US_EXPERT) == 0)	/* newbie help */
	more(ENTERMESSAGE, 0);

    if (curr_rm == 1 || curr_rm == 2) {		/* mail and yell stuff */
	scratch_msgPtr = ((post_t *) xmalloc(sizeof(post_t)));
	memset(scratch_msgPtr, 0, sizeof(post_t));

	if (enterhow == 5) {
	    if ((get_newmessage_replystuff(scratch_msgPtr)) == -1)
		fail = 1;
	    else		/* reply (yell or mail) */
		strcpy(names, scratch_msgPtr->refauthor);
	} else if (curr_rm == 1 && msgtype != MES_DESC) {
	    if ((names = get_multimail_names((enterhow == 5) ? 1 : 0, names)) == NULL)
		fail = 1;
	    else if (!strlen(names))
		fail = 1;
        }
	xfree(scratch_msgPtr);
    }
    if (!fail) {
	temp_mes_type = set_message_type(msgtype, enterhow);

	if (!(temp_mes_type == MES_ANON || temp_mes_type == MES_AN2))
	    mono_change_online(usersupp->username, "", 8);

	fail = make_message(temp, names, temp_mes_type, enterhow);

	if (!(temp_mes_type == MES_ANON || temp_mes_type == MES_AN2))
	    mono_change_online(usersupp->username, "", -8);
    }
    if (fail) {
	xfree(names);
	return 0;
    }
    fflush(stdout);
    if (curr_rm == 2 && enterhow == 5)
	save_handled_yell(names);
    else if (curr_rm == 1)
	save_mail(names);
    else if (save_message(temp, curr_rm, names) != 0)
	cprintf("\1f\1rOops! Something went wrong saving your post.\1a\n");

    unlink(temp);
    usersupp->posted++;
    xfree(names);
    return 1;
}

int
we_can_post(void)
{
    /* priv check for guests and cursed users is handled in validate_read_command() */
    /* if we are in the docking bay, the user must be Sysop (or Technician) */
    if (curr_rm == 0) {
	cprintf("\1f\1r\nOpen the door please, HAL.");
	fflush(stdout);
	sleep(1);
	if (usersupp->priv < PRIV_TECHNICIAN) {
	    cprintf("\n\1f\1rI'm sorry Dave, but I can't do that.\n");
	    return FALSE;
	} else {
	    cprintf("\n\1f\1rAre you sure you want me to open the \1yDocking Bay\1w>\1r doors, Dave? \1a");
	    if (yesno_default(FALSE) == FALSE) {
		cprintf("\1f\1rGoodbye, Dave\n\1a");
		return FALSE;
	    }
	}
    }
    if ((!may_write_room(*usersupp, quickroom, curr_rm)) && (curr_rm != 1)) {
	cprintf("\n\1f\1gYou are not allowed to post here.\1a\n");
	return FALSE;
    }
    return TRUE;
}

char *
get_multimail_names(int reply, char *mailnames)
{
    char name[L_USERNAME + 1];

    if ((usersupp->priv & PRIV_ALLUNVALIDATED)) {
	strcpy(mailnames, "Sysop");
    } else {
	while (1) {
	    if (reply) {
		strcpy(name, mailnames);
		strcpy(mailnames, "");
	    } else {
		if (!strlen(mailnames)) {
		    cprintf("\1f\1yRecipient:\1c ");
		} else {
		    cprintf("\1f\1gPress \1w<\1renter\1w>\1g to finish.\n");
		    cprintf("\1f\1yNext recipient\1w:\1c ");
		}
		strcpy(name, get_name(2));
	    }

	    if (strlen(name) == 0)
		break;

	    if (check_user(name) != TRUE) {
		cprintf("\1f\1rNo such user.\1a\n");
		continue;
	    }
	    if (is_enemy(name, username)) {
		/* check if we're on the X-enemy-list of the receiver */
		cprintf("\1f\1rYou are not allowed to mail \1g%s\1a\n", name);
		if (usersupp->priv > PRIV_SYSOP) {
		    cprintf("\1f\1r...but since you _are_ %s, I will let you mail the alien.\1a\n" WIZARD);
		    log_sysop_action("Overrode the X-Enemy-list and Mail>ed %s.", name);
		} else {
		    continue;
		}
	    }
	    if (!strlen(mailnames))
		strcpy(mailnames, name);
	    else {
		mailnames = (char *) realloc(mailnames, strlen(mailnames) + L_USERNAME + 2);
		sprintf(mailnames + strlen(mailnames), ",%s", name);
	    }
	    if (reply) {
		break;
	    }
	}			/* while */
    }
    return mailnames;
}

int
set_message_type(const int msgtype, const int enterhow)
{
    int temp_message_type;

    if (curr_rm == 2 && enterhow == 5)	/* yellreply! */
	temp_message_type = ((usersupp->priv & PRIV_WIZARD) ? MES_WIZARD :
		     ((usersupp->priv < PRIV_SYSOP) ? msgtype : MES_SYSOP));
    else if (msgtype == MES_NORMAL) {
	temp_message_type = MES_NORMAL;
	if (curr_rm != 1) {	/* no anon for direct mail */
	    if (quickroom.flags & QR_ANONONLY)
		temp_message_type = MES_ANON;
	    if (quickroom.flags & QR_ANON2) {
		cprintf("\1f\1gAnonymous \1w(\1rY/N\1w)\1g?\1a ");
		if (yesno() == 1)
		    temp_message_type = MES_AN2;
	    }
	}
    } else			/* temp_message_type reverts to the MES_* type as was called */
	temp_message_type = msgtype;
    return temp_message_type;
}

void 
move_message_to_trash(void)
{
    if (move_message(curr_rm, reply_num, 144, "") != 0) 
	cprintf("\1f\1rMoving message to Trash> failed.\1a\n");
}

int
save_handled_yell(char *yellname)
{
    save_mail(yellname);
    if (move_message(curr_rm, reply_num, 148, "") != 0)
	cprintf("\1f\1rOops! Could not move the yell to Handled!\1a\n");
    else
	cprintf("\1f\1g%s has been moved to `\1yHandled Yells\1g'.\1a\n", config.message);

    if (save_message(temp, 148, usersupp->username) != 0)
	cprintf("\1f\1rOops! Something went wrong saving your reply to Handled>.\1a\n");
    log_it("yells", "%s answered a yell from %s.", usersupp->username, yellname);
    return 0;
}

int
save_mail(char *names)
{
    char *begin, *end;

    strcat(names, ",");
    end = strchr(names, ',');
    *end = '\0';
    end++;
    begin = names;

    for (;;) {
	if (save_message(temp, 1, begin) != 0)
	    cprintf("\1f\1rOops! Something went wrong saving your mail to %s.\1a\n", begin);
	if (!strlen(end))
	    break;
	begin = end;
	end = strchr(begin, ',');
	*end = '\0';
	end++;
    }

    if (save_message(temp, 1, usersupp->username) != 0)		/* own copy */
	cprintf("\1f\1rOops! Something went wrong saving your copy of the mail.\1a\n");
    return 0;
}


/*-----------------------------------------------------------------------------
* mode: 1 -> normal posting. (EDIT_NORMAL)
*	2 -> Ctrl-D-posting. (EDIT_CTRLD)
*	3 -> editor-write it. (EDIT_EDITOR)
*	5 -> a reply
*	9 -> a forced message.
*-------------------------------------------------------------------------------
* The different message-fields      |  message fields defined for
* used for actually reading,saving  |  struct {...} post_t;  (in typedefs.h)
* post in read,write_post_header()  |   
*-----------------------------------+-------------------------------------------
* A: username of the author	    + char author[L_USERNAME + 1]
* C: aliasname if used		    + char alias[L_USERNAME + 1]
* D: <notused in write at present>  X char modifier[L_USERNAME + 1]
* I: <notused in write at present>  X long num
* L: number of lines in message     + int lines
* M: the message body!		    | ---
* O: room the post originated from  + char origroom[L_QUADNAME + 1]
* R: recipient if mail		    + char recipient[400]
* S: subject if used		    + char subject[L_SUBJECT + 1]
* T: time when the post was made    + date_t date
* U: <notused in write at present>  X date_t moddate
* X: replied user                   | char refauthor[L_USERNAME + 1]
* Y: <notused in write at present>  X long orignum
* ?: message type		    + long type
* ?: <notused in write at present>  | int quad
* Q: post # replying to             + long ref
* Z: post type replying to          + long reftype
------------------------------------------------------------------------------*/

int
make_message(const char *filename, char *const mailnames, int type, int mode)
{
    FILE *fp;
    int lines = 1, i;
    post_t *newmessagePtr;

    fp = xfopen(filename, "w", TRUE);

    newmessagePtr = ((post_t *) xmalloc(sizeof(post_t)));
    memset(newmessagePtr, 0, sizeof(post_t));

    strcpy(newmessagePtr->author, usersupp->username);
    newmessagePtr->type = type;
    newmessagePtr->date = time(NULL);

    if ((type == MES_ANON || type == MES_AN2) && (quickroom.flags & QR_ALIASNAME))
	get_newmessage_alias(newmessagePtr);

/* WARNING!  Mail> and Yells> MUST have subjectline flags in the quickroom!! */

    if (((quickroom.flags & QR_SUBJECTLINE) && (type != MES_DESC)) || curr_rm == 1) {
	if (mode == 5) {	/* reply */
	    if ((get_newmessage_replystuff(newmessagePtr)) == -1) {
		xfree(newmessagePtr);
		return -1;
	    }
	} else
	    get_newmessage_subjectline(newmessagePtr);
    }

/* note: mailnames and recipient are NOT the same, recipient is only for display
 *       the actual names malloc'd in entmsg is what's parsed to send in save_mail */

    if (strlen(mailnames)) {
	if (strlen(mailnames) > 100) {	/* fix array overflow on multimail display */
	    strncpy(newmessagePtr->recipient, mailnames, 100);
	    strcat(newmessagePtr->recipient, ">> CC: more recipients.");
	    for (i = 1; i < strlen(mailnames); i++)
	        if ((i % 78) == 0)
		    newmessagePtr->recipient[i - (36 + strlen(usersupp->username))] = '\n';
	} else
	    strcpy(newmessagePtr->recipient, mailnames);
    }

    write_post_header(fp, *newmessagePtr);
    xfree(newmessagePtr);
    fflush(fp);

    msgform(filename, 5);
    cprintf("\1a\1c\n");

    switch (get_buffer(fp, mode, &lines)) {
	case 's':
	case 'S':
	    write_post_footer(fp, lines);
	    fclose(fp);
	    return 0;

	case 'a':
	case 'A':
	default:
	    fseek(fp, 0L, SEEK_SET);
	    putc(0, fp);
	    putc(0, fp);
	    fclose(fp);
	    unlink(filename);
    }
    return 2;
}

void
get_newmessage_alias(post_t * const newmessPtr)
{
    cprintf("\1f\1gDo you want to add an aliasname to the post? \1w(\1gy/n\1w) \1a");
    if (yesno() == YES) {
	if ((usersupp->config_flags & CO_USEALIAS) && (strlen(usersupp->alias))) {
	    strcpy(newmessPtr->alias, usersupp->alias);
	} else {
	    cprintf("\1f\1gAlias: ");
	    getline(newmessPtr->alias, L_USERNAME, 1);
	}
    }
    return;
}

int
get_newmessage_replystuff(post_t * const newmessPtr)
{

    FILE *reply_fp;
    char mail_filename[100];
    post_t *post_replying_to;

    if (curr_rm == 1) {
	sprintf(mail_filename, "%s/mail/%ld", getuserdir(usersupp->username), reply_num);
	reply_fp = fopen(mail_filename, "r");
    } else
	reply_fp = fopen(post_to_file(curr_rm, reply_num, NULL), "r");

    if (reply_fp == NULL) {
	cprintf("\1f\1rHmm.. that post no longer seems to exist.\1a\n");
	return -1;
    }
    if ((post_replying_to = (post_t *) xmalloc(sizeof(post_t))) == NULL) {
	fclose(reply_fp);
	return -1;
    }
    memset(post_replying_to, 0, sizeof(post_t));
    if (read_post_header(reply_fp, post_replying_to) == -1) {
	cprintf("\1r\1fTrouble reading the reply header.\1a\n");
	fclose(reply_fp);
	xfree(post_replying_to);
	return -1;
    }
    newmessPtr->ref = reply_num;
    newmessPtr->reftype = post_replying_to->reftype;
    strncpy(newmessPtr->subject, post_replying_to->subject, L_SUBJECT);
    if (post_replying_to->type == MES_AN2 || post_replying_to->type == MES_ANON)
	strcpy(newmessPtr->refauthor, "Anonymous");
    else
	strncpy(newmessPtr->refauthor, post_replying_to->author, L_USERNAME);

    if (strlen(newmessPtr->subject) >= L_SUBJECT)
	strcat(newmessPtr->subject, "");
    if (strlen(newmessPtr->refauthor) >= L_USERNAME)
	strcat(newmessPtr->refauthor, "");

    xfree(post_replying_to);
    return 0;
}

void
get_newmessage_subjectline(post_t * const newmessPtr)
{
    switch (curr_rm) {
	case 55:		/* ascii art */
	    cprintf("\1y\1fArtist/Source:\1c ");
	    break;
	case 38:		/* creative */
	    cprintf("\1y\1fAuthor/Source:\1c ");
	    break;
	default:
	    cprintf("\1f\1ySubject:\1c ");
    }
    getline(newmessPtr->subject, L_SUBJECT, 0);
    return;
}

/*************************************************
* msgform()
*
* flag: 3 -> copy to ClipBoard
*       5 -> show only post header (when posting something new)
*	7 -> put the contents in 'tmpfile'
*************************************************/

int
msgform(const char *mfile, int flag)
{
    char *format, *subject;
    post_t *post;
    FILE *fp;

    if ((fp = xfopen(mfile, "r", FALSE)) == NULL)
	return -1;

    if ((post = (post_t *) xmalloc(sizeof(post_t))) == NULL) {
	fclose(fp);
	return -1;
    }
    memset(post, 0, sizeof(post_t));

    if (read_post_header(fp, post) == -1) {
	cprintf("\1r\1fArgh! can't read header.  Please report this if possible!!\1a\n");
	fclose(fp);
	xfree(post);
	return -1;
    }
#ifdef KNOB_FILTER
    if (EQ(post->author, "Bing") && (usersupp->config_flags & CO_WHAKFILTER)
	&& !(EQ(usersupp->username, "Bing"))) {

	cprintf("\1f\1w[ \1g%s by \1yBing\1g killed \1w]\n", config.message);
	xfree(post);
	fflush(stdout);
	return 0;
    }
#endif

    if ((format = ((char *) xmalloc(sizeof(char) * 500))) == NULL) {
	xfree(post);
	return -1;
    }
    if ((subject = ((char *) xmalloc(sizeof(char) * 100))) == NULL) {
	xfree(post);
	xfree(format);
	return -1;
    }
    strcpy(format, "");
    strcpy(subject, "");

    /* dirty hack, to fix *anon* no-alias posts from dissappearing with CLients */
    cprintf("                            ");

    if (EQ(post->author, usersupp->username))
	curr_post_is_mine = TRUE;
    else
	curr_post_is_mine = FALSE;

    line_total = post->lines;

    format_post_subjectline(post, subject, (flag == 5) ? 1 : 0);
    format_post_formatline(post, subject, format, (flag == 5) ? 1 : 0);
    process_post(post, subject, format, flag);

    line_count = 0;
    fmout2(fp, (flag == 3) ? 1 : (flag == 7) ? 7 : 0);
    increment(0);

    xfree(post);
    xfree(subject);
    xfree(format);
    fclose(fp);
    return 0;
}

int
format_post_subjectline(const post_t * const post, char *subject, int reply)
{
    if (strlen(post->subject)) {
	if (curr_rm == 55 /* ascii art */ )
	    sprintf(subject, "\1f\1yArtist/Source: %s", post->subject);
	else if (curr_rm == 38)	/* creativity */
	    sprintf(subject, "\1f\1y\1fAuthor/Source: %s", post->subject);
	else
	    sprintf(subject, "\1f\1ySubject: %s", post->subject);
    }
    return 0;
}

int
format_post_formatline(const post_t * const post, char *subject, char *format, int reply)
{
    /* 3 main formats, normal, anon, and room_desc */

    /* anon */
    if (post->type == MES_AN2 || post->type == MES_ANON) {
	strcpy(format, "\1f\1gFrom \1b*anonymous*");
	if (strlen(post->alias))
	    sprintf(format + strlen(format), "\1g Alias: '%s'\1a", post->alias);
	if (EQ(usersupp->username, post->author))
	    sprintf(format + strlen(format), "\1w (\1bthis is your post\1w)");

	/* quadinfo */
    } else if (post->type == MES_DESC) {
	sprintf(format, "\1f\1gLast Modified: %s\1f by %s", printdate(post->date, 0), post->author);
	strcpy(subject, "");
	curr_line = 4;

	/* normal */
    } else {
	sprintf(format, "\1f\1g%s\1f ", printdate(post->date, 0));

	/* cast in switch to get rid of compiler whinging */
	switch ((int) (post->type)) {
	    case MES_WIZARD:
	    case MES_SYSOP:
		sprintf(format + strlen(format), "\1gfrom %s%s \1w( %s%s \1w)",
			ADMINCOL, post->author, ADMINCOL, config.admin);
		break;

	    case MES_TECHNICIAN:
		sprintf(format + strlen(format), "\1gfrom %s%s \1w( %s%s \1w)",
			PROGRAMMERCOL, post->author, PROGRAMMERCOL, config.programmer);
		break;

	    case MES_GAMEMASTER:
		sprintf(format + strlen(format), "\1gfrom \1p%s \1w( \1p%s \1w)",
			post->author, "Game Master");
		break;

	    case MES_ROOMAIDE:
		sprintf(format + strlen(format), "\1gfrom %s%s \1w( %s%s \1w)",
		   ROOMAIDECOL, post->author, ROOMAIDECOL, config.roomaide);
		break;

	    case MES_FORCED:
		sprintf(format + strlen(format), "\1gfrom %s \1w(\1b FORCED MESSAGE \1w)",
			post->author);
		break;

	    case MES_SYSTEM:
		sprintf(format + strlen(format), "\1gfrom \1c%s \1w(\1cAutomatic Post\1w)",
			post->author);
		break;

	    default:
		sprintf(format + strlen(format), "\1gfrom \1n%s\1N", post->author);
		break;
	}
	if (strlen(post->recipient)) {
	    sprintf(format + strlen(format), " \1gto %s", post->recipient);
	}
	strcpy(profile_default, post->author);
    }

    if (strlen(post->origroom)
	&& strcasecmp(post->origroom, quickroom.name)
	&& post->type != MES_DESC)
	sprintf(format + strlen(format), " \1gin %s\1w>", post->origroom);

    if (strlen(post->modifier))
	sprintf(format + strlen(format), "\1w[\1g%s\1w] ", post->modifier);

    if (post->moddate)		/* last modification date */
	sprintf(format + strlen(format), "Orig: %s\1f ", printdate(post->moddate, 0));

    return 0;
}

int
process_post(const post_t * const post, const char *subject, const char *format, const int flag)
{
    switch (flag) {
	case 7:		/* write to tmpfile */
	    break;
	case 3:		/* clip post */
	    clip_log("\n");
	    clip_log(format);
	    clip_log(subject);
	    clip_log("\n\1a\1c");
	    break;
	default:
	    cprintf("\n");
	    curr_line = 1;
	    increment(0);
	    cprintf("%s", format);
	    if (post->type == MES_DESC) {
		cprintf("\n");
		show_room_aides();
	    }
	    if (post->ref && (quickroom.flags & QR_SUBJECTLINE) && post->type != MES_DESC) {
		cprintf("\n\1f\1ySubject: \1a\1gRe: \1y%s \1gby %s \1w(\1r#%ld\1w)\1a",
			(strlen(post->subject) == 0) ? "\1w[\1yNo subject\1w]" : post->subject,
		   (post->reftype == MES_ANON || post->reftype == MES_AN2) ?
			"\1b*anonymous*" : post->refauthor, post->ref);
	    } else if (strlen(subject))
		cprintf("\n\1f\1g%s\1a", subject);
	    if (flag == 5)	/* stop, after only displaying header */
		break;
	    cprintf("\1a\1c\n");
	    increment(0);
	    break;
    }
    return 0;
}


int
show_message(int room, long number)
{
    char string[300];
    strcpy(string, post_to_file(room, number, usersupp->username));

    if (!fexists(string))
	return 0;

#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_S);
#endif
    msgform(string, 0);
#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_E);
#endif
    return 1;
}

int
set_read_bounds(long *lower_bound, long *upper_bound)
{
    room_t scratch;

    scratch = read_quad(curr_rm);
    if (curr_rm != 1) {
	*lower_bound = scratch.lowest;
	*upper_bound = scratch.highest;
    } else {			/* don't mung mail */
	*lower_bound = 0;
	*upper_bound = usersupp->mailnum;
    }
    return 0;
}


    /* read menu starts reading messages:
     * number == 0 :      start with lastseen message
     * number < 0  :      start `number' messages back.
     * number > 0  :      start at message `number'
     * direction > 0 :    read forward.
     * direction < 0 :    read backwards.
     */
void
read_menu(long number, int direction)
{

    int read_command, read_position_modified;
    long start, current, temp_lowest, temp_highest;

    if ((start = get_read_start_number(number, direction)) == -6666)
	return;			/* found no new posts */
    if (start < 0)
	start = 0;		/* ugly fix for users getting  bigboote-d */

    set_read_bounds(&temp_lowest, &temp_highest);

/***********  Main Read Loop  ***********/

    for (current = start;
	 (current >= temp_lowest) && (current <= (temp_highest));
	 current += direction) {

	/* see if new posts while we were reading, x-ing, flaming, etc.. */
	if (temp_highest >= current - 3)	/* but only worry about it at last posts */
	    set_read_bounds(&temp_lowest, &temp_highest);

	read_command = show_message(curr_rm, current);

	if (current > usersupp->lastseen[curr_rm])
	    usersupp->lastseen[curr_rm] = current;	/* update lastseen */

	if (read_command == 0)
	    continue;

	read_position_modified = FALSE;

/***********  Main Read Command Loop  ***********/

	while (!read_position_modified) {

	    display_read_prompt(current, direction);	/* show the prompt */
	    are_there_held_xs();	/* check if there are x-es HERE!! */

	    if (usersupp->flags & US_NOPROMPT)	/* dump quad to screen */
		break;

	    read_command = get_single_quiet("1234567890 aAbBcCdDeEfFgGhHiIjJlkKLpPrRqQmMnNsStTvVWwxXyYzZ?!#\006\005\014\018\022\030<>%\":~,.*");

	    read_command = validate_read_command(read_command);		/* priv check */

	    switch (read_command) {

		case -1:	/* we're an asshole, guest, newbie, etc..  and we */
		    break;	/* can't use this command */

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		    nox = 1;
		    express(read_command - 38);
		    break;

		case 'a':
		    cprintf("\1f\1gAgain.\1a\n");
		    current -= direction;
		    read_position_modified = TRUE;
		    break;

		case 'A':
		    toggle_away();
		    cprintf("\n");
		    break;

		case 'b':
		    cprintf("\1f\1gBackwards\1a\n");
		    direction = -1;
		    read_position_modified = TRUE;
		    break;

		case 'B':
		    cprintf("\1f\1gBackwards from the end\1a\n");
		    current = temp_highest + 1;
		    direction = -1;
		    read_position_modified = TRUE;
		    break;

		case 'C':
		    cprintf("\1f\1gCopy this %s.\n", config.message);
		    xcopy_message(current, TRUE);
		    break;

		case 'c':
		    nox = 1;
		    cprintf("\1gPress \1w<\1rshift-c\1w>\1g to access the config menu.\n");
		    express(3);
		    break;

		case '*':
		    clip_post(current);
		    cprintf("\n");
		    break;

		case 'd':
		    cprintf("\1f\1pChannel Config: \1a");
		    chat_subscribe();
		    break;

		case 'D':
		    xdelete_message(current);
		    break;

		case 'e':
		    cprintf("\1f\1gEnter %s.\1a\n", config.message);
		    direction = 1;
		    nox = 1;
		    entmsg(MES_NORMAL, EDIT_NORMAL);
		    break;

		case 'E':
		    cprintf("\1f\1gEnter Editor-%s.\1a\n", config.message);
		    direction = 1;
		    nox = 1;
		    entmsg(MES_NORMAL, EDIT_EDITOR);
		    break;

		case 005:
		    nox = 1;
		    enter_message_with_title();
		    break;

		case 'f':
		case 'F':
		    cprintf("\1f\1gForward.\1a\n");
		    direction = 1;
		    read_position_modified = TRUE;
		    break;

		case '':
		    cprintf("\1f\1gFriends Online.\1a\n");
		    friends_online();
		    break;

		case 'g':
		case 'G':
		    cprintf("\1f\1gGoto.\1a\n");
		    cmdflags &= ~C_ROOMLOCK;
		    storeug(curr_rm);
		    mark_as_read(curr_rm);
		    gotonext();
		    return;

		case 'h':
		case 'H':
		    nox = TRUE;
		    cprintf("\1f\1gHelpfiles.\1a\n\n");
		    help_topics();
		    break;

		case 'i':
		    cprintf("\1f\1g%s Info.\1a\n", config.forum);
		    show_desc(curr_rm);
		    break;

		case 'I':
		    toggle_interbbs();
		    cprintf("\n");
		    break;

		case 'J':
		    cprintf("\1f\1rJump:\nNon Destructive Jump to %s name/number\1w: \1c", config.forum);
		    fflush(stdout);
		    if (!jump(0))
			break;
		    return;

		case 'j':
		    cprintf("\1f\1gJump to %s name/number\1w: \1c", config.forum);
		    fflush(stdout);
		    if (!jump(1))
			break;
		    return;

		case 'k':
		    cprintf("\1f\1gKnown %s\1w:\n\n", config.forum_pl);
		    show_known_rooms(1);
		    break;

		case 'K':
		    cprintf("\1f\1gAll %s\1w:\n\n", config.forum_pl);
		    show_known_rooms(0);
		    break;

		case 'L':
		    cprintf("\1f\1g%s with unread %s.\n", config.forum_pl, config.message_pl);
		    show_known_rooms(2);
		    break;

		case 'l':
		    cprintf("\1f\1gLogout.\1a");
		    if (user_terminate() == TRUE)
			logoff(0);
		    break;

		case 'm':
		    cprintf("\1f\1gMisc: \1a");
		    nox = 1;
		    misc_menu();
		    break;

		case 'M':
		    xcopy_message(current, FALSE);
		    break;

		case 'n':
		case 'N':
		case ' ':
		    cprintf("\1f\1gNext.\1a\n");
		    if (no_new_posts_here(temp_highest, direction, current)) {
			return;
		    }
		    read_position_modified = TRUE;
		    break;

		case '\016':	/* this is ctrl-n  */
		    direction = 1;
		    nox = 1;
		    cprintf("\1f\1rCtrl\1w-\1rN\1w-\1gEnter message.\1a\n");
		    entmsg(MES_NORMAL, EDIT_CTRLD);
		    break;

		case 'p':
		    cprintf("\n");
		    profile_user();
		    cprintf("\n");
		    break;

		case 'P':
		    lookup_anon_post(current);
		    break;

		case 'q':
		    nox = 1;
		    q_menu();
		    break;

		case 'Q':
		    nox = 1;
		    cprintf("\1f\1rAsk a Question.\1a\n");
		    express(1);
		    break;

		case 'r':
		case 'R':
		    direction = 1;
		    nox = 1;
		    cprintf("\1f\1gReply.\1a\n");
		    reply_num = current;
		    entmsg(MES_NORMAL, 5);
		    reply_num = 0;
		    break;

		case 'S':
		    cprintf("\1f\1gSearch.\1a\n");
		    search();
		    break;

		case 's':
		    cprintf("\1f\1gStop.\1a\n");
		    return;

		case 't':
		    cprintf("\1f\1gDate: \1w%s \1f\1w(\1gCET\1w)\1a\n", printdate(time(0), 0));
		    break;

		case 'T':
		    IFSYSOP {
			cprintf("\1f\1r Trash Message.\1a\n");
			reply_num = current;
			move_message_to_trash();
			reply_num = 0;
			read_position_modified = TRUE;
		    } else
			cprintf("\1f\1gDate: \1w%s \1f\1w(\1gCET\1w)\1a\n", printdate(time(0), 0));
		    break;

		case 'v':
		case 'V':
		    nox = TRUE;
		    express(-1);
		    break;

		case 'W':
		    cprintf("\1f\1gShort Wholist.\1a\n");
		    show_online(3);
		    cprintf("\n");
		    break;

		case 'w':
		    cprintf("\1f\1gWhich aliens are online?\1a\n");
		    show_online(1);
		    cprintf("\n");
		    break;

		case 'x':
		    nox = TRUE;
		    cprintf("\1f\1gSend %s %s.\1a\n", config.express, config.x_message);
		    express(0);
		    break;

		case 'X':
		    change_express(0);
		    cprintf("\n");
		    break;

		case 'y':
		case 'Y':
		    cprintf("\1f\1gYell-menu.\1a \n");
		    nox = TRUE;
		    yell_menu();
		    break;

		case 'z':  /* ugly, probably shouldn't be allowed at long */
		case 'Z':  /* prompt */
		    cprintf("\1f\1gZap %s.\1a\n", config.forum);
		    forget();
		    direction = 1;
		    start = get_read_start_number(0, direction);
		    if (start == -6666)
			return;	
		    if (start < 0)
		        start = 0;
		    current = start - 1;
		    set_read_bounds(&temp_lowest, &temp_highest);
		    read_position_modified = TRUE;
		    break;

		case 12:	/* <ctrl-l> */
		case 18:	/* <ctrl-r> */
		    cprintf("c");
		    fflush(stdout);
		    break;

		case 030:	/* ctrl-x for xlog */
		    cprintf("\1f\1gRead X-Log.\1a\n");
		    old_express();
		    break;

		case '?':
		    online_help('l');
/*		    more(MENUDIR "/menu_read", 1); */
		    break;

		case '"':
		    cprintf("\1f\1rQuote %s %ss.\1a\n", config.express, config.x_message);
		    quoted_Xmsgs();
		    break;

		case '%':
		    change_atho(0);
		    break;

		case '#':
		    direction = 1;
		    current = numeric_read(current);
		    break;

		case '!':
		    nox = 1;
		    feeling();	/* the feeling menu */
		    break;

		case '<':
		    cprintf("\1f\1gChange your Friends-list.\1a\n\n");
		    menu_friend(FRIEND);
		    break;

		case '>':
		    cprintf("\1f\1gChange your Enemylist.\1a\n\n");
		    menu_friend(ENEMY);
		    break;

		case ',':
		    show_online(2);
		    break;

		case '.':
		    IFSYSOP
			send_silc();
		    break;

		case '~':
		case ':':
		    nox = 1;
		    cprintf("\1f\1gEmote.\1a\n");
		    express(2);
		    break;

		default:	/* lessee if this gets rid of the input-buffer crash */
		    cprintf("\1f\1rAyee! Let's try that again...\1a\n");
		    read_position_modified = TRUE;
		    break;
	    }			/* switch */
	}			/* while */
    }				/* for */
    return;
}

/***********************************************
* yell()
***********************************************/
void
yell()
{
    int fail;

    more(YELLINFO, 1);

    cprintf("\1f\1gDo you really want to \1w<\1rY\1w>\1gell to the Administrators?\1a ");
    if (yesno() == NO) {
	yell_menu();
	return;
    }
    fail = make_message(temp, "Sysop", MES_NORMAL, 1);
    if (fail) {
	return;
    }
    if (save_message(temp, 2, "Sysop") != 0)
	cprintf("\1f\1rOops! Something went wrong saving your yell.\1a\n");
    if (save_message(temp, 1, usersupp->username) != 0)
	cprintf("\1f\1rOops! Something went wrong saving the yell in your mail.\1a\n");
    unlink(temp);
    usersupp->posted++;
    return;
}

/* delete_mail() *
 * mail a user when post is deleted by QL 
 */
int
delete_mail(int room, int num)
{
    int ch;
    post_t old_post, new_post;
    FILE *old_fp, *new_fp;

    old_fp = xfopen(post_to_file(room, num, NULL), "r", FALSE);
    if (old_fp == NULL) {
	cprintf("\1f\1rCould not read original article.\1a\n");
	return -1;
    }
    new_fp = xfopen(temp, "w", FALSE);
    if (new_fp == NULL) {
	fclose(old_fp);
	cprintf("\1f\1rCould not mail to original author.\1a\n");
	return -1;
    }
    if (read_post_header(old_fp, &old_post) == -1) {
	cprintf("\1r\1fCould not read original article.\1a\n");
	return -1;
    }
    if (check_user(old_post.author) == FALSE) {
	cprintf("\1f\1rOriginal author \1w`\1y%s\1w' \1rdoes not exist. Not mailing a copy.\1a\n", old_post.author);
	return -1;
    }
    cprintf("\1f\1gPreparing mail to %s.\1a\n", old_post.author);

    /* make header for new post */
    strcpy(new_post.author, usersupp->username);
    new_post.type = MES_ROOMAIDE;
    new_post.date = time(0);
    new_post.ref = 0;
    strcpy(new_post.recipient, old_post.author);
    sprintf(new_post.subject, "Deleted message notification.");

    write_post_header(new_fp, new_post);

    /* zut */

    /* read message body and write them to the new file */
    fprintf(new_fp, "\1w ----- deleted message start -----\n\1c");

    /* write message header ---- !!!!!!! */
    while (1) {
	ch = getc(old_fp);
	if (ch == 0 || ch == EOF)
	    break;
	putc(ch, new_fp);
    }
    fclose(old_fp);

    fprintf(new_fp, "\1a\1w ------ deleted message end ------\n\1a");

    write_post_footer(new_fp, old_post.lines + 2);

    fclose(new_fp);
    save_message(temp, 1, new_post.recipient);
    save_message(temp, 1, usersupp->username);
    unlink(temp);
    return 0;
}

void
direct_mail()
{
    int this_is_an_incredibly_long_variable_name_muhahhahha;

    this_is_an_incredibly_long_variable_name_muhahhahha = curr_rm;
    curr_rm = 1;
    nox = 1;
    entmsg(MES_NORMAL, EDIT_NORMAL);
    curr_rm = this_is_an_incredibly_long_variable_name_muhahhahha;
    return;
}

/* this function makes a room totally `unread' , so to speak */
void
reset_lastseen()
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

/*************************************************
* unread_room()
* this functions finds a room with unread messages
* and it returns its number
*************************************************/
int
unread_room()
{
    return mono_sql_uf_unread_room(usersupp->usernum);
}

/*************************************************
* new_quadinfo()
* don't return 1 if the curr_rm is garbagequad
* or yells.
*************************************************/
int
new_quadinfo()
{
    if ((quickroom.roominfo != usersupp->roominfo[curr_rm]) && (curr_rm != 2 || curr_rm != 5)) {
	usersupp->roominfo[curr_rm] = quickroom.roominfo;
	return TRUE;
    }
    return FALSE;
}

void
post_file(const char *fromfile, post_t mesg)
{
    make_auto_message(temp, fromfile, mesg);
    if (save_message(temp, curr_rm, mesg.recipient) != 0)
	cprintf("\1f\1rOops! Something went wrong saving your %s.\1a\n", config.message);
    return;
}

void
search()
{
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

    return;
}

/*  newbie_mark_as_read
 * called in main.c if usersupp->timescalled == 1
 * called BEFORE read_menu()
 * marks all messages on the BBS starting with quad #2 as read except
 * the last number_to_leave_unread */
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

long
get_read_start_number(const long read_number, const int read_direction)
{
    long start_at;
    room_t scratch;

    scratch = read_quad(curr_rm);

    /* mail uses the userfile, NOT he quickroom */
    if (curr_rm == 1) {
	if (usersupp->lastseen[curr_rm] <= usersupp->mailnum)
	    start_at = usersupp->lastseen[curr_rm] + 1;
	else
	    start_at = usersupp->mailnum;

    }
    /* make sure lastseen is something reasonable */
    else {
	start_at = usersupp->lastseen[curr_rm] + 1;
	if (usersupp->lastseen[curr_rm] < scratch.lowest)
	    usersupp->lastseen[curr_rm] = scratch.lowest;
	else if (usersupp->lastseen[curr_rm] > scratch.highest)
	    usersupp->lastseen[curr_rm] = scratch.highest;
    }
#ifdef DEBUG
    cprintf("\ndebug: modified lastseen = %d\n", usersupp->lastseen[curr_rm]);
#endif

    if (read_number > 0)	/* check if we want to read a specific number */
	start_at = read_number;
    else if (read_number < 0)	/* check if we're reading old messages */
	start_at += read_number;

    if (read_direction < 0)
	start_at--;

    /* check if message still exists */
    if (curr_rm != 1) {
	if (start_at < scratch.lowest)
	    start_at = scratch.lowest;
	if (start_at > scratch.highest)
	    start_at = -6666;
    }
    return start_at;
}

void
display_read_prompt(const long current_post, const int direction)
{
    room_t scratch;

    scratch = readquad(curr_rm);
    if (!(current_post == scratch.highest))
	cprintf("\1a\1f\1w[\1%c%s \1g#%ld\1w> (\1g%ld remaining\1w)] ",
		(scratch.flags & QR_PRIVATE) ? 'r' :
		(scratch.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'y',
		scratch.name, current_post,
		(direction > 0) ? (scratch.highest - current_post)
		: (current_post - scratch.lowest));
    else
	cprintf("\1a\1f\1w[\1%c%s \1g#%ld\1w> (\1glast\1w)] ",
		(scratch.flags & QR_PRIVATE) ? 'r' :
		(scratch.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'y',
		scratch.name, current_post);
    if (!(usersupp->flags & US_NOCMDHELP)) {
	cprintf("\n%s\n", LONG_HELPPROMPT);
	cprintf("\1f\1gEnter %s command\1w ->\1c ", config.forum);
    } else
	cprintf("-> \1a");
    return;
}

int
validate_read_command(int read_command)
{
    IFTWIT
    {				/* make nasty users repent for what they have done */
	if (strchr("1234567890!AcCdDeEIMrRqQxXvVwWzZ\005\030!%:~.*&(+\'", read_command))
	    if (!((curr_rm == 13) && (strchr("eE", read_command)))) {
		more(TWITMSG, 1);
		read_command = -1;
	    }
    }
    else
    IFGUEST
    {				/* more bofh functions, different message */
	if (strchr("1234567890!cCdDeEIMmrRqQvVxXyYzZ(%\"\006\005\030!<>:~.*", read_command)) {
	    more(GUESTMSG, 1);
	    read_command = -1;
	}
    }
    else
    IFUNVALID
    {				/* no x related functions for unvalidated users */
	if (strchr("1234567890AcdDeEIrRxXvV+\005\030\014!:~.*", read_command)) {
	    more(UNVALIDMSG, 0);
	    read_command = -1;
	}
    }
    else
    IFDEGRADED
    {				/* remove some functions degraded users have */
	if (strchr("1234567890AcdDeEIrRvVxX(+\005\030!:~.\'*", read_command)) {
	    more(DEGRADEDMSG, 0);
	    read_command = -1;
	}
    }
    if (usersupp->priv & PRIV_DELETED) {
	more("share/messages/deleted_goodbye", 1);
	cprintf("\1f\1g\nPress any key to log off...\1a");
	inkey();
	logoff(0);
    }
    return read_command;

}

/* wrapper for copy and move, copies if parameter 2 is true, otherwise moves */
int
xcopy_message(const long current_post, const int ifcopy)
{
    int tempint;
    char tempstr[100];
    room_t scratch;
    post_t post;
    FILE *fp;

    if (!ifcopy &&
	!((curr_post_is_mine)
	  || (is_ql(usersupp->username, read_quad(curr_rm)))
	  || (usersupp->priv >= PRIV_SYSOP)
	  || (curr_rm == 1)))
	return -1;
    cprintf("\1f\1g%s %s.\n", (ifcopy) ? "Copy" : "Move", config.message);
    cprintf("Enter destination %s name/number: \1c", config.forum);
    strcpy(tempstr, get_name(3));
    tempint = get_room_name(tempstr);
    if (tempint == -2)
	return -1;
    else if (tempint == -1) {
	cprintf("\1f\1rNo such %s.\1a\n", config.forum);
	return -1;
    }
    scratch = read_quad(tempint);
    if (!may_write_room(*usersupp, scratch, tempint)) {
	cprintf("You're not allowed to %s the %s there.\n",
		(ifcopy) ? "copy" : "move", config.message);
	return -1;
    }
    if (ifcopy) {
	sprintf(tempstr, BBSDIR "save/quads/%d/%ld", curr_rm, current_post);
	fp = xfopen(tempstr, "r", FALSE);
	read_post_header(fp, &post);
	fclose(fp);
	if (post.type == MES_ANON || post.type == MES_AN2) {
	    cprintf("\1gYou can't copy anonymous posts.\n");
	    return -1;
	}
    }
    cprintf("\1f\1g%s this %s to `\1y%s\1g'? \1w(\1gy\1w/\1gn\1w)\1c ",
	    (ifcopy) ? "Copy" : "Move", config.message, scratch.name);
    if (yesno() == YES) {
	tempint = (ifcopy) ? copy_message(curr_rm, current_post, tempint, usersupp->username)
	    : move_message(curr_rm, current_post, tempint, usersupp->username);
	if (tempint == 0)
	    cprintf("\1f\1g%s %s.\1a\n", (ifcopy) ? "copied" : "moved", config.message);
	else
	    cprintf("\1f\1rOops, couldn't %s %s.\1a\n",
		    (ifcopy) ? "copied" : "moved", config.message);
	if (ifcopy)
	    log_it("copylog", "%s copied message by %s to %s", usersupp->username,
		   post.author, scratch.name);
    }
    return 0;
}

int
xdelete_message(const long current_post)
{
    int tempint;

    if ((curr_post_is_mine)
	|| (is_ql(usersupp->username, read_quad(curr_rm)))
	|| (usersupp->priv >= PRIV_SYSOP)
	|| (curr_rm == 1)) {

	cprintf("\1f\1rDelete.\1a\n");
	cprintf("\1f\1gAre you sure you want to delete this %s? \1w(\1gy\1w/\1gn\1w)\1a", config.message);
	if (yesno() == TRUE) {
	    if (!curr_post_is_mine && curr_rm != 1)
		delete_mail(curr_rm, current_post);
	    tempint = trash_message(curr_rm, current_post, usersupp->username);
	    if (tempint == 0)
		cprintf("\1f\1g%s deleted.\1a\n", config.message);
	    else
		cprintf("\1f\1rOops, %s could not be deleted.\1a\n", config.message);
	}
    } else
	cprintf("\1f\1rI'm sorry Dave, but I can't do that..\1a\n");
    return 0;
}

void
enter_message_with_title(void)
{
    if (usersupp->priv & (PRIV_TECHNICIAN | PRIV_SYSOP | PRIV_WIZARD)) {
	cprintf("\1f\1rEnter %s with title.\1a\n", config.message);
	fflush(stdout);
	if (usersupp->priv & PRIV_WIZARD)
	    entmsg(MES_WIZARD, EDIT_NORMAL);
	else if (usersupp->priv & PRIV_SYSOP)
	    entmsg(MES_SYSOP, EDIT_NORMAL);
	else
	    entmsg(MES_TECHNICIAN, EDIT_NORMAL);
    }
    return;
}

void
lookup_anon_post(const long current_post)
{
    char tempstr[100];
    room_t scratch;
    post_t post;
    FILE *fp;

    scratch = read_quad(curr_rm);

    if ((usersupp->priv >= PRIV_SYSOP)
	|| is_ql(usersupp->username, scratch)) {

	sprintf(tempstr, BBSDIR "save/quads/%d/%ld", curr_rm, current_post);
	fp = xfopen(tempstr, "r", FALSE);
	read_post_header(fp, &post);
	fclose(fp);

	if (post.type == MES_ANON || post.type == MES_AN2) {
	    scratch = read_quad(curr_rm);
	    cprintf("\n\1f\1w[ \1gPost was by\1w: \1y%s\1w ]\1a\n", post.author);
	    log_sysop_action("looked at anonymous post #%d by %s in %s>", current_post, post.author, scratch.name);
	} else
	    cprintf("\1f\1wNot an anonymous post.\1a\n");
    } else
	cprintf("\n");
    return;
}

long
numeric_read(const long current_post)
{
    room_t scratch;
    char tempstr[12];
    long templong;

    scratch = read_quad(curr_rm);
    cprintf("\1f\1gJump to a specific message number.\1a\n");
    cprintf("\1f\1gWhich message would you like to read? \1a");
    fflush(stdout);
    getline(tempstr, 9, 1);
    templong = atol(tempstr);
    if ((templong < scratch.lowest) || (templong > scratch.highest)) {
	cprintf("\1f\1rThat doesn't seem to be a post number, Dave.\1a\n");
	return current_post;
    } else {
	return templong;
    }
}

void
clip_post(const long current_post)
{
    room_t scratch;
    char tempstr[100];

    scratch = read_quad(curr_rm);

    cprintf("\1f\1gClip this %s.\1a\n", config.message);
    msgform(post_to_file(curr_rm, current_post, usersupp->username), 3);
    sprintf(tempstr, "\1f\1w[\1%c%s\1w> \1g%s #%ld \1w]\n\n",
	    (scratch.flags & QR_PRIVATE) ? 'r' :
	    (scratch.flags & (QR_ANONONLY | QR_ANON2)) ? 'p' : 'g',
	    scratch.name, config.message, current_post);
    clip_log(tempstr);
    return;
}

int
no_new_posts_here(const long highest, const int direction, const long current)
{
    return
	(((curr_rm == 1) && (direction > 0) && (current == usersupp->mailnum)) ||
       ((curr_rm != 1) && (direction > 0) && (current == highest))) ? 1 : 0;
}

void
purge_mail_quad(void)
{
    long i = 0, mail_total;
    char mail_filename[100];

    if (((mail_total = count_mail_messages()) > MAX_USER_MAILS) &&
	(usersupp->priv < PRIV_TECHNICIAN)) {

	while (mail_total - MAX_USER_MAILS > 0) {
	    sprintf(mail_filename, "%s/mail/%ld", getuserdir(usersupp->username), i);
	    if (fexists(mail_filename)) {
		unlink(mail_filename);
		mail_total--;
	    }
	    i++;
	}
    }
    return;
}

long
count_mail_messages(void)
{
    long i;
    char mail_filename[100];
    struct dirent **namelist;

    sprintf(mail_filename, "%s/mail/.", getuserdir(usersupp->username));
    i = scandir(mail_filename, &namelist, 0, alphasort);

    return i -= 2;		/* don't count . and .. */
}

void
x_to_mail(const char *x, char *to_user)
{
    FILE *fp;
    post_t mesg;
    char filename[120], *p;
    int no_of_lines = 5;

    if ((check_user(to_user)) == FALSE)
	return;

    cprintf("\1f\1gSave %s %s to \1y%s\1g's Mail? \1w(\1gy\1w/\1gn\1w) \1c", 
		config.express, config.x_message, to_user);

    if (yesno() == NO)
	return;

    filename[0] = '\0';
    mesg.type = MES_NORMAL;
    strcpy(mesg.author, usersupp->username);
    time(&mesg.date);

    strcpy(mesg.subject, "Saved X message.");

    sprintf(filename, "%s%s%ld", getuserdir(to_user), "/mail/", 
				 get_new_mail_number(to_user));

    if ((fp = xfopen(filename, "a", FALSE)) == NULL) {
	cprintf("\n\n\1r\1fWeird error number 494-FF-A24\nSave failed.\n\1a");
	return;
    }

/* count lines */
    for (p = (char *) x; *p != '\0'; p++)
        if( *p == '\n' || *p == '\r')
            no_of_lines++;

/* write file header */

    fprintf(fp, "%c%c%c", 255, (int) mesg.type, 0);
    fprintf(fp, "L%3d%c", no_of_lines, 0);
    fprintf(fp, "T%ld%c", mesg.date, 0);
    fprintf(fp, "A%s%c", mesg.author, 0);
    fprintf(fp, "S%s%c", mesg.subject, 0);
    putc('M', fp);

/* body of post */

    fprintf(fp, "%s%s%s%s",
	    "\n\1f\1b*** \1gYou logged off while I was sending you this", 
	    " eXpress message \1b***\1a\1c\n\n",
            x, "\n");

/* end of post */
    fprintf(fp, "\n%c", 0);
    fclose(fp);
}
