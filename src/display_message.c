/*
 * $Id$
 *
 * Contains all functions for displaying a message or post.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"
#include "routines2.h"
#include "sql_forum.h"
#include "sql_message.h"
#include "ext.h"
#include "display_message.h"

#ifdef CLIENTSRC
  #include "telnet.h"
#endif

static char * format_message(message_t * message, unsigned int forum);
static int format_header(message_t *message, char **string );
static int format_date(unsigned int locale, message_t *message, char **string);
static int format_username(message_t *message, char **string );
static int format_title(message_t *message, char **string );
static int format_subject(message_t *message, char **string );
static int format_content(message_t *message, char **string );
static int format_footer(unsigned int forum, message_t *message, char **string );

void
display_message(unsigned int num, unsigned int forum)
{
    message_t message;
    char *string = NULL;

    memset(&message, 0, sizeof(message_t));

    switch (mono_sql_mes_retrieve(num, forum, &message)) {

        case 0:
            break;
        case -1:
            cprintf("\1f\1rError retrieving message.\1a\n");
            return;
            break;
        case -2:
            cprintf("\1f\1rNo such message.\1a\n");
            return;
            break;
        case -3:
            cprintf("\1f\1rInternal Error.\1a\n");
            return;
            break;
        default:
            cprintf("\1f\1rInternal Error.\1a\n");
            return;

    }
    string = format_message( &message, forum );
    xfree(message.content);

#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_S);
#endif

    more_string( string );

#ifdef CLIENTSRC
    putchar(IAC);
    putchar(POST_E);
#endif

    xfree(string);

    return;
}

/*
 * Displays header as show when entering a new message.
 */
void
display_header(message_t *message)
{

    char *string = NULL;

    string = (char *)xmalloc( sizeof(char) );
    memset( string, 0, sizeof(string) );
 
    (void) format_username(message, &string);

    more_string( string );
    xfree(string);

    return;
}

static char *
format_message( message_t *message, unsigned int forum)
{
    char *string = NULL;

    /*
     * malloc() here and realloc in all following functions.
     */
    string = (char *)xmalloc( sizeof(char) );
    memset( string, 0, sizeof(string) );


    (void) format_header(message, &string);
    (void) format_content(message, &string);
    (void) format_footer(forum, message, &string);

    return string;

}

/*
 * Appends message header to string.
 * This is a nasty looking thing, but cleaning up is a later worry <heh>
 */
static int
format_header( message_t *message, char **string )
{

    /*
     * Takes up 2-3 lines for a post header, but looks prettier.
     * Small header is Ye Olde Format. (1 line).
     */
    if(usersupp->config_flags & CO_EXPANDHEADER) {
        (void) format_date(LOCALE_DEFAULT, message, string);
        (void) format_username(message, string);
        (void) format_title(message, string);
    } else {
        (void) format_username(message, string);
        (void) format_title(message, string);
        (void) format_date(LOCALE_DEFAULT, message, string);
    }

    (void) format_subject(message, string);

    return 0;
}

/*
 * Puts formatted date into string according to locale rules.
 */
static int
format_date(unsigned int locale, message_t *message, char **string)
{

    struct tm *tp;
    char fmt_date[1000];

    tp = localtime(&message->date);

    switch(locale) {

        case LOCALE_US:
            if(usersupp->config_flags & CO_EXPANDHEADER)
                strftime( fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %B %d, %Y \1w(\1g%I:%M %p\1w)\1a", tp );
            else
                strftime( fmt_date, 1000, " \1f\1g%a %b %d, %Y \1w(\1g%I:%M %p\1w)\1a", tp );
            break;

        case LOCALE_DEFAULT:
        case LOCALE_EUROPE:
            if(usersupp->config_flags & CO_EXPANDHEADER)
                strftime( fmt_date, 1000, "\n\1f\1gPosted\1w: \1g%A %d %B, %Y \1w(\1g%H:%M\1w)\1a", tp );
            else
                strftime( fmt_date, 1000, " \1f\1g%a %d %b, %Y \1w(\1g%H:%M\1w)\1a", tp );
            break;

        case NO_LOCALE:
        default:
            return 0;
            break;

    }
    fmt_date[strlen(fmt_date)] = '\0';

    *string = (char *)xrealloc( *string, strlen(*string)+strlen(fmt_date) );
    strcat( *string, fmt_date );

    return 0;
}

static int
format_username(message_t *message, char **string)
{

    char fmt_username[100];
    char tmp_username[L_USERNAME+1];

    if( (mono_sql_u_id2name(message->author, tmp_username)) == -1 ) {
        if(usersupp->config_flags & CO_EXPANDHEADER)
            sprintf(fmt_username, "\n\1f\1gFrom\1w: \1rDeleted %s\1a", config.user );
        else
            sprintf(fmt_username, "\n\1f\1gFrom \1rDeleted %s\1a", config.user );
    } else {

    switch(message->type) {

        case MES_ANON:
            if(usersupp->config_flags & CO_EXPANDHEADER)
                sprintf(fmt_username, "\n\1f\1gFrom\1w: \1bAnonymous %s%s\1a"
                ,config.user
                ,(message->author == usersupp->usernum) ? " \1w(\1bthis is your post\1w)" : "" );
            else
                sprintf(fmt_username, "\1f\1gFrom \1bAnon %s\1a"
                ,(message->author == usersupp->usernum) ? " \1w(\1bthis is your post\1w)" : "" );
            break;

        case MES_AN2:
            if(usersupp->config_flags & CO_EXPANDHEADER)
                sprintf(fmt_username, "\n\1f\1gFrom\1w: \1bAnonymous %s \1w`\1b%s\1w'%s\1a"
                ,config.user, message->alias
                ,(message->author == usersupp->usernum) ? " \1w(\1bthis is your post\1w)" : "" );
            else
                sprintf(fmt_username, "\1f\1gFrom \1bAnon \1w`\1b%s\1w'%s\1a"
                ,message->alias
                ,(message->author == usersupp->usernum) ? " \1w(\1bthis is your post\1w)" : "" );
            break;
  
        default:
            switch(message->type) {
               case MES_WIZARD:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\n\1f\1gFrom\1w: \1w%s\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\1f\1gFrom \1w%s\1a", tmp_username );
                    break;
               case MES_SYSOP:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\1f\1gFrom\1w: \1p%s\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\n\1f\1gFrom \1p%s\1a", tmp_username );
                    break;
               case MES_TECHNICIAN:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\n\1f\1gFrom\1w: \1b%s\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\1f\1gFrom \1b%s\1a", tmp_username );
                    break;
               case MES_ROOMAIDE:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\n\1f\1gFrom\1w: \1r%s\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\1f\1gFrom \1r%s\1a", tmp_username );
                    break;
               case MES_FORCED:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\n\1f\1gFrom\1w: \1y%s \1w(\1rFORCED MESSAGE\1w)\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\1f\1gFrom \1y%s \1w(\1rFORCED MESSAGE\1w)\1a", tmp_username );
                    break;
               case MES_NORMAL:
               default:
                    if(usersupp->config_flags & CO_EXPANDHEADER)
                        sprintf(fmt_username, "\n\1f\1gFrom\1w: \1y%s\1a", tmp_username );
                    else
                        sprintf(fmt_username, "\1f\1gFrom \1y%s\1a", tmp_username );
                    break;
            }
            break;
        }
    }
    fmt_username[strlen(fmt_username)] = '\0';

    *string = (char *)xrealloc( *string, strlen(*string)+strlen(fmt_username) );
    strcat( *string, fmt_username );

    return 0;
}

static int
format_title(message_t *message, char **string)
{

    char fmt_title[100];

    switch(message->priv) {
        case MES_WIZARD:
            sprintf(fmt_title, " \1f\1w( %s )\1a", config.wizard );
            break;

        case MES_SYSOP:
            sprintf(fmt_title, " \1f\1w( \1p%s \1w)\1a", config.sysop );
            break;

        case MES_TECHNICIAN:
            sprintf(fmt_title, " \1f\1w( \1b%s \1w)\1a", config.programmer );
            break;

        case MES_ROOMAIDE:
            sprintf(fmt_title, " \1f\1w( \1r%s \1w)\1a", config.roomaide );
            break;

        default:
            return -1;
            break;

    }
    fmt_title[strlen(fmt_title)] = '\0';

    *string = (char *)xrealloc( *string, strlen(*string)+strlen(fmt_title) );
    strcat( *string, fmt_title );

    return 0;

}


static int
format_subject(message_t *message, char **string)
{

    char fmt_subject[100];

    if(((message->subject == NULL) || (strlen(message->subject) == 0)) && (usersupp->config_flags & CO_EXPANDHEADER))
        sprintf(fmt_subject, "\n\1f\1gSubject\1w: \1w[\1yno subject\1w]\n");
    else if((message->subject != NULL) || (strlen(message->subject) > 0))
        sprintf(fmt_subject, "\n\1f\1gSubject\1w: \1y%s\1a\n", message->subject);
    else
        sprintf(fmt_subject, "\n");

    fmt_subject[strlen(fmt_subject)] = '\0';

    *string = (char *)xrealloc( *string, strlen(*string)+strlen(fmt_subject) );
    strcat( *string, fmt_subject );

    return 0;

}

static int
format_content(message_t *message, char **string )
{

    /*
     * Empty lines around content?
     */
    if( usersupp->config_flags & CO_NEATMESSAGES ) {
        *string = (char *)xrealloc( *string, strlen(*string)+strlen("\n") );
        strcat( *string, "\n" );
    }

    /*
     * Set content colour.
     */
    *string = (char *)xrealloc( *string, strlen(*string)+strlen("\1a\1c") );
    strcat( *string, "\1a\1c" );

    /*
     * Append actual message content.
     */
    *string = (char *)xrealloc( *string, strlen(*string)+strlen(message->content) );
    strcat( *string, message->content );

    if( usersupp->config_flags & CO_NEATMESSAGES ) {
        *string = (char *)xrealloc( *string, strlen(*string)+strlen("\n") );
        strcat( *string, "\n" );
    }
    return 0;

}

static int
format_footer(unsigned int forum, message_t *message, char **string )
{
    char fmt_footer[200], col;
    room_t *quad;

    quad = (room_t *)xmalloc( sizeof(room_t) );
    memset(quad, 0, sizeof(room_t) );

    (void) mono_sql_f_read_quad(forum, quad);

    if((quad->flags & QR_ANONONLY) || (quad->flags & QR_ANON2) || (quad->flags & QR_ALIASNAME)) col = 'p';
    else if (quad->flags & QR_PRIVATE) col = 'r';
    else col = 'y';

    if(usersupp->config_flags & CO_EXPANDHEADER)
        if( (quad->highest - message->num) > 0 )
            sprintf(fmt_footer, "\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1g%lu remaining\1w)] -> "
                ,col, quad->name, config.message, message->num, quad->highest, (quad->highest - message->num) );
        else
            sprintf(fmt_footer, "\1f\1w[\1%c%s\1w> \1g%s %u of %lu \1w(\1glast in this %s\1w)] -> "
                ,col, quad->name, config.message, message->num, quad->highest, config.forum );
    else
        if( (quad->highest - message->num) > 0 )
            sprintf(fmt_footer, "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1g%lu remaining\1w)] -> "
                ,col, quad->name, message->num, (quad->highest - message->num) );
        else
            sprintf(fmt_footer, "\1f\1w[\1%c%s\1w> \1g#%u \1w(\1glast\1w)] -> "
                ,col, quad->name, message->num );

    xfree(quad);

    *string = (char *)xrealloc( *string, strlen(*string)+strlen(fmt_footer) );
    strcat( *string, fmt_footer );

    return 0;
}
/* eof */

void
showmessages()
{
    mlist_t *list = NULL;
   
    enter_message(1,1);

    (void) mono_sql_mes_list_forum(1,0,&list);

    while(list != NULL) {
        display_message(list->id,1);
        inkey();
        cprintf("\1f\1gRead next %s\1a\n", config.message);
        list = list->next;
    }
    mono_sql_mes_free_list(list);

    return;
}
