/*
 * $Id$
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_MMAN_H	/* configure checks for this */
  #include <sys/mman.h>
#else
  #include <asm/mman.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
#include "input.h"
#include "display_message.h"

static int get_alias(room_t *quad, message_t *message);
static int get_priv(message_t *message, int flag);
static int get_subject(message_t *message);
static int get_content(message_t *message, int flag);
static int fill_buffer(char **content);

#ifndef DEBUG_SQL_MES
  #define DEBUG_SQL_MES
#endif

/*
 *	1: normal message.
 *	2: upload message.
 *	3: editor message.
 *	4: titled message.
 */

int
enter_message(unsigned int forum, unsigned int flag)
{
    room_t *quad;
    message_t *message;

    quad = (room_t *)xcalloc( 1, sizeof(room_t) );

    (void) mono_sql_f_read_quad(forum, quad);

    /* if( (!may_post(usersupp, quad)) ) {
     *     xfree(quad);
     *     return -1;
     * } 
     */

    message = (message_t *)xcalloc( 1, sizeof(message_t) );
    
    message->forum = forum;
    message->author = usersupp->usernum;
    message->content = NULL;
    message->deleted = 'n';

    (void) get_priv(message, flag);
    flag = 1;
    (void) get_alias(quad, message);

    /*
     * Display just the username/alias. The rest we can format through
     * proper input selection. Nice idea, coz quite often when the BBS
     * lags, it takes forever to re-display the header.
     */
    (void) display_header(message);

    /*
     * Get the subject line.
     */
    (void) get_subject(message);

    switch( get_content(message, flag)) {

        case -1: /* Empty post */
            cprintf("\1f\1rEmpty %s not saved!\n", config.message_pl);
            xfree(message->content);
            xfree(message);
            xfree(quad);
            return -1;

        case -2: /* <A>bort */
            cprintf("\1f\1r%s aborted.\n", config.message);
            xfree(message->content);
            xfree(message);
            xfree(quad);
            return -1;

        default:
            break;
            
    }

    if( (mono_sql_mes_add(message, forum)) == -1)
        cprintf("\1f\1rCould not save your %s!\n", config.message);
    else
        cprintf("\1f\1gSaved.\n");

    xfree(message->content);
    xfree(message);
    xfree(quad);

    return 0;
}

static int
get_priv(message_t *message, int flag)
{

    if(flag != 4)
        strcpy(message->priv, "normal");
    else {
        if(usersupp->priv & PRIV_WIZARD) {
            strcpy(message->priv, "emp");
        } else if(usersupp->priv & PRIV_SYSOP) {
            strcpy(message->priv, "sysop");
        } else if(usersupp->priv & PRIV_TECHNICIAN) {
            strcpy(message->priv, "tech");
        } else if(usersupp->priv & PRIV_ROOMAIDE) {
            strcpy(message->priv, "host");
        } else {
            strcpy(message->priv, "normal");
        }
    }
    return 0;
}

static int
get_alias(room_t *quad, message_t *message)
{

    if((quad->flags & QR_ALIASNAME) || (quad->flags & QR_ANON2)) {
        cprintf("\1f\1gDo you want to add an aliasname to this %s? \1w(\1gY\1w/\1gn\1w) \1c", config.message);
        if (yesno_default(YES) == YES) {
            if ((usersupp->config_flags & CO_USEALIAS) && (strlen(usersupp->alias))) {
                sprintf(message->alias, usersupp->alias);
            } else {
                do {
                    cprintf("\1f\1bAlias\1w: \1c");
                    getline(message->alias, L_USERNAME, 1);
                } while ( (check_user(message->alias) == TRUE) && (!(EQ(message->alias, usersupp->username))) );
            }
            if(strlen(message->alias))
                strcpy( message->type, "alias");
            else {
                strcpy( message->type, "anon");
            }
        }
    if( !(strlen(message->alias)))
        strcpy(message->alias, "");
    } else {
        strcpy(message->type, "normal");
    }

    return 0;
}

/*
 * Format this exacly like when displaying a message.
 */
static int
get_subject(message_t *message)
{
    cprintf("\n\1f\1gSubject\1w: \1y");
    getline(message->subject, L_SUBJECT-1, 1);

    if(!(strlen(message->subject)))
        strcpy(message->subject, "" );

    cprintf("\1a\1c");

    return 0;
}

#define POST_LINE_LENGTH	200

/*
 * mode = 1: normal message.
 * mode = 2: upload message.
 * mode = 3: editor message.
 */

static int
get_content(message_t *message, int mode)
{
    FILE *fp;
    int lines = 0;

    fp = xfopen(temp, "a", TRUE);

    if( usersupp->config_flags & CO_NEATMESSAGES )
        cprintf("\n\1a\1c");

    switch(get_buffer(fp, mode, &lines)) {

        case 's':
        case 'S':
            fclose(fp);
            return fill_buffer(&message->content);
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

static int
fill_buffer(char **content)
{

     char *post = NULL;
     int fd = -1;
     struct stat buf;

     /*
      * Open temp file.
      */
     if( (fd = open(temp, O_RDONLY)) == -1) {
         log_it("sqlpost", "Can't open() temp file %s!", temp);
         return -1;
     }

     /*
      * Determine file size.
      */
     fstat(fd, &buf);

     /*
      * mmap() tempfile.
      */ 
#ifdef HAVE_SYS_MMAN_H
     if( (post = mmap(post, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED ) { 
#else
     if( (post = mmap(post, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == -1 ) { 
#endif
         log_it("sqlpost", "Can't mmap() temp file %s!", temp);
         return -1;
     }

     /*
      * Reserve memory for the post content.
      */
     *content = (char*)xcalloc(1, strlen(post));
     if( *content == NULL ) {
         log_it("sqlpost", "Unable to malloc() message content.");
         return -1;
     }

     /*
      * Copy post content into mem.
      */
     if( (*content = memcpy(*content, post, strlen(post))) == NULL) {
         log_it("sqlpost", "memcpy() of message content failed!");
         return -1;
     }

     /*
      * The Terminator; he said he'd be back!
      */
#ifdef SHIT
     *content[ strlen(*content)-1 ] = '\0';
#else
     strcat(*content, "\0");
#endif

     /*
      * And close, destroy, kill etc.
      */
     if( (munmap(post, buf.st_size)) == -1) {
         log_it("sqlpost", "Can't munmap() temp file %s!", temp);
     }
     close(fd);
     unlink(temp);

     return 0;
}
