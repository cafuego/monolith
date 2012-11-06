/* $Id$ */

int mono_add_loggedin(const user_t *);
pid_t mono_return_pid(const char *);
int mono_remove_loggedin( const char *user);
int mono_remove_ghosts( void );
unsigned int mono_fix_usercount( void );
int mono_change_online(const char *, const char *tmp_string, int ch);
int mono_lock_rooms( int key );
int mono_lock_shm(int key);
btmp_t *mono_read_btmp(const char *name);
int mono_connect_shm(void);
int mono_detach_shm(void);
btmp_t *mono_search_guide(void);
express_t *mono_find_xslot(const char *name);
int mono_find_x_ing( const char *name, char *xer );
char * who_am_i(const char *);

extern bigbtmp_t *shm;
