/*
 * $Id$
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <string.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "monolith.h"

#include "defs.h"
#include "extra.h"
#include "sysconfig.h"

#include "monosql.h"
#include "sql_utils.h"
/* #include "sql_log.h" */
#include "sql_config.h"

/*
 * Read a configuration from the table
 */

int
mono_sql_read_config(int num, config_t * data)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    config_t config;

    memset(&config, 0, sizeof(config_t));

    mono_sql_query(&res, "SELECT * FROM %s WHERE id=%d", CONFIG_TABLE, num);

    if (mysql_num_rows(res) == 0) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    if (mysql_num_rows(res) > 1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);

    /*
     * Skip the ID (row 0)
     */
    strcpy(config.bbsname, row[1]);
    strcpy(config.forum, row[2]);
    strcpy(config.forum_pl, row[3]);
    strcpy(config.message, row[4]);
    strcpy(config.express, row[6]);
    strcpy(config.x_message, row[7]);
    strcpy(config.x_message_pl, row[8]);
    strcpy(config.user, row[9]);
    strcpy(config.user_pl, row[10]);
    strcpy(config.username, row[11]);
    strcpy(config.doing, row[12]);
    strcpy(config.location, row[13]);
    strcpy(config.idle, row[14]);
    strcpy(config.chatmode, row[15]);
    strcpy(config.chatroom, row[16]);
    strcpy(config.admin, row[17]);
    strcpy(config.wizard, row[18]);
    strcpy(config.sysop, row[19]);
    strcpy(config.programmer, row[20]);
    strcpy(config.roomaide, row[21]);
    strcpy(config.guide, row[22]);

    (void) mono_sql_u_free_result(res);

    *data = config;
    return 0;
}

/*
 * Save a modified config to the table. if num == 0 we have a new config, else
 * replace an old one.
 */
int
mono_sql_save_config(int num, config_t * config)
{
    MYSQL_RES *res;

    if (num == 0) {
	if ((mono_sql_query(&res, "INSERT INTO " CONFIG_TABLE " VALUES (0,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
			    config->bbsname, config->forum, config->forum_pl,
		       config->message, "dummy", config->express,
		      config->x_message, config->x_message_pl, config->user,
			    config->user_pl, config->username, config->doing,
			    config->location, config->idle, config->chatmode,
			    config->chatroom, config->admin, config->wizard,
			config->sysop, config->programmer, config->roomaide,
			    config->guide)) == -1) {
	    (void) mono_sql_u_free_result(res);
	    return -1;
	}
    } else {
	if ((mono_sql_query(&res, "REPLACE INTO " CONFIG_TABLE " VALUES(%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
			    num,
			    config->bbsname, config->forum, config->forum_pl,
		       config->message, "dummy", config->express,
		      config->x_message, config->x_message_pl, config->user,
			    config->user_pl, config->username, config->doing,
			    config->location, config->idle, config->chatmode,
			    config->chatroom, config->admin, config->wizard,
			config->sysop, config->programmer, config->roomaide,
			    config->guide)) == -1) {
	    (void) mono_sql_u_free_result(res);
	    return -1;
	}
    }


    (void) mono_sql_u_free_result(res);
    return 0;
}

/*
 * Deleting fucks up id order, so select() can't cope. Fix first, then
 * enable this. No sooner.
 */
int
mono_sql_delete_config(int num)
{
    MYSQL_RES *res;

    return -1;

    if ((mono_sql_query(&res, "DELETE FROM " CONFIG_TABLE " WHERE id=%d", num)) == -1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    (void) mono_sql_u_free_result(res);
    return 0;
}

/*
 * Check for a config with a given name and return TRUE if
 * it exists.
 */
int
mono_sql_config_exists(const char *name)
{
    MYSQL_RES *res;

    mono_sql_query(&res, "SELECT * FROM %s WHERE name='%s'", CONFIG_TABLE, name);

    if (mysql_num_rows(res) == 0) {
	(void) mono_sql_u_free_result(res);
	return FALSE;
    }
    (void) mono_sql_u_free_result(res);
    return TRUE;
}
/* eof */
