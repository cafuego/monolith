/* $Id$ */

void getword(char *word, char *line, char stop);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);
void hprintf(const char *);
void read_files( int room, long number );
void parse_input( void );
int www_msgform( void );
void fmout2(FILE * fp, char flag);
char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *cl);

