/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/file.h>		/* for flock */
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"

room_t quickroom_s;
FILE *fp;

int
main(int argc, char **argv)
{

    int i, j = 0;

    set_invocation_name(argv[0]);

    mono_connect_shm();

    fp = fopen( BBSDIR "www/bbs/rooms/rooms.html", "w");
    if (fp == NULL)
	return 1;

    /* MIME header for HTML documents */

    chdir(BBSDIR);

    fprintf(fp, "<HTML><HEAD>\n");
    fprintf(fp, "<TITLE>The list of know rooms</TITLE>\n");
    fprintf(fp, "<!--#include virtual=\"/include/head.html\"-->\n");
    fprintf(fp, "</HEAD>\n");
    fprintf(fp, "<!--#include virtual=\"/include/body.html\"-->\n");

    fprintf(fp, "<MAP NAME=\"bbs_quads\">\n");
    fprintf(fp, "<AREA SHAPE=rect COORDS=\"145,54,263,77\" HREF=\"/bbs/\">\n");
    fprintf(fp, "<AREA SHAPE=poly COORDS=\"2,2,298,2,298,52,143,52,143,95,3,95,2,2\" HREF=\"/\">\n");
    fprintf(fp, "</MAP>\n");

    fprintf(fp, "<p align=\"center\">\n");
    fprintf(fp, "<img src=\"/pix/banners/bbs_quadrants.jpg\" height=\"32\" width=\"468\">\n");
    fprintf(fp, "<p align=\"left\">Don't want others to see what you are reading?\n");
    fprintf(fp, "Then why not use our <a href=\"https://cal022011.student.utwente.nl/bbs/rooms/\">Secure Webserver</a>?<p>\n");

    fprintf(fp, "<CENTER><TABLE BORDER=2 CELLPADDING=2>");

//    fprintf( fp , "<TR><TD ALIGN=CENTER COLSPAN=3><H3>The list of known quadrants</H3></TD></TR>\n");

    for (i = 0; i < MAXQUADS; i++) {
	quickroom_s = read_quad(i);

	if (quickroom_s.name[0] != '\0' &&
	    (!(quickroom_s.flags & QR_PRIVATE) || !(quickroom_s.flags & QR_INUSE)) &&
	    (i == 0 || (i > 10 && i != 13))) {

	    if (j++ % 3 == 0)
		if (j == 1) {
		    fprintf(fp, "<TR>\n");
		} else {
		    fprintf(fp, "\n</TR><TR>\n");
		}
	    fprintf(fp, "<TD ALIGN=CENTER>");

	    fprintf(fp, "<A HREF=\"/cgi-bin/interface?quad=%d&number=%ld\">%s</A>"
		    ,i, quickroom_s.highest, quickroom_s.name);

	    fprintf(fp, "\n</TD>\n");
	}
    }

    fprintf(fp, "</TABLE></CENTER>\n");
    fprintf(fp, "<!--#include virtual=\"/include/foot.html\"--> ");
    fprintf(fp, " </BODY></HTML>\n");

    fclose(fp);
    mono_detach_shm();
    return 0;
}
