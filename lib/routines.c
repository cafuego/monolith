/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifdef HAVE_TERMBITS_H
#undef HAVE_TERMIO_H
#include <termbits.h>
#endif

#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif

#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "monolith.h"
#include "telnet.h"

#define extern
#include "routines.h"
#undef extern

#include "btmp.h"
#include "log.h"
#include "inter.h"
#include "routines2.h"
#include "userfile.h"

static struct termio stty0;	/* SUN terminal settings */


float
time_function(const int action)
{
    static struct timeval start_time;
    struct timeval stop_time;
    float seconds = 0;

    struct timezone porcupine;  /* placeholder in gettimeofday call */

    if (action == TIME_START)
	gettimeofday(&start_time, &porcupine);
    else {
	gettimeofday(&stop_time, &porcupine);
	if ((seconds = stop_time.tv_sec - start_time.tv_sec) > 180)
	    return seconds;
        seconds = (stop_time.tv_usec - start_time.tv_usec);
    }
    return seconds / 1000000;
}

int
set_timezone( const char *tz )
{
    int ret;
    char str[50];

    if ( !tz || !strlen(tz) ) return -1;

    ret = sprintf( str, "TZ=%s", tz );
    if ( ret == -1 ) return -1;

    ret = putenv( str );
    if ( ret == -1 ) return -1;
    tzset();
    return ret;
}

/* date()
 * return value points to static buffer, must be used
 * before next call to ctime()
 */
char *
date()
{
    time_t t;

    (void) time(&t);
    return ctime(&t);
}

/* printfile()
 * dumps file to stdout, does not use colours.
 * returns -1 on error
 */
int
printfile(const char *name)
{
    int a = 0;
    FILE *fp;

    fp = xfopen(name, "r", FALSE);

    if (fp == NULL) {
	mono_errno = E_NOFILE;
	return -1;
    }
    while ((a = getc(fp)) != EOF) {
	if (a == 10)
	    putc(13, stdout);
	putc(a, stdout);
    }

    (void) fclose(fp);
    (void) fflush(stdout);

    return 0;
}
void *
xcalloc(size_t numb, size_t size)
{
    register void *value = calloc(numb, size);

    if (value == NULL) {
	mono_errno = E_NOMEM;
	perror("memory exhausted\n");
	(void) log_it("errors", "Memory exhausted.");
	exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MEMORY_LEAK
    allocated_ctr++;
    allocated_total++;
#endif

    return value;
}

/*
 * xmalloc() - our own malloc, with error notify 
 */
void *
xmalloc(size_t size)
{
    register void *value = malloc(size);

    if (value == NULL) {
	mono_errno = E_NOMEM;
	perror("memory exhausted\n");
	(void) log_it("errors", "Memory exhausted.");
	exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MEMORY_LEAK
    allocated_ctr++;
    allocated_total++;
#endif

    return value;
}

/*
 * xrealloc() - our own realloc, with error notify and extra +1 allocation.
 */
void *
xrealloc(void *pointer, size_t size)
{
    register void *value = realloc(pointer, size+1);

    if (value == NULL) {
        mono_errno = E_NOMEM;
        perror("memory exhausted\n");
        (void) log_it("errors", "Memory exhausted.");
        exit(EXIT_FAILURE);
    }
    return value;
}

void
xfree(void *pointer)
{

    fprintf(stderr, "\n\rTrying to free %u bytes at 0x%p\n", sizeof(pointer), pointer);
    fflush(stderr);

    if (pointer == NULL)
	return;

#ifdef DEBUG_MEMORY_LEAK
    allocated_ctr--;
#endif

    free(pointer);
    return;
}

/* xopen() - open() with error notify */
#define eopen xopen
int
xopen(const char *name, int mode, int critical)
{
    int ret;

    ret = open(name, mode, S_IRWXU);

    if (ret < 0) {
	(void) log_it("errors", "Cannot open() file '%s', mode=%d err=%s", name, mode, strerror(errno));
	(void) fprintf(stderr, "Could not open a file, error has been logged.\n");
	if (critical == TRUE)
	    exit(EXIT_FAILURE);
    }
    return ret;
}

/* fexists()
 * returns TRUE if a file exists, FALSE if it doesn't.
 */
int
fexists(const char *fname)
{
    /* access is nice, but it uses the REAL uid, in stead of EFFECTIVE uid */
#ifdef ACCESS
    if (access(fname, F_OK) == 0) {
	return TRUE;
    } else {
	return FALSE;
    }
#endif
    int fd;

    fd = open(fname, O_RDONLY);
    if (fd == -1)
	return FALSE;
    (void) close(fd);
    return TRUE;
}

/*
 * back()
 * does a number of destructive backspaces
 */
void
back(unsigned int spaces)
{
    unsigned int a;

    for (a = 0; a < spaces; a++) {
	putchar(8);
	putchar(32);
	putchar(8);
    }
}

/*************************************************
* pattern()
*************************************************/
int
pattern(const char *search, const char *patn)
{
    if (strstr(search, patn) != NULL)
	return TRUE;
    else
	return FALSE;
}

/*************************************************
* externstty()
*************************************************/

/* needed to set terminal parameters for things not controlled by
 * yawc, like shell, subsystem, info system, etc */

int
externstty()
{
    struct termio live;

    /* maybe add error checking ? */
    ioctl(0, TCGETA, &live);
    live.c_iflag = live.c_iflag | ICRNL;
    ioctl(0, TCSETA, &live);

    return 0;
}

/*************************************************
* sttybbs()
*************************************************/

int
sttybbs(int sflag)
{
    struct termio live;

    /* add error checking? */

    ioctl(0, TCGETA, &live);
    live.c_iflag = ISTRIP | IXON | IXANY | ICRNL;
    live.c_oflag = OPOST | ONLCR;
    live.c_lflag = NOFLSH;
    if (sflag == 1)
	live.c_lflag = ISIG | NOFLSH;
    live.c_line = 0;
    live.c_cc[0] = 15;
    live.c_cc[1] = 3;
    live.c_cc[2] = 8;
    live.c_cc[3] = 24;
    live.c_cc[4] = 1;
    live.c_cc[5] = 255;
    live.c_cc[6] = 0;
    live.c_cc[7] = 0;
    ioctl(0, TCSETA, &live);

    return 0;
}

int
taboo(const char *iname)
{
    FILE *fp;
    char *aaa, bbb[30];
    size_t s;
    int frog;
    int a;

    fp = xfopen(TABOONAMES, "r", FALSE);
    if (fp == NULL) {
	mono_errno = E_NOFILE;
	return 0;
    }
    aaa = strdup(iname);
    if (aaa == NULL) {
	mono_errno = E_NOMEM;
	return 0;
    }
    s = strlen( iname );
    for (a = 0; a <= s; ++a)
	aaa[a] = tolower(iname[a]);

    frog = 0;
    while (fscanf(fp, "%s", bbb) != EOF) {
	if (bbb[0] == '#')
	    continue;
	if (strstr(aaa, bbb) != NULL)
	    frog++;
    }

    xfree(aaa);
    (void) fclose(fp);
    return frog;
}

/*************************************************
* all returned as a pointer to the malloc'ed
* string.
*
* !! Don't use 2 or more of these in the same 
*    printf. It all points to the same static string
*************************************************/

char *
printdate(time_t timetoprint, int format)
{
    struct tm *tp;
    static char whole[60];

    tp = localtime(&timetoprint);

    strcpy(whole, "ERROR!");

    switch (format) {
	case -1:
	    (void) strftime(whole, sizeof(whole), "%A %d %b %Y %H:%M:%S %Z", tp);
	    break;
	case 0:
	    (void) strftime(whole, sizeof(whole), "\1f\1g%A %d %B %Y %X %Z\1a", tp);
	    break;
	case 1:
	    (void) strftime(whole, sizeof(whole), "%d %b %Y %X %Z", tp);
	    break;
	case 2:
	    (void) strftime(whole, sizeof(whole), "%X", tp);
	    break;
    }

    return whole;
}

void
restore_term()
{
    ioctl(0, TCSETA, &stty0);
    return;
}

int
store_term()
{
    ioctl(0, TCGETA, &stty0);
    return 0;
}

/* string remove colors */
int
strremcol(char *p)
{

    char *q, *r;

    r = q = strdup(p);
    if (r == NULL) {
	mono_errno = E_NOMEM;
	return -1;
    }
    *p = '\0';
    while (*r) {
	if (*r == 1)
	    r++;
	else
	    *p++ = *r;
	r++;
    }
    xfree(q);
    *p = '\0';
    return 0;
}

FILE *
xfopen(const char *name, const char *mode, int fatal)
{
    FILE *ret;

    ret = fopen(name, mode);
    if (ret == NULL) {
	(void) log_it("errors", "Cannot fopen() file: %s, mode: %s, err=%s", name, mode, strerror(errno));
	(void) fprintf(stderr, "Could not open a file, error has been logged.\n");
	if (fatal == TRUE)
	    /* logoff(1); */
	    exit(EXIT_FAILURE);
    }
    return ret;
}

/*************************************************
* de_colorize()
* de-colorizes a file, meaning that it takes away
* the colorcodes that are in it. all \01c's and so
* on... used by clip_board()
*************************************************/
int
de_colorize(const char *fname)
{

    FILE *fp, *fp2;
    char work[100], *file;
    int b;

    file = tempnam(BBSDIR "/tmp", "decol");
    if (file == NULL)
	return -1;

    fp = xfopen(fname, "r", FALSE);
    if (fp == NULL) {
	mono_errno = E_NOFILE;
	return -1;
    }
    fp2 = xfopen(file, "w", FALSE);

    if (fp2 == NULL) {
	(void) fclose(fp);
	mono_errno = E_NOFILE;
	return -1;
    }
    while ((b = fgetc(fp)) != EOF) {
	if (b == '\01')		/* * read in the next character and skip 'em
				 * * * * * * both */
	    getc(fp);
	else
	    putc(b, fp2);
    }
    (void) fclose(fp);
    (void) fclose(fp2);
    /*  if ( rename (file, fname ) == -1 ) return 0; */
    (void) sprintf(work, "mv -f %s %s", file, fname);
    xfree(file);
    if (system(work))
	return -1;
    return 0;
}

/*************************************************
*
* file_line_len()
*
* returns the amount of lines in the (ascii) file
* 'fname'. Used to count the -- more (35%) -- at
* places where text-files are used.
*
*************************************************/

size_t
file_line_len(FILE * fp)
{
    char work[100];
    size_t l = 0;

    while (!feof(fp)) {
	if (fgets(work, 90, fp))
	    l++;
    }
    rewind(fp);
    return l;
}


#define BUFSIZE 200
int
copy(const char *source, const char *dest)
{

    int fd1, fd2;
    unsigned int a;
    char buf[BUFSIZE + 1];

    if (fexists(dest)) {
	(void) log_it("errors", "copy(), %s->%s failed, file already exists", source, dest);
	return -1;
    }

    fd1 = xopen(source, O_RDONLY, FALSE);
    if (fd1 == -1) {
	mono_errno = E_NOFILE;
	return -1;
    }
    fd2 = open(dest, O_WRONLY | O_CREAT, 0600);
    if (fd2 == -1) {
	(void) close(fd1);
	mono_errno = E_NOPERM;
	return -1;
    }
    while ((a = read(fd1, buf, BUFSIZE))) {
	if (a == -1) {
	    (void) close(fd1);
	    (void) close(fd2);
	    return -1;
	}
	write(fd2, buf, a);
    }
    (void) close(fd1);
    (void) close(fd2);
    return 0;
}

/*
 * Returns file contents in a char *
 */
char *
map_file(const char *filename)
{
    char *tmpbuf = NULL, *message = NULL;
    struct stat buf;
    int fd = -1;

    /*
     * Open file.
     */
    if( (fd = open(filename, O_RDONLY)) == -1) {
        (void) log_it("errors", "Cannot open() %s, mode: O_RDONLY, %s", filename, strerror(errno) );
        return NULL;
    }

    /*
     * Determine file size.
     */
    if( (fstat(fd, &buf)) == -1) {
        (void) close(fd);
        (void) log_it("errors", "Cannot fstat() %s, %s", filename, strerror( errno ) );
        return NULL;
    }

    /*
     * mmap() file.
     */ 
#ifdef HAVE_MAP_FAILED
    if( (tmpbuf = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED ) { 
#else
    if( (tmpbuf = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == ((__ptr_t) -1) ) { 
#endif
        (void) xfree(tmpbuf);
        (void) close(fd);
        (void) log_it("errors", "Can't mmap() file: %s; error: %s", filename, strerror(errno));
        return NULL;
    }

    /*
     * Close file.
     */
    (void) close(fd);

    /* 
     * Check for weirdness (we can't more_string a 0 length string)
     */
    if( (tmpbuf == NULL) || (strlen(tmpbuf) < 1) ) {
        (void) xfree(tmpbuf);
        (void) log_it("errors", "Bad mmap(): %s, %s", filename, strerror(errno));
        (void) log_it("errors", "File size was %lu bytes.", buf.st_size);
        return NULL;
    }

    /*
     * Copy buffer contents into `message'.
     */
    message = (char *) xmalloc( strlen(tmpbuf) );
    (void) memset(message, 0, strlen(tmpbuf) );
    (void) snprintf(message, strlen(tmpbuf), "%s", tmpbuf );

    /*
     * Kill buffer.
     */
    if( (munmap(tmpbuf, buf.st_size)) == -1) {
        (void) xfree(tmpbuf);
        (void) log_it("errors", "Can't munmap() file %s; err=%s", filename,strerror(errno));
    }

    return message;
}

/* eof */
