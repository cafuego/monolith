/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/file.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h> 
#include <dirent.h>

#include <mysql.h>

#include "monolith.h"
#include "extra.h"
#include "libmono.h"
#include "cgi.h"
#include "registration.h"
#include "routines2.h"

#define GIFDIR "/gifs/"

#define HIGHEST -1
#define LOWEST -2

#define DEBUG

/* ------------------- GLOBAL VARIABLES -------------------- */
/* userstructure guest, for read-access check */

/* a quickroom for roomlist action */
room_t quickroom_s;

/* what the script is going to do: */
int action;

#define ACT_POST		1	/* post a message */
#define ACT_PROFILE		2	/* profile someone */
#define ACT_WHOLIST		3	/* view a wholist */
#define ACT_READ		4	/* read a mesage */
#define ACT_INFO		5	/* read roominfo */
#define ACT_ROOMLIST		6	/* get the list of known quads */
#define ACT_USERLIST		7	/* get the userlist */
#define ACT_UNUSED		8	/* show available categories */
#define ACT_STATUS		9	/* prints the top navigation window */

/* name and structure of user we are */
char username[L_USERNAME + 1];
user_t *usersupp;

/* profname, name of user to be profiled */
char profname[L_USERNAME + 1];

/* type of message a user is posting */
int posttype;
char *post;

/* number and structure of room we're operating on */
int room;
room_t scratch;

/* message in room we're operating on */
long number;

typedef struct
  {
    char *name;
    char *val;
  }
entry;

/* this defines the entries as they're decoded from the input */
#define MAX_ENTRIES 10
entry entries[MAX_ENTRIES];
int number_of_entries = 0;

/* current topic the user is doing or wants to jump to */
int category;

/* General room topics, used for sorting rooms */
char *categories[] =
{
  "Admin", "BBS Issues", "Art and Culture", "Discussion", "Computers and Internet",
  "Languages", "Recreation", "Recreation - Chatting", "Recreation - Silly",
  "Support"
};

/* ------------------------- PROTOTYPES ------------------------ */
void which_room (const char *plus);
void parse_cgi_input (void);
void parse_input (void);

void show_posted (void);
void show_message (void);
void show_wholist (void);
void show_info (void);
void show_profile (void);
void make_post (void);
void show_userlist (void);
void show_room_list (void);

void make_html_head (const char *title);
void make_html_foot (void);
void script( void );
void make_html_statusbar (void);
void make_html_controls (void);
char *make_html_name (const char *name);

void print_user_stats (user_t * structure);
char * format_roomname( char * name, char *output );
int print_birthday( date_t bday );

/* ------------------------- FUNCTIONS ------------------------ */

/* this functions parses both GET and POST information, and stores
 * it in the global variables entries, and number_of_entries. 
 * It exit(1)'s when there is no information to decode */

void
parse_cgi_input ()
{
  int m, a, x;
  int cl;
  char *co;

  strcpy( username, getenv( "REMOTE_USER") );

  if (strcmp (getenv ("REQUEST_METHOD"), "POST") == 0)
    {
      cl = atoi (getenv ("CONTENT_LENGTH"));

      if (cl == 0)
	{
	  number_of_entries = 0;
	  return;
	}
      for (x = 0; cl && (!feof (stdin)); x++)
	{
	  m = x;
	  entries[x].val = fmakeword (stdin, '&', &cl);
	  plustospace (entries[x].val);
	  unescape_url (entries[x].val);
	  entries[x].name = makeword (entries[x].val, '=');
	  number_of_entries = x + 1;
	}
    }
  else if (strcmp (getenv ("REQUEST_METHOD"), "GET") == 0)
    {

      co = getenv ("QUERY_STRING");
      if (co == NULL)
	{
	  number_of_entries = 0;
	  return;
	}
      for (a = 0; co[0] != '\0'; a++)
	{
	  m = a;
	  entries[a].val = (char *)xmalloc (128 * sizeof (char));
	  entries[a].name = (char *)xmalloc (128 * sizeof (char));
	  getword (entries[a].val, co, '&');
	  plustospace (entries[a].val);
	  unescape_url (entries[a].val);
	  getword (entries[a].name, entries[a].val, '=');
	  number_of_entries = a + 1;
	}
    }
  else
    {
      number_of_entries = 0;
      return;
    }
  return;
}

/* this function, parses the entries[] array, that is now filled
 * with information to some useful data */
void
parse_input ()
{
  unsigned int i;

  /* default values, for security */
  strcpy (profname, "");
  usersupp = readuser ( username );
  room = 0;
  post = NULL;
  number = HIGHEST;
  posttype = MES_NORMAL;
  action = ACT_READ;
  read_forum( room, &scratch );

#ifdef DEBUG
  printf ("<!-- There are %d entries -->\n", number_of_entries);
  for (i = 0; i < number_of_entries; i++)
    printf ("<!-- entry[%d].name = %s -->\n", i, entries[i].name);
#endif /* DEBUG */

  /* we have put everything in the entries now, now start parsing 
   * those */
  for (i = 0; i < number_of_entries; i++)
    {
      printf ("<!-- parsing entry[%d] -->\n", i);
      if ( EQ(entries[i].name, "quad") || EQ(entries[i].name, "q"))
	{
	  room = atoi (entries[i].val);
	}
      else if (EQ (entries[i].name, "post"))
	{
	  post = entries[i].val;
	}
      else if (EQ (entries[i].name, "number"))
	{
	  if (EQ (entries[i].val, "highest"))
	    {
	      number = HIGHEST;
	    }
	  else if (EQ (entries[i].val, "lowest"))
	    {
	      number = LOWEST;
	    }
	  else
	    {
	      number = atol (entries[i].val);
	    }
	  if (number == 0)
	    {
	      number = LOWEST;
	    }
	}
      else if (EQ (entries[i].name, "profname"))
	{
	  strncpy (profname, entries[i].val, L_USERNAME);
	  profname[L_USERNAME] = '\0';
	}
      else if (EQ (entries[i].name, "category"))
	{
	  category = atoi (entries[i].val);
	}
      else if (EQ (entries[i].name, "posttype"))
	{
	  /* check later if we're ALLOWED to post like this */
	  if (EQ (entries[i].val, "emp"))
	    posttype = MES_WIZARD;
	  else if (EQ (entries[i].val, "fc"))
	    posttype = MES_SYSOP;
	  else if (EQ (entries[i].val, "sa"))
	    posttype = MES_TECHNICIAN;
	  else if (EQ (entries[i].val, "ql"))
	    posttype = MES_ROOMAIDE;
	  else if (EQ (entries[i].val, "normal"))
	    posttype = MES_NORMAL;
	}
      else if ( EQ(entries[i].name, "action") || EQ(entries[i].name, "a") )
	{
	  if (EQ (entries[i].val, "i"))
	    action = ACT_INFO;
	  else if (EQ (entries[i].val, "profile"))
	    action = ACT_PROFILE;
	  else if (EQ (entries[i].val, "post"))
	    action = ACT_POST;
	  else if ( EQ(entries[i].val, "read") || EQ(entries[i].val, "r"))
	    action = ACT_READ;
	  else if (EQ (entries[i].val, "w"))
	    action = ACT_WHOLIST;
	  else if (EQ (entries[i].val, "roomlist"))
	    action = ACT_ROOMLIST;
	  else if (EQ (entries[i].val, "userlist"))
	    action = ACT_USERLIST;
	  else if (EQ (entries[i].val, "s"))
	    action = ACT_STATUS;
	}
    }
  /* do additional checks  */

  if (room <= 13 || room > MAXQUADS || room < 0)
    room = 0;
  read_forum( room, &scratch );
  if (number == HIGHEST)
    {
      number = scratch.highest;
    }
  else if (number == LOWEST)
    number = scratch.lowest;
  return;
}

/* this function reads the message from a file, and prints it */
void
fmout2 (FILE * fp, char flag)
{
  int a;
  char aaa[512], *p;

  strcpy (aaa, "");
  p = aaa;

  for (;;)
    {
      a = getc (fp);

      *p++ = a;
      *p++ = 0;
      p--;

      if (a == 4)
	{
	  break;
	}
      if (strlen (aaa) > 511)
	p = aaa;

      if (a == 13 || a == 10 || a == 0)
	{
	  hprintf (aaa);
	  strcpy (aaa, "");
	  p = aaa;
	}
      if (a == 0)
	break;
    }
  return;
}

int
main (int argc, char **argv)
{
  set_invocation_name (argv[0]);
  mono_setuid ("guest");

  /* MIME header for HTML documents */
  printf ("Content-type: text/html%c%c", 10, 10);

  parse_cgi_input ();

  chdir (BBSDIR);
  mono_sql_connect();
  mono_connect_shm ();

  /* this translates the arguments to the room and message number */
  parse_input ();

  if (post != NULL)
    show_posted ();
  else
    switch (action)
      {

      case ACT_INFO:
	show_info ();
	break;

      case ACT_READ:
	show_message ();
	break;

      case ACT_PROFILE:
	show_profile ();
	break;

      case ACT_POST:
	make_post ();
	break;

      case ACT_WHOLIST:
	show_wholist ();
	break;
      
      case ACT_STATUS:
        make_html_statusbar();
        break;
    
      case ACT_ROOMLIST:
	show_room_list ();
	break;

      case ACT_USERLIST:
	show_userlist ();
	break;
  }

  mono_detach_shm ();
  mono_sql_detach();
  return 0;
}

void
script() {
  printf("<script language=\"JavaScript\">\n");
  printf("<!--\n");
  printf("function lowest(num, name) {\n");
  printf("    alert('Sorry, message '+num+' is the lowest message\\nin '+name+'.');\n");
  printf("}\n");
  printf("function highest(num, name) {\n");
  printf("    alert('Sorry, message '+num+' is the highest message\\nin '+name+'.');\n");
  printf("}\n");
  printf("// -->\n");
  printf("</script>\n");
  return;
}

void
make_html_statusbar ()
{
  time_t curr_time;
  struct tm *tp;

  time (&curr_time);
  tp = localtime (&curr_time);

  printf ("<html><head>\n");
  printf ("<title>BBS Navigation Window</title>\n");
  printf ("<link rev=\"made\" href=\"mailto:webmaster@monolith.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("<meta http-equiv=\"Refresh\" CONTENT=\"60; ");
  printf ("URL=http://monolith.student.utwente.nl/cgi-bin/interface?a=s\">\n");
  printf ("<link rel=\"StyleSheet\" href=\"/include/interface.css\" type=\"text/css\">\n");
  script();
  printf ("</head>\n");
  printf ("<body bgcolor=\"black\" text=\"white\" link=\"#FF0000\" alink=\"#00FF00\" vlink=\"#AAAAAA\">\n");

  printf ("<!-- statusbar -->\n");
  printf ("<nobr>\n");
  printf ("<table border=\"0\" bordercolor=\"white\" width=\"100%%\" cellpadding=\"2\" bgcolor=\"white\" cellspacing=\"0\" align=\"center\">\n");
  printf ("<tr>\n");
  printf ("<td align=\"left\" valign=\"middle\" bgcolor=\"white\"><p class=\"bbsname\"><font color=\"blue\" face=\"Helvetica\"><b>Monolith BBS</b></font></p></td>\n");
  printf ("<td align=\"left\" valign=\"middle\" bgcolor=\"white\"><p class=\"bbsname\"><font color=\"blue\" face=\"Helvetica\">Logged in as <b>%s</b></font></p></td>\n", username );
  printf ("<td align=\"right\" valign=\"middle\" bgcolor=\"white\"><p class=\"online\"><font color=\"black\" face=\"Helvetica\"><b>Currently online: %d Users.</b></font></p></td>\n",
	  shm->user_count);
  printf ("<td align=\"right\" valign=\"middle\" bgcolor=\"white\"><p class=\"time\"><font color=\"black\" face=\"Helvetica\"><b>%02d<blink>:</blink>%02d CET</b></font></p></td>\n",
	  tp->tm_hour,
	  tp->tm_min);
  printf ("</tr>\n");
  printf ("</table>\n");
  printf ("</nobr>\n");
  
  printf ("<center><p class=\"control\">\n");
  printf ("<b><a onMouseover=\"window.status ='Go back to the Monolith Community homepage.'; return true\" href=\"/\" target=\"_top\">home</a></b> | \n");
  printf ("<b><a onMouseover=\"window.status ='View the current wholist.'; return true\" href=\"/cgi-bin/interface?a=w\" target=\"main_window\">wholist</a></b> | \n");
  printf ("<b><a onMouseover=\"window.status ='View the full list of available rooms.'; return true\" href=\"/cgi-bin/interface?a=roomlist\" target=\"main_window\">quadrants</a></b> | \n");
  printf ("<b><a onMouseover=\"window.status ='Profile a user.'; return true\" href=\"/bbs/users/profile.html\" target=\"main_window\">profile</a></b> | \n");
  printf ("<b><a onMouseover=\"window.status ='View the online helpfiles.'; return true\" href=\"/bbs/info/help/\" target=\"main_window\">help</a></b>\n");
  printf ("</p></center>\n");

  printf("</body>\n</html>\n");
  
  return;
}

void
make_html_controls ()
{
  char *format = NULL;
  printf ("<p align=\"center\" class=\"control\">\n");
  if( number == scratch.lowest ) {
      printf ("<a onMouseover=\"window.status ='This is the lowest message!'; return true\" href=\"javascript:lowest(%ld,'%s')\">previous</a> |",number, format_roomname(scratch.name,format));
      xfree(format);
  } else {
      printf ("<a onMouseover=\"window.status ='Read the previous message..'; return true\" href=\"/cgi-bin/interface?a=r&q=%d&number=%ld\">previous</a> |", room, number - 1);
  }
  printf (" reply |");
  printf (" <a onMouseover=\"window.status ='Enter a new message in %s.'; return true\" href=\"/cgi-bin/interface?a=post&q=%d\">enter</a> |", format_roomname(scratch.name, format), room);
  xfree(format);
  if( number == scratch.highest ) {
      printf ("<a onMouseover=\"window.status ='This is the highest message!'; return true\" href=\"javascript:highest(%ld,'%s')\">next</a> |",number, format_roomname(scratch.name, format));
      xfree(format);
  } else {
      printf (" <a onMouseover=\"window.status ='Read the next message.'; return true\" href=\"/cgi-bin/interface?a=r&q=%d&number=%ld\">next</a> |", room, number + 1);
  }
  printf (" <a onMouseover=\"window.status ='View the quadrant info for %s.'; return true\" href=\"/cgi-bin/interface?a=i&q=%d\">info</a> |", format_roomname(scratch.name, format), room);
  xfree(format);
  printf (" skip |");
  printf (" goto");
  printf ("</p>\n");
  return;
}

void
which_room (const char *plus)
{
  char *format = NULL;

  printf ("<p class=\"header\"><font color=\"white\">[</font> <a class=\"%s\" onMouseover=\"window.status ='View the quadrant info for %s.'; return true\" href=\"/cgi-bin/interface?a=i&q=%d\">%s</a><font color=\"white\">&gt;</font> <font color=\"green\">message %ld </font><font color=\"white\">(</font><font color=\"green\">%ld remaining</font><font color=\"white\">) ]</font>\n",
	  (scratch.flags & QR_PRIVATE) ? "private" : (scratch.flags & (QR_ANONONLY | QR_ANON2)) ? "anon" : "normal",
	  format_roomname(scratch.name,format), room,
	  scratch.name,
	  number,
	  scratch.highest - number);
  xfree(format);
  return;
}

/* creates the head for the html page */
void
make_html_head (const char *title)
{
  printf ("<html><head>\n");
  printf ("<title>%s</title>\n", title);
  printf ("<link rev=\"made\" href=\"mailto:webmaster@monolith.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("<link rel=\"StyleSheet\" href=\"/include/interface.css\" type=\"text/css\">\n");
  script();
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

void
show_posted ()
{
  char failmsg[100];
  post_t mess;
  FILE *fp;
  char tempfile[100], *p;
  int lines;

  strcpy (failmsg, "");

  memset( &mess, 0, sizeof( post_t ) );

  if (!check_user (username))
    sprintf (failmsg, "Unknown user: %s.", username);

  if (strlen (failmsg))
    {
      make_html_head ("Sorry, posting failed");
      printf ("<center><h3>%s</h3></center>", failmsg);
      make_html_controls();
      make_html_foot ();
      return;
    }
  lines = 2;
  p = post;
  while (*p)
    {
      if (*p == '\n')
	lines++;
      p++;
    }

  if (posttype == MES_WIZARD && !(usersupp->priv & PRIV_WIZARD))
    posttype = MES_NORMAL;
  if (posttype == MES_SYSOP && !(usersupp->priv & PRIV_SYSOP))
    posttype = MES_NORMAL;
  if (posttype == MES_ROOMAIDE && !(usersupp->flags & US_ROOMAIDE))
    posttype = MES_NORMAL;
  if (posttype == MES_TECHNICIAN && !(usersupp->priv & PRIV_TECHNICIAN))
    posttype = MES_NORMAL;

  strcpy (mess.author, usersupp->username);
  mess.type = posttype;
  time (&mess.date);
  strcpy (mess.subject, "");

  sprintf (tempfile, BBSDIR "/tmp/wwwpost.%d", getpid ());
  fp = xfopen (tempfile, "w", FALSE);

  write_post_header (fp, mess);
  fputs (post, fp);
  if (post[strlen (post) - 1] != '\n')
    fputs ("\n", fp);
  write_post_footer (fp, lines);
  fclose (fp);

  /* post is now ready and put in tempfile */
  save_message (tempfile, room, NULL);
  unlink (tempfile);

  make_html_head ("Post succeeded");
  log_it ("webpost", "%s posted a message in room #%d", usersupp->username, room);
  printf ("<center><h3 align=\"center\">Your message has been posted.</h3></center>");
  printf ("<p align=\"center\"><a onmouseover=\"window.status= 'Read the message you just posted.'; return true\" href=\"/cgi-bin/interface?a=r&q=%d&number=highest\">Read it!</a>\n", room);
  make_html_controls();
  make_html_foot ();
  return;
}


void
show_message ()
{
  FILE *fp;
  int is_deleted;
  char mfile[200];
  char title[100];
  post_t message;

  if (may_read_room ( *usersupp, scratch, room) == 0)
    {
      sprintf (title, "Access Denied\n");
      make_html_head (title);
      printf ("<center><h3>Sorry, this room is invite-only.</h3></center>\n");
      make_html_controls ();
      make_html_foot ();
      return;
    }
  strcpy (mfile, post_to_file (room, number, NULL));
  fp = xfopen (mfile, "r", FALSE);

  if (fp == NULL)
    {
      is_deleted = TRUE;
      sprintf (title, "Deleted message in %s&gt;\n", scratch.name);
    }
  else
    {
      read_post_header (fp, &message);
      is_deleted = FALSE;
      if( (message.type ==  MES_ANON ) || (message.type == MES_AN2 ) )
          sprintf (title, "Message by *Anonymous* in %s\n", scratch.name );
      else
          sprintf (title, "Message by %s in %s\n", message.author, scratch.name);
    }

  make_html_head (title);
  make_html_controls ();

  if (is_deleted) {
      printf ("<center><h3>Message deleted.</h3></center>\n");
  } else {
  printf ("<!-- message window -->\n");
  printf ("<table border=\"0\" width=\"100%%\" border=\"0\" cellpadding=\"2\">\n");
  printf ("<tr>\n");
  printf ("<td align=\"left\" valign=\"middle\">\n");

  if (message.type != MES_ANON && message.type != MES_AN2)
    {
      printf ("<p class=\"header\"><font color=\"green\">%s from</font> ", printdate (message.date, -1));

      switch ((int)message.type)
	{
	case MES_WIZARD:
	  printf ("<font color=\"white\">Emperor</font> %s", make_html_name (message.author));
	  break;

	case MES_SYSOP:
	  printf ("<font color=\"white\">Fleet Commander</font> %s", make_html_name (message.author));
	  break;

	case MES_TECHNICIAN:
	  printf ("<font color=\"blue\">Systems Analyst</font> %s", make_html_name (message.author));
	  break;

	case MES_ROOMAIDE:
	  printf ("<font color=\"red\">Quadrant Leader</font> %s", make_html_name (message.author));
	  break;

	case MES_FORCED:
	  printf ("%s (FORCED MESSAGE ) ", make_html_name (message.author));
	  break;

	default:
	  printf ("<font color=\"yellow\">%s</font>", make_html_name (message.author));
	  break;
	}
    }
  else
    {
      printf ("<p class=\"header\"><font color=\"green\">From</font> <font color=\"blue\">*Anonymous*</font>\n");
    }
  printf ("</p>\n");

  if (strlen (message.subject))
    {
      printf ("<p class=\"subject\"><font color=\"yellow\">Subject: ");
      hprintf (message.subject);
      printf ("</font></p>\n");
    }

  printf ("</td>\n");
  printf ("</tr>\n");
  printf ("<tr>\n");
  printf ("<td>\n");
  printf ("<!- message body -->\n");
  printf ("<pre><span class=\"body\"><font color=\"#00DDDD\">\n");

  fmout2 (fp, 0);

  printf ("</font></span></pre>\n");
  printf ("</td>\n");
  printf ("</tr>\n");
  printf ("<tr>\n");
  printf ("<td align=\"left\" valign=\"middle\">\n");
  printf ("<!-- message footer -->\n");

  which_room ("");

  printf ("</td>\n");
  printf ("</tr>\n");
  printf ("</table>\n");

  }

  fclose (fp);

  make_html_controls ();

  make_html_foot ();
}


void
show_info ()
{
  char mfile[200];
  FILE *fp;
  char title[100];
  int is_deleted;
  post_t message;

  if (may_read_room (*usersupp, scratch, room) == 0)
    {
      sprintf (title, "Permission Denied\n");
      make_html_head (title);
      printf ("<center><h3>This room is invite-only.</h3></center>\n");
      make_html_foot ();
      return;
    }

  sprintf (mfile, "%s/%d/description", QUADDIR, room);
  fp = xfopen (mfile, "r", FALSE);
  if (fp == NULL)
    {
      is_deleted = TRUE;
    }
  else
    {
      is_deleted = FALSE;
      read_post_header (fp, &message);
    }
  sprintf (title, "Information about %s&gt;", scratch.name);

  make_html_head (title);
  printf ("<b>%s</b> from %s", printdate (message.date, -1), make_html_name(message.author));
  printf ("<hr size=1><br>\n");
  printf ("<center><table border=0 width=\"100%%\"><tr><td bgcolor=\"black\"><pre>\n");
  hprintf ("w");
  fmout2 (fp, 0);
  which_room ("");

  printf ("</pre></td></tr></table></center>\n");
  fclose (fp);
  make_html_foot ();
}


void
show_profile ()
{
  user_t *user = NULL;

  make_html_head ("showing profile");

  if (strlen (profname))
    user = readuser (profname);

  if (user == NULL)
    {
      printf ("<H1>No such user.</H1>\n");
    }
  else
    {
      print_user_stats (user);
    }
  make_html_foot ();
}

void
show_wholist ()
{

  int tdif, idletime, min, hour, i;
  int fr;
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
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("<meta http-equiv=\"Refresh\" CONTENT=\"60; ");
  printf ("URL=http://monolith.student.utwente.nl/cgi-bin/interface?a=w\">\n");
  printf ("<link rel=\"StyleSheet\" href=\"/include/interface.css\" type=\"text/css\">\n");
  script();
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

      if ( ! EQ( username, "Guest" ) ) {
         fr = is_friend( username, bp->username );
      } else
         fr = 0;

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
      printf("  <td align=\"left\"><p class=\"username\"><a onMouseover=\"window.status ='Profile user %s.'; return true\" href=\"/cgi-bin/interface?action=profile&profname=%s\"><font color=\"%s\"><b>%s</b></font></a></td>\n", bp->username, newname, fr?"cyan":"#00FF00", bp->username);
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
make_post ()
{
  char title[80];

  sprintf (title, "New message in %s&gt;\n", scratch.name);
  make_html_head (title);

  printf ("<center><form action=\"/cgi-bin/interface\" method=\"post\">\n");
  if (strlen (username))
    printf ("<b>Name: </b> <input type=\"text\" name=\"username\" value=\"%s\" size=\"%d\" maxsize=\"%d\">\n", username, L_USERNAME, L_USERNAME);
  else
  printf ("<input type=\"hidden\" name=\"quad\" value=%d>\n", room);

  printf ("<input type=radio name=\"posttype\" value=\"normal\">Normal Post ");
  printf ("<input type=radio name=\"posttype\" value=\"ql\">As Quadrant Leader ");
  printf ("<input type=radio name=\"posttype\" value=\"sa\">As Systems Analyst ");
  printf ("<input type=radio name=\"posttype\" value=\"fc\">As Fleet Commander ");
  printf ("<input type=radio name=\"posttype\" value=\"emp\">As Emperor<br>");

#ifdef ANONPOST
  printf ("<input type=checkbox name=\"anon\" value=\"true\">Anonymous  ");
  printf ("<b>Alias (optional): </b> <input type=\"text\" name=\"alias\" size=\"%d\">\n", L_USERNAME);
#endif

  printf ("<p><textarea name=\"post\" cols=60 rows=10></textarea>\n");
  printf ("<hr size=1>\n");
  printf ("<p><center><input type=submit value=\"Post it\">  <input type=reset value=\"Erase all\">\n");
  printf ("</form></center>\n");

  make_html_foot ();
}


void
print_user_stats (user_t * structure)
{

  int a;
  char *p, has_picture = 0, work[80], work2[100];
  long timecall, curtime;
  room_t bing;

  sprintf (work, "%s%s.gif", GIFDIR, structure->username);
  while ((p = (char *) index (work, ' ')) != 0)
    *p = '_';
  sprintf (work2, "/usr/bbs/www%s", work);
  a = open (work2, O_RDONLY, 0);
  if (a >= 0)
    has_picture = 1;
  close (a);
  if (!has_picture)
    {
      sprintf (work, "%s%s.jpg", GIFDIR, structure->username);
      while ((p = (char *) index (work, ' ')) != 0)
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
      printf ("<td align=left><b>%s, %s</b></td>", structure->RGstate, structure->RGcountry);
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
	  printf ("<td align=left><a target=\"new\" href=\"%s\"><b>%s</b></a></td>\n", structure->RGurl, structure->RGurl);
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
              read_forum( structure->RA_rooms[a], &bing );
              if (may_read_room (*usersupp, bing, structure->RA_rooms[a])) 
                {
	          printf ("<font color=\"#ffffff\">%d.</font> <a href=\"/cgi-bin/interface?action=info&quad=%d\">%s</a><br>", structure->RA_rooms[a], structure->RA_rooms[a], bing.name );
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

void
show_userlist ()
{

  DIR *userdir;
  struct dirent *tmpdirent;
  user_t *tmpuser;

  char name[L_USERNAME + 1];
  int i = 0;

  userdir = opendir (USERDIR);
  if (userdir == NULL)
    {
      printf ("opendir() problems!\n");
      return;
    }
  printf ("<html><head>\n");
  printf ("<title>List of all BBS Users</title>\n");
  printf ("<link rev=\"made\" href=\"mailto:webmaster@cal044202.student.utwente.nl\">\n");
  printf ("<meta name=\"description\" content=\"Monolith BBS!\">\n");
  printf ("<meta name=\"keywords\" content=\"Monolith monolith BBS bbs\">\n");
  printf ("</head>\n");
  printf ("<body bgcolor=\"white\" text=\"black\" link=\"#FFFFFF\" alink=\"#00FFFF\" vlink=\"#008F93\">\n");

  printf ("<center>\n");
  printf ("<img src=\"/pix/banners/bbs_users_userlist.jpg\" height=\"32\" width=\"468\">\n");
  printf ("<p align=\"center\"><font size=\"-1\">( This page contains a very large table and may take a while to load )</font>\n");
  printf ("</center><p><p>\n");
  printf ("<center>\n");


  printf ("<table border=0 cellspacing=3 cellpadding=1>\n");
  while ((tmpdirent = readdir (userdir)) != NULL)
    {

      if (tmpdirent->d_name[0] == '.')	/* ignore . files */
	continue;

      strcpy (name, tmpdirent->d_name);
      name2file (name);

      tmpuser = readuser (name);

      if (tmpuser == NULL || strcasecmp (name, name2file (tmpuser->username)) != 0)
	{
	  if (tmpuser != NULL)
	    free (tmpuser);
	  continue;
	}
      if (tmpuser->priv & PRIV_DELETED)
	{
	  free (tmpuser);
	  continue;
	}
      if ((i % 5) == 0)
	printf ("<tr>\n");

      printf ("<td ");

      if (tmpuser->priv & PRIV_WIZARD)
	{
	  printf ("bgcolor=\"#BBBBBB\">\n");
	}
      else if (tmpuser->priv & PRIV_SYSOP)
	{
	  printf ("bgcolor=\"#BBBBBB\">\n");
	}
      else if (tmpuser->priv & PRIV_TECHNICIAN)
	{
	  printf ("bgcolor=\"#0000A0\">\n");
	}
      else if (tmpuser->flags & US_ROOMAIDE)
	{
	  printf ("bgcolor=\"#A00000\">\n");
	}
      else if (tmpuser->flags & US_GUIDE)
	{
	  printf ("bgcolor=\"#00A0A0\">\n");
	}
      else
	{
	  printf ("bgcolor=\"#00A000\">\n");
	}

      printf ("%s", make_html_name (tmpuser->username));
      printf ("<br></td>\n");
      free (tmpuser);
      if ((i % 5) == 4)
	printf ("</tr>\n");
      i++;
    }
  printf ("</table>\n</center>\n");
  closedir (userdir);
  make_html_foot ();
  return;
}


void
show_room_list ()
{
  int i = 0, j = 0, k = 0;
  char *format = NULL;

  chdir (BBSDIR);

  make_html_head ("Available chat areas");

  printf ("<h3 align=\"center\">Available discussion and chat areas</h3>\n");

  printf("<p><a name=\"top\"></p><p align=\"center\">\n");


  for(k = 0; k < 10; k++ ) {
      printf("<a onMouseover=\"window.status ='View rooms in the %s category.'; return true\" href=\"#%s\">%s</a>", categories[k], categories[k], categories[k]);
  	  if(k % 5 == 4)
  	    printf("<br>\n");
  	  else
  	    printf(" | \n");
  }
  
  printf("<hr width=\"300\" align=\"center\">\n");
   
  printf("<table align=\"center\" border=\"0\" cellpadding=\"2\" cellspacing=\"0\" width=\"94%%\">\n");

  /* loop for each category and print a table heading */
  
  
  for(k = 0; k < 10; k++ ) {
  
    printf("<tr><th colspan=\"2\" align=\"left\"><a name=\"%s\"></a><p class=\"subject\"><font color=\"yellow\" face=\"Helvetica\"><b>%s</b></font></p></th></tr>\n", categories[k], categories[k] );
  
    for (i = 0; i < MAXQUADS; i++) {

      if ( (i > 0 && i < 10) || i == 13 ) 
          continue;

      read_forum( i, &quickroom_s );

      if ( ! EQ( quickroom_s.category, categories[k] ) )
          continue;

      if ( quickroom_s.name[0] == '\0' )
          continue;

      if ( ! ( quickroom_s.flags & QR_INUSE ) )
          continue;

   /*   if ( quickroom_s.flags & QR_PRIVATE ) 
           continue; */

      if (may_read_room ( *usersupp, quickroom_s, i) == 0)
           continue;

           {
	    if (j++ % 2 == 0) {
	      if (j == 1) {
		    printf ("<tr>\n");
	      } else {
		    printf ("\n</tr><tr>\n");
	      }
	    } printf ("<td align=\"left\" valign=\"top\" width=\"50%%\">");

	    printf ("<font color=\"white\">%d - </font><a class=\"%s\" onMouseover=\"window.status ='Read messages in %s.'; return true\" href=\"/cgi-bin/interface?action=read&quad=%d&number=highest\"><font color=\"#%s\">%s</font></a>"
		  ,i ,(quickroom_s.flags & QR_PRIVATE) ? "q_private" : (quickroom_s.flags & (QR_ANONONLY | QR_ANON2)) ? "q_anon" : "q_normal"
                  ,format_roomname( quickroom_s.name, format ), i
		  ,(quickroom_s.flags & QR_PRIVATE) ? "FF0000" : (quickroom_s.flags & (QR_ANONONLY | QR_ANON2)) ? "FF00FF" : "00FF00"
          ,quickroom_s.name);
            xfree(format);

	    printf ("\n</td>\n");
	  }
    } /* end for maxrooms */
    printf("</tr>\n");
    printf("<tr><td colspan=\"2\" align=\"center\" valign=\"middle\"><hr width=\"300\"><p align=\"center\">Back to <a href=\"#top\">top</a>.</td></tr>\n");
  } /* end for categories */
  printf ("</table>\n");
  make_html_foot ();
  return;
}

char *
format_roomname( char * name, char *format )
{

    int i = 0;

    format = (char *) xmalloc( (char) 2 * L_QUADNAME);
    memset(format, 0, sizeof(format));
    
    for( i = 0; i < strlen(name); i++) {
        if(name[i] == '\'')
            strcat(format, "\\");
        format[strlen(format)] = name[i];
    }

    return format;

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
