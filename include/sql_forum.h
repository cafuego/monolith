extern unsigned int mono_sql_f_get_new_message_id( unsigned int forum );
extern int mono_sql_f_add_old_forum(unsigned int , room_t * const );
extern int mono_sql_f_remove_forum(int);
extern int mono_sql_f_read_quad(unsigned int num, room_t * room);
extern int mono_sql_f_write_quad(unsigned int,  room_t * const);
extern int mono_sql_f_kill_forum(unsigned int );
extern int mono_sql_f_rename_forum( unsigned int , char *);

extern int mono_sql_f_name2id(const char *forumname, unsigned int *forumid);
extern int mono_sql_f_partname2id(const char *forumname, unsigned int *forumid);
extern int mono_sql_f_update_forum(unsigned int forum_id, const room_t * q);

#define F_TABLE "forum"
