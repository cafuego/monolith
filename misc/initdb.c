/* $Id$ */
/* program to create initial forums & topics */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"

static int create_forums( void );
static int create_topics( void );
static int create_users( void );
static int create_configs( void );

int
main( int argc, char *argv[] )
{
    set_invocation_name(argv[0]);
    mono_setuid("guest");

    mono_sql_connect();

    create_users();
    create_forums();
    create_topics();
    return 0;

    mono_sql_detach();
}

static int 
create_users( void )
{
    printf( "Creating users...\n" );
    return 0;
}

static int 
create_forums( void )
{
    printf( "Creating forums...\n" );
    return 0;
}

static int 
create_topics( void )
{
    unsigned int i;
    int ret;
    topic_t top;

    printf( "Creating topics...\n" );

    top.topic_id = 0;
    strcpy( top.name, "general" );

    for (i=0; i<MAXQUADS; i++ ) {
        top.forum_id = i;
        printf( "Creating first topic in forum %u..", i ); fflush(stdout);
        ret = mono_sql_t_create_topic( &top );
        if ( ret == 0 ) {
           printf( "done.\n" );
        } else {
           printf( "failed.\n" );
        }
    }

    return 0;
}

#define BUFSIZE		3000
#define DELIM 		"|"
#define CONFIGURATIONS	BBSDIR "etc/configurations.def"

static int 
create_configs( void )
{
    FILE *fp;
    char buffer[BUFSIZE];
    config_t *config;

    fp = xfopen(CONFIGURATIONS, "r", FALSE);
    if (fp == NULL)
	return FALSE;

    config = (config_t *) xmalloc( sizeof(config_t) );
    memset(config, 0, sizeof(config) );

    while (fgets(buffer, BUFSIZE, fp) != NULL) {

	/* don't parse comment lines */
	if ((buffer[0] == '#') || (buffer[0] == '-'))
	    continue;

	/* remove trailing '\n' */
	buffer[strlen(buffer) - 1] = '\0';

	strcpy(config->bbsname, strtok(buffer, DELIM));
	strcpy(config->forum, strtok(NULL, DELIM));
	strcpy(config->forum_pl, strtok(NULL, DELIM));
	strcpy(config->message, strtok(NULL, DELIM));
	strcpy(config->message_pl, strtok(NULL, DELIM));
	strcpy(config->express, strtok(NULL, DELIM));
	strcpy(config->x_message, strtok(NULL, DELIM));
	strcpy(config->x_message_pl, strtok(NULL, DELIM));
	strcpy(config->user, strtok(NULL, DELIM));
	strcpy(config->user_pl, strtok(NULL, DELIM));
	strcpy(config->username, strtok(NULL, DELIM));
	strcpy(config->doing, strtok(NULL, DELIM));
	strcpy(config->location, strtok(NULL, DELIM));
	strcpy(config->chatmode, strtok(NULL, DELIM));
	strcpy(config->chatroom, strtok(NULL, DELIM));
	strcpy(config->admin, strtok(NULL, DELIM));
	strcpy(config->wizard, strtok(NULL, DELIM));
	strcpy(config->sysop, strtok(NULL, DELIM));
	strcpy(config->programmer, strtok(NULL, DELIM));
	strcpy(config->roomaide, strtok(NULL, DELIM));
	strcpy(config->guide, strtok(NULL, DELIM));
	strcpy(config->idle, strtok(NULL, DELIM));

        if( (mono_sql_save_config(0, config)) == 0 )
            printf("Saved %s...\n", config->bbsname);
        else
            printf("Error saving %s!\n", config->bbsname);
    }
    fclose(fp);
    xfree(config);
    return 0;

}
