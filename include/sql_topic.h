#define T_TABLE "topic"

extern int mono_sql_t_new_topic(unsigned int topic_id, topic_t * top);
extern int mono_sql_t_rename_topic( unsigned int topic_id, const char *newname );
extern int mono_sql_kill_topic(int topic_id);

