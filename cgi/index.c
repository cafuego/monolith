/* a www interface to the yawc messages system */
/* by michel */

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>		/* for flock */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"

#define DEBUG 2

int room;
room_t scratch;

typedef struct {
    char name[128];
    char val[128];
} entry;

extern void getword(char *word, char *line, char stop);
extern char x2c(char *what);
extern void unescape_url(char *url);
extern void plustospace(char *str);

void parse_input(void);
int main(int, char **);
void fmout2(FILE * fp, char flag);
int www_msgform(void);

void
parse_input()
{
    int m, a;
    char *cl;
    entry entries[10];

    cl = getenv("QUERY_STRING");
    if (cl == NULL) {
	printf("No query information to decode.\n");
	exit(1);
    }
    for (a = 0; cl[0] != '\0'; a++) {
	m = a;
	getword(entries[a].val, cl, '&');
	plustospace(entries[a].val);
	unescape_url(entries[a].val);
	getword(entries[a].name, entries[a].val, '=');
    }

    room = atoi(entries[0].val);

}

/* this function reads the message from a file, and prints it */
void
fmout2(FILE * fp, char flag)
{
    int a;
    char aaa[200], *p;

    strcpy(aaa, "");
    p = aaa;

    for (;;) {
	a = getc(fp);
	if (a <= 0)
	    break;
	if (a > 126)
	    continue;
	if (a >= 32 || a == 9 || a == 10 || a == 13) {
	    *p++ = a;
	    *p++ = 0;		/* silly way to add the \0 */
	    p--;
	}
	if (a == 1) {
	    a = getc(fp);	/* ignore this, and following char */
	    continue;
	}
	if ((a == 13) || (a == 10)) {
	    printf("%s", aaa);
	    strcpy(aaa, "");
	    p = aaa;
	}
    }
    return;
}


/* this gets one field from the file fp points to */
void
fpgetfield(FILE * fp, char string[])
{
    /* level-2 break out next null-terminated string */
    int a, b;

    strcpy(string, "");
    a = 0;
    do {
	b = getc(fp);
	if (b < 1) {
	    string[a] = 0;
	    return;
	}
	string[a] = b;
	++a;
    }
    while (b != 0);

}


/*************************************************
* msgform()
* this displays the actual message in html
*************************************************/

int
www_msgform()
{
    long spos;
    int b, e, mtype, aflag, finished_flag = 0;
    char bbb[120], format[160];	/* format is for the header      */
    char mfile[50], poster[25], subject[81], datum[40];
    FILE *fp;
    time_t curr_time;

    spos = 0;
    sprintf(mfile, QUADDIR "%d/description", room);

    fp = fopen(mfile, "rb");
    if (fp == NULL) {
	printf("Couldn't open message file.<BR>\n");
	printf("File: %s\n", mfile);
	printf("Pos: %ld\n", spos);
	exit(1);
    }
    fseek(fp, spos, 0);
    e = getc(fp);
    if (e != 255) {
	printf("\n ### AUGH! - cannot locate message at pos (%ld) ###\n", spos);
	fclose(fp);
	return 1;
    }
    mtype = getc(fp);
    aflag = getc(fp);

    strcpy(format, "");
    strcpy(subject, "");

    do {
	b = getc(fp);

	if (b == 'M') {
	    printf("<HTML><TITLE>\n");
	    printf("Quadrant Info for %s\n", scratch.name);
	    printf("</TITLE><BASE HREF=\"http://cal022011.student.utwente.nl/\">\n");

	    printf("<link rev=\"made\" href=\"mailto:webmaster@cal022011.student.utwente.nl\">\n");
	    printf("<meta name=\"description\" content=\"Monolith BBS!\">\n");
	    printf("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
	    printf("</head>\n");
	    printf("<body bgcolor=\"#FFFFFF\" text=\"#000000\" link=\"#008740\" alink=\"#00FFFF\" vlink=\"#008F93\">\n");

	    printf("<H2><ALIGN=LEFT>%s\n", scratch.name);
	    printf("<ALIGN=RIGHT>%s</H2><BR>\n", datum);
	    printf("<H3>Last modified by: <A HREF=\"/cgi-bin/profile?name=%s\">%s</A></H3>\n", poster, poster);

//            printf("<table border=\"0\" bgcolor=\"#000000\">\n");
	    //            printf("<tr><td>\n");
	    printf("<hr><br><PRE> <!-- Begin of message -->\n");
//            printf("<font color=\"#ffffff\">\n");
	    fmout2(fp, 0);
	    printf("<br><hr></PRE> <!-- End of message -->\n");
//            printf("</td></tr>\n");
	    //            printf("</table>\n");

	    printf("<FONT SIZE=1><HR><CENTER><ADDRESS>");
	    printf("(<A HREF=mailto:webmaster@cal022011.student.utwente.nl");
	    printf(">webmaster@cal022011.student.utwente.nl</A>)");
	    time(&curr_time);
	    printf("</ADDRESS>&copy; Monolith Development -- %s",
		   ctime(&curr_time));
	    printf("</CENTER></FONT><P>\n\n");

	    printf("</BODY></HTML>\n");
	}
	if ((b != 'M') && (b > 0)) {
	    fpgetfield(fp, bbb);
	}
	if (b == 'Y') {
	    sprintf(format + strlen(format), "(#%s) ", bbb);
	    continue;
	}
	if (b == 'D') {		/* deleted/moved post */
	    sprintf(format + strlen(format), "[%s] ", bbb);
	    continue;
	}
	if (b == 'A') {		/* Author                               */
	    if (mtype == MES_ANON || mtype == MES_AN2) {
		switch (mtype) {
		    case MES_ANON:
			sprintf(poster, "*****");
			break;

		    case MES_AN2:
			sprintf(poster, "*anonymous*");
			break;

		    default:
			break;
		}
	    } else {
		switch (mtype) {
		    case MES_WIZARD:
			sprintf(poster, "<B>Emperor %s</B>", bbb);
			break;

		    case MES_SYSOP:
			sprintf(poster, "<B>Fleet Commander %s </B>", bbb);
			break;

		    case MES_TECHNICIAN:
			sprintf(poster, "<I>Systems Analyst %s</I>", bbb);
			break;

		    case MES_ROOMAIDE:
			sprintf(poster, "<I>RoomAide %s</I>", bbb);
			break;

		    case MES_FORCED:
			sprintf(poster, "%s (FORCED MESSAGE ) ", bbb);
			break;

		    default:
			sprintf(poster, "%s ", bbb);
			break;
		}
	    }
	    continue;
	}
	if (b == 'E') {
	    /*
	     * might have to do something else with the usernumber later?
	     *
	     * this finished_flag is here because of the call to msgform() by
	     * make_message: so that we can see the header of our post.
	     * [not used anymore?]
	     */
	    finished_flag = 1;
	}
	if (mtype != MES_ANON && mtype != MES_AN2) {
	    if (b == 'O' && EQ(bbb, scratch.name) && mtype != MES_DESC) {
		sprintf(format + strlen(format), "in %s> ", bbb);
	    }
	    if (b == 'R') {	/* replty to */
		sprintf(format + strlen(format), "to %s ", bbb);
	    }
	    if (b == 'T') {
		sprintf(datum, "%s ", printdate(atol(bbb), -1));
	    }
	}
    }
    while ((b != 'M') && (b > 0));

    fclose(fp);
    return (0);
}

int
main(int argc, char **argv)
{
    set_invocation_name(argv[0]);

    printf("Content-type: text/html%c%c", 10, 10);
    /* MIME header for HTML documents */

    parse_input();
    /* this translates the arguments to the room and message number */

#ifdef DEBUG
    printf("<!-- DEBUG: Trying to read roominfo for room #%d.-->\n"
	   ,room);
#endif

    chdir(BBSDIR);
    mono_connect_shm();
    read_forum( room, &scratch );
    mono_detach_shm();
    /* this reads the necessary structure, roominfo, quickroom etc files */

    if ((room <= 10 && room != 0) || scratch.flags & QR_PRIVATE) {
	/* the rooms <= 10 are for sysops only */
	printf("<HTML><HEAD><TITLE>Not Allowed</TITLE></HEAD><BODY>\n");
	printf("<H2>Sorry, but you're not allowed to read these rooms via");
	printf(" the world wide web.</H2></BODY>\n");
	printf("<P><ADDRESS>Kirth</ADDRESS>\n");
	exit(1);
    }
    /* okay, we're allowed to read the message, so let's go! */
    www_msgform();
    return 0;
}
