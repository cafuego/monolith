/* $Id$ */

/*----------------------------------------------------------------------------*/
/* quad_content.c 
 * routines for storing/manipulation/evaluation of a quadrant's content
 
 bug:  occasional segfault after heavy category editing/renaming.  i've had 
 trouble re-creating it, so it hasn't been fixed.  *argh*  otherwise harmless 
 though, doesn't mung the files, just boots you and leaves the qc locked.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "monolith.h"

#include "quadcont.h"

#include "rooms.h"		/* for readquad, leave_n_unread_posts, etc. */
#include "routines2.h"		/* for more, etc. */
#include "userfile.h"		/* for writeuser */
#include "ext.h"		/* for curr_rm and such */
#include "libquad.h"		/* for may_read_room, etc */
#include "routines.h"		/* for xmalloc, etc */
#include "input.h"		/* for getline */

/*----------------------------------------------------------------------------*/

void
qc_menu(void)
{
    register char menucmd = '\0';

    cprintf("\n\n\1f\1w<\1re\1w>\1gdit \1w<\1rl\1w>\1gock menu \1w<\1rc\1w>\1gategories l\1w<\1ri\1w>\1gsts \1w<\1rq\1w>\1guit\nQC main menu command: \1a");
    menucmd = get_single_quiet("elcqui ");
    switch (menucmd) {
	case 'e':
	    cprintf("\1a\1f\1rEdit\1a\n");
	    qc_edit_room();
	    break;
	case 'l':
	    qc_lock_menu();
	    break;
	case 'c':
	    qc_categories_menu();
	    break;
	case 'i':
	    qc_lists_menu();
	    break;
#ifdef QC_DEBUG
	case 'u':
	    cprintf("\1a\1f\1rUser Menu\1a\n");
	    qc_user_menu(0);
	    break;
#endif
	case ' ':
	case 'q':
	    cprintf("\1f\1rQuitting..\1a");
	    return;
	default:
	    cprintf("\n\1f\1c weirdness..\1a\n");
	    return;
    }				/* switch */
    qc_menu();
}

/*----------------------------------------------------------------------------*/
/*  lists:  uncoded as of yet, prolly should be able to list quads that have
 * a qc file with last modified date and whodunit, quads that don't have a 
 * qcfile.
 */
void
qc_lists_menu()
{
    register char menucmd = '\0';

    cprintf("\n\n\1f\1w<\1rn\1w>\1gofile \1w<\1rq\1w>\1guit\nQC list command: \1a");
    menucmd = get_single_quiet("nq ");
    switch (menucmd) {
	case 'n':
	    cprintf("\1a\1f\1rNo qc files list:\1a\n");
	    qc_show_noqcfile();
	    break;
	case ' ':
	case 'q':
	    cprintf("\1f\1rQuitting..\1a");
    }
    return;
}

																																								/*----------------------------------------------------------------------------*//* displays list of quads with no qc file
																																								 */

void
qc_show_noqcfile(void)
{
    int i, j = 0;
    room_t scratch;
    qc_thingme *temp_read = NULL;

    for (i = 0; i < MAXQUADS; i++) {
	if (i < 10)		/* don't bother with useless quads */
	    continue;
	scratch = readquad(i);
	if ((!(scratch.flags & QR_INUSE)) || (scratch.flags & QR_NOZAP) ||
	    (scratch.flags & QR_PRIVATE))
	    continue;
	if ((temp_read = qc_read_file(i)) != NULL)
	    xfree(temp_read);
	else {
	    cprintf("\n\1f\1y%3d\1w> \1r%s", i, scratch.name);
	    j++;
	}
	if ((j) && ((j % (usersupp->screenlength - 5)) == 0)) {
	    cprintf("\n\1c\1f<more>\1g");
	    inkey();
	    j = 0;
	}
    }
    return;
}

/*----------------------------------------------------------------------------*/
void
qc_edit_room(void)
{
    qc_thingme *temp_quad = NULL;
    register char editcmd = '\0';

    if (curr_rm == TEMPLATE_QUAD) {
	cprintf("\n\1a\1f\1cThis quad is where the qc template is stored.  Aborting.\1a");
	return;
    }
    if ((temp_quad = qc_read_file(curr_rm)) == NULL) {
	cprintf("\n\1a\1f\1gNo qc record is found for this quad.  Create one? \1w(\1ry\1w/\1rn\1w) \1a");
	if (yesno()) {
	    qc_create_file(0);
	    if ((temp_quad = qc_read_file(curr_rm)) == NULL)
		return;
	} else
	    return;
    }
    if (!qc_change_lock_status(curr_rm, 1))	/* lock curr_rm */
	return;
    qc_display(temp_quad, 1);
    xfree(temp_quad);
    temp_quad = NULL;
    cprintf("\n\1a\1f\1w<\1rf\1w>\1glags \1w<\1rs\1w>\1get value \1w<\1r?\1w> <\1rq\1w>\1guit\n\1rCommand\1w: \1a");
    editcmd = get_single_quiet("fsq? ");
    switch (editcmd) {
	case 'f':
	    cprintf("\1a\1f\1rFlags\1w: \1a\n");
	    qc_set_flags(0);
	    if (!qc_change_lock_status(curr_rm, 0))	/* unlock curr_rm */
		cprintf("\n\1f\1yCouldn't unlock quad \1w%d\1y.\1a\n", curr_rm);
	    qc_edit_room();
	    break;
	case '?':
	    qc_change_lock_status(curr_rm, 0);
	    more(QC_QUAD_EDIT_DOC, 0);
	    qc_edit_room();
	    break;
	case 's':
	    cprintf("\1a\1f\1rSet qc value\1w: \1a");
	    while (qc_set_value(-1)) {
		if ((temp_quad = qc_read_file(curr_rm)) == NULL) {
		    if (!qc_change_lock_status(curr_rm, 0))	/* unlock curr_rm */
			cprintf("\n\1f\1yCouldn't unlock quad \1w%d\1y.\1a\n", curr_rm);
		    return;
		}
		qc_display(temp_quad, 1);
		xfree(temp_quad);
		cprintf("\1a\1f\1gSet qc value\1w: \1a");
	    }
	    if (!qc_change_lock_status(curr_rm, 0))	/* unlock curr_rm */
		cprintf("\n\1f\1yCouldn't unlock quad %d.\1a\n", curr_rm);
	    qc_edit_room();
	    break;
	case ' ':
	case 'q':
	    if (!qc_change_lock_status(curr_rm, 0))	/* unlock curr_rm */
		cprintf("\n\1f\1yUhh-ohh, couldn't unlock quad \1w%d\1y.\n", curr_rm);
	    cprintf("\n\1f\1gQuitting..\1a");
	    return;
	default:
	    cprintf("\n\1f\1yQuad edit lock on \1w%d\1y may not have been released.\1a\n", curr_rm);
	    return;
    }
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_add_category(void)
{
    qc_thingme *tmp_write, *tmp_tmplt;
    int i, cat_slot = 0, fail = 0;
    char category[L_CONTENT_CATEGORY];

    /* find an unused slot */
    if ((tmp_tmplt = qc_read_file(TEMPLATE_QUAD)) == NULL)
	return;
    for (i = 0; i < NO_OF_CATEGORIES; i++)
	if (strcmp(tmp_tmplt->category_name[i], "<unused>") == 0)
	    break;
	else if (i == NO_OF_CATEGORIES - 1) {
	    cprintf("\n\1f\1gAll slots are used. \1a");
	    fail = 1;
	}
    if (!fail) {
	cat_slot = i;
	cprintf("\n\1f\1gNew category name for slot \1w%d\1g, or \1w<\1rret\1w>\1g exits: \1a", i + 1);
	getline(category, L_CONTENT_CATEGORY - 1, 1);
	if (strlen(category) == 0)
	    fail = 1;
	else if (strcmp(category, "<unused>") == 0) {
	    cprintf("\1f\1gI can't do that, Dave. \1a");
	    fail = 1;
	} else
	    for (i = 0; i < NO_OF_CATEGORIES; i++)
		if (strcmp(tmp_tmplt->category_name[i], category) == 0) {
		    cprintf("\n\1f\1gThat name has already been used. \1a");
		    fail = 1;
		}
    }
    xfree(tmp_tmplt);
    if (!fail) {
	cprintf("\n\n\1f\1gNew category name: \1y%s\1g  Save? \1w(\1rY\1w/\1rn\1w) \1a", category);
	if (!(yesno_default(1)))
	    return;
	/* ok, if you say so.. write 'em */
	for (i = 0; i < MAXQUADS; i++) {
	    tmp_write = NULL;
	    if ((tmp_write = qc_read_file(i)) == NULL)
		continue;
	    strcpy(tmp_write->category_name[cat_slot], category);
	    if (i == TEMPLATE_QUAD) {	/* mark who made changes in Mail only */
		tmp_write->last_mod_on = time(NULL);
		strcpy(tmp_write->last_mod_by, usersupp->username);
	    }
	    if (qc_write_file(tmp_write, i) != 1) {	/* ack. hope we have backups */
		cprintf("\n\1a\1f\1r*argh*  Horked write, quad %d during update\n\1a", i);
		xfree(tmp_write);
		break;
	    }
	    xfree(tmp_write);
	}			/* for i */
	cprintf("\n\1a\1f\1g\nDone. \1a");
    }
    cprintf("\1a\1f\1gPress a key. \1a");
    inkey();
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_delete_category(void)
{
    qc_thingme *tmp_write, *tmp_tmplt;
    int i, cat_slot = 0;

    if ((cat_slot = qc_get_category_slot()) == -1)
	return;
    if ((tmp_tmplt = qc_read_file(TEMPLATE_QUAD)) == NULL)
	return;

    cprintf("\n\1a\1f\1rWARNING\1w: This will delete category \1y%d\1w> \1r%s\1w in ALL rooms.\1a\n\n",
	    cat_slot + 1, tmp_tmplt->category_name[cat_slot]);
    cprintf("\1a\1f\1gAre you sure? \1w(\1ry\1w/\1rN\1w) \1a");
    xfree(tmp_tmplt);

    if (!(yesno_default(0)))
	return;

    for (i = 0; i < MAXQUADS; i++) {
	tmp_write = NULL;
	if ((tmp_write = qc_read_file(i)) == NULL)	/* quad has no qc file, skip */
	    continue;
	strcpy(tmp_write->category_name[cat_slot], "<unused>");
	tmp_write->content_quotient[cat_slot] = 0;
	if (i == TEMPLATE_QUAD) {	/* mark who made changes in template only */
	    tmp_write->last_mod_on = time(NULL);
	    strcpy(tmp_write->last_mod_by, usersupp->username);
	}
	if (qc_write_file(tmp_write, i) != 1) {		/* hope we have backups */
	    cprintf("\n\1a\1f\1r*argh*  Horked write, quad %d during update\n\1a", i);
	    xfree(tmp_write);
	    break;
	}
	xfree(tmp_write);
    }				/* for */
    cprintf("\n\1f\1gDeleted.  Press a key..\1a");
    inkey();
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_rename_category(void)
{
    qc_thingme *tmp_write, *tmp_tmplt;
    int i, cat_slot = 0, fail = 0;
    char category[L_CONTENT_CATEGORY];

    if ((cat_slot = qc_get_category_slot()) == -1)
	return;
    if ((tmp_tmplt = qc_read_file(TEMPLATE_QUAD)) == NULL)
	return;
    if (strcmp(tmp_tmplt->category_name[cat_slot], "<unused>") == 0) {
	cprintf("\n\1f\1gCan't rename an unused slot. \1a");
	fail = 1;
    } else {
	cprintf("\n\1f\1gRename category \1y%d\1w> \1y%s\1g, to: \1a\n\n", cat_slot + 1,
		tmp_tmplt->category_name[cat_slot]);
	getline(category, L_CONTENT_CATEGORY - 1, 1);
	if (strlen(category) == 0) {
	    cprintf("\1f\1gNull input, Quitting.\1a  ");
	    fail = 1;
	} else if (strcmp(category, "<unused>") == 0) {
	    cprintf("\1f\1gCan't rename to <unused>.\1a ");
	    fail = 1;
	} else
	    for (i = 0; i < NO_OF_CATEGORIES; i++)	/* is already used? */
		if (strcmp(tmp_tmplt->category_name[i], category) == 0) {
		    cprintf("\n\1f\1gThat name is already used.\1a ");
		    fail = 1;
		    break;
		}
    }
    if (!fail) {
	cprintf("\n\1f\1g   Old category name\1w:\1y %s", tmp_tmplt->category_name[cat_slot]);
	cprintf("\n\1g   New category name\1w: \1y%s\n\n", category);
	cprintf("\1gSave this, Dave?  \1w(\1rY\1w/\1rn\1w) \1a");
	if (!(yesno_default(1)))
	    fail = 1;
    }
    xfree(tmp_tmplt);
    if (fail) {
	cprintf("\1f\1gPress a key. \1a");
	inkey();
	return;
    }
    for (i = 0; i < MAXQUADS; i++) {
	tmp_write = NULL;
	if ((tmp_write = qc_read_file(i)) == NULL)
	    continue;
	strcpy(tmp_write->category_name[cat_slot], category);
	if (i == TEMPLATE_QUAD) {	/* mark who made global changes in Mail only */
	    strcpy(tmp_write->last_mod_by, usersupp->username);
	    tmp_write->last_mod_on = time(NULL);
	}
	if (qc_write_file(tmp_write, i) != 1) {		/* don't crash horribly */
	    cprintf("\n\1a\1f\1r*argh*  Horked write, quad %d during update\n\1a", i);
	    xfree(tmp_write);	/* don't leave memory allocated either */
	    break;
	}
	xfree(tmp_write);
    }				/* for */
    cprintf("\n\1f\1gCategory renamed.  Press a key..\1a");
    inkey();
    return;
}


/*----------------------------------------------------------------------------*/

void
qc_categories_menu(void)
{
    qc_thingme *tmp_tmpl = NULL;
    register char cmd = '\0';
    int i;

    if ((tmp_tmpl = qc_read_file(TEMPLATE_QUAD)) == NULL) {
	/* uhh ohh..  now check for serious fuckup  */
	for (i = 0; i < MAXQUADS; i++)
	    if ((tmp_tmpl = qc_read_file(i)) != NULL) {
		xfree(tmp_tmpl);
		break;
	    }
	if (i < MAXQUADS) {	/* argh, found qcfile and no template! */
	    cprintf("\n\1a\1f\1rqc FATAL:  found qcfile and no template!\n");
	    cprintf("Please report this error, and don't fiddle with things unless ");
	    cprintf("you\nknow what you're doing!  Press a key. \1a");
	    qc_set_lockout();	/* better lock things up */
	    inkey();
	    return;
	}
	/* else, first run..  create template */
	cprintf("\nNo template was found..  Create one? (y/n) ");
	if (yesno()) {
	    qc_create_file(1);
	    if ((tmp_tmpl = qc_read_file(TEMPLATE_QUAD)) == NULL)
		return;
	} else
	    return;
    }
    if (!qc_set_lockout()) {
	cprintf("\n\1f\1gPress a key..\1a");
	xfree(tmp_tmpl);
	inkey();
	return;
    }
    qc_display(tmp_tmpl, 2);
    xfree(tmp_tmpl);
    tmp_tmpl = NULL;

    cprintf("\n\1a\1f\1w<\1ra\1w>\1gdd \1w<\1rd\1w>\1gelete \1w<\1rr\1w>\1gename \1w<\1rq\1w>\1guit\n\1rCommand: \1a");
    cmd = get_single_quiet("adrq ");
    switch (cmd) {
	case 'a':
	    cprintf("\1f\1r  Add a category.\1a\n");
	    qc_add_category();
	    break;
	case 'd':
	    cprintf("\1f\1r  Delete a category.\1a\n");
	    qc_delete_category();
	    break;
	case 'r':
	    cprintf("\1f\1r  Rename a category.\1a\n");
	    qc_rename_category();
	    break;
	case ' ':
	case 'q':
	    cprintf("\1f\1r  Quitting..\1a");
	    if (!qc_clear_lockout())	/* unlock template */
		cprintf("\n\1f\1gCheck lock status.\1a\n");
	    return;
	default:
	    cprintf("\n\1f\1r  Argh.  Weird shit just happened..\1a\n");
    }				/* switch */
    if (!qc_clear_lockout()) {	/* unlock template */
	cprintf("\n\1f\1gCheck lock status..\1a\n");
	return;
    }
    qc_categories_menu();
}


/*----------------------------------------------------------------------------*/

int
qc_user_menu(int newbie)
{
    qc_thingme *user_input;
    int i, items_left = 0, user_value, *eval;
    char foo;
    room_t scratch;

    if (qc_lockout())
	return 0;
    else if ((user_input = qc_read_file(TEMPLATE_QUAD)) == NULL)	/* no template! */
	return 0;
    cprintf("\1g");		/* make it green, white looks like hell */
    if (newbie) {
#ifdef QC_DEBUG
	return 0;
#endif
	more(QC_NEWUSER_HELP, 0);

    cprintf("\1f\1gPress a key..\n");
    inkey();
    }
    more(QC_USERDOC, 0);

    cprintf("\n\1f\1w -- \1gPress a key \1w -- \1a");
    inkey();
    for (i = 0; i < NO_OF_CATEGORIES; i++)
	if (strcmp(user_input->category_name[i], "<unused>") == 0)
	    continue;
	else {
	    items_left++;
	    user_input->content_quotient[i] = CONTENT_SCALE;
	}
    for (i = 0; i < NO_OF_CATEGORIES; i++) {
	if (strcmp(user_input->category_name[i], "<unused>") == 0)
	    continue;
	cprintf("\n\n\1f\1w[ \1gCategory \1w%d\1g of \1w%d ]\1g"
		,i + 1, items_left + i);
	cprintf("\n");
	cprintf("\n -> Press a number from \1w0\1g to \1w%d\1g", CONTENT_SCALE);
	cprintf("\n ->   or  \1w<\1rb\1w>\1g to go back to last category,");
	cprintf("\n ->   or  \1w<\1r?\1w>\1g to see the helpscreen again,");
	cprintf("\n ->   or  \1w<\1rq\1w>\1g to quit.");
	cprintf("\n\n    Rate your interest in \1w:\n   \1y'%s'\1g on"
		,user_input->category_name[i]);
	cprintf(" a scale of \1w0\1g to \1w%d: [\1r%d\1w] \1a"
		,CONTENT_SCALE, user_input->content_quotient[i]);
	foo = '\0';		/* reset foo */
	foo = get_single_quiet("0987654321b\?q\r\n");
	switch (foo) {
	    case 'b':
		if (i == 0) {
		    cprintf("\n\n\1f\1gYou are already at the first category. Press a key..");
		    inkey();
		    i--;	/* ugly! */
		} else {
		    items_left++;
		    i -= 2;	/* twice as ugly (: */
		}
		continue;
	    case '\r':
	    case '\n':
		cprintf("\n");
		user_input->content_quotient[i] = user_input->content_quotient[i];
		items_left--;
		break;
	    case '\?':
		more(QC_USERDOC, 0);
		cprintf("\1f\1w-- \1gpress a key \1w-- ");
		inkey();
		i--;
		continue;
	    case 'q':
		if (items_left) {
		    cprintf("\n\1f\1gThere are still \1w%d\1g items left. Quitting now will \1rabort\1g the\nselection algorithm.\n\nAre you SURE you want to quit now? \1w(\1ry\1w/\1rN\1w) \1a", items_left);
		    if (!yesno_default(0)) {
			i--;
			continue;
		    } else if (newbie) {
			cprintf("\n\1f\1g You will be given \1rall\1g available quads to read.");
			cprintf("\nYou can then sort out what you want to read yourself.  (-: \n\1w -- \1g Press a key \1w-- \1a");
			inkey();
		    }
		    cprintf("\n\n\1f\1gQuitting.\1a ");
		    xfree(user_input);
		    return 0;
		}
	    default:{
		    if ((user_value = qc_get_pos_int(foo, (CONTENT_SCALE < 10) ? 1 : 2)) != -1) {
			if (user_value > CONTENT_SCALE) {
			    cprintf("\n\1f\1gInvalid input..  need a number between \1w0\1g and \1w%d\1g.", CONTENT_SCALE);
			    cprintf("\n\1w -- \1gPress a key \1w-- \1a");
			    inkey();
			    i--;
			    continue;
			} else {
			    user_input->content_quotient[i] = user_value;
                        }
                    }
		    items_left--;
		}		/* default */
	}			/* switch (foo) */
    }				/* for i */

    /* insert real menu edit screen here at some later date..  yeah. */

    if ((eval = qc_evaluate_quads(user_input, newbie, usersupp)) == NULL) {
	cprintf("\n\n\1f\1cArgh!  Something really bad happened with the selection code.");
	cprintf("\nThe coder should obviously have slept before he wrote it.");
	cprintf("\nPress any key to send him a porcupine.. \1a");
	inkey();
	xfree(user_input);
	return 0;
    }
    for (i = 0; i < MAXQUADS; i++) {
	if (eval[i] == -5)	/* skip these, regardless */
	    continue;
	else if ((eval[i] == -1) || (eval[i] == -2)) {	/* stuff to zap */
	    if (newbie) {
		/* zap it quietly if newbie */
		scratch = readquad(i);
		usersupp->forget[i] = scratch.generation;
		usersupp->generation[i] = -1;
	    }
	} else {
	    if (newbie)
		leave_n_unread_posts(i, eval[i]);
	}
    }				/* for i */
    /* insert display actions here */
    if (newbie) {
	writeuser(usersupp, 0);
	cprintf("\n\1f\1g");
	more(QC_NEWUSER_DONE, 0);
#ifdef QC_DEBUG
	cprintf("\nWrote user..\n");
#endif
    } else {			/* not newbie..  display / mail results */

	cprintf("\n\n\1f\1gDo you want to: \1w<\1rv\1w>\1giew or \1w<\1rm\1w>\1gail yourself the results? ");
	foo = '\0';
	foo = get_single_quiet("vmVM");		/* vroom, VROOM?  (: */
	switch (foo) {
	    case 'm':
	    case 'M':
		cprintf("\1cMailing..");
		if (!(qc_mail_results(eval)))
		    cprintf("\n\n*argh*  Problems mailing results, please report this bug. \1a");
		else
		    cprintf("\1g    Done.\n\n");
		break;
	    case 'v':
	    case 'V':	
		cprintf("\1cView\n\n");
		qc_show_results(eval);
		cprintf("\n\n");
		break;
	}			/* switch */

#ifdef QC_DEBUG  
	if (usersupp->priv >= PRIV_TECHNICIAN) {
	    cprintf("\n\1rDebug:  user_input:\n");
	    qc_display(user_input, 0);
	    cprintf("\n\1rDebug:  return vector:\n");
	    for (i = 0; i < MAXQUADS; i++)
		cprintf("\1f\1r%3d:\1c%3d ", i, eval[i]);
	    inkey();
	}
#endif

	xfree(eval);
	xfree(user_input);
    }
    more(QC_USERDOC_DISCLAIMER, 0);
    return 1;
}

/*----------------------------------------------------------------------------*/
void 
qc_show_results(const int *eval)
{
    int i, j = 0, items_left = 0;
    room_t scratch;

    for (i = 0; i < MAXQUADS; i++) {
	if (eval[i] == -1) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] != scratch.generation) {
		if (!items_left)
		    cprintf("\n\1f\1gSelection algorithm suggests zapping the following quads according to\nyour interests..\1a\n\n");
		cprintf("\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		j++;
		items_left++;
	    }
	}
	if ((j) && ((j % (usersupp->screenlength - 5)) == 0)) {
	    cprintf("\1w\1f -- \1gmore\1w --\1a\n");
	    inkey();
	    j = 0;
	}
    }				/* for i */
    if (items_left) {
	cprintf("\n\1f\1w -- \1gpress a key \1w--\1a");
	inkey();
    }
    j = items_left = 0;
    for (i = 0; i < MAXQUADS; i++) {
	if (eval[i] > 0) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] == scratch.generation) {
		if (!items_left)
		    cprintf("\n\1f\1gSelection algorithm suggests unzapping the following quads according to\nyour interests..\1a\n\n");
		cprintf("\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		j++;
		items_left++;
	    }
	}
	if ((j) && (j % (usersupp->screenlength - 5) == 0)) {
	    cprintf("\1w\1f-- \1gmore\1w --\1a\n");
	    inkey();
	    j = 0;
	}
    }				/* for i */
    if (items_left) {
	cprintf("\n\1f\1w -- \1gok.. press any key \1w--\1a");
	inkey();
    }
    return;
}

/*----------------------------------------------------------------------------*/

int 
qc_mail_results(const int *eval)
{

#ifndef QC_CONVERTED_TO_NEW_MESSAGE_CODE

    cprintf("\n\1f\1rThis function has been temporarily disabled.\n");

#else
    for (i = 0; i < MAXQUADS; i++) {
	if (eval[i] == -1) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] != scratch.generation) {
		if (!items_left)
		    fprintf(fp, "\n\1f\1gSelection algorithm suggests zapping the following quads:\1a\n\n");
		fprintf(fp, "\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		items_left++;
	    }
	}
    }
    items_left = 0;
    for (i = 0; i < MAXQUADS; i++) {
	if (eval[i] > 0) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] == scratch.generation) {
		if (!items_left)
		    fprintf(fp, "\n\1f\1gSelection algorithm suggests unzapping the following quads:\1a\n\n");
		fprintf(fp, "\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		items_left++;
	    }
	}
    }

#endif

    return 1;
}

/*----------------------------------------------------------------------------*/

int
qc_create_file(int create_template)
{
    FILE *qcPtr;
    qc_thingme *tmp_tmpl;
    qc_thingme temp_blank =
    {0,
     {"",},
     {0,},
     {0,}, "", 0};
    int j;
    char filename[80];

    (void) sprintf(filename, QC_FILEDIR "%d/qcfile",
		   ((create_template) ? TEMPLATE_QUAD : curr_rm));
    if ((qcPtr = fopen(filename, "w")) == NULL) {
	cprintf("\1\1f\1cCan't create qc file.\1a\n");

	return 0;
    }
    fwrite(&temp_blank, sizeof(qc_thingme), 1, qcPtr);
    fclose(qcPtr);

    if ((qcPtr = fopen(filename, "r+")) == NULL) {
	cprintf("\1f\1cAck!  Can't seem to open new quad content file for writing..\1a\n");
	return 0;
    }
    if (create_template)
	for (j = 0; j < NO_OF_CATEGORIES; j++)
	    strcpy(temp_blank.category_name[j], "<unused>");
    else {			/* need template to copy stuff */
	if ((tmp_tmpl = qc_read_file(TEMPLATE_QUAD)) == NULL) {		/* eek */
	    cprintf("\n\1\f\1cCreate failed, couldn't read template!  Press a key. \1a");
	    fclose(qcPtr);
	    unlink(filename);	/* clean up mess */
	    inkey();
	    return 0;
	}
	for (j = 0; j < NO_OF_CATEGORIES; j++)
	    strcpy(temp_blank.category_name[j], tmp_tmpl->category_name[j]);
    }
    temp_blank.quad_number = ((create_template) ? TEMPLATE_QUAD : curr_rm);
    for (j = 0; j < NO_OF_CATEGORIES; j++)
	temp_blank.content_quotient[j] = 0;
    for (j = 0; j < ARGH_QUAD_FLAGS; j++)
	if (j == 0)
	    temp_blank.flags[j] = NEWBIE_NUM_UNREAD;
	else
	    temp_blank.flags[j] = 0;
    strcpy(temp_blank.last_mod_by, usersupp->username);
    temp_blank.last_mod_on = time(NULL);
    fwrite(&temp_blank, sizeof(qc_thingme), 1, qcPtr);

    fclose(qcPtr);
#ifdef QC_DEBUG
    cprintf("\nqc file created.\n");
#endif
    return 1;
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  qc_thingme * qc_read_file(char * qc_file_name, int the_quad_no)
 *  reads a quad's record from database and returns a pointer to a quad's
 *  content quotient structure.
 *
 *  WARNING:  returns pointer to self allocated memory, make sure to
 *            free!
 */

qc_thingme *
qc_read_file(int quad_num)
{
    qc_thingme temp_read =
    {0,
     {"",},
     {0,},
     {0,}, "", 0}, *qc_record;
    FILE *qcPtr;
    char filename[80];

    (void) sprintf(filename, QC_FILEDIR "%d/qcfile", quad_num);
    if ((qcPtr = /*x */ fopen(filename, "r" /*, FALSE */ )) == NULL) {
#ifdef QC_DEBUG
/*      cprintf("Debug: Read open 'r' failed on file: %s\n", filename); */
#endif
	return NULL;
    }
    fread(&temp_read, sizeof(qc_thingme), 1, qcPtr);
    fclose(qcPtr);

    qc_record = (qc_thingme *) xmalloc(sizeof(qc_thingme));
    *qc_record = temp_read;
    return qc_record;
}

/*----------------------------------------------------------------------------*/

/*
 * int qc_write_file writes a single quad unless a lock exists
 */

int
qc_write_file(qc_thingme * qc_record, int quad_no)
{
    int j;
    char filename[80];
    char lockedby[L_USERNAME + 1];
    FILE *qcPtr;
    qc_thingme temp_write =
    {0,
     {"",},
     {0,},
     {0,}, "", 0};

    (void) sprintf(filename, QC_FILEDIR "%d/qcfile", quad_no);

    if ((qcPtr = fopen(filename, "r+")) == NULL) {
	cprintf("\1f\1cAck!  Write open 'r+' failed on file: %s\n", filename);
	cprintf("\nWrite aborted..  Press a key.\1a");
	inkey();
	return 0;
	/* check for locks */
    } else if ((qc_lockout()) || (qc_quadlocked(quad_no))) {

	(void) sprintf(filename, QC_EDIT_LOCKDIR "%d/qc.lock", quad_no);

	if ((qc_lockout()) && (quad_no != 1)) {
	    if (strcmp(usersupp->username, qc_who_locked_it(QC_LOCKOUT, lockedby)) != 0) {
		cprintf("\n\1f\1gCan't write changes, edit lockout in effect by \1c%s.\1g",
			qc_who_locked_it(QC_LOCKOUT, lockedby));
		cprintf("\nWrite aborted..  Press a key.\1a");
		inkey();
		return -1;
	    }
	} else if (qc_quadlocked(quad_no)) {
	    (void) sprintf(filename, QC_EDIT_LOCKDIR "%d/qc.lock", quad_no);
	    if (strcmp(usersupp->username, qc_who_locked_it(filename, lockedby)) != 0) {
		cprintf("\n\1f\1cHmmm. can't write, a lock in effect in this quad. ");
		cprintf("\1gCheck the lock status.\nWrite aborted..  Press a key.\1a");
		inkey();
		return -1;
	    }
	}
    }
    /* okok, write it */
    temp_write.quad_number = qc_record->quad_number;
    for (j = 0; j < NO_OF_CATEGORIES; j++)
	strcpy(temp_write.category_name[j], qc_record->category_name[j]);
    for (j = 0; j < NO_OF_CATEGORIES; j++)
	temp_write.content_quotient[j] = qc_record->content_quotient[j];
    for (j = 0; j < ARGH_QUAD_FLAGS; j++)
	temp_write.flags[j] = qc_record->flags[j];
    strcpy(temp_write.last_mod_by, qc_record->last_mod_by);
    temp_write.last_mod_on = qc_record->last_mod_on;
    fwrite(&temp_write, sizeof(qc_thingme), 1, qcPtr);
    fclose(qcPtr);
    return 1;
}

/*----------------------------------------------------------------------------*/
/* void qc_display:  does what it sounds like.
 * values of how:
 * 0  user display
 * 1  quad edit menu display
 * 2  categories edit
 */
void
qc_display(qc_thingme * temp_display, int how)
{
    int i, slen;		/* slen for columnizing output */
    room_t scratch;

    if (!how)
	cprintf("\n\n");
    else if (how == 1) {
	scratch = readquad(curr_rm);
	cprintf("\n\n\1a\1f\1gQuad `quotient' info for quad \1y%d\1w> \1r%s\1g.\1a\n\n", temp_display->quad_number, scratch.name);
    } else if (how == 2)
	cprintf("\n\n\1a\1f\1gEdit Categories.\1a\n\n");

    for (i = 0; i < NO_OF_CATEGORIES / 2; i++) {
	cprintf("\1a\1f\1r%2d\1w>  [%s\1f%d\1w] \1g%s\1a", i + 1,
	     ((temp_display->content_quotient[i] == CONTENT_SCALE) ? "\1y" :
	((temp_display->content_quotient[i] > (CONTENT_SCALE / 2)) ? "\1r" :
	 ((temp_display->content_quotient[i] != 0) ? "\1p" : "\1b"))),
	 temp_display->content_quotient[i], temp_display->category_name[i]);
	slen = (strlen(temp_display->category_name[i]) + 10);
	while (slen < 40) {
	    cprintf(" ");
	    slen++;
	}
	cprintf("\1f\1r%2d\1w>  [%s\1f%d\1w] \1g%s\n\1a",
		i + 1 + NO_OF_CATEGORIES / 2,
		((temp_display->content_quotient[i + NO_OF_CATEGORIES / 2]
		  == CONTENT_SCALE) ? "\1y" :
		 ((temp_display->content_quotient[i + NO_OF_CATEGORIES / 2]
		   > (CONTENT_SCALE / 2)) ? "\1r" :
		  ((temp_display->content_quotient[i + NO_OF_CATEGORIES / 2]
		    != 0) ? "\1p" : "\1b"))),
		temp_display->content_quotient[i + NO_OF_CATEGORIES / 2],
		temp_display->category_name[i + NO_OF_CATEGORIES / 2]);
    }

    if (how == 1) {
	cprintf("\n\1a\1f\1gFlags\1w:\1a");
	for (i = 0; i < ARGH_QUAD_FLAGS; i++) {
	    if (i == 0)
		cprintf("\n\1a\1f\1r%d\1w> \1gNewbie Read\1w: [\1y%d\1w] <%s\1w>\1a",
			(i + 1), temp_display->flags[0],
			((temp_display->flags[0] < 0)
			 ? "\1rAuto-Zap" : "\1gNewbie posts left unread"));
	    /*     else  
	     * cprintf("\n\1a\1f\1r%d\1w> [\1y%d\1w] <\1gunused\1w>\1a", 
	     * (i + 1), temp_display->flags[i]); */
	}
	cprintf("\n\1a\1f\1gLast modified %s by \1c%s\1a\n", printdate(temp_display->last_mod_on, -1),
		temp_display->last_mod_by);
    }
    return;
}

/*----------------------------------------------------------------------------*/

int
qc_get_category_slot(void)
{
    int cat_slot;

    for (;;) {
	cprintf("\1f\1gWhich slot? \1w<\1renter\1w>\1g quits\1w :\1a ");
	if ((cat_slot = qc_get_pos_int('\0', (NO_OF_CATEGORIES < 10) ? 1 :
				       2)) == -1)
	    return -1;
	cat_slot--;
	if ((cat_slot >= 0) && (cat_slot < NO_OF_CATEGORIES))
	    break;
	cprintf("\1f\1gInvalid input.\1a");
    }
    return cat_slot;
}

/*----------------------------------------------------------------------------*/
/* int qc_set_value : returns 1 if change was made, -1 if no change was made,
 * and 0 on error.
 */

int
qc_set_value(int cat_slot)
{
    char instring[4], cat_name[L_CONTENT_CATEGORY + 1];
    int j, old_quotient = 0, new_quotient;
    qc_thingme *quotient_set = NULL, *temp_write = NULL;

    if (cat_slot < 0) {
	quotient_set = qc_read_file(curr_rm);
	if (quotient_set == NULL)
	    return 0;
	if ((cat_slot = qc_get_category_slot()) == -1) {
	    xfree(quotient_set);
	    return 0;
	} else if (strcmp(quotient_set->category_name[cat_slot], "<unused>") == 0) {
	    cprintf("\n\1f\1gI'm sorry, Dave, I can't set values for an unused slot.\nPress a key. \1a");
	    xfree(quotient_set);
	    inkey();
	    return -1;
	}
	strcpy(cat_name, quotient_set->category_name[cat_slot]);
	old_quotient = quotient_set->content_quotient[cat_slot];
	xfree(quotient_set);
	quotient_set = NULL;
    }				/* if (cat_slot < 0) */
    for (;;) {
	cprintf("\1f\1gNew value \1w[\1y%d\1w]\1g ", old_quotient);
	cprintf("or \1w<\1y%d\1w>\1g flags category as major category: ",
		CONTENT_SCALE);
	getline(instring, 3, 1);
	if (strlen(instring) == 0)
	    return -1;

	new_quotient = atoi(instring);
	if ((new_quotient >= 0) && (new_quotient <= CONTENT_SCALE)) {
	    if (new_quotient != CONTENT_SCALE)
		break;
	    else {
		/* check to see if there's already a major category */
		quotient_set = qc_read_file(curr_rm);
		for (j = 0; j < NO_OF_CATEGORIES; j++)
		    if (quotient_set->content_quotient[j] == CONTENT_SCALE)
			break;
		xfree(quotient_set);
		if (j == NO_OF_CATEGORIES)
		    break;
		cprintf("\n\1g\1fThere is already a major category at slot \1w%d\1g.", j + 1);
		cprintf("\nYou have to lower the quotient in that slot by at least one");
		cprintf("\nbefore you make this slot the major category.\nPress a key\1a");
		inkey();
		return -1;
	    }
	}
	cprintf("\1f\1gInvalid input.\1a");
    }				/* for (;;) */

    /* write it */
    temp_write = qc_read_file(curr_rm);
    temp_write->content_quotient[cat_slot] = new_quotient;
    strcpy(temp_write->last_mod_by, usersupp->username);
    temp_write->last_mod_on = time(NULL);
    if (qc_write_file(temp_write, curr_rm) != 1) {
	xfree(temp_write);
	return 0;
    }
    xfree(temp_write);
    return 1;
}

/*----------------------------------------------------------------------------*/

int
qc_set_flags(int which_flag)
{
    qc_thingme *temp_qc = NULL;	/* they're not even flags, they're ints */
    int new_value;		/* so sue me.  -russ */
    char instring[5];

    if ((temp_qc = qc_read_file(curr_rm)) == NULL)
	return 0;

    cprintf("\n\1f\1gSetting value for flag \1y%d\1w: ", (which_flag + 1));
    switch (which_flag) {
	case 0:
	    cprintf("\1gNew account read stuff flag.\n\n");
	    cprintf("Auto-Zap automagically zaps 'crap' quads that for some reason we don't\n");
	    cprintf("want new users to have see. (they can always \1w<\1rj\1w>\1gump if they want..)");
	    cprintf("\nEnable Auto-Zap? \1w(\1ry\1w/\1rN\1w)\1a ");
	    if (yesno_default(0)) {
		new_value = -1;
		break;
	    }
	    if (temp_qc->flags[0] < 0)
		new_value = NEWBIE_NUM_UNREAD;
	    else
		new_value = temp_qc->flags[0];
	    cprintf("\n\1f\1gOk, not setting Auto-Zap.  I need a (reasonable) positive integer number\n");
	    cprintf("of posts to leave unread in that quad for a new user.  Setting this\n");
	    cprintf("flag to \1y0\1g will mark this entire quad as read for a new user.\n\n");
	    cprintf("New Value from \1y0 \1gto \1y100\1w: [\1y%d\1w] \1a", new_value);
	    getline(instring, 3, 1);
	    if (strlen(instring) != 0)
		new_value = (atoi(instring));
	    if ((new_value < 0) || (new_value > 100)) {
		cprintf("\1f\1gI did say reasonable, no?   Setting to \1w[\1y%d\1w]\1g.\1a\n",
			NEWBIE_NUM_UNREAD);
		new_value = NEWBIE_NUM_UNREAD;
	    }
	    if (new_value == temp_qc->flags[0]) {
		xfree(temp_qc);
		return 0;
	    }
	    break;
	    /* insert more cases here when flags are added..  whatever */
	default:
	    xfree(temp_qc);
	    return 0;
    }				/* switch */

    temp_qc->flags[which_flag] = new_value;
    strcpy(temp_qc->last_mod_by, usersupp->username);
    temp_qc->last_mod_on = time(NULL);
    if (qc_write_file(temp_qc, curr_rm) != 1) {
	xfree(temp_qc);
	return 0;
    }
    xfree(temp_qc);
    return 1;
}

/*----------------------------------------------------------------------------*/

void
qc_lock_menu(void)
{
    register char lockcmd = '\0', togglecmd = '\0';

    cprintf("\1a\1f\1r\n\nLock Menu:  \1w<\1rs\1w>\1gtatus \1w<\1rt\1w>\1goggle-locks \1w<\1rr\1w>\1geset-lockfiles \1w<\1r?\1w> <\1rq\1w>\1guit\n\1rLock command: \1a");
    lockcmd = get_single_quiet("strq? ");
    switch (lockcmd) {
	case 's':
	    cprintf("\1a\1f\1gStatus.\1a");
	    qc_show_lock_status();
	    qc_lock_menu();
	    break;
	case '?':
	    more(QC_LOCK_DOC, 0);
	    qc_lock_menu();
	    break;
	case 't':{
		cprintf("\1a\1f\1gToggle.\1a\n");
		cprintf("\n\1a\1f\1gqua\1w<\1rd\1w> <\1rl\1w>\1gockout \1w<\1rq\1w>\1guit \1a");
		togglecmd = get_single_quiet("dlq ");
		switch (togglecmd) {
		    case 'd':
			cprintf("\1a\1f\1cEdit lock/unlock current quad.\1a\n");
			qc_toggle_edit_lock();
			break;
		    case 'l':	/* toggle lockout */
			cprintf("\1a\1f\1cEdit lock/unlock entire qc.\n\1a");
			qc_toggle_lockout();
			break;
		    case ' ':
		    case 'q':
			cprintf("\n\1a\1f\1gQuitting..\1a");
			break;
		    default:
			cprintf("\n\1f\1ceek! an erroreth hath occureth.\1a\n");
			break;
		}		/* inner switch (togglecmd) */
		qc_lock_menu();
		break;
	    }			/* case 't' */
	case 'r':		/* prolly should be emp only as well.. */
	    cprintf("\1f\1rReset Lockfiles\nClear ALL locks? \1w(\1ry\1w/\1rN\1w)\1a ");
	    if (yesno_default(0))
		qc_clear_all_locks();
	    qc_lock_menu();
	    break;
	case ' ':
	case 'q':
	    cprintf("\n\1f\1rQuitting..\1a");
	    return;
	default:
	    cprintf("\nHrrm.. that shouldn't have happened.\n");
	    return;
    }				/* switch */
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_toggle_edit_lock(void)
{
    int islocked;
    char filename[80];
    char whodunit[L_USERNAME + 1];

    islocked = qc_quadlocked(curr_rm);
    sprintf(filename, QC_EDIT_LOCKDIR "%d/qc.lock", curr_rm);
    cprintf("\n\1a\1f\1gQuad \1y%d\1g is currently: %s\1c%s\1g.\1a",
	    curr_rm,
	    ((islocked) ? "\1rlocked\1g by " : "\1gunlocked"),
	    ((islocked) ? (qc_who_locked_it(filename, whodunit)) : ""));
    cprintf("\1a\1f\1g  Change? \1w(\1ry\1w/\1rN\1w) \1a");
    if (yesno_default(0))
	qc_change_lock_status(curr_rm, ((islocked) ? 0 : 1));
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_toggle_lockout(void)
{
    int i, islocked;
    char whodunit[L_USERNAME + 1];

    islocked = qc_lockout();
    cprintf("\n\1a\1f\1gDatabase is currently %s\1c%s\1g.\1a",
	    ((islocked) ? "\1rlocked\1g by " : "\1gunlocked"),
	    ((islocked) ? (qc_who_locked_it(QC_LOCKOUT, whodunit)) : ""));
    cprintf("\1a\1f\1g  Change? \1w(\1ry\1w/\1rN\1w) \1a");
    if (!yesno_default(0))
	return;
    if (islocked) {
	if (!qc_clear_lockout())
	    cprintf("\1a\1f\1c\neek. lockout failed to release.  Database still disabled.\1a\n");
	else
	    cprintf("\n\1a\1f\1gFull database lockout released.  Database enabled.\1a");
    } else {
	for (i = 0; i < MAXQUADS; i++)
	    if (qc_quadlocked(i))
		break;
	if (i < MAXQUADS)
	    cprintf("\n\1a\1f\1rNOTE:  \1gSome quad(s) are still edit locked. Check Status..\n\1a");
	if (qc_set_lockout())
	    cprintf("\1a\1f\1r\nFull Database lock set, database and user functions disabled.\1a");
    }
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_clear_all_locks(void)
{
    int i;

    for (i = 0; i < MAXQUADS; i++)
	if (qc_quadlocked(i))
	    qc_clear_quadlock(i);
    if (qc_lockout())
	qc_clear_lockout();
    cprintf("\n\1a\1f\1gOk..  \1rEVERYTHING\1g was unlocked.. check your zipper.\1a\n");
}


/*----------------------------------------------------------------------------*/
/* int qc_change_lock_status(int quad_number, int lock_it, int change_mode)
 * changes the lock status and returns 1 on success, 0 on fail.
 * quad_number:
 * - the quad number if (change_mode == 1)
 * - meaningless if (change_mode != 1)
 * values of lock_it:
 * - 0 if to unlock, 1 if to lock.
 */

int
qc_change_lock_status(int quad_number, int lock_it)
{
    char lockfilename[80];
    char who_locketh_it[L_USERNAME + 1];

    if (lock_it) {		/* if we're locking */

	if ((!qc_quadlocked(quad_number))
	    && (!qc_lockout())) {	/* if nothing specific is locked. */

	    cprintf("\1a\1f\1gSetting quad edit-lock on quad \1y%d\1g.\1a", quad_number);
	    return qc_set_quadlock(quad_number);
	} else if (qc_lockout()) {
	    cprintf("\n\1a\1f\1bPermission denied\1w:  \1gFull edit lock in effect.\1a\n");
	    return 0;
	} else if (qc_quadlocked(quad_number)) {
	    (void) sprintf(lockfilename, QC_EDIT_LOCKDIR "%d/qc.lock", quad_number);
	    cprintf("\n\1a\1f\1gQuad \1y%d\1g is already edit-locked by \1c%s\1g.\1a\n",
	       quad_number, qc_who_locked_it(lockfilename, who_locketh_it));
	    return 0;
	} else {
	    cprintf("\1a\1f\1gSetting edit-lock on quad \1y%d\1g.\1a", quad_number);
	    return qc_set_quadlock(quad_number);
	}
    } else if (!(lock_it)) {	/* if we're unlocking */
	if (!qc_quadlocked(quad_number))
	    cprintf("\n\1a\1f\1cHrm..  quad \1y%d\1c was already unlocked.. \1a\n", quad_number);
	else {
	    if (qc_clear_quadlock(quad_number)) {
		cprintf("\n\1a\1f\1gEdit lock on quad \1y%d\1g released.\1a\n", quad_number);
		return 1;
	    }
	}
	cprintf("\1a\1f\1r\nMad Man Murad obviously should have used some mental floss before\n");
	cprintf("writing this code.  Press any key to send him a porcupine..\1a\n");
	inkey();
    }				/* else (lock_it) */
    return 0;
}


/*----------------------------------------------------------------------------*/

void
qc_show_lock_status(void)
{
    int i = 0, j = 0;
    char filename[80];
    char who_dood_it[L_USERNAME + 1];

    cprintf("\n\n\1a\1f\1wEdit Lock Status: \n\n\1a");
    if (qc_lockout()) {
	cprintf("\1a\1f\1gALL editing has been \1rlocked \1gout by \1c%s\1g.\1a\n\n", qc_who_locked_it(QC_LOCKOUT, who_dood_it));
	i = 1;
    }
    cprintf("\1a\1f\1gLocal quad-edit lock in effect in quads:\1a\n");
    for (i = 0; i < MAXQUADS; i++)
	if (qc_quadlocked(i)) {
	    (void) sprintf(filename, QC_EDIT_LOCKDIR "%d/qc.lock", i);
	    cprintf("\1a\1f\1y  %d\1w>\1g by \1c%s\1g.\n\1a", i, qc_who_locked_it(filename, who_dood_it));
	    j++;
	}
    if (!(j))
	cprintf("\1a\1f\1rNo\1g quads are locally edit-locked.\1a");

    return;
}

/*----------------------------------------------------------------------------*/

int
qc_set_lockout(void)
{
    FILE *lockoutPtr;

    if ((lockoutPtr = fopen(QC_LOCKOUT, "w")) == NULL) {
	cprintf("\1f\1c*argh*  failed to create lockout file.\n");
	cprintf("file: %s.\1a\n", QC_LOCKOUT);
	return 0;
    }
    fprintf(lockoutPtr, "%s", usersupp->username);
    fclose(lockoutPtr);
#ifdef QC_DEBUG
    cprintf("\nFull qc edit lockout set by %s.\n", usersupp->username);
#endif
    return 1;
}

/*----------------------------------------------------------------------------*/
/* char *qc_who_locked_it(const char * filename, char * wholocked_it)
 * a classic whodunit..  HINT:  The Butler didn't do it.
 */

char *
qc_who_locked_it(const char *filename, char *wholocked_it)
{
    FILE *lockPtr;
    char temp_read[L_USERNAME + 1];

    if ((lockPtr = fopen(filename, "r")) == NULL)
	return NULL;
    fscanf(lockPtr, "%s", temp_read);
    strcpy(wholocked_it, temp_read);
    /* put multiple word user handles back together */
    while (!feof(lockPtr)) {
	strcat(wholocked_it, " ");
	fscanf(lockPtr, "%s", temp_read);
	strcat(wholocked_it, temp_read);
    }
    fclose(lockPtr);
    return wholocked_it;
}

/*----------------------------------------------------------------------------*/

int
qc_set_quadlock(int quad_no)
{
    FILE *quadlockPtr;
    char quadlockfile[80];

    (void) sprintf(quadlockfile, QC_EDIT_LOCKDIR "%d/qc.lock", quad_no);

    if ((quadlockPtr = fopen(quadlockfile, "w")) == NULL) {
	cprintf("\1f\1rcreate failed on file: %s\1a\n", quadlockfile);
	return 0;
    }
    fprintf(quadlockPtr, "%s", usersupp->username);
    fclose(quadlockPtr);
#ifdef QC_DEBUG
    cprintf("\nQuad lock set by %s.\n", usersupp->username);
#endif
    return 1;
}
/*----------------------------------------------------------------------------*/

int
qc_clear_quadlock(int quad_no)
{
    char quadlockfile[80];
    if (!qc_quadlocked(quad_no))
	return 0;
    (void) sprintf(quadlockfile, QC_EDIT_LOCKDIR "%d/qc.lock", quad_no);
    unlink(quadlockfile);
    return 1;
}

/*----------------------------------------------------------------------------*/

int
qc_clear_lockout(void)
{
    if (!qc_lockout())
	return 0;
    unlink(QC_LOCKOUT);
    return 1;
}

/*----------------------------------------------------------------------------*/

int
qc_lockout(void)
{				/* the equivalent of fexists, FEXISTS, whatever.. */
    FILE *yadda_yadda;

    if ((yadda_yadda = fopen(QC_LOCKOUT, "r")) != NULL) {
	fclose(yadda_yadda);
	return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

int
qc_quadlocked(int quad_no)
{
    FILE *yadda_yadda;
    char quadlockfile[80];

    (void) sprintf(quadlockfile, QC_EDIT_LOCKDIR "%d/qc.lock", quad_no);
    if ((yadda_yadda = fopen(quadlockfile, "r")) != NULL) {
	fclose(yadda_yadda);
	return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

/* int * qc_evaluate_quads(const qc_thingme * user_input)
 * returns a pointer to an malloc'd int array. Make sure to free() from calling
 * function!
 * In the return array, each int in the array is the number of posts in the
 * corresponding quad to be left unread or -1 if that quad is to be zapped, -2
 * if the autozap flag is set for newbies or -5 if the quad was skipped
 * intentionally for some reason.  (can't read, write, QR_ZAP QR_INUSE, etc.)
 * -10 if it couldn't find a qcfile for that quad.
 * 
 * one could conceivably adjust the selection algorithm by adjusting the
 * either the else block that sets initial weights by incrementing or
 * decrementing the strong/weak/zero_set values, or the scalar weight of
 * weak accepts and rejects in the reduction block (the for (;;) block), but
 * as of now, 2 weaks = 1 strong, and the initial weights are fairly
 * linear.  have fun with that.. (:
 */

int *
qc_evaluate_quads(const qc_thingme * user_input, int newuser, const user_t * user)
{
    int kazam[MAXQUADS], user_q[NO_OF_CATEGORIES], read_q[NO_OF_CATEGORIES];
    int *kabong;
    int i, j, temp_int;		/* hrm, there seems to be a global temp already */
    int s_accept, s_reject, zero_set, w_accept, w_reject;

    qc_thingme *temp_read;
    room_t scratch;

    for (i = 0; i < MAXQUADS; i++) {
	kazam[i] = -5;
	if ((i < 10) || (i == 148))
	    continue;		/* Admin quads make no sense here anyways.. */
	/* skip other user useless stuff */
	scratch = readquad(i);
	if ((!i_may_read_forum(scratch, i)) ||
	    (!(scratch.flags & QR_INUSE)))
	    continue;
	if ((scratch.flags & QR_PRIVATE) && (usersupp->priv < PRIV_SYSOP))
	    continue;		/* let sysops play with invite quads if they want (: */

	temp_read = NULL;

	if ((temp_read = qc_read_file(i)) == NULL) {	/* no file for this quad? */
	    kazam[i] = ((newuser) ? NEWBIE_NUM_UNREAD : -10);
	    continue;
	}
	if ((temp_read->flags[0] == -1) && (newuser)) {		/* AutoZap flag */
	    kazam[i] = -2;
	    xfree(temp_read);
	    continue;
	}
	if ((i <= 20) && (newuser)) {	/* don't evaluate info quads, do read */
	    kazam[i] = temp_read->flags[0];	/* no. of posts to leave unread */
	    continue;
	}
	s_accept = s_reject = zero_set = w_accept = w_reject = 0;
	/* read in 'quotients' */
	for (j = 0; j < NO_OF_CATEGORIES; j++) {
	    user_q[j] = read_q[j] = 0;
	    if (strcmp(temp_read->category_name[j], "<unused>") == 0)
		continue;
	    user_q[j] = user_input->content_quotient[j];
	    read_q[j] = temp_read->content_quotient[j];
	    if ((!user_q[j]) && (read_q[j] == CONTENT_SCALE)) {
		/* user zero of major category */
		kazam[i] = -1;	/* user doesn't want quad based on this category */
		break;
	    } else if ((user_q[j] == CONTENT_SCALE) &&
		       (read_q[j] == CONTENT_SCALE)) {
		/* user max of major category user wants quad based on category, give twice */
		kazam[i] = temp_read->flags[0] * 2;
		break;		/* the number of unreads.. or whatever */
	    } else if (!read_q[j])	/* skip a category evaluated zero */
		continue;
	    /* ok, now the less obvious stuff */
	    else {		/* set initial weights..  yadda, yadda. */
		temp_int = ((user_q[j] + read_q[j]) * (user_q[j] + read_q[j])) -
		    ((user_q[j] - read_q[j]) * (user_q[j] - read_q[j]));
		temp_int = (int) (sqrt(temp_int) * 100);

		if (temp_int >= CONTENT_SCALE * 2 * 70)		/* >= 70%  etc. */
		    s_accept += 2;	/*  err on side of caution */
		else if (temp_int > CONTENT_SCALE * 2 * 53)
		    w_accept++;
		else if (temp_int > CONTENT_SCALE * 2 * 43)
		    zero_set++;	/* wider lower limit on the zero set, err on */
		else if (temp_int > CONTENT_SCALE * 2 * 25)	/* the side of caution */
		    w_reject++;
		else if (temp_int <= CONTENT_SCALE * 2 * 25) {
		    if (user_q[j] == 0)		/* special case: user zero */
			if (read_q[j] > (CONTENT_SCALE * 2 / 3))
			    s_reject++;
			else
			    w_reject++;
		    else
			s_reject++;
		} else
		    cprintf("\n\1a\1c Argh.. eval code is buggered.\1a ");
	    }
	}			/* j */
	if (kazam[i] == -5) {
	    for (;;) {		/* reduce this rubbish */
		if (w_accept >= 2) {
		    s_accept++;
		    w_accept -= 2;
		    continue;
		}		/* weak accept is now 0 or 1 */
		if (w_reject >= 2) {
		    s_reject++;
		    w_reject -= 2;
		    continue;
		}		/* weak reject is now 0 or 1 */
		if (s_accept && s_reject) {
		    s_accept--;
		    s_reject--;
		    zero_set++;
		    continue;
		}		/* zero at either strong accept or strong reject or both */
		if (w_accept && w_reject) {
		    w_accept--;
		    w_reject--;
		    zero_set++;
		    continue;
		}		/* zero at either weak accept or weak reject or both */
		break;
	    }			/* for (;;) */
	    if (s_accept && !s_reject) {
		kazam[i] = temp_read->flags[0];
	    } else if (!s_accept && s_reject) {
		kazam[i] = -1;
	    } else if (!(s_accept || s_reject)) {	/* all zero */
		if (!(w_accept || w_reject))
		    kazam[i] = ((zero_set) ? 0 : -1);
		else
		    kazam[i] = ((w_accept) ? temp_read->flags[0] : -1);
            }
	}			/* if (kazam[i] == -5) */
	xfree(temp_read);

    }				/* i */
    /* malloc the return array */
    kabong = (int *) xmalloc(sizeof(int) * MAXQUADS);
    if (kabong == NULL)		/* argh */
	return NULL;
    for (i = 0; i < MAXQUADS; i++)
	kabong[i] = kazam[i];
    return kabong;
}

/*----------------------------------------------------------------------------*/

   /* eof */
