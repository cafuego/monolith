/* $Id$ */

/* prototypes */

extern int printfile ( const char *name);
extern int xopen ( const char *name, int mode, int critical );
extern char *date (void);
extern int fexists ( const char *fname);
extern void back ( unsigned int spaces);
extern int pattern (const char *, const char *);
extern int externstty (void);
extern int sttybbs (int sflag);
extern int taboo (const char *iname );
extern char *printdate (time_t timetoprint, int format);
extern int resetclient (void);
extern void *xcalloc (size_t, size_t);
extern void *xmalloc (size_t);
extern void *xrealloc (void *, size_t);
extern void xfree( void *pointer );
extern int store_term (void);
extern void restore_term (void);
extern int strremcol( char *);
extern FILE *xfopen ( const char *, const char *, int fatal );
extern int de_colorize( const char * );
extern size_t file_line_len(FILE * fp);
extern int copy( const char *source, const char *destination );
