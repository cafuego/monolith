
/* prototypes */

void newbie_mark_as_read (int number_to_leave_unread);
void search(void);
void reset_lastseen_message(void);
int delete_message_wrapper(const unsigned int, const int);
int copy_message_wrapper(const unsigned int, const int, const int);
void lookup_anon_author(const unsigned int);
void x_message_to_mail(const char *, char *);
void message_clip(const char *);
void message_2_temp(const char *, char );
void purge_mail_quad(void);
int count_mail_messages(void);
void search_via_sql(unsigned int forum);
void rate_message(message_t * , unsigned int , unsigned int );
