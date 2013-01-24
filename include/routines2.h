/* $Id$ */

/* prototypes */

int increment(int );
void interr ( const char *);
int yesno (void);
int yesno_default (int);
char get_single ( const char *);
char get_single_quiet ( const char *);
int more_string(char * const );
int more( const char *, int );
void more_wrapper(const unsigned int, const long, void *);
void execute_unix_command( const char * );
void print_system_config( void );
int print_birthday(date_t bday);
void modify_birthday(date_t * bday);
void configure_sql(void);
int qc_get_pos_int(const char, int);
char * m_strcat(char *, const char *);
#ifdef CLIENTSRC
void save_colour(int key);
#endif
