
/* prototypes */

extern void newbie_mark_as_read (int number_to_leave_unread);
extern void search(void);
extern void reset_lastseen_message(void);
extern int delete_message_wrapper(const unsigned int, const int);
extern int copy_message_wrapper(const unsigned int, const int, const int);
extern void lookup_anon_author(const unsigned int);
extern void x_message_to_mail(const char *, char *);
extern void message_clip(const char *);
extern void message_2_temp(const char *, char );
extern void purge_mail_quad(void);
extern int count_mail_messages(void);

