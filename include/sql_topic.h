#define T_TABLE "topics"

int mono_sql_t_create_topic( const topic_t * top );
int mono_sql_t_rename_topic( unsigned int topic_id, const char *newname );
int mono_sql_kill_topic(int topic_id);

int mono_sql_t_updated_highest( unsigned int, unsigned int m_id );
int mono_sql_t_get_new_message_id( unsigned int , unsigned int * );

