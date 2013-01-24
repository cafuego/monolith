/* $Id$ */

/* prototypes */

#define TIME_STOP 0
#define TIME_START 1
float time_function(const int );

int set_timezone( const char * );
int printfile ( const char *name);
int xopen ( const char *name, int mode, int critical );
char *date (void);
int fexists ( const char *fname);
void back ( unsigned int spaces);
int pattern (const char *, const char *);
int externstty (void);
int sttybbs (int sflag);
int taboo (const char *iname );
char *printdate (time_t timetoprint, int format);
int resetclient (void);
void *xcalloc (size_t, size_t);
void *xmalloc (size_t);
void *xrealloc (void *, size_t);
void xfree( void *pointer );
int store_term (void);
void restore_term (void);
int strremcol( char *);
FILE *xfopen ( const char *, const char *, int fatal );
int de_colorize( const char * );
size_t file_line_len(FILE * fp);
int copy( const char *source, const char *destination );
char *map_file(const char *filename);
