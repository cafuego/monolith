/* $Id$ */

/* prototypes */
char * post_to_file(int quadno, long number, const char *name);
extern int change_RAship( int room, const char *rausername, int action);

extern int may_read_room(user_t user, room_t room, int a);
extern int may_write_room(user_t user, room_t room, int a);

extern room_t read_quad(int);
extern int write_quad(room_t, int);
extern int dump_quickroom(void);

extern long get_new_post_number(int);
extern long get_new_mail_number(const char *);
extern int get_room_number(const char *name);

extern int new_quadinfo(void);
extern void show_desc(int);
extern void show_room_aides(void);
extern void show_qls(void);

extern int is_ql(const char *, room_t);
extern int kickout(const char *user, int room);
extern int invite(const char *user, int room);

extern int unread_room(void);

extern void fpgetfield(FILE *, char *);
extern int make_auto_message(const char *fromfile, const char *tofile, post_t );
extern int read_post_header( FILE *fp, post_t * );
extern int write_post_header( FILE *fp, post_t );
extern int write_post_footer(FILE *, int);

extern int save_message(const char *tempfile, int room, const char *recipient);
extern int delete_message(int room, long post, const char *mailbox );
extern int trash_message(int room, long post, const char *mailbox );
extern int move_message(int room, long post, int dest, const char *mailbox );
extern int copy_message(int room, long post, int dest, const char *mailbox );

extern char * search_msgbase( char *string, int room, long start, user_t *user );

/* eof */
