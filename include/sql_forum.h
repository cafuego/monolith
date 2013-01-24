int mono_sql_f_add_old_forum(unsigned int , room_t * const );
int mono_sql_f_remove_forum(int);
int mono_sql_f_read_quad(unsigned int num, room_t * room);
int mono_sql_f_write_quad(unsigned int,  room_t * const);
int mono_sql_f_kill_forum(unsigned int );
int mono_sql_f_rename_forum( unsigned int , char *);

int mono_sql_f_name2id(const char *forumname, unsigned int *forumid);
int mono_sql_f_partname2id(const char *forumname, unsigned int *forumid);
int mono_sql_f_update_forum(unsigned int forum_id, const room_t * q);

int mono_sql_f_get_highest( forum_id_t num);

void mono_sql_f_fix_quickroom(void);

#define F_TABLE "forum"
