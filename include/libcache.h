typedef struct user_cache_type {
    char name[L_USERNAME + L_BBSNAME + 2];
    unsigned int user_number;
    int friend;
    int quickx;
    struct user_cache_type *next;
    } user_cache_t;

extern void show_user_cache(void);
extern void flush_user_cache(void);
extern int is_cached_username(const char *);
extern int is_cached_usernumber(const unsigned int );
extern void start_user_cache(void);
extern char * cached_x_to_name(const int);
extern int cached_name_to_x(const char *);
extern int is_cached_friend(const char *);
extern int is_cached_enemy(const char *);
extern int mono_cached_sql_u_name2id(const char * , unsigned int * );
extern int mono_cached_sql_u_id2name(const unsigned int , char * );
