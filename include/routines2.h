/* $Id$ */

/* prototypes */

extern int increment(int );
extern void interr ( const char *);
extern int yesno (void);
extern int yesno_default (int);
extern char get_single ( const char *);
extern char get_single_quiet ( const char *);
extern int more_string(char * const );
extern int more( const char *, int );
extern void more_wrapper(const unsigned int, const long, const char *);
extern void execute_unix_command( const char * );
extern void print_system_config( void );
extern int print_birthday(date_t bday);
extern void modify_birthday(date_t * bday);
extern int qc_get_pos_int(const char, int);
extern char * m_strcat(char *, const char *);
#ifdef CLIENTSRC
extern void save_colour(int key);
#endif
