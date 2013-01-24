/* $Id$ */

/* defines */
#define DELIM	"|"
#define NSUBEXP  64
#define SHIX_SCOREFILE BBSDIR "/etc/shix.scores"

#define MAGIC   0234

typedef struct {
    char word[100];
    int score;
} shix_t;

typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	unsigned int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

/* prototypes */
void regerror(char *);
int shix( const char *message );
regexp * regcomp( char *exp);
int regexec( regexp *prog, char *string);

int shix_strmatch(char *, char *);
int shix_valid( char * );
