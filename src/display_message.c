/*
 * $Id$
 *
 * Contains all functions for displaying a message or post.
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"

#include "routines2.h"
#include "input.h"
#include "messages.h"
#include "usertools.h"

#ifdef CLIENTSRC
#include "telnet.h"
#endif

#include "display_message.h"

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
    char filename[L_FILE + 1];
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
	return 1;
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

static char *
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
	/* header_string = m_strcat(header_string, " "); */
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

static char *
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


static char *
format_info(message_header_t * header, char *header_string)
{
    userlist_t *p, *q;
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
		  "\1g\nUser Options\1w: %s%s %s %s",
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

    if ((snprintf(infostring, sizeof(infostring) - 1,
		  "\n\1g%s Category\1w: \1y%s\1a\n\n",
		  config.forum, quad.category)) == -1)
	infostring[sizeof(infostring) - 1] = '\0';
    header_string = m_strcat(header_string, infostring);

    return header_string;
}

static char *
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
		     (usersupp->config_flags & CO_EXPANDHEADER) ? "User" : "",
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
    else if (header->mod_type & MOD_EDIT) {
        if( header->banner_type & ANON_BANNER ) {
            if(strlen(header->alias)) {
	        snprintf(mod_line, sizeof(mod_line) - 1,
		    "\1f\1gOriginally posted\1w: \1gUnknown date \1w[\1r%s: \"%s\"\1w]\1a\n",
		    (usersupp->config_flags & CO_EXPANDHEADER) ? "Edited" : "E", header->alias );
            } else {
	        snprintf(mod_line, sizeof(mod_line) - 1,
		    "\1f\1gOriginally posted\1w: \1gUnknown date \1w[\1r%s: %s %s\1w]\1a\n",
		    (usersupp->config_flags & CO_EXPANDHEADER) ? "Edited" : "E",
                    (usersupp->config_flags & CO_EXPANDHEADER) ? "Anonymous" : "Anon", "User" );
            }
        } else {
	    snprintf(mod_line, sizeof(mod_line) - 1,
		 "\1f\1gOriginally posted\1w: %s \1w[\1r%s: %s\1w]\1a\n",
		 datestring, (usersupp->config_flags & CO_EXPANDHEADER) ? "Edited" : "E",
                 header->modified_by );
        }
    } else
	strcpy(mod_line, "\1f\1rUnknown mod_type!\1a\n");

    mod_line[sizeof(mod_line) - 1] = '\0';

    header_string = m_strcat(header_string, mod_line);
    return header_string;
}

char *
format_content(message_header_t * header, const int forum, char *content_string)
{

    char *body = NULL, *p;
    char filename[L_FILE + 1];
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
    if (quad.flags & QR_GUESSNAME)
	col =  'c';


    if (usersupp->config_flags & CO_EXPANDHEADER &&
	!(usersupp->config_flags & CO_MONOHEADER))
	if (high == num)
	    snprintf(promptstring, sizeof(promptstring) - 1,
		     "\1f\1w[\1%c%s\1w> \1g%s %u of %u \1w(\1glast in this %s\1w)] -> ",
		     col, quad.name, config.message, num,
		     high, config.forum);
	else
	    snprintf(promptstring, sizeof(promptstring) - 1,
		     "\1f\1w[\1%c%s\1w> \1g%s %u of %u \1w(\1g%u remaining\1w)] -> ",
		     col, quad.name, config.message, num,
		     high, (direction > 0) ? high - num : num - low);
    else if (high == num)
	snprintf(promptstring, sizeof(promptstring) - 1,
		 "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1glast\1w)] -> ",
		 col, quad.name, num);
    else
	snprintf(promptstring, sizeof(promptstring) - 1,
		 "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1g%u remaining\1w)] -> ",
	     col, quad.name, num, (direction > 0) ? high - num : num - low);

    promptstring[sizeof(promptstring) - 1] = '\0';

    cprintf("%s", promptstring);
}

/* eof */
