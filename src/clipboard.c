/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"

#include "ext.h"

#include "clipboard.h"
#include "read_menu.h"
#include "express.h"
#include "input.h"
#include "routines2.h"

static void wacky_email_stuff(void);

#define ACCEPTED "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@1234567890%_.- "

void
clip_board()
{

    int cmd = 0;

    while (cmd != 'Q' && cmd != ' ' && cmd != '\r') {
	display_short_prompt();

	cprintf( _("\01f\01gClipboard: \01a"));
	IFNEXPERT
	{
	    more(MENUDIR "/menu_clipboard", 1);
	    display_short_prompt();
	    cprintf("\01f\01gClipboard: \01a");
	}

	cmd = get_single_quiet("DEMRS?Q \r");

	switch (cmd) {

	    case 'D':
		cprintf(_("\01f\01gDelete ClipBoard.\n"));
		cprintf(_("\01f\01yAre you sure you want to delete your clipboard? (y/N) "));
		if (yesno_default(NO) == NO) {
		    cprintf(_("\1f\1gClipboard not deleted.\n"));
		} else {
		    close(creat(CLIPFILE, 0600));
		    cprintf(_("\1f\1gClipboard deleted.\n"));
		}
		break;

	    case 'E':
		cprintf(_("\01f\01gEdit ClipBoard.\n"));
		editor_edit(CLIPFILE);
		break;

	    case 'M':
                (void) mail_clipboard(CLIPFILE);
		break;

	    case 'R':
		cprintf("\01f\01gRemove Colorcodes.\n");
		if (de_colorize(CLIPFILE) != 0)
		    cprintf("\01f\01r\nWarning\01w:\01g Couldn't remove colours.");
		break;

	    case 'S':
		cprintf("\01f\01gShow ClipBoard contents.\01c\n\n");
		cprintf("\01w----- CLIPBOARD -----\n\01g");
		more(CLIPFILE, 1);
		cprintf("\01f\01w----- CLIPBOARD -----\n");
		break;

	    case 'Q':
	    case ' ':
	    case '\r':
		cprintf("\01f\01gQuit.\n");
		break;


	    case '?':
		cprintf("\01f\01gShow menu.\n");
		more(MENUDIR "/menu_clipboard", 1);
		break;

	    default:
		break;

	}
    }
}

/*************************************************
* wacky_email_stuff()
*
* sends a clipboard and prompts for a recipient.
* IFSYSOP-function.
*************************************************/

static void
wacky_email_stuff()
{

    size_t accepted;

    char subject[50];
    char recipient[60];
    char work[150];

    cprintf("\01g\01f\nWould you like to remove the color codes? \01w(\01gy/n\01w)\01g ");
    if (yesno() == YES) {
	if (de_colorize(CLIPFILE) != 0)
	    cprintf("\01f\01rWarning\01w:\01g Couldn't remove colours.");
    }
    cprintf("\01f\01g\nEnter a recipient:\01c ");
    getline(recipient, sizeof(recipient), 1);
    accepted = strspn(recipient, ACCEPTED);

    if (accepted < strlen(recipient)) {
	cprintf("\01r\01f\nInvalid email address, can't send message!\01a\n");
    } else {
	cprintf("\01f\01gSubject for the email: \01c");
	getline(subject, sizeof(subject), 1);
	if (strspn(subject, ACCEPTED) < strlen(subject)) {
	    cprintf("\01r\01fSubject contains invalid characters, can't send message!\01a\n");
	    return;
	}
	if (strlen(subject) == 0)
	    strcpy(subject, "Clipboard-Contents");
	log_it("email", "%s sent clipboard to %s", usersupp->username, recipient);
	(void) sprintf(work, "/bin/mail -s '%s' %s < %s", subject, recipient, CLIPFILE);
	(void) system(work);
	cprintf("\01g\01fMail was sent to \01c%s\01a\n", recipient);
    }
}


/*************************************************
*
* clip_log()
*
* just call this function with the line you want
* to have added to the your clipfile.
*
*************************************************/

int
clip_log(const char *line)
{
    FILE *fp;

    fp = xfopen(CLIPFILE, "a", FALSE);
    if (fp == NULL) {
	cprintf("\1fCould not open your clipboard file.\n");
	return -1;
    }
    fprintf(fp, "%s", line);
    fclose(fp);
    return 0;
}

/*************************************************
* notebook()
*
* made from the suggestion of Grey Wolf at
* Helluva BBS - I think it's a neat idea. :-)
*
* for_who:	1 -> RoomAide, meaning for curr_rm
*		2 -> Sysop, meaning personal file
*************************************************/

void
notebook(int for_who)
{

    int cmd, a;
    char nbname[70];
    char tempstr[180];
    FILE *fp;

    /* if roomnotebook */
    if (for_who == 1)
	sprintf(nbname, "%sroomnb%d", NOTEBOOKDIR, curr_rm);
    else {
	sprintf(nbname, "%sOwn_%s", NOTEBOOKDIR, usersupp->username);
	name2file(nbname);
    }
    cprintf("\n");
    for (;;) {

	if (!usersupp->flags & US_EXPERT)
	    more(MENUDIR "/menu_note", 1);

	cprintf("\1a\1f\1pNoteBook-> ");

	cmd = get_single_quiet("?CDELSQ \r\b");

	cprintf("\1f\1y");

	switch (cmd) {
	    case 'C':
		cprintf("Copy'ing the NoteBook to the end of the ClipBoard.\n");
		sprintf(tempstr, "cat %s >> %s", nbname, CLIPFILE);
		system(tempstr);
		break;

	    case 'D':
		cprintf("Delete NoteBook!\n");
		cprintf("Are you sure? ");
		if (yesno_default(NO) == YES)
		    fclose(fopen(nbname, "w"));
		break;

	    case 'E':
		cprintf("Enter a new NoteBook.\n");
		fp = xfopen(nbname, "w", FALSE);
		if (!fp) {
		    cprintf("\1f\1rNotebook could not be opened.\n");
		    break;
		}
		get_buffer(fp, EDIT_NORMAL, &a);
		fclose(fp);
		break;

	    case 'L':
		cprintf("Locally edit the NoteBook.\n");
		editor_edit(nbname);
		break;

	    case 'S':
		cprintf("Show NoteBook contents.\n\n");
		cprintf("\1p-- start of notebook -----\n\1g");
		more(nbname, 1);
		cprintf("\1f\1p-- end of notebook -----\n");
		break;

	    case '?':
		cprintf("Help.\n");
		more(MENUDIR "/menu_note", 1);
		break;

	    default:
		cprintf("\n");
		return;
	}
    }
}

int
mail_clipboard(const char *file)
{

#ifdef HAVE_SENDMAIL
    FILE *sendmail = NULL;
    char *message = NULL;
#else
    char work[L_USERNAME + RGemailLEN + 85];
#endif
    char subject[50];
    size_t accepted;

    accepted = strspn(usersupp->RGemail, ACCEPTED);

    if (accepted < strlen(usersupp->RGemail)) {
        cprintf(_("\1r\1fInvalid email address, can't send message!\1a\n"));
        return -1;
    }

    IFSYSOP {
        cprintf(_("\1f\1g\nDo you wish to send it to yourself? \1c"));
        if (yesno() == NO) {
            nox = 1;
            wacky_email_stuff();
            return 0;
        }
    }
    cprintf(_("\1f\1gE-Mail yourself the ClipBoard.\n"));

    cprintf(_("\1g\1fAre you sure you want to email your clipboard? \1w(\1gy\1w/\1gN\1w) \1c"));

    if (yesno_default(NO) == NO)
        return 0;

    cprintf(_("\1g\1f\nWould you like to remove the color codes? \1w(\1gY/n\1w)\1g \1c"));
    if (yesno_default(YES) == YES)
        if (de_colorize(file) != 0)
            cprintf(_("\1f\1r\nWarning\1w:\1g Couldn't remove colours.\1a"));

    cprintf("\1f\1gSubject for the email: \1c");
    getline(subject, sizeof(subject), 1);

#ifndef HAVE_SENDMAIL
    if (strspn(subject, ACCEPTED) < strlen(subject)) {
        log_it("email", "Email to %s failed.\n%s tried to send clipboard with subject %s", usersupp->RGemail, usersupp->username, subject);
        cprintf(_("\1r\1fSubject contains invalid characters, can't send message!\1a\n"));
        return 1;
    }
#endif

    if (strlen(subject) == 0)
        sprintf(subject, "%s@%s BBS: Clipboard-Contents", usersupp->username, BBSNAME);

#ifdef HAVE_SENDMAIL
    if( (sendmail = popen( SENDMAIL " -t", "w" )) == NULL ) {
	log_it("email", "Error trying to send clipboard to %s.\nCan't popen(%s)", usersupp->RGemail, SENDMAIL);
        cprintf(_("\1f\1rError trying to send clipboard to %s.\nCan't popen(%s)\n"), usersupp->RGemail, SENDMAIL );
	return 2;
    }
    if ((message = map_file(file)) == NULL) {
        xfree(message);
	log_it("email", "Error trying to send clipboard to %s.\nCan't mmap(%s)", usersupp->RGemail, file);
	cprintf(_("\1f\1rError trying to send clipboard to %s.\nCan't mmap(%s)\n"), usersupp->RGemail, file);
	return 2;
    }

    fprintf(sendmail, "To: %s <%s>\n", usersupp->RGname, usersupp->RGemail);
    fprintf(sendmail, "Subject: %s\n", subject);
    fprintf(sendmail, "%s\n", message);
    xfree(message);

    if( pclose(sendmail) == -1 ) {
	log_it("email", "Error trying to send clipboard to %s.\nCan't pclose(%X)", usersupp->RGemail, sendmail);
	cprintf(_("\1f\1rError trying to send clipboard to %s.\nCan't pclose(%X)\n"), usersupp->RGemail, sendmail);
	return 2;
    }
#else
    sprintf(work, "/bin/mail -s '%s' %s < %s", subject, usersupp->RGemail, file);
    if (system(work) == -1) {
        log_it("email", "Error trying to send clipboard to %s.", usersupp->RGemail );
        cprintf(_("\1f\1rError trying to send clipboard to %s."), usersupp->RGemail );
        return 2;
    }
#endif

    log_it("email", "%s sent clipboard to %s", usersupp->username, usersupp->RGemail);
    cprintf(_("\1g\1fMail was sent to \1c%s\1a\n"), usersupp->RGemail);

    return 0;
}
