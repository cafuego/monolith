/* $Id$ */

/* prototypes */
void config_menu(void);
void change_host (user_t *);
void q_menu (void);
void change_profile ( const user_t *);
void edit_profile ( const user_t *);
void voting_booth (void);
void toggle_away (void);
void toggle_beeps (void);
void toggle_interbbs( void );
void print_user_stats ( const user_t *, const user_t *);
void show_online( int );
void profile_user( void );
void change_atho( int );
void change_flying( void );
void change_url( void );
void kickout_user( void );
void menu_options( void );
void test_ansi_colours(user_t * );
void mono_show_config( unsigned int num);
void menu_message(void);

/*** variables ***/
/* extern char profile_default[L_USERNAME+L_BBSNAME+2]; */

