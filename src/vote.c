/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define VOTEDIR BBSDIR "/etc/votes"

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"

#define extern
#include "vote.h"
#undef extern

#include "routines2.h"
#include "input.h"
#include "uadmin.h"

/*************************************************
* voting_booth()
*
* VotingBooth, where the users can vote yes or no
*   on the items (topics) in VOTEDIR:
*
* VOTEDIR/itemx, where x is the number of the
*   item, contains the text that is voted upon.
*
* VOTEDIR/votingsx, where x is the number of
*   the item, contains the name of the user that
*   voted, and 1 if s/he voted yes, 0 if s/he
*   voted no. + a \n just so that the file is more
*   readable. (c:
* if you want only sysops to be able to read the
* results of an item; put a "#SYSOPONLY -1" in the
* voting-file of that item.
*************************************************/

void
voting_booth()
{
    int cmd = '\0';
    int item = 1, a;
    char votefile[50];
    char col[200];

    more("share/messages/vote_info", 1);
    do {
	sprintf(votefile, "%s/item%d", VOTEDIR, item);

	cprintf("\1a\1c\1f");

	if (!fexists(votefile) || more(votefile, 1) == -1) {
	    cprintf("\1r\007No more Items forwards.\n");
	    item--;
	    continue;
	} else
	    cprintf("\n");

	for (;;) {
	    cprintf("\n\1w[ \1yVoting Booth: \1gItem \1p%d \1w] [ \1y<\1r?\1y>\1g for Help \1w]: ", item);
	    cmd = get_single_quiet("ANPQRV? \r\n*");

	    if (cmd == '?')
		cprintf("\1w\n\nYour options: \1y<\1gA\1y>\1ggain \1y<\1gN\1y>\1gext \1y<\1gP\1y>\1grevious \1y<\1gQ\1y>\1guit \1y<\1gV\1y>\1gote \n");
	    else
		break;
	}

	switch (cmd) {
	    case 'A':
		cprintf("Again.\n");
		break;

	    case 'N':
		cprintf("Next Item.\n");
		item++;
		break;

	    case 'P':
		cprintf("Previous Item.\n");
		item--;
		if (item < 1) {
		    cprintf("\1r\n\007No more previous Items.\n");
		    item = 1;
		}
		break;

	    case ' ':
	    case 'Q':
	    case CR:
	    case LF:
		cprintf("Quit.\n");
		break;

	    case 'R':
		if (usersupp->priv < PRIV_WIZARD)
		    break;
		cprintf("Check Results.\n");
		check_voting_results(item);
		break;

	    case 'V':
		cprintf("Vote about this Item.\n");
		vote_about(item);
		break;

	    case '*':
		IFSYSOP {
		    for (;;) {
			cprintf("\n\n\1pSysop \1yVotingBooth \1gCmd:\1w ");
			cmd = get_single_quiet("ADE Q?\r");
			if (cmd == '?')
			    cprintf("\1w\n\nYour options:\t\1y<\1rA\1y>\1gdd Item \1y<\1rD\1y>\1gDelete Item \1y<\1rE\1y>\1gdit Item \1y<\1rQ\1y>\1guit ");
			else
			    break;
		    }

		    switch (cmd) {
			case 'A':
			    cprintf("Add new Item.\n");
			    for (;;) {
				sprintf(col, "%s/item%d", VOTEDIR, ++item);
				if (fexists(col) == FALSE)
				    break;	/* found an empty one */
			    }

			    sprintf(votefile, "%s/item%d", VOTEDIR, item);

			    a = editor_edit(votefile);
			    if (a == 1)
				log_sysop_action("added a VotingBoothItem (%d)", item);

			    sprintf(col, "%s/votings%d", VOTEDIR, item);
			    close(creat(col, 0640));

			    break;

			case 'D':
			    cprintf("Delete this Item.\n");
			    cprintf("\n  Are you sure? (y/n) ");
			    if (yesno() == NO)
				break;
			    unlink(votefile);
			    sprintf(col, "%s/votings%d", VOTEDIR, item);
			    unlink(col);
			    log_sysop_action("deleted VotingBoothItem %d", item);

			    item--;
			    break;

			case 'E':
			    cprintf("Edit this Item.\n");
			    a = editor_edit(votefile);
			    if (a == 1)
				log_sysop_action("edited VotingBoothItem %d", item);

			    cprintf("Do you want to reset the Result-file? ");
			    if (yesno() == YES) {
				sprintf(col, "%s/votings%d", VOTEDIR, item);
				close(creat(col, 0644));
			    }
			    break;

			default:
			    cprintf("\n");
			    break;
		    }
		}

	}

    }
    while (cmd != 'Q' && cmd != ' ' && cmd != '\r' && cmd != '\n');

    cprintf("\n");
}

/*************************************************
* vote_about()
*************************************************/

void
vote_about(int item)
{

    char file[50];
    FILE *fp;
    int voteresult;
    char votename[L_USERNAME + 1];
    char myname[L_USERNAME + 1];

    sprintf(file, "%s/votings%d", VOTEDIR, item);

    strcpy(myname, usersupp->username);
    name2file(myname);

    fp = xfopen(file, "r", FALSE);
    if (!fp) {
	cprintf("\1r\1fCan not open results file.\1a\n");
	return;
    }
    /* don't use fscanf, use fgets, and sscanf, k */
    while (fscanf(fp, "%s %d\n", /* & */ votename, &voteresult) != EOF) {
	if (EQ(votename, myname)) {
	    cprintf("\1r\n\007You have already voted about this item.\n");
	    return;
	}
    }

    fclose(fp);

    fp = xfopen(file, "a", FALSE);
    if (!fp) {
	cprintf("\1r\1fCan not open results file.\1a\n");
	return;
    }
    cprintf("\1yWhat is your vote: \1g<\1rY\1g>\1es or \1g<\1rN\1g>\1yo? \1w");
    voteresult = yesno();

    fprintf(fp, "%s %d\n", myname, voteresult);
    fclose(fp);
    return;
}

/*************************************************
* check_voting_results()
*************************************************/

void
check_voting_results(int item)
{

    char file[50];
    FILE *f;
    int voteresult;
    char votename[L_USERNAME + 1];
    int totalYES = 0;
    int totalNO = 0;
    char work[140];
    unsigned int i;


    sprintf(file, "%s/votings%d", VOTEDIR, item);

    f = xfopen(file, "r", FALSE);
    if (!f) {
	cprintf("\1r\1fCan not open results file.\n");
	return;
    }
    while (fscanf(f, "%s %d\n", /* & */ votename, &voteresult) != EOF) {
	if (strcmp(votename, "#SYSOPONLY") == 0) {
	    IFNSYSOP
		cprintf("\1r\n\007Sorry, but only Sysops can see the results for this item.\n");
	    else
	    continue;
	    return;
	}
	if (voteresult == 1)
	    totalYES++;
	else if (voteresult == 0)
	    totalNO++;
	else
	    cprintf("%s voted something else than Yes and No?!?! (c:\n", votename);
    }

    fclose(f);

    cprintf("\n\1w\1f");
    sprintf(work, "\n\1w* \1pItem \1w%d \1pcurrently has \1w%d \1gYes\1p-votes and \1w%d \1rNo\1p-votes. (Total: \1w%d \1pvotes) \1w*\n", item, totalYES, totalNO, totalYES + totalNO);
    for (i = 0; i < strlen(work) - 28; i++)
	cprintf("*");
    cprintf(work);
    for (i = 0; i < strlen(work) - 28; i++)
	cprintf("*");
    cprintf("\n");

    return;
}
