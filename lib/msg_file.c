/* library-ish functions for dealing with message header files and message text files */
/* in other words, anything todo with message system file ops */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "log.h"
#include "btmp.h"
#include "routines.h"
#include "sql_message.h"
#include "sql_user.h"
#include "userfile.h"
#include "libquad.h"
#include "libcache.h"

#define extern
#include "msg_file.h"
#undef extern

static char * get_flag(unsigned long mod_banner);
static char * get_reason(int mod_reason);


int
write_message_header(const char *header_filename, message_header_t *header)
{
    size_t sz;
    FILE *fp;

    if ((fp = xfopen(header_filename, "w", FALSE)) == NULL)
	return -1;

    sz = fwrite(header, sizeof(message_header_t), 1, fp);

    if (ferror(fp)) {
	printf("\n\1f\1rError: File error at write_message_header()\n");
	unlink(header_filename);
	fclose(fp);
	return -1;
    }
    fclose(fp);

    return 0;
}

int
read_message_header(const char *header_filename, message_header_t *header)
{
    FILE *fp;

    memset(header, 0, sizeof(message_header_t));

    if (!fexists(header_filename))
	return -1;
    if ((fp = xfopen(header_filename, "r", FALSE)) == NULL) 
	return -1;

    fread(header, sizeof(message_header_t), 1, fp);

    if (ferror(fp)) {
        printf("\n\1r\1fError: File error at read_message_header()\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}
char *
info_filename(char * filename, unsigned int forum)
{
    snprintf(filename, L_FILENAME, FORUMDIR "/%u/info", forum);
    return filename;
}

char *
info_header_filename(char * filename, unsigned int forum)
{
    snprintf(filename, L_FILENAME, FORUMDIR "/%u/info.h", forum);
    return filename;
}

char *
message_filename(char * filename, unsigned int forum, unsigned int number)
{
    snprintf(filename, L_FILENAME, FORUMDIR "/%u/%u.t", forum, number);
    return filename;
}

char *
message_header_filename(char * filename, unsigned int forum, unsigned int number)
{
    snprintf(filename, L_FILENAME, FORUMDIR "/%u/%u.h", forum, number);
    return filename;
}

char *
mail_filename(char * filename, const char *name, const unsigned int number)
{
    snprintf(filename, L_FILENAME, "%s/mail/%u.t", getuserdir(name), number);
    return filename;
}

char *
mail_header_filename(char * filename, const char *name, const unsigned int number)
{
    snprintf(filename, L_FILENAME, "%s/mail/%u.h", getuserdir(name), number);
    return filename;

}

int
message_copy(const unsigned int from_forum, const unsigned int to_forum, const unsigned int message_id, const char *recipient_name, const unsigned int modification_type)
{
    char to_file[L_FILENAME + 1], from_file[L_FILENAME + 1];
    char to_header_file[L_FILENAME + 1], from_header_file[L_FILENAME + 1];
    char to_name[L_USERNAME + 1];
    unsigned int new_message_id, success = 0;
    message_header_t *to_header = NULL;
    room_t forum;

    if (from_forum == MAIL_FORUM) {
	mail_filename(from_file, who_am_i(NULL), message_id);
	mail_header_filename(from_header_file, who_am_i(NULL), message_id);
    } else {
	message_filename(from_file, from_forum, message_id);
	message_header_filename(from_header_file, from_forum, message_id);
    }

    if (!fexists(from_file) || !fexists(from_header_file)) {
	printf("\n\1f\1rfexists() returned 0 for header and/or text at message_copy()\1a");
        return -1;
    }

    if (to_forum == MAIL_FORUM) {
	strcpy(to_name, recipient_name);
	if ((new_message_id = get_new_mail_number(to_name)) == -1) {
	    printf("\n\1r\1fCouldn't get new mail number for %s\n\1a", recipient_name);
	    return -1;
	}
	mail_filename(to_file, to_name, new_message_id);
	mail_header_filename(to_header_file, to_name, new_message_id);
    } else {
	if ((new_message_id = get_new_message_id(to_forum)) == -1) {
	    printf("\n\1r\1fCouldn't get new post number for Forum #%u\n\1a", to_forum);
	    return -1;
	}
	message_filename(to_file, to_forum, new_message_id);
	message_header_filename(to_header_file, to_forum, new_message_id);
    }

    for (;;) {
	if ((to_header = (message_header_t *) xmalloc(sizeof(message_header_t))) == NULL) 
	    break;

	if ((read_message_header(from_header_file, to_header)) == -1) {
	    xfree(to_header);
	    break;
	}
        read_forum( to_forum, &forum );

	to_header->orig_m_id = to_header->m_id;
	to_header->orig_f_id = to_header->f_id;
	to_header->orig_t_id = to_header->t_id;
	to_header->orig_date = to_header->date;
	to_header->mod_type = modification_type;
	strcpy(to_header->orig_forum, to_header->forum_name);
	strcpy(to_header->modified_by, who_am_i(NULL));
	strcpy(to_header->forum_name, forum.name);
	to_header->quad_flags = forum.flags;
	to_header->m_id = new_message_id;
	to_header->f_id = to_forum;
        to_header->date = time(NULL);
	if (to_forum == MAIL_FORUM) {
	    to_header->banner_type |= MAIL_BANNER;
	    strcpy(to_header->recipient, to_name);
	}

#ifndef ADDED_BY_PETER_FOR_TESTING
        if (to_forum != MAIL_FORUM ) {
	    unsigned long temp_banner;

/* added this to get rid of potential issues copying mails.  */

	    if (to_header->banner_type & MAIL_BANNER) { 
		temp_banner = to_header->banner_type;
		to_header->banner_type &= ~MAIL_BANNER;
                (void) save_to_sql(to_header, from_file);
		to_header->banner_type = temp_banner;
	    } else
		(void) save_to_sql(to_header, from_file);
	}
#endif


	if ((write_message_header(to_header_file, to_header)) == -1) {
	    xfree(to_header);
	    break;
	}

	xfree(to_header);
	success = 1;
	break;
    }

    if (copy(from_file, to_file) != 0)
	return -1;

    return 0;
}

int message_move(const unsigned int from_forum, const unsigned int to_forum, const unsigned int message_id, const char *recipient_name) 
{
    if (message_copy(from_forum, to_forum, message_id, recipient_name, MOD_MOVE) != 0) {
	printf("\1f\1r\nmessage_copy() returned error at message_move()\1a\n");
	return -1;
    } else if (message_delete(from_forum, message_id) != 0) {
	printf("\1f\1r\nmessage_delete() returned error at message_move()\1a\n");
	return -1;
    }
    return 0;
}

int message_delete(const unsigned int from_forum, const unsigned int message_id)
{
    char from_file[L_FILENAME + 1], from_header[L_FILENAME + 1];
    int ret = 0;

    if (from_forum == MAIL_FORUM) {
	mail_filename(from_file, who_am_i(NULL), message_id);
	mail_header_filename(from_header, who_am_i(NULL), message_id);
    } else {
	message_filename(from_file, from_forum, message_id);
	message_header_filename(from_header, from_forum, message_id);
    }

    if (fexists(from_file)) 
	unlink(from_file);
    else {
	ret = -1;
    }

    if (fexists(from_header)) 
	unlink(from_header);
    else {
	ret = -1;
    }

    /* Mark SQL message as deleted. */
    if (from_forum != MAIL_FORUM)
        (void) mono_sql_mes_mark_deleted(message_id, from_forum);

    return ret;
}

/* 
 * peter, i removed ext.h, as this is a library function.  (no /src externals)
 * patched stuff appropriately..  this function still breaks stuff from copy
 * and move.  
 */
void
save_to_sql(const message_header_t *header, const char *filename)
{
    message_t *message = NULL;

    message = (message_t *)xcalloc( 1, sizeof(message_t) );

    /*
     * BEWARE! SQL messages do _not_ generate their own ID!!!
     */
    message->m_id = header->m_id;
    message->f_id = header->f_id;
    message->t_id = 0;
    mono_cached_sql_u_name2id(header->author, &message->a_id);
    sprintf(message->alias, "%s", header->alias);
    sprintf(message->subject, "%s", header->subject);
    sprintf(message->flag, "%s", get_flag(header->banner_type));
    message->reply_m_id = header->reply_m_id;
    message->reply_f_id = header->reply_f_id;
    message->reply_t_id = header->reply_t_id;
    (void) mono_cached_sql_u_name2id( header->reply_to_author, &message->reply_a_id );
    strcpy(message->reply_alias, "" );
    message->orig_m_id = header->orig_m_id;
    message->orig_f_id = header->orig_f_id;
    message->orig_t_id = header->orig_t_id;
    (void) mono_cached_sql_u_name2id( header->modified_by, &message->orig_a_id );
    message->orig_date = header->orig_date;
    sprintf(message->mod_reason, "%s", get_reason(header->mod_type));
    if( (message->content = map_file(filename)) == NULL ) {
        printf("SQL DEBUG: Unable to save message to SQL (content is missing)\n" );
        printf("   Error has been logged, don't bother reporting it.\n" );
        xfree(message->content);
        xfree(message);
        return;
    }
    if( (mono_sql_mes_add(message)) == -1) {
        printf("SQL DEBUG: Unable to save message to SQL (query fuckswed up)\n");
        printf("   Error has been logged (argh), don't bother reporting it.\n" );
    }

    xfree(message->content);
    xfree(message);
    return;
}

static char *
get_flag(unsigned long mod_banner)
{

    switch(mod_banner) {
        case AUTOMAIL_BANNER:
        case SYSTEM_BANNER:
            return "auto";
            break;
        case EMP_BANNER:
            return "emp";
            break;
        case SYSOP_BANNER:
            return "sysop";
            break;
        case ADMIN_BANNER:
            return "admin";
            break;
        case TECH_BANNER:
            return "echo";
            break;
        case QL_BANNER:
            return "roomaide";
            break;
        case FORCED_BANNER:
            return "forced";
            break;
        case YELL_BANNER:
            return "yell";
            break;
        case ANON_BANNER:
            return "anon";
            break;
        case NO_BANNER:
        default:
            return "normal";
            break;
    }
}

static char *
get_reason(int mod_reason)
{
    switch(mod_reason) {

        case MOD_MOVE:
            return "moved";
            break;

        case MOD_COPY:
           return "copied";
           break;

        default:
            return "";
            break;
    }
}

size_t
get_filesize(const char *filename)
{
    struct stat filestat;
    int ret;
    
    ret = stat(filename, &filestat);

    if ( ret == -1 ) {
         fprintf( stderr, "%s: %s", filename, strerror(mono_errno) );
         log_it( "errors", "%s: %s", filename, strerror(mono_errno) );
 
         return  0;
    }
    return (size_t) filestat.st_size;
}

int
count_dir_files(const char *filename)
{
    int i, j;
    struct dirent **namelist;

    i = scandir(filename, &namelist, 0, 0);
    for (j = i - 1; j >= 0; j--) 
	xfree(namelist[j]);

    return i - 2;		/* don't count . and .. */
}

void 
init_message_header(message_header_t * header)
{
   strcpy(header->author, "");
   strcpy(header->subject, "");
   strcpy(header->alias, "");

   header->line_total = 0;  /* unused */

   header->anonymous = 0;   /* unused */

   strcpy(header->recipient, "");
   strcpy(header->forum_name, "");
   strcpy(header->banner, "");
   header->banner_type = 0;  	/* the NO_BANNER mask */

   strcpy(header->reply_to_author, "");
   header->reply_m_id = 0;
   header->reply_f_id = 0;
   header->reply_t_id = 0;
   header->reply_anonymous = 0;  /* unused */
 
   header->m_id = 0;
   header->f_id = 0;
   header->t_id = 0;

   strcpy(header->modified_by, "");
   strcpy(header->orig_forum, "");
   header->mod_type = 0;
   header->orig_m_id = 0;
   header->orig_f_id = 0;
   header->orig_t_id = 0;

   header->quad_flags = 0;
}

/* eof */





#ifdef OHH_SHIT_WE_HAVE_TO_CONVERT_AGAIN_ARRRRRRGH

/* must be compiled and run 3 times..  called with forum parm 0 to MAXQUADS.
 *
 * 1> with CONVERT_MAIL_QUAD defined and CONVERT_INFO #undefined
 * 2> with CONVERT_INFO #defined and CONVERT_MAIL_QUAD #undefined
 * 3> with both CONVERT_INFO and CONVERT_MAIL_QUAD #undefined. (actual msgbase)
 */

int   
convert_message_base(int forum)
{
#define CONVERT_MAIL_QUAD
//#define CONVERT_INFO
    room_t quad;
    message_header_t * header;
    post_t post;
    FILE *postfile, *fp;
    char filename[L_FILENAME + 1], infofile[L_FILENAME + 1];
    char hugestring[100000];
    int i, j, lowest_message, highest_message;

#ifdef CONVERT_MAIL_QUAD
#undef CONVERT_INFO
    user_t *user;
    char name[L_USERNAME + 1];
    DIR* userdir;
    struct dirent *tmpdirent;

    if (forum != MAIL_FORUM)
	return -1;

    userdir = opendir(USERDIR);

    if (userdir == NULL) {
        printf("opendir(%s) problems!\n", USERDIR);
	return -1;
    } else
        printf("opened (%s)\n", USERDIR);

    rewinddir(userdir);
    
    while ((tmpdirent = readdir(userdir)) != NULL) {

        if (tmpdirent->d_name[0] == '.')        /* ignore . files */
            continue;

        strcpy(name, tmpdirent->d_name);
	printf("\nName: %s\n", name);
        user = readuser(name);
        if (user == NULL)
            continue;
        if (EQ(user->username, "Sysop"))
            continue;
	highest_message = user->mailnum;
lowest_message = 1;
	forum = MAIL_FORUM;
        read_forum( forum, &quad );
#endif

#ifndef CONVERT_MAIL_QUAD
#ifndef CONVERT_INFO
    if (forum == 1)
	return -1;
#endif
    read_forum( forum, &quad );
    highest_message = quad.highest;
    lowest_message = quad.lowest;
#endif

for (i = 1; i <= highest_message; i++) {

#ifdef CONVERT_MAIL_QUAD
        postfile = fopen(post_to_file(forum, i, name), "r");
#endif

#ifndef CONVERT_MAIL_QUAD
  #ifndef CONVERT_INFO
        postfile = fopen(post_to_file(forum, i, NULL), "r");
  #endif
  #ifdef CONVERT_INFO
    sprintf(infofile, QUADDIR "/%u/description", forum);
    postfile = fopen(infofile, "r");
    highest_message = 1;
  #endif
#endif

	if (postfile == NULL) {
	    continue;
	}

        if (read_post_header(postfile, &post) == -1) {
            printf("\1r\1fTrouble reading the reply header.\1a\n");
            fclose(postfile);
	    return -1;
	}

        header = (message_header_t *) xmalloc(sizeof(message_header_t));
	memset(header, 0, sizeof(message_header_t));
        init_message_header(header);
    
        header->f_id = forum;
	header->m_id = i;
        strcpy(header->forum_name, quad.name);
        strcpy(header->author, post.author);
        header->date = post.date;
      
        switch(post.type) {
	    case MES_NORMAL:
		header->banner_type = NO_BANNER;
		break;
	    case MES_ANON:       
		header->banner_type = ANON_BANNER;
		break;
	    case MES_AN2:       
		header->banner_type = ANON_BANNER;
		break;
	    case MES_DESC:     
		header->banner_type = INFO_BANNER;
		break;
	    case MES_FORCED:  
		header->banner_type = FORCED_BANNER;
		break;
	    case MES_ROOMAIDE:    
		header->banner_type = QL_BANNER;
		break;
	    case MES_TECHNICIAN: 
		header->banner_type = TECH_BANNER;
		break;
	    case MES_SYSOP:     
		header->banner_type = SYSOP_BANNER;
		break;
	    case MES_WIZARD:   
		header->banner_type = EMP_BANNER;
		break;
	    case MES_GAMEMASTER:  
		header->banner_type = NO_BANNER;
		break;
	    case MES_SYSTEM:     
		header->banner_type = SYSTEM_BANNER;
	    default:
		header->banner_type = NO_BANNER;
        }

#ifdef CONVERT_MAIL_QUAD
	header->banner_type |= MAIL_BANNER;
        strncpy(header->recipient, post.recipient, L_RECIPIENT - 1);
	header->recipient[L_RECIPIENT - 1] = '\0';
#endif
        if (header->banner_type == ANON_BANNER)
	    if (strlen(post.alias))
	        strcpy(header->alias, post.alias);

        if (strlen(post.subject))
	    strcpy(header->subject, post.subject);

	header->reply_m_id = post.ref;
	header->quad_flags = quad.flags;

#ifdef CONVERT_MAIL_QUAD
        write_message_header(mail_header_filename(filename, name, i), header);
#endif
#ifndef CONVERT_MAIL_QUAD
  #ifndef CONVERT_INFO
        write_message_header(message_header_filename(filename, forum, i), header);
  #endif
  #ifdef CONVERT_INFO
        write_message_header(info_header_filename(filename, forum), header);
  #endif
#endif
	xfree(header);
	header = NULL;
#ifdef CONVERT_MAIL_QUAD   
        if ((fp = xfopen(mail_filename(filename, name, i), "w", FALSE)) == NULL) {
#endif
#ifndef CONVERT_MAIL_QUAD
  #ifndef CONVERT_INFO
        if ((fp = xfopen(message_filename(filename, forum, i), "w", FALSE)) == NULL) {
  #endif
  #ifdef CONVERT_INFO
        if ((fp = xfopen(info_filename(filename, forum), "w", FALSE)) == NULL) {
  #endif
#endif
    	    printf("\n\n\1r\1ffailed to open %s.\n\1a", filename);
	    return -1;
        }
	strcpy(hugestring, "");

	for (j = 0; !feof(postfile); j++)
	    hugestring[j] = (char) fgetc(postfile);
    	fclose(postfile);
	hugestring[j - 1] = '\0';
    	fputs(hugestring, fp);

	fflush(fp);
	fclose(fp);
#ifdef CONVERT_MAIL_QUAD
	unlink(postfile);
#endif
    }
#ifdef CONVERT_MAIL_QUAD
    xfree(user);
    user = NULL;
}
closedir(userdir);
#endif
return 0;
}

#endif



