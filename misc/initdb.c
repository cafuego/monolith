/* $Id$ */
/* program to create initial forums & topics */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <string.h>

#include MYSQL_HEADER

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

