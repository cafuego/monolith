/* $Id$ */

/* prototypes */
extern void yell( void );
extern void search( void );
extern void enter_message_with_title(void);
extern void direct_mail( void );

extern int make_message( const char *, char * const, int, int);
extern int show_message( int, long );
extern void purge_mail_quad( void );
extern long count_mail_messages( void );
extern int we_can_post( void );

extern int msgform( const char *, int );

extern void read_menu( long, int );
extern int entmsg( int, int);

extern void post_file(const char *, post_t );
extern void make_post_header(FILE * fp, post_t );

extern int get_room_name( const char * );
extern int delete_mail( int room, int num );
extern void newbie_mark_as_read (int number_to_leave_unread);
extern void display_read_prompt(const long, const int);
extern int validate_read_command(int);

extern int xcopy_message(const long, const int);
extern int xdelete_message(const long);

extern void lookup_anon_post(const long);
extern long numeric_read(const long);
extern long get_read_start_number(const long, const int);
extern void clip_post(const long);
extern int set_read_bounds(long *, long *);
extern int no_new_posts_here(const long, const int, const long);


extern char * get_multimail_names( int , char *);
extern int set_message_type(const int, int);
extern int save_handled_yell(char *);
extern int save_mail(char *);

extern void get_newmessage_subjectline(post_t * const);
extern int get_newmessage_replystuff(post_t * const);

extern void get_newmessage_alias(post_t * const);

extern int format_post_subjectline(const post_t * const, char *, int);
extern int format_post_formatline(const post_t * const, char *, char *, int);
extern int process_post(const post_t *const, const char *, const char *, const int);
