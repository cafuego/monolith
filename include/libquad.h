/* $Id$ */

/* prototypes */
extern int change_QL(unsigned int, const char *, int);
extern room_t read_quad(unsigned int);
extern int write_quad(room_t, int);
extern int dump_quickroom(void);
extern unsigned long get_new_mail_number(const char *);
extern unsigned int get_room_number(const char *name);
extern int is_ql(const char *, room_t);
extern int kickout(const char *user, unsigned int room);
extern int invite(const char *user, unsigned int room);
extern char * search_msgbase(char *, unsigned int, unsigned long, user_t *);

#define PORCUPINE
#ifdef PORCUPINE
extern void fpgetfield(FILE *, char *);
extern int make_auto_message(const char *fromfile, const char *tofile, post_t );
extern int read_post_header( FILE *fp, post_t * );
extern int write_post_header( FILE *fp, post_t );
extern int write_post_footer(FILE *, int);
char * post_to_file(unsigned int, unsigned long, const char *);
extern unsigned long get_new_post_number(unsigned int);
extern int save_message(const char *, unsigned int , const char *);
extern int delete_message(unsigned int , unsigned long , const char *mailbox );
extern int trash_message(unsigned int , unsigned long , const char *mailbox );
extern int move_message(unsigned int,unsigned long,unsigned int,const char *);
extern int copy_message(unsigned int,unsigned long,unsigned int,const char *);
#endif

/* eof */
