/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"
#include "registration.h"
#include "cgi.h"

#define GIFDIR "/gifs/"

void getword(char *word, char *line, char stop);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);
void no_color_print(char *input);
void print_user_stats(user_t * structure);
int print_birthday(date_t bday);

typedef struct {
    char name[128];
    char val[128];
} entry;

user_t *usersupp;

void
no_color_print(char *input)
{
    char *p;
    p = input;
    while (*p) {
	if (*p == '') {
	    p++;
	    if (*p == 0)
		p--;
	} else
	    putchar(*p);
	p++;
    }
}

int
main(int argc, char *argv[])
{

    char *cl;
    user_t *user;
    entry entries[10000];
    register int x, m = 0;

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    printf("Content-type: text/html\n\n");

    if (strcmp(getenv("REQUEST_METHOD"), "GET")) {
	printf("This script should be referenced with a METHOD of GET.\n");
	printf("If you don't understand this, see this ");
	printf("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\">forms overview</A>.%c", 10);
	exit(1);
    }
    cl = getenv("QUERY_STRING");
    if (cl == NULL) {
	printf("No query information to decode.\n");
	fflush(stdout);
	exit(1);
    }
    for (x = 0; cl[0] != '\0'; x++) {
	m = x;
	getword(entries[x].val, cl, '&');
	plustospace(entries[x].val);
	unescape_url(entries[x].val);
	getword(entries[x].name, entries[x].val, '=');
    }
    user = readuser(entries[0].val);
    usersupp = readuser("Guest");

    if (user == NULL) {
	printf("<H1>No such user.</H1>\n");
	return 1;
    }
    mono_connect_shm();
    print_user_stats(user);
    mono_detach_shm();

    return 0;
}

void
print_user_stats (user_t * structure)
{

  int a;
  char *p, has_picture = 0, work[80], work2[100];
  long timecall, curtime;
  room_t bing;

  sprintf (work, "%s%s.gif", GIFDIR, structure->username);
  while ((p = (char *) strchr (work, ' ')) != 0)
    *p = '_';
  sprintf (work2, "/usr/bbs/www%s", work);
  a = open (work2, O_RDONLY, 0);
  if (a >= 0)
    has_picture = 1;
  close (a);
  if (!has_picture)
    {
      sprintf (work, "%s%s.jpg", GIFDIR, structure->username);
      while ((p = (char *) strchr (work, ' ')) != 0)
	*p = '_';
      sprintf (work2, "/usr/bbs/www%s", work);
      a = open (work2, O_RDONLY, 0);
      if (a >= 0)
	has_picture = 1;
      close (a);
    }
  printf ("<HTML><HEAD><TITLE>\n");
  printf ("%s\'s Profile</title>\n", structure->username);
  printf ("<link rev=\"made\" href=\"mailto:webmaster@cal044202.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("</head>\n");
  printf ("<body bgcolor=\"black\" text=\"#FFFFFF\" link=\"#008740\" alink=\"#00FFFF\" vlink=\"#008F93\">\n");

  time (&curtime);
  timecall = (((long) curtime - (long) structure->laston_from) / 60);

  if (has_picture == 1)
    printf ("<IMG SRC=\"%s\" ALT=\"Picture\" BORDER=0 WIDTH=120 ALIGN=RIGHT>\n", work);

  printf ("<b><font size=+2><tt>%s</tt></font></b><br>", structure->username);

  if (structure->priv & PRIV_WIZARD)
    {
      printf ("<a href=\"/bbs/info/help/emperors.html\">");
      printf ("<b><font size=+1 color=white>***Emperor***</font></b></a>\n");
    }
  if (structure->priv & PRIV_SYSOP)
    {
      printf ("<a href=\"/bbs/info/help/fleet_commanders.html\">");
      printf ("<b><font color=purple size=+1>**Fleet Commander**</font></b></a>\n");
    }
  if (structure->priv & PRIV_TECHNICIAN)
    {
      printf ("<a href=\"/bbs/info/help/system_analysts.html\">");
      printf ("<b><font color=blue>*Systems Analyst*</font></b></a>\n");
    }
  if (structure->flags & US_ROOMAIDE)
    {
      printf ("<a href=\"/bbs/info/help/quadrant_leaders.html\">");
      printf ("<b><font color=red>Quadrant Leader</font></b></a>\n");
    }
  if (structure->flags & US_GUIDE)
    {
      printf ("<a href=\"/bbs/info/help/help_terminals.html\">");
      printf ("<b><font color=cyan>Help Terminal</font></b></a>\n");
    }
  if (structure->priv & PRIV_DEGRADED)
    {
      printf ("<B>Degraded</B> ");
    }
  if (structure->priv & PRIV_TWIT)
    {
      printf ("<blink>CURSED</blink> ");
    }
  if (structure->priv & PRIV_DELETED)
    {
      printf ("<B>Deleted</B> ");
    }
  if (structure->xtrapflag)
    {
      printf ("<I> ");
      hprintf (structure->xtrapflag);
      printf ("</I>");
    }
  if (structure->flags & US_COOL)
    printf ("<I>*COOL AS ICE*</I> ");

  if (structure->flags & US_DONATOR)
    printf ("<b><font color=green> $DONATOR$</font></b> ");

  printf ("<p>\n\n");

// trying to format some info here...
  printf ("<table border=\"0\">\n");
// Start with real name.
  printf ("<tr><b><td align=left><b>Name:</b></td>");
  if (structure->hidden_info & H_REALNAME)
    {
      printf ("<td align=left><font color=red>Hidden.</font></td>");
    }
  else
    {
      printf ("<td align=left><b>%s</b></td>", structure->RGname);
    }
  printf ("</tr>\n");
// Then street.
  printf ("<tr><td align=left><b>Address:</b></td>");
  if (structure->hidden_info & H_ADDRESS)
    {
      printf ("<td align=left><font color=red>Hidden.</font></td>");
    }
  else
    {
      printf ("<td align=left><b>%s</b></td>", structure->RGaddr);
    }
  printf ("</tr>\n");
// Then zipcode. and city.
  printf ("<tr>\n<td></td>");
  if (structure->hidden_info & H_CITY)
    {
      printf ("\n");
    }
  else
    {
      printf ("<td align=left><b>%s, %s.</b></td>", structure->RGzip, structure->RGcity);
    }
  printf ("</tr>\n");
// Then country.
  printf ("<tr><td align=left><b>Country:</b></td>");
  if (structure->hidden_info & H_COUNTRY)
    {
      printf ("<td align=left><font color=red>Hidden.</font></td>");
    }
  else
    {
      printf ("<td align=left><b>%s</b></td>", structure->RGstate);
    }
  printf ("</tr>\n");
  printf ("<tr><td colspan=2 height=5></td></tr>\n");
// The phone number next.
  printf ("<tr>\n<td align=left><b>Phone:</b></td>");
  if (structure->hidden_info & H_PHONE)
    {
      printf ("<td align=left><font color=red>Hidden.</font></td>\n");
    }
  else
    {
      printf ("<td align=left><b>%s</b></td>\n", structure->RGphone);
    }
  printf ("</tr>\n");
// Email address...
  printf ("<tr>\n<td align=left><b>Email:</b></td>");
  if (structure->hidden_info & H_EMAIL)
    {
      printf ("<td align=left><font color=red>Hidden.</font></td>\n");
    }
  else
    {
      printf ("<td align=left><a href=\"mailto:%s\"><b>%s</b></a></td>\n", structure->RGemail, structure->RGemail);
    }
  printf ("</tr>\n");
// And the URL.
  if (strlen (structure->RGurl) < 11)
    {
      /* do nothing */
    }
  else
    {
      printf ("<tr>\n<td align=left><b>Homepage:</b></td>");
      if (structure->hidden_info & H_URL)
	{
	  printf ("<td align=left><font color=red>Hidden.</font></td>\n");
	}
      else
	{
	  printf ("<td align=left><a href=\"%s\"><b>%s</b></a></td>\n", structure->RGurl, structure->RGurl);
	}
      printf ("</tr>\n");
    }
// Other misc info...
  printf ("<tr><td colspan=2 height=5></tr>\n");
  printf ("<tr><td align=left><b>Flying:</b></td><td align=left>");
  hprintf (structure->doing);
  printf ("</td></tr>\n");

  /* if user has a birthday set and hasn't hidden it
   * then print it here.              - Caf. 10/2/98
   */
  if ((structure->birthday.day) && !(structure->hidden_info & H_BIRTHDAY))
    {
      printf ("<tr><td align=left><b>Birthday:</b></td><td align=left><b>");
      print_birthday(structure->birthday);
      printf ("</b></td></tr>\n" );
    }

  /* Modified:
   * Now prints Quad names which link to quad info, which (I hope)
   * links to reading. Quads are no longer printed if viewing user
   * (Guest) isn't allowed to read them.            - Caf. 10/2/98
   */
  if (structure->flags & US_ROOMAIDE)
    {
      printf ("<tr><td align=\"left\" valign=\"top\"><b>Quadrant Leader in:</b></td>");
      printf ("<td align=\"left\" valign=\"top\"><b>");
      for (a = 0; a < 5; a++)
	{
	  if (structure->RA_rooms[a] >= 0)
	    {
              bing = read_quad( structure->RA_rooms[a] );
              if (may_read_room (*usersupp, bing, structure->RA_rooms[a])) 
                {
	          printf ("<font color=\"#ffffff\">%d.</font> <a href=\"/cgi-bin/interface?a=r&q=%d&message=highest\">%s</a><br>", structure->RA_rooms[a], structure->RA_rooms[a], bing.name );
                }
	    }
	}
      printf ("</b></td></tr>\n");
    }
  /* sigh, there's no way i can check this:
   * if ( structure->on_line ) 
   * printf( "ONLINE for %d:%2.2d from %s<BR>\n" , timecall / 60, 
   * timecall % 60, structure->lasthost );   
   * else {
   */
  printf ("<tr><td align=left><b>Last on:</b></td><td align=left>");
  a = timecall / 60 / 24;
  printf ("<b>%s to %s (%d day%s ago)</b>", printdate (structure->laston_from, 1)
	  ,printdate (structure->laston_to, 2), a, (a == 1) ? "" : "s");
  if (structure->flags & US_HIDDENHOST)
    {
      printf ("</td>\n");
    }
  else
    {
      printf ("<b> from %-16.16s</b></td>\n", structure->lasthost);
    }
  printf ("</tr>\n");
  /* } */
  printf ("</table></b>");
  printf ("<p><hr size=1>\n");

  printf ("<PRE>\n");

  p = read_profile (structure->username);
  if (p != NULL)
    {
      hprintf (p);
      xfree (p);
    }
  else
    {
      printf ("No profile.<br>\n");
    }

  printf ("\n</PRE>\n");
  printf ("</BODY></HTML>\n");
  return;
}

int
print_birthday(date_t bday)
/* convert to time_t and user strftime() to print it */
{
    struct tm t;
    char text[50];

    if (bday.day == 0)
	return -1;

    t.tm_sec = t.tm_min = 0;
    t.tm_hour = 12;
    t.tm_mday = bday.day;
    t.tm_mon = bday.month;
    t.tm_year = bday.year;

    mktime(&t);

    if (bday.year == 100) {
	strftime(text, 50, "%d %B", &t);
    } else
	strftime(text, 50, "%d %B %Y", &t);

    printf("%s", text);
    fflush(stdout);
    return 0;
}
