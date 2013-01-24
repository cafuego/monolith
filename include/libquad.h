/* $Id$ */

/* prototypes */
int change_QL(unsigned int, const char *, int);
int read_forum( unsigned int, room_t * );
int write_forum( unsigned int, room_t * );
int write_quad(room_t, int);
int dump_quickroom(void);
unsigned long get_new_mail_number(const char *);
unsigned int get_room_number(const char *name);
int is_ql(const char *, room_t);
int kickout(const char *user, unsigned int room);
int invite(const char *user, unsigned int room);
char * search_msgbase(char *, unsigned int, unsigned long, user_t *);
unsigned int get_new_message_id(const unsigned int); 

/* eof */
