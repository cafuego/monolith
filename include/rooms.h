/* $Id$ */

/* prototypes */
void killroom( void );
void look_into_quickroom( int );
void edit_room_field( room_t *, unsigned int, int );
void move_rooms( void );
void reset_room( void );
void whoknows( void );
void forget( void );
void editroom( void );
void reset_lastseen(void);
void usergoto (int where, int old_rm, int);
int jump( int );
void gotonext( void );
void gotocurr( void );
void skiproom( void );
int  goto_next_skipped( const int ); 
void show_known_rooms( int param );
char * known_rooms_list ( const user_t *, int );
void do_kickout( void );
void do_invite( void );
void print_type( void );
void room_debug_info( void );
void create_room( void );
void show_room_flags( void );
int check_messages(room_t room, int which);
void mark_as_read(int);
void leave_n_unread_posts(int, int); 
int is_zapped(int room, const room_t *);
void check_generation(void);
void zap_all( void );
char * unread_rooms_list(const user_t * );
void erase_all_messages( long );
room_t readquad( unsigned int ); /*EEK!*/
void edithosts_menu(void);
void invite_menu(void);
void kickout_menu(void);
void print_forumlist_list(forumlist_t *);
int get_room_name( const char * );
int unread_room(void);
int new_quadinfo(void);
int we_can_post(const unsigned int);
int i_may_read_forum(const unsigned int);
int i_may_write_forum(const unsigned int);
 
