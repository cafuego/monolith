/*
 * $Id$
 *
 * Contains all functions for displaying a message or post.
 */


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#else
#include <asm/mman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

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
#include "routines2.h"
#include "ext.h"
#include "input.h"
#include "messages.h"
#include "usertools.h"

#ifdef CLIENTSRC
#include "telnet.h"
#endif

#define extern
#include "display_message.h"
#undef extern

static char *format_header(message_header_t *, unsigned int, char *);
static char *format_date(message_header_t *, char *);
static char *format_author(message_header_t *, char *);
static char *format_banner(message_header_t *, char *);
static char *format_subject(message_header_t *, char *);
static char *format_content(message_header_t *, const int, char *);
static char *format_reply(message_header_t *, char *);
static char *format_mod_line(message_header_t *, char *);
static char *format_info(message_header_t *, char *);

int
display_message(unsigned int forum, unsigned int num, const unsigned int mode)
{
    message_header_t *header = NULL;
    char *header_string = NULL;
    char filename[L_FILENAME + 1];
    int m_exists;
    room_t quad;

    read_forum( forum, &quad );

    header = (message_header_t *) xmalloc(sizeof(message_header_t));
    header_string = (char *) xmalloc(sizeof(char) * 2);
    strcpy(header_string, "\n");

    if (mode & DISPLAY_INFO)
	info_header_filename(filename, forum);
    else if (forum == MAIL_FORUM)
	mail_header_filename(filename, who_am_i(NULL), num);
    else
	message_header_filename(filename, forum, num);

    m_exists = read_message_header(filename, header);

    if (m_exists == -1) {
	xfree(header_string);
	xfree(header);
	return 0;
    }
    header_string = format_header(header, forum, header_string);
    header_string = format_content(header, forum, header_string);

    if (mode & DISPLAY_2_CLIP)
	message_clip(header_string);
    else if (mode & DISPLAY_2_TEMP_W)
	message_2_temp(header_string, 'w');
    else if (mode & DISPLAY_2_TEMP_A)
	message_2_temp(header_string, 'a');
    else {

#ifdef CLIENTSRC
	putchar(IAC);
	putchar(POST_S);
#endif

	more_string(header_string);

#ifdef CLIENTSRC
	putchar(IAC);
	putchar(POST_E);
#endif
    }

    if (!(header->banner_type & (INFO_BANNER | ANON_BANNER))) 
	strcpy(profile_default, header->author);

    message_reply_name(header->author);
    message_reply_id(header->m_id);

    xfree(header);
    xfree(header_string);
    fflush(stdout);

    return 1;
}

unsigned int
message_reply_id(unsigned int message_id)
{
    static unsigned int reply_id = 0;

    if (!message_id)
	return reply_id;

    reply_id = message_id;
    return 0;
}

char *
message_reply_name(char *message_author)
{
    static char reply_author[L_USERNAME + 1];

    if (message_author == NULL)
	return reply_author;

    strcpy(reply_author, message_author);
    return NULL;
}

char *
format_header(message_header_t * header, unsigned int forum, char *header_string)
{
    unsigned int length, length2;

    if (header->banner_type & INFO_BANNER)
	header_string = format_info(header, header_string);

    else if (usersupp->config_flags & CO_MONOHEADER) {
	if (!(header->banner_type & ANON_BANNER)) {
	    header_string = format_date(header, header_string);
	    header_string = m_strcat(header_string, " ");
	}
	header_string = format_author(header, header_string);
	header_string = format_banner(header, header_string);
	header_string = m_strcat(header_string, "\n");
	length = length2 = strlen(header_string);
	header_string = format_subject(header, header_string);
	if (strlen(header_string) > length2)
	    header_string = m_strcat(header_string, " ");
	header_string = format_reply(header, header_string);
	if (strlen(header_string) > length + 1)
	    header_string = m_strcat(header_string, "\n");
	header_string = format_mod_line(header, header_string);		/* has own "\n" */
    } else {
	header_string = format_author(header, header_string);
	// header_string = m_strcat(header_string, " ");
	header_string = format_banner(header, header_string);
	header_string = m_strcat(header_string, "\n");
	if (!(header->banner_type & ANON_BANNER)) {
	    header_string = format_date(header, header_string);
	    header_string = m_strcat(header_string, "\n");
	}
	length = strlen(header_string);
	header_string = format_subject(header, header_string);
	if (strlen(header_string) > length)
	    header_string = m_strcat(header_string, "\n");
	length = strlen(header_string);

	header_string = format_reply(header, header_string);
	if (strlen(header_string) > length)
	    header_string = m_strcat(header_string, "\n");
	header_string = format_mod_line(header, header_string);		/* has own "\n" */
    }
    header_string = m_strcat(header_string, "\1a\1c");
    return header_string;
}

/* Displays only the message header.  */

void
display_message_header(message_header_t * header)
{

    char *string = NULL;
    unsigned int length;

    string = (char *) xmalloc(2 * sizeof(char));
    strcpy(string, "\n");

    if (header->banner_type & INFO_BANNER)
	string = format_info(header, string);
    else if (usersupp->config_flags & CO_MONOHEADER) {
	if (!(header->banner_type & ANON_BANNER)) {
	    string = format_date(header, string);
	    string = m_strcat(string, " ");
	}
	string = format_author(header, string);
	string = format_banner(header, string);
	string = m_strcat(string, "\n");
	length = (strlen(string));
	string = format_subject(header, string);
	if (strlen(string) > length)
	    string = m_strcat(string, "\n");
    } else {
	string = format_author(header, string);
	string = format_banner(header, string);
	string = m_strcat(string, "\n");
	if (!(header->banner_type & ANON_BANNER)) {
	    string = format_date(header, string);
	    string = m_strcat(string, "\n");
	}
	length = (strlen(string));
	string = format_subject(header, string);
	if (strlen(string) > length)
	    string = m_strcat(string, "\n");
    }
    string = m_strcat(string, "\1a\1c");
    more_string(string);
    xfree(string);

    return;
}

char *
format_date(message_header_t * header, char *header_string)
{

    struct tm *tp;
    char fmt_date[120];
    static char datestring[200];

    tp = localtime(&header->date);

    if (header->banner_type & INFO_BANNER)
	strcpy(datestring, "\1f\1gLast Modified\1w:\1g ");
    else {
	if (usersupp->config_flags & CO_EXPANDHEADER &&
	    !(usersupp->config_flags & CO_MONOHEADER))
	    strcpy(datestring, "\1f\1gEntered\1w: ");
	else
	    strcpy(datestring, "\1f\1g");
    }

    if (usersupp->config_flags & CO_MONOHEADER)
	strftime(fmt_date, sizeof(fmt_date) - 1,
		 "\1g%a, %b %d %H:%M:%S %Z %Y\1a", tp);

    else if (usersupp->config_flags & CO_LONGDATE)
	if (usersupp->config_flags & CO_EUROPEANDATE)
	    strftime(fmt_date, sizeof(fmt_date) - 1,
		     "\1g%A, %d %B %H:%M:%S %Z %Y\1a", tp);
	else
	    strftime(fmt_date, sizeof(fmt_date) - 1,
		     "\1g%A, %B %d %I:%M:%S %p %Z %Y\1a", tp);
    else if (usersupp->config_flags & CO_EUROPEANDATE)
	strftime(fmt_date, sizeof(fmt_date) - 1,
		 "\1g%a, %d %b %H:%M:%S %Z %Y\1a", tp);
    else
	strftime(fmt_date, sizeof(fmt_date) - 1,
		 "\1g%a, %b %d %I:%M:%S %p %Z %Y\1a", tp);

    strcat(datestring, fmt_date);
    strcat(datestring, "");

    header_string = m_strcat(header_string, datestring);
    return header_string;
}


char *
format_info(message_header_t * header, char *header_string)
{
#define SQL_QL_LIST

#ifdef SQL_QL_LIST
    userlist_t *p, *q;
#else
    int i;
#endif
    room_t quad;
    char infostring[100], c;

    header_string = format_date(header, header_string);
    header_string = format_author(header, header_string);

    read_forum( header->f_id, &quad );

    if (quad.flags & QR_PRIVATE)
	c = 'r';
    else
	c = (quad.flags & (QR_ANONONLY | QR_ALIASNAME | QR_ANON2)) ? 'p' : 'y';

    if ((snprintf(infostring, sizeof(infostring) - 1,
		  "\1f\1g%s Name\1w:\1%c %s\n\1gAccess Type\1w: %s",
		  config.forum, c, quad.name,
		  (c == 'r') ? "\1rInvite Only\1g" : "\1gPublic")) == -1)
	infostring[sizeof(infostring) - 1] = '\0';
    header_string = m_strcat(header_string, infostring);

    if ((snprintf(infostring, sizeof(infostring) - 1,
		  "\1g\n%s Options\1w: %s%s %s %s", config.user,
		  (c == 'p') ? ((quad.flags & QR_ANON2) ?
			"\1bAnonymous Option\1g " : "\1bAnonymous Only\1g ")
		  : "",
		  (c == 'p') ?
		  ((quad.flags & QR_ALIASNAME) ? "\1b(aliased)\1g" : "")
		  : "\1gNormal", config.message,
		  (quad.flags & QR_SUBJECTLINE) ? "with \1ySubjectline" : ""
	 )) == -1)
	infostring[sizeof(infostring) - 1] = '\0';
    header_string = m_strcat(header_string, infostring);

#ifdef SQL_QL_LIST
    if ((mono_sql_uf_list_hosts_by_forum(header->f_id, &p)) == -1)
	header_string = m_strcat(header_string, "\1f\1r\nError loading hostlist in format_info()\n\1a");
    else {
	if ((snprintf(infostring, sizeof(infostring) - 1,
		      "\n\1f\1g%s%s%s", config.roomaide,
		      "(s)", "\1w:\1r ")) == -1)
	    infostring[sizeof(infostring) - 1] = '\0';
	header_string = m_strcat(header_string, infostring);
	if (!p)
	    header_string = m_strcat(header_string, "Sysop");
	else {
	    q = p;
	    while (q) {
		strcpy(infostring, q->name);
		if (q->next)
		    strcat(infostring, ", ");
		q = q->next;
		header_string = m_strcat(header_string, infostring);
	    }
	    dest_userlist(p);
	}
    }
#else
    snprintf(infostring, sizeof(infostring) - 1,
	     "\n\1f\1g%s%s", config.roomaide, "(s)\1w:\1r ");
    header_string = m_strcat(header_string, infostring);
    for (i = 0; i < NO_OF_QLS; i++) {
	strcpy(infostring, "");
	if (strlen(quad.qls[i]))
	    sprintf(infostring, ", %s", quad.qls[i]);
	else if (!i && !strlen(quad.qls[i]))
	    strcpy(infostring, "Sysop");
	if (strlen(infostring))
	    header_string = m_strcat(header_string, infostring);
    }

#endif

    if ((snprintf(infostring, sizeof(infostring) - 1,
		  "\n\1g%s Category\1w: \1y%s\1a\n\n",
		  config.forum, quad.category)) == -1)
	infostring[sizeof(infostring) - 1] = '\0';
    header_string = m_strcat(header_string, infostring);

    return header_string;
}

char *
format_author(message_header_t * header, char *header_string)
{

    static char authorstring[150];

    strcpy(authorstring, "");

    if (usersupp->config_flags & CO_MONOHEADER) {

	if (header->banner_type & ANON_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom \1b*anonymous*%s%s%s%s%s\1a",
	      (strlen(header->alias)) ? " \1gAlias: \"" : "",
		     header->alias,
		     (strlen(header->alias)) ? "\"" : "",
		     (!strcmp(header->author, usersupp->username)) ?
		     " \1w(\1bthis is your post\1w)" : "",
		     "                         ");  
		/* hack to fix ^^^^ client dissappearing post bug */

	if (header->banner_type & EMP_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1w%s\1a", header->author);

	if (header->banner_type & ADMIN_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1w%s\1a", header->author);

	if (header->banner_type & (SYSOP_BANNER | SYSTEM_BANNER))
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1w%s\1a", header->author);

	if (header->banner_type & TECH_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1b%s\1a", header->author);

	if (header->banner_type & QL_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1r%s\1a", header->author);

	if (header->banner_type & FORCED_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1r%s%s\1a",
		     header->author, "\1w  (\1rKickout Notification\1w)");

	if (header->banner_type & INFO_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1r by %s\n\1a", header->author);

	if (header->banner_type & NO_BANNER || !strlen(authorstring))
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gfrom \1y\1n%s\1N\1a", header->author);
    } else {

	if (header->banner_type & ANON_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom \1b*Anonymous%s%s*%s%s%s%s%s\1a",
		     (usersupp->config_flags & CO_EXPANDHEADER) ? " " : "",
		     (usersupp->config_flags & CO_EXPANDHEADER) ? config.user : "",
	      (strlen(header->alias)) ? " \1gAlias: \"" : "",
		     header->alias,
		     (strlen(header->alias)) ? "\"" : "",
		     (!strcmp(header->author, usersupp->username)) ?
		     " \1w(\1bthis is your post\1w)" : "", 
		     "                         ");
		/* hack to fix ^^^^ client dissappearing post bug */

	if (header->banner_type & EMP_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1w%s\1a",
		     (usersupp->config_flags & CO_EXPANDHEADER) ? ":" : "",
		     header->author);

	if (header->banner_type & ADMIN_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1w%s\1a",
		   (usersupp->config_flags & CO_EXPANDHEADER) ? "\1w:" : "",
		     header->author);

	if (header->banner_type & (SYSOP_BANNER | SYSTEM_BANNER))
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1w%s\1a",
		   (usersupp->config_flags & CO_EXPANDHEADER) ? "\1w:" : "",
		     header->author);

	if (header->banner_type & TECH_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1b%s\1a",
		     (usersupp->config_flags & CO_EXPANDHEADER) ? ":" : "",
		     header->author);

	if (header->banner_type & QL_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1r%s\1a",
		   (usersupp->config_flags & CO_EXPANDHEADER) ? "\1w:" : "",
		     header->author);

	if (header->banner_type & FORCED_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1r%s%s\1a",
		   (usersupp->config_flags & CO_EXPANDHEADER) ? "\1w:" : "",
		     header->author, "\1w  (\1rKickout Notification\1w)");

	if (header->banner_type & INFO_BANNER)
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1r by %s\n\1a", header->author);

	if (header->banner_type & NO_BANNER || !strlen(authorstring))
	    snprintf(authorstring, sizeof(authorstring) - 1,
		     "\1f\1gFrom%s \1y\1n%s\1N\1a",
		   (usersupp->config_flags & CO_EXPANDHEADER) ? "\1w:" : "",
		     header->author);
    }

    authorstring[sizeof(authorstring) - 1] = '\0';

    header_string = m_strcat(header_string, authorstring);
    return header_string;
}

char *
format_banner(message_header_t * header, char *header_string)
{

    char mesg_banner[100 + L_RECIPIENT];

    if (!header->banner_type)
	return header_string;

    strcpy(mesg_banner, "");

    if (header->banner_type & EMP_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( %s ) \1a", config.wizard);

    } else if (header->banner_type & ADMIN_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1w%s \1w) \1a", config.admin);

    } else if (header->banner_type & SYSOP_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1w%s \1w) \1a", config.sysop);

    } else if (header->banner_type & TECH_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1b%s \1w) \1a", config.programmer);

    } else if (header->banner_type & QL_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1r%s \1w) \1a", config.roomaide);

    } else if (header->banner_type & SYSTEM_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1pSystem Message \1w) \1a");

    } else if (header->banner_type & CUSTOM_BANNER) {
	snprintf(mesg_banner, sizeof(mesg_banner) - L_RECIPIENT - 10,
		 "\1f\1w ( \1g%s \1w) \1a", header->banner);
    }
    if (header->banner_type & MAIL_BANNER) {
	strcat(mesg_banner, " \1f\1gto \1y");
	strcat(mesg_banner, header->recipient);
	strcat(mesg_banner, "\1a");
    }
    if (header->banner_type & YELL_BANNER)
	strcat(mesg_banner, " \1f\1gto \1ySysop\1a");

    mesg_banner[sizeof(mesg_banner) - 1] = '\0';

    if (strlen(mesg_banner))
	header_string = m_strcat(header_string, mesg_banner);

    return header_string;
}

char *
format_subject(message_header_t * header, char *header_string)
{

    char mesg_subject[100];

    if (!strlen(header->subject))
	return header_string;

    strcpy(mesg_subject, "");

    if (!strlen(header->subject) && usersupp->config_flags & CO_EXPANDHEADER &&
	!(usersupp->config_flags & CO_MONOHEADER))
	strcpy(mesg_subject, "\1f\1gSubject\1w: \1w[\1yno subject\1w]");

    else if (strlen(header->subject))
	snprintf(mesg_subject, sizeof(mesg_subject) - 1,
		 "\1f\1gSubject\1w: \1y%s\1a", header->subject);

    mesg_subject[sizeof(mesg_subject) - 1] = '\0';

    header_string = m_strcat(header_string, mesg_subject);
    return header_string;
}

char *
format_reply(message_header_t * header, char *header_string)
{
    char mesg_reply[100];

    if (!header->reply_m_id)
	return header_string;

    if (usersupp->config_flags & CO_MONOHEADER)
	snprintf(mesg_reply, sizeof(mesg_reply) - 1,
		 "\1f\1gby %s \1w(\1r#%u\1w)",
		 header->reply_to_author,
		 header->reply_m_id);
    else
	snprintf(mesg_reply, sizeof(mesg_reply) - 1,
		 "\1f\1g%s\1w: \1g%s%s#%u by \1y%s",
		 (usersupp->config_flags & CO_EXPANDHEADER) ? "Reply" : "Re",
	   (usersupp->config_flags & CO_EXPANDHEADER) ? config.message : "",
		 (usersupp->config_flags & CO_EXPANDHEADER) ? " " : "",
		 header->reply_m_id,
		 header->reply_to_author);

    mesg_reply[sizeof(mesg_reply) - 1] = '\0';

    header_string = m_strcat(header_string, mesg_reply);
    return header_string;
}

char *
format_mod_line(message_header_t * header, char *header_string)
{
    char mod_line[150];
    char datestring[50];
    struct tm *tp;

    if (!header->orig_m_id)
	return header_string;

    tp = localtime(&header->orig_date);

    if (usersupp->config_flags & CO_EUROPEANDATE)
	strftime(datestring, sizeof(datestring) - 1,
		 "\1g%a %d\1w/\1g%m\1w/\1g%Y %H:%M", tp);
    else
	strftime(datestring, sizeof(datestring) - 1,
		 "\1g%a %m\1w/\1g%d\1w/\1g%Y %I:%M %p", tp);

    if (header->mod_type & MOD_MOVE)
	snprintf(mod_line, sizeof(mod_line) - 1,
		 "\1f\1gOrigin\1w: %s \1w[\1r#%u\1w]\1g in \1y%s \1w[\1r%s: %s\1w]\1a\n",
		 datestring, header->orig_m_id, header->orig_forum,
		 (usersupp->config_flags & CO_EXPANDHEADER) ? "Moved" : "M",
		 header->modified_by);
    else if (header->mod_type & MOD_COPY)
	snprintf(mod_line, sizeof(mod_line) - 1,
		 "\1f\1gOrigin\1w: %s \1w[\1r#%u\1w]\1g in \1y%s \1w[\1r%s: %s\1w]\1a\n",
		 datestring, header->orig_m_id, header->orig_forum,
		 (usersupp->config_flags & CO_EXPANDHEADER) ? "Copied" : "C",
		 header->modified_by);
    else
	strcpy(mod_line, "\1f\1rUnknown mod_type!\1a\n");

    mod_line[sizeof(mod_line) - 1] = '\0';

    header_string = m_strcat(header_string, mod_line);
    return header_string;
}

char *
format_content(message_header_t * header, const int forum, char *content_string)
{

    char *body = NULL, *p;
    char filename[L_FILENAME + 1];
    FILE *fp;
    size_t filesize;

    body = (char *) xmalloc(5 * sizeof(char));

    if (usersupp->config_flags & CO_NEATMESSAGES)
	strcpy(body, "\n\1c");
    else
	strcpy(body, "\1c");

    if (header->banner_type & INFO_BANNER)	/* do this first! */
	info_filename(filename, header->f_id);
    else if (forum == MAIL_FORUM)
	mail_filename(filename, who_am_i(NULL), header->m_id);
    else
	message_filename(filename, header->f_id, header->m_id);

    filesize = get_filesize(filename);

    if (!filesize) {
	content_string = m_strcat(content_string, "\n\1f\1rget_filesize() returned 0\n\n");
	log_it("sqlpost", "file %s: get_filesize() returned 0 size", message_filename);
	xfree(body);
	return content_string;
    }
    body = (char *) realloc(body, filesize + strlen(body) + 3);
    for (p = body; *p != '\0'; p++) ;

    if ((fp = xfopen(filename, "r", FALSE)) != NULL) {
	for (; !feof(fp); p++)
	    *p = fgetc(fp);
	fclose(fp);
    }
    *(--p) = '\0';

    strcat(body, (usersupp->config_flags & CO_NEATMESSAGES) ? "\n" : "");
    content_string = m_strcat(content_string, body);
    xfree(body);
    return content_string;
}


void
show_long_prompt(const unsigned int forum, const unsigned int num, const int direction)
{
    char promptstring[200];
    char col = 'y';
    room_t quad;
    int high, low;

    read_forum( forum, &quad );

    high = (forum == MAIL_FORUM) ? usersupp->mailnum : quad.highest;
    low = (forum == MAIL_FORUM) ? 1 : quad.lowest;

    
    if (quad.flags & (QR_ANONONLY | QR_ANON2 | QR_ALIASNAME))
	col = 'p';
    if (quad.flags & QR_PRIVATE) 
	col =  'r';

    if (usersupp->config_flags & CO_EXPANDHEADER &&
	!(usersupp->config_flags & CO_MONOHEADER))
	if (high == num)
	    snprintf(promptstring, sizeof(promptstring) - 1,
		     "\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1glast in this %s\1w)] -> ",
		     col, quad.name, config.message, num,
		     high, config.forum);
	else
	    snprintf(promptstring, sizeof(promptstring) - 1,
		     "\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1g%lu remaining\1w)] -> ",
		     col, quad.name, config.message, num,
		     high, (direction > 0) ? high - num : num - low);
    else if (high == num)
	snprintf(promptstring, sizeof(promptstring) - 1,
		 "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1glast\1w)] -> ",
		 col, quad.name, num);
    else
	snprintf(promptstring, sizeof(promptstring) - 1,
		 "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1g%lu remaining\1w)] -> ",
	     col, quad.name, num, (direction > 0) ? high - num : num - low);

    promptstring[sizeof(promptstring) - 1] = '\0';

    cprintf("%s", promptstring);
}

#ifdef MERGE_CODE_FOR_THE_RECORD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#else
#include <asm/mman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

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
#include "routines2.h"
#include "sql_forum.h"
#include "sql_message.h"
#include "ext.h"
#include "display_message.h"
#include "input.h"

#ifdef CLIENTSRC
#include "telnet.h"
#endif

static char *format_message(message_t * message);
static int format_header(message_t * message, char **string);
static int format_date(message_t * message, char **string);
static int format_username(message_t * message, char **string);
static int format_title(message_t * message, char **string);
static int format_subject(message_t * message, char **string);
static int format_content(message_t * message, char **string);
static int format_footer(message_t * message, char **string);
static int get_message_type(const char *type);
static int get_message_priv(const char *priv);

void
display_message(message_t * message)
{
    char *string = NULL;

    /*
     * Don't show deleted messages, but show warning if the user wants it...
     */
    if (message->deleted == 'y') {
	if (usersupp->priv < PRIV_SYSOP) {
	    if (usersupp->config_flags & CO_DELETEDINFO)
		cprintf("\n\1f\1w[\1rDeleted %s.\1w]\1a\n", config.message);
	    return;
	} else {
	    /*
	     * For admins, the whole message is shown if DELETEDINFO is
	     * not set.
	     */
	    if (!(usersupp->config_flags & CO_DELETEDINFO)) {
		cprintf("\n\1f\1w[\1rDeleted %s.\1w]\1a\n", config.message);
		return;
	    }
	}
    }
    cprintf("\n\1f\1wDEBUG: Rating: ");
    if (message->score < 0)
	cprintf("\1r");
    else if (message->score < 2)
	cprintf("\1a\1y");
    else if (message->score < 4)
	cprintf("\1y");
    else
	cprintf("\1g");
    printf("%.3f", message->score);
    fflush(stdout);

    string = format_message(message);

#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_S);
#endif

    more_string(string);

#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_E);
#endif

    xfree(string);
    fflush(stdout);

    return;
}

static char *
format_message(message_t * message)
{
    char *string = NULL;

    /*
     * malloc() here and realloc in all following functions.
     */
    string = (char *) xmalloc(sizeof(char));
    memset(string, 0, sizeof(string));

    (void) format_header(message, &string);
    (void) format_content(message, &string);
    (void) format_footer(message, &string);

    return string;

}

/*
 * Displays header as show when entering a new message.
 */
void
display_header(message_t * message)
{

    char *string = NULL;

    string = (char *) xmalloc(sizeof(char));
    memset(string, 0, sizeof(string));

    (void) format_username(message, &string);
    (void) format_title(message, &string);

    more_string(string);
    xfree(string);

    return;
}

/*
 * Appends message header to string.
 * This is a nasty looking thing, but cleaning up is a later worry <heh>
 */
static int
format_header(message_t * message, char **string)
{

    /*
     * Takes up 2-3 lines for a post header, but looks prettier.
     * Small header is Ye Olde Format. (1 line).
     */

    if (usersupp->config_flags & CO_EXPANDHEADER) {
	(void) format_date(message, string);
	(void) format_username(message, string);
	(void) format_title(message, string);
	(void) format_admininfo(message, string);
    } else {
	(void) format_username(message, string);
	(void) format_title(message, string);
	(void) format_date(message, string);
	(void) format_admininfo(message, string);
    }

    (void) format_subject(message, string);

    return 0;
}

/*
 * Puts formatted date into string according to locale rules.
 */
static int
format_date(message_t * message, char **string)
{

    struct tm *tp;
    char fmt_date[1000];

    tp = localtime(&message->date);

    if (usersupp->config_flags & CO_LONGDATE) {
	if (usersupp->config_flags & CO_EUROPEANDATE) {
	    if (usersupp->config_flags & CO_EXPANDHEADER) {
		strftime(fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %d %B, %Y \1w(\1g%H:%M\1w)\1a", tp);
	    } else {
		strftime(fmt_date, 1000, " \1f\1g%A %d %B, %Y \1w(\1g%H:%M\1w)\1a", tp);
	    }
	} else {
	    if (usersupp->config_flags & CO_EXPANDHEADER) {
		strftime(fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %B %d, %Y \1w(\1g%I:%M %p\1w)\1a", tp);
	    } else {
		strftime(fmt_date, 1000, " \1f\1g%A %B %D, %Y \1w(\1g%I:%M %p\1w)\1a", tp);
	    }
	}
    } else {
	if (usersupp->config_flags & CO_EUROPEANDATE) {
	    if (usersupp->config_flags & CO_EXPANDHEADER) {
		strftime(fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %d %B, %Y \1w(\1g%H:%M\1w)\1a", tp);
	    } else {
		strftime(fmt_date, 1000, " \1f\1g%A %d %B, %Y \1w(\1g%H:%M\1w)\1a", tp);
	    }
	} else {
	    if (usersupp->config_flags & CO_EXPANDHEADER) {
		strftime(fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %B %d, %Y \1w(\1g%I:%M %p\1w)\1a", tp);
	    } else {
		strftime(fmt_date, 1000, " \1f\1g%A %B %D, %Y \1w(\1g%I:%M %p\1w)\1a", tp);
	    }
	}
    }

    fmt_date[strlen(fmt_date)] = '\0';

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_date));
    strcat(*string, fmt_date);

    return 0;
}

static int
format_username(message_t * message, char **string)
{

    char fmt_username[100];
    int type = 0, priv = 0;

    /*
     * Convert type and priv to something useful.
     */
    type = get_message_type(message->type);
    priv = get_message_priv(message->priv);

    if (check_user(message->a_name) == FALSE) {
	if (usersupp->config_flags & CO_EXPANDHEADER)
	    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1rDeleted %s\1a", config.user);
	else
	    sprintf(fmt_username, "\n\1f\1gFrom \1rDeleted %s\1a", config.user);
    } else {

	switch (type) {

	    case MES_ANON:
		if (usersupp->config_flags & CO_EXPANDHEADER)
		    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1bAnonymous %s%s\1a"
			    ,config.user
			    ,EQ(message->a_name, usersupp->username) ? " \1w(\1bthis is your post\1w)" : "");
		else
		    sprintf(fmt_username, "\n\1f\1gFrom \1bAnon %s\1a"
			    ,EQ(message->a_name, usersupp->username) ? " \1w(\1bthis is your post\1w)" : "");
		break;

	    case MES_AN2:
		if (usersupp->config_flags & CO_EXPANDHEADER)
		    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1bAnonymous %s \1w`\1b%s\1w'%s\1a"
			    ,config.user, message->alias
			    ,EQ(message->a_name, usersupp->username) ? " \1w(\1bthis is your post\1w)" : "");
		else
		    sprintf(fmt_username, "\n\1f\1gFrom \1bAnon \1w`\1b%s\1w'%s\1a"
			    ,message->alias
			    ,EQ(message->a_name, usersupp->username) ? " \1w(\1bthis is your post\1w)" : "");
		break;

	    default:
		switch (priv) {
		    case MES_WIZARD:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1w%s\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1w%s\1a", message->a_name);
			break;
		    case MES_SYSOP:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\1f\1gFrom\1w: \1p%s\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1p%s\1a", message->a_name);
			break;
		    case MES_TECHNICIAN:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1b%s\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1b%s\1a", message->a_name);
			break;
		    case MES_ROOMAIDE:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1r%s\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1r%s\1a", message->a_name);
			break;
		    case MES_FORCED:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1y%s \1w(\1rFORCED MESSAGE\1w)\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1y%s \1w(\1rFORCED MESSAGE\1w)\1a", message->a_name);
			break;
		    case MES_NORMAL:
		    default:
			if (usersupp->config_flags & CO_EXPANDHEADER)
			    sprintf(fmt_username, "\n\1f\1gFrom\1w: \1y\1n%s\1N\1a", message->a_name);
			else
			    sprintf(fmt_username, "\n\1f\1gFrom \1y\1n%s\1N\1a", message->a_name);
			break;
		}
		break;
	}
    }
    fmt_username[strlen(fmt_username)] = '\0';

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_username));
    strcat(*string, fmt_username);

    return 0;
}

static int
format_title(message_t * message, char **string)
{

    char fmt_title[100];
    int priv = 0;

    priv = get_message_priv(message->priv);

    switch (priv) {
	case MES_WIZARD:
	    sprintf(fmt_title, " \1f\1w( %s )\1a", config.wizard);
	    break;

	case MES_SYSOP:
	    sprintf(fmt_title, " \1f\1w( \1p%s \1w)\1a", config.sysop);
	    break;

	case MES_TECHNICIAN:
	    sprintf(fmt_title, " \1f\1w( \1b%s \1w)\1a", config.programmer);
	    break;

	case MES_ROOMAIDE:
	    sprintf(fmt_title, " \1f\1w( \1r%s \1w)\1a", config.roomaide);
	    break;

	default:
	    return -1;
	    break;

    }
    fmt_title[strlen(fmt_title)] = '\0';

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_title));
    strcat(*string, fmt_title);

    return 0;

}

static int
format_admininfo(message_t * message, char **string)
{
    char fmt_admin[100];

    if (usersupp->priv < PRIV_SYSOP)
	return -1;

    strcpy(fmt_admin, "");

    if (message->deleted == 'y')
	sprintf(fmt_admin, " \1f\1w(\1rDeleted %s\1w)", config.message);

    fmt_admin[strlen(fmt_admin)] = '\0';

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_admin));
    strcat(*string, fmt_admin);

    return 0;

}

static int
format_subject(message_t * message, char **string)
{

    char fmt_subject[100];

    if (((message->subject == NULL) || (strlen(message->subject) == 0) || (EQ(message->subject, "(null)"))) && (usersupp->config_flags & CO_EXPANDHEADER))
	sprintf(fmt_subject, "\n\1f\1gSubject\1w: \1yNo subject.\n");
    else if ((message->subject != NULL) || (strlen(message->subject) > 0))
	sprintf(fmt_subject, "\n\1f\1gSubject\1w: \1y%s\1a\n", message->subject);
    else
	sprintf(fmt_subject, "\n");

    fmt_subject[strlen(fmt_subject)] = '\0';

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_subject));
    strcat(*string, fmt_subject);

    return 0;

}

/*
 * Note that the return values in here aren't used yes.. but they should.
 */
static int
format_content(message_t * message, char **string)
{

    char *content = NULL;
    char filename[100];

    /* Find message filename. */
    sprintf(filename, "%s", mono_sql_mes_make_file(message->forum, message->num));

    /* Empty lines around content? */
    if (usersupp->config_flags & CO_NEATMESSAGES) {
	*string = (char *) xrealloc(*string, strlen(*string) + strlen("\n"));
	strcat(*string, "\n");
    }
    /* Set content colour, based on score (TODO) */
    *string = (char *) xrealloc(*string, strlen(*string) + strlen("\1a\1x"));
    if (message->score > 1.99)
	strcat(*string, "\1a\1w");
    else
	strcat(*string, "\1a\1c");

    /*
     * Get file contents into mem.
     */
    if ((content = map_file(filename)) == NULL) {
	content = (char *) xmalloc(100);
	sprintf(content, "%s\n", "\1f\1rAIIEE! Real nasty error: message file missing!");
    }
    /* Append actual message content. */
    *string = (char *) xrealloc(*string, strlen(*string) + strlen(content));
    strcat(*string, content);
    (void) xfree(content);

    if (usersupp->config_flags & CO_NEATMESSAGES) {
	*string = (char *) xrealloc(*string, strlen(*string) + strlen("\n"));
	strcat(*string, "\n");
    }
    return 0;

}

static int
format_footer(message_t * message, char **string)
{
    char fmt_footer[200], col;

    if ((message->f_flags & QR_ANONONLY) || (message->f_flags & QR_ANON2) || (message->f_flags & QR_ALIASNAME))
	col = 'p';
    else if (message->f_flags & QR_PRIVATE)
	col = 'r';
    else
	col = 'y';

    if (usersupp->config_flags & CO_EXPANDHEADER)
	if (message->f_remaining > 0)
	    sprintf(fmt_footer, "\n\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1g%lu remaining\1w)] -> "
		    ,col, message->f_name, config.message, message->num, message->f_highest, message->f_remaining);
	else
	    sprintf(fmt_footer, "\n\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1glast in this %s\1w)] -> "
		    ,col, message->f_name, config.message, message->num, message->f_highest, config.forum);
    else if (message->f_remaining > 0)
	sprintf(fmt_footer, "\n\1f\1w[\1%c%s\1w> \1g#%u \1w(\1g%lu remaining\1w)] -> "
		,col, message->f_name, message->num, message->f_remaining);
    else
	sprintf(fmt_footer, "\n\1f\1w[\1%c%s\1w> \1g#%u \1w(\1glast\1w)] -> "
		,col, message->f_name, message->num);

    *string = (char *) xrealloc(*string, strlen(*string) + strlen(fmt_footer));
    strcat(*string, fmt_footer);

    return 0;
}

static int
get_message_type(const char *type)
{
    if (EQ(type, "anon"))
	return MES_ANON;
    else if (EQ(type, "alias"))
	return MES_AN2;
    else
	return MES_NORMAL;
}

static int
get_message_priv(const char *priv)
{
    if (EQ(priv, "emp"))
	return MES_WIZARD;
    else if (EQ(priv, "sysop"))
	return MES_SYSOP;
    else if (EQ(priv, "tech"))
	return MES_TECHNICIAN;
    else if (EQ(priv, "host"))
	return MES_ROOMAIDE;
    else if (EQ(priv, "normal"))
	return MES_NORMAL;
    else
	return MES_NORMAL;
}

#endif
/* eof */
