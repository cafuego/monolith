extern unsigned int mono_sql_f_get_new_message_id( unsigned int forum );
extern int mono_sql_f_add_old_forum(unsigned int , room_t * const );
extern int mono_sql_f_remove_forum(int);
extern int mono_sql_f_read_quad(unsigned int num, room_t * room);
extern int mono_sql_f_write_quad(unsigned int,  room_t * const);
extern int mono_sql_f_kill_forum(int );
extern int mono_sql_f_rename_forum( unsigned int , char *);

#define F_TABLE "forum"
