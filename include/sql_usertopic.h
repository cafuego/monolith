/* $Id$ */

extern int mono_sql_ut_add_entry(unsigned int, unsigned int );
extern int mono_sql_ut_kill_topic(unsigned int );
extern int mono_sql_ut_kill_user(unsigned int);
extern int mono_sql_ut_new_user(unsigned int);
extern int mono_sql_ut_update_lastseen( unsigned int user_id,
                             unsigned int topic_id, unsigned int message_id );
extern int mono_sql_ut_query_lastseen( unsigned int user_id,
                             unsigned int topic_id, unsigned int *message_id );

#define UT_TABLE	"usertopic"
