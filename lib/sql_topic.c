/* $Id$ */
/* operatoins we can do on topics */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "routines.h"
#include "monolith.h"
#include "monosql.h"
#include "sql_utils.h"
// #include "sql_forumtopic.h"
#define FT_TABLE	"forumtopic"

#define extern
#include "sql_topic.h"
#undef extern

int
mono_sql_t_create_topic( const topic_t * top)
{
    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;
   
    /* check if topic already exists */
    ret = escape_string(top->name, &esc_name);

    if ( ret ) {
	printf("could not escape topicname (#%u).\n", top->topic_id);
	return -1;
    }

    ret = mono_sql_query(&res, "INSERT INTO " T_TABLE
	 " (topic_id,name) " "VALUES (%u,'%s')" 
	   ,top->topic_id, esc_name );

    xfree(esc_name);

    if ( ret == -1 ) return -1;

    ret = mono_sql_query( &res, "INSERT INTO " FT_TABLE 
 	" (topic_id,forum_id,owner) VALUES (%u,%u,'%s')"
	, top->topic_id, top->forum_id, 'y' );

    return ret;
}

int 
mono_sql_t_rename_topic( unsigned int topic_id, const char *newname )
{
    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;

    if (escape_string(newname, &esc_name) != 0) {
	fprintf( stderr, "could not escape topicname (#%u).\n", topic_id);
	return -1;
    }
    ret = mono_sql_query(&res, "UPDATE " T_TABLE
	 " SET name='%s' WHERE topic_id=%u", esc_name, topic_id );

    mono_sql_u_free_result(res);
    xfree(esc_name);

    return ret;
}

int
mono_sql_kill_topic(int topic_id)
{
    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE " T_TABLE " WHERE topic_id=%u",
			 topic_id);

    mono_sql_u_free_result(res);

    if (ret != 0) {
	return -1;
    }
    return 0;
}

int
mono_sql_t_updated_highest( unsigned int topic, unsigned int m_id )
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "UPDATE " T_TABLE " SET highest=%u WHERE topic_id=%u", m_id, topic );

    if( ret == -1 ) {
//        (void) mono_sql_u_free_result(res);
        return -1;
    }
//    (void) mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_t_get_new_message_id( unsigned int topic, unsigned int *highest )
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0;
    unsigned int max;

    ret = mono_sql_query(&res, "SELECT highest FROM " T_TABLE " WHERE topic_id=%u", topic );

    if (ret == -1) {
        (void) mono_sql_u_free_result(res);
        return -1;
    }
    if ( mysql_num_rows(res) == 0) {
        (void) mono_sql_u_free_result(res);
        return -1;
    }
    if ( mysql_num_rows(res) > 1) {
        (void) mono_sql_u_free_result(res);
        return -1;
    }

    row = mysql_fetch_row(res);
    sscanf( row[0], "%u", &max );
    (void) mono_sql_u_free_result(res);

    max++;

    ret = mono_sql_query(&res, "UPDATE " T_TABLE 
          " SET highest=%u WHERE id=%u", max, topic);

    (void) mono_sql_u_free_result(res);

    if ( ret == -1 ) {
        return -1;
    }

    *highest = max;

    return 0;
}

/* eof */
