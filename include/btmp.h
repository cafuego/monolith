/* $Id$ */

extern int mono_add_loggedin(const user_t *);
extern pid_t mono_return_pid(const char *);
extern int mono_remove_loggedin( const char *user);
extern int mono_remove_ghosts( void );
extern int mono_change_online(const char *, const char *tmp_string, int ch);
extern int mono_send_signal_to_the_queue(void);
extern int mono_lock_rooms( int key );
extern int mono_lock_shm(int key);
extern int mono_lock_queue(int key);
extern int mono_show_queue(void);
extern btmp_t *mono_read_btmp(const char *name);
extern int mono_connect_shm(void);
extern int mono_detach_shm(void);
extern btmp_t *mono_search_guide(void);
extern express_t *mono_find_xslot(const char *name);
extern int mono_find_x_ing( const char *name, char *xer );
extern char * who_am_i(const char *);

extern bigbtmp_t *shm;
