
char * show_user_cache(void);
void flush_user_cache(void);
int is_cached_username(const char *);
int is_cached_usernumber(const unsigned int );
void start_user_cache(const unsigned int);
char * cached_x_to_name(const int);
int cached_name_to_x(const char *);
int is_cached_friend(const char *);
int is_cached_enemy(const char *);
int mono_cached_sql_u_name2id(const char * , unsigned int * );
int mono_cached_sql_u_id2name(const unsigned int , char * );
