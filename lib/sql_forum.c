/* $Id$ */
/* operatoins we can do on forums */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

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
#include "monosql.h"
#include "routines.h"
#include "sql_utils.h"

#define extern
#include "sql_forum.h"
#undef extern

/* nasty comaptible stuff */
int
mono_sql_f_write_quad(unsigned int num, room_t * const q)
{
    printf("debug: writing quad %u\n", num);
    mono_sql_f_kill_forum(num);
    return mono_sql_f_add_old_forum(num, q);
}

int
mono_sql_f_read_quad(unsigned int num, room_t * room)
{
    int i, rows;
    MYSQL_RES *res;
    MYSQL_ROW row;
    room_t r;

    i = mono_sql_query(&res, "SELECT "
	"name,category_old,flags,highest,lowest,generation,roominfo,maxmsg "
		       "FROM " F_TABLE " WHERE id=%u", num);

    if (i == -1) {
	fprintf(stderr, "Error: no results from query\n");
	mysql_free_result(res);
	return -1;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	mysql_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);

    /* sscanf error codes need to be checked ! */
    strcpy(r.name, row[0]);
    strcpy(r.category, row[1]);
    sscanf(row[2], "%u", &r.flags);
    sscanf(row[3], "%lu", &r.highest);
    sscanf(row[4], "%lu", &r.lowest);
    sscanf(row[5], "%c", &r.generation);
    sscanf(row[6], "%c", &r.roominfo);
    sscanf(row[7], "%u", &r.maxmsg);

    mysql_free_result(res);

    *room = r;
    return 0;
}


int
mono_sql_f_add_old_forum(unsigned int forum_id, room_t * const q)
{

    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;

    if (escape_string(q->name, &esc_name) != 0)  {
        printf( "could not escape forumname (#%u).\n", forum_id );
        return -1;
    }

    ret = mono_sql_query(&res, "INSERT INTO " F_TABLE
			 " (id,name,category_old,flags,highest,lowest,generation,roominfo,maxmsg) "
		       "VALUES ( %u, '%s', '%s', %u, %u, %u, %d, %d, %u )\n"
	   ,forum_id, esc_name, q->category, q->flags, q->highest, q->lowest
			 ,q->generation, q->roominfo, q->maxmsg);

    xfree(esc_name);
    mysql_free_result(res);

    return ret;

}

int
mono_sql_f_rename_forum( unsigned int forum_id, char *newname )
{
    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;

    if (escape_string( newname, &esc_name) != 0)  {
        printf( "could not escape forumname (#%u).\n", forum_id );
        return -1;
    }

    ret = mono_sql_query(&res, "UPDATE " F_TABLE " SET name='%s' WHERE id=%u" ,
           esc_name, forum_id );

    xfree(esc_name);
    mysql_free_result(res);

    if (ret != 0) {
	return -1;
    }
    return 0;
}

int
mono_sql_f_update_forum(unsigned int forum_id, const room_t * q)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " F_TABLE " SET "
			 " (name='%s',category_old='%s',flags=%u,highest=%u,lowest=%u,generation=%d,roominfo=%d,maxmsg=%u) "
			 "WHERE id=%u\n"
		 ,q->name, q->category, q->highest, q->lowest, q->generation
			 ,q->roominfo, q->maxmsg, forum_id);

    if (ret == -1) {
	fprintf(stderr, "No results from query.\n");
	mysql_free_result(res);
	return -1;
    }
    mysql_free_result(res);
    return 0;

}
int
mono_sql_f_kill_forum(unsigned int forum_id)
{

    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " F_TABLE " WHERE (id='%u')",
			 forum_id);
    mysql_free_result(res);
    return ret;

    return 0;
}

int
mono_sql_f_name2id(const char *forumname, unsigned int *forumid)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *esc_name;

    escape_string(forumname, &esc_name);

    i = mono_sql_query(&res, "SELECT id FROM " F_TABLE " WHERE name='%s'", esc_name);

    xfree(esc_name);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) != 1) {
	mysql_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", forumid);

    mysql_free_result(res);
    return 0;
}

int
mono_sql_f_partname2id(const char *forumname, unsigned int *forumid)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *esc_name;

    escape_string(forumname, &esc_name);

    i = mono_sql_query(&res, "SELECT id FROM " F_TABLE " WHERE name like '%%%s%%'", esc_name);

    xfree(esc_name);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) < 1) {
	mysql_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", forumid);

    mysql_free_result(res);
    return 0;
}

unsigned int
mono_sql_f_get_new_message_id( unsigned int forum )
{

    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret = 0;

    ret = mono_sql_query(&res, "SELECT highest FROM " F_TABLE " WHERE id=%u", forum );

    if (ret == -1) {
        (void) mysql_free_result(res);
        return 0;
    }
    if ( mysql_num_rows(res) == 0) {
        (void) mysql_free_result(res);
        return 0;
    }
    if ( mysql_num_rows(res) > 1) {
        (void) mysql_free_result(res);
        return 0;
    }

    row = mysql_fetch_row(res);

    ret = atoi(row[0]);
    (void) mysql_free_result(res);

    ret++;

    if( (mono_sql_query(&res, "UPDATE " F_TABLE " SET highest=%u WHERE id=%u", ret, forum)) == -1) {
        (void) mysql_free_result(res);
        return 0;
    }

    (void) mysql_free_result(res);

    return ret;
}
/* eof */
