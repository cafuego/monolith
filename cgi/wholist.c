/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>

#include "monolith.h"
#include "libmono.h"
#include "cgi.h"


#define DEFAULT 0x1000000

int main (void);
void hprintf (const char *);
int special_char (char);
int color_code (char);
void show_wholist (void);
void make_html_head (const char *);
void make_html_foot (void);
char * make_html_name (const char *);


int
main()
{

    mono_connect_shm();
    printf("Content-type: text/html\n\n");
    show_wholist();
    mono_detach_shm();
    return 0;


}

void
show_wholist ()
{

  int tdif, idletime, min, hour, i;
  time_t t;
  struct tm *tp;
  char newname[21], *p;
  btmp_t *bp;

  t = time(0);
  tp = localtime(&t);

  printf ("<html><head>\n");
  printf ("<title>Monolith Wholist</title>\n");
  printf ("<link rev=\"made\" href=\"mailto:webmaster@monolith.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta http-equiv=\"Refresh\" content=\"60; url=/cgi-bin/wholist2\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("<link rel=\"StyleSheet\" href=\"/include/interface.css\" type=\"text/css\">\n");
  printf ("</head>\n");
  printf ("<body bgcolor=\"black\" text=\"white\" link=\"#FF0000\" alink=\"#00FF00\" vlink=\"#AAAAAA\">\n");

  printf ("<h3 align=\"center\">Monolith BBS Wholist</h3>\n");
  
  printf("<center>\n");
  printf("<table border=\"0\" cellpadding=\"5\" cellspacing=\"0\" width=\"94%%\" align=\"center\">\n");
  printf("<tr><td align=\"center\" colspan=\"4\">\n");
  if (shm->user_count == 1)
	printf("<p class=\"header\" align=\"center\">There is <font color=\"yellow\">one</font> user online ");
  else if (shm->queue_count == 0)
	printf("<p class=\"header\" align=\"center\">There are <font color=\"yellow\">%d</font> users online ", shm->user_count);
  else
	printf("<p class=\"header\" align=\"center\">There are <font color=\"yellow\">%d</font> users online <font color=\"yellow\">(</font> <font color=\"yellow\">%d</font> queued <font color=\"white\">)</font> ", shm->user_count, shm->queue_count);
  printf("at %02d:%02d CET.</TD></TR>\n", tp->tm_hour, tp->tm_min);
  printf("</td></tr>\n");
  
  printf("<tr><td align=\"center\" colspan=\"4\"><p>&nbsp;</p></td></tr>\n");

  printf("<tr>\n");
  printf("<td align=\"left\"><font color=\"#00FF00\"><b>Username</b></font></td>\n");
  printf("<td align=\"left\"><font color=\"#00FF00\"><b>Location</b></font></td>\n");
  printf("<td align=\"right\"><font color=\"#00FF00\"><b>Time</b></font></td>\n");
  printf("<td align=\"left\"><font color=\"#00FF00\"><b>Flying</b></font></td>\n");
  printf("</tr>\n");

  i = shm->first;
  while( i != -1 ) {

      bp = &(shm->wholist[i]);

      strcpy(newname, bp->username);
      while ((p = strchr(newname, ' ')) != 0)
	    *p = '+';
	  
	  tdif = time(0) - bp->logintime;
	  tdif /= 60;
	  min = tdif % 60;
	  hour = tdif / 60;
	  idletime = time(0) - bp->idletime;
	  idletime /= 60;

      printf("<tr>\n");
      printf("  <td align=\"left\"><p class=\"username\"><a onMouseover=\"window.status ='Profile user %s.'; return true\" href=\"/cgi-bin/interface?action=profile&profname=%s\"><font color=\"green\"><b>%s</b></font></a></td>\n",bp->username, newname, bp->username);
      printf("  <td align=\"left\"><p class=\"username\"><b><font color=\"green\">%s</font></b></td>\n", EQ(bp->remote,"") ? "&nbsp;" : bp->remote);	  
	  printf("  <td align=\"right\"><b><font color=\"yellow\">%2d</font><font color=\"white\">:</font><font color=\"yellow\">%02d</font></b></td>\n", hour, min);
	  printf("  <td align=\"left\">");
	  if (bp->flags & B_REALIDLE)
	    printf("<p><font color=\"green\">Abducted by aliens for</font> <font color=\"yellow\">%2.2d</font><font color=\"white\">:</font><font color=\"yellow\">%2.2d</font></td>\n", idletime / 60, idletime % 60);
	  else {
	    printf("<p><span style=\"font-weight: bold\"><font color=\"#00FF00\"><b>");
	    hprintf( bp->doing );
	    printf("</b></font></span>");
	  }
	  printf("</td>\n");
	  printf("</tr>\n");
	  
      i = bp->next;
  }
  
  printf("<tr><td align=\"center\" colspan=\"4\"><p>&nbsp;<p align=\"center\">This wholist is automagically updated every minute.</td></tr>\n");

  printf("</table>\n");
 
  printf("<center>\n");

  make_html_foot ();
}


void
hprintf(const char *string)
{
    const register char *p;
    static int new = DEFAULT, old = DEFAULT;

    p = string;
    do {
	if (*p == 1) {		/* CTRL-A */
	    p++;
	    new = color_code(*p);
	    if (old != new) {	/*  color changed!! */
		if (old == DEFAULT)
		    printf("<font color=#%06x>", new);
		else if (new == DEFAULT)
		    printf("</font>");
		else
		    printf("</font><font color=#%06x>", new);
	    }
	    old = new;
	} else {
	    if (!special_char(*p))
		printf("%c", *p);
	}
    }
    while (*p++);
    return;
}

int
color_code(char c)
{
    int new;

    switch (c) {
	case 'a':
	    new = DEFAULT;
	    break;
	case 'r':		/* red */
	    new = 0xff0000;
	    break;
	case 'g':		/* green */
	    new = 0x00ff00;
	    break;
	case 'y':		/* yellow */
	    new = 0xffff00;
	    break;
	case 'b':		/* blue */
	    new = 0x0000ff;
	    break;
	case 'p':		/* purple */
	    new = 0xff00ff;
	    break;
	case 'c':		/* cyan */
	    new = 0x00ffff;
	    break;
	case 'w':		/* white */
	    new = 0xffffff;
	    break;
	default:
	    new = 0x00ff00;
	    break;
    }

    return new;
}

int
special_char(char c)
{
    switch (c) {
	case 0:
	    return 1;
	case 13:
	    printf("\n");
	    return 1;
	case '<':
	    printf("&lt;");
	    return 1;
	case '>':
	    printf("&gt;");
	    return 1;
	case '&':
	    printf("&amp;");
	    return 1;
	case '"':
	    printf("&quot;");
	    return 1;
    }
    return 0;
}

void
make_html_head (const char *title)
{
  printf ("<html><head>\n");
  printf ("<title>%s</title>\n", title);
  printf ("<link rev=\"made\" href=\"mailto:webmaster@monolith.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("<link rel=\"StyleSheet\" href=\"/include/interface.css\" type=\"text/css\">\n");
  printf ("</head>\n");
  printf ("<body bgcolor=\"black\" text=\"DDFFFF\" link=\"#FF0000\" alink=\"#00FF00\" vlink=\"#AAAAAA\">\n");
  return;
}
/* creates the footer for the html page */
void
make_html_foot ()
{
  time_t curr_time;

  printf ("<p align=\"center\"><font size=\"-1\">Comments to: ");
  printf ("<A HREF=mailto:webmaster@monolith.student.utwente.nl");
  printf (">webmaster@monolith.student.utwente.nl</A>");
  time (&curr_time);
  printf ("<br>&copy; The Monolith Community -- %s",
	  ctime (&curr_time));
  printf ("</font></p>\n\n");

  printf ("</BODY></HTML>\n");
}

/* generates a clickable name */
char *
make_html_name (const char *name)
{
  static char html[200];
  char filename[L_USERNAME + 1];

  strcpy (filename, name);
  name2file (filename);

  sprintf (html, "<a onMouseover=\"window.status ='Profile user %s.'; return true\" class=\"username\" href=\"/cgi-bin/interface?action=profile&profname=%s\">%s</a> ", name, filename, name);
  return html;
}
