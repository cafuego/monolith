/* $Id$ */

extern void getword(char *word, char *line, char stop);
extern char x2c(char *what);
extern void unescape_url(char *url);
extern void plustospace(char *str);
extern void hprintf(const char *);
extern void read_files( int room, long number );
extern void parse_input( void );
extern int www_msgform( void );
extern void fmout2(FILE * fp, char flag);
extern char *makeword(char *line, char stop);
extern char *fmakeword(FILE *f, char stop, int *cl);

