/* $Id$ */

/* prototypes */
extern int change_QL(unsigned int, const char *, int);
extern room_t read_quad(unsigned int);
extern int read_forum( unsigned int, room_t * );
extern int write_forum( unsigned int, room_t * );
extern int write_quad(room_t, int);
extern int dump_quickroom(void);
extern unsigned long get_new_mail_number(const char *);
extern unsigned int get_room_number(const char *name);
extern int is_ql(const char *, room_t);
extern int kickout(const char *user, unsigned int room);
extern int invite(const char *user, unsigned int room);
extern char * search_msgbase(char *, unsigned int, unsigned long, user_t *);
extern unsigned int get_new_message_id(const unsigned int); 

/* eof */
