/* $Id$ */

/* prototypes */
extern void change_host (user_t *);
extern void q_menu (void);
extern void change_profile ( const user_t *);
extern void edit_profile ( const user_t *);
extern void voting_booth (void);
extern void toggle_away (void);
extern void toggle_beeps (void);
extern void toggle_interbbs( void );
extern void print_user_stats ( const user_t *, const user_t *);
extern void show_online( int );
extern void profile_user( void );
extern void change_atho( int );
extern void change_flying( void );
extern int change_alias( void );
extern void change_url( void );
extern void kickout_user( void );
extern void menu_options( void );
extern void test_ansi_colours(user_t * );  /* russ */
extern void mono_show_config( unsigned int num);
extern void menu_message(void);

/*** variables ***/
extern char profile_default[L_USERNAME+L_BBSNAME+2];

