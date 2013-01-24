/* $Id$ */

/*----------------------------------------------------------------------------*/
/* qc.c 
 * routines for storing/manipulation/evaluation of a quadrant's category content
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
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
#include "ext.h"		/* for curr_rm and such */

#include "qc.h"

#include "menu.h"		/* for dynamic menus */
#include "rooms.h"		/* for readquad, leave_n_unread_posts, etc. */
#include "routines2.h"		/* for more, etc. */
#include "enter_message.h"	/* for self-mail of results */
#include "input.h"		/* for xgetline */

static void qc_admin_menu(void);
static void qc_categories_menu(void);

static int _sql_qc_add_category(const char *cat_name);
static int _sql_qc_name2id(const char *name, unsigned int *id);
static int _sql_qc_id2name(char *name, unsigned int id);
static int _sql_qc_rename_category(const char *cat_name, unsigned int cat_id);
static int _sql_qc_delete_category(unsigned int cat_id);
static int _sql_qc_fetch_quad_qc(unsigned int quad, qc_record * qc);
static int _sql_qc_fetch_all_quads_qc(qc_record ***);
static int _sql_qc_update_f_quotient(unsigned int, unsigned int, unsigned int);
static int _sql_qc_update_u_quotient(unsigned int, unsigned int, unsigned int);
static int _sql_qc_count_categories(int *);
static int _sql_qc_fiddle_with_flags(unsigned int, unsigned int);
static int _sql_qc_set_newbie_unread(unsigned int, unsigned int);
static int _sql_qc_get_user_id_list(int ***);
static int _sql_qc_fetch_user_qc(unsigned int u_id, qc_record * qc);
static int _sql_qc_user_on_file(unsigned int u_id, int *rows);
static int _sql_qc_add_user(const int id);
static int _sql_qc_delete_user(const int id);
static int _sql_qc_get_f_max_and_slot(unsigned int *, unsigned int, unsigned int *);

#ifdef QC_TIME_FUNCTIONS
/* these are unused as of yet..  */
static int _sql_qc_get_u_last_mod(unsigned int, unsigned long *);
static int _sql_qc_get_f_last_mod(unsigned long *);
static int _sql_qc_get_t_last_mod(unsigned long *);
#endif

static int qc_set_values(int);
static void qc_set_flags(void);

static int qc_evaluate(unsigned int, int ***);
static int qc_mail_results(int **);
static void qc_show_results(int **);

static void qc_add_category(void);
static void qc_delete_category(void);
static void qc_rename_category(void);

static int qc_get_category_slot(void);

/* user function lockout stuff */
static void qc_lock_menu(void);
static void qc_show_lock_status(void);
static int qc_set_lockout(void);
static char *qc_who_locked_it(char *);
static void qc_clear_lockout(void);
static int qc_islocked(void);


/*----------------------------------------------------------------------------*/
void
new_qc(void)
{
    qc_admin_menu();
}

int
_sql_qc_count_categories(int *num)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret;

    ret = mono_sql_query(&res, 
	"SELECT COUNT(cat_id) FROM %s WHERE f_id=0", QC_TABLE);
    if (ret == 0) {
	row = mysql_fetch_row(res);
	sscanf(row[0], "%u", num);
	mono_sql_u_free_result(res);
    }
    return ret;
}

int
_sql_qc_add_user(int id)
{
    MYSQL_RES *res;
    int ret, count;
    unsigned int i;

    ret = _sql_qc_count_categories(&count);
    if (ret == -1)
	return -1;

    for (i = 0; i < count; i++) {
	ret = mono_sql_query(&res,
			     "INSERT INTO %s (id, cat_id) VALUES (%u, %u)",
			     USER_QC_TABLE, id, i);
	if (ret == -1)
	    return -1;
	else
	    mono_sql_u_free_result(res);
    }
    return 0;
}


int
_sql_qc_delete_user(int id)
{
    MYSQL_RES *res;

    return mono_sql_query(&res,
			  "DELETE FROM %s where id=%u", USER_QC_TABLE, id);
}
int
_sql_qc_add_category(const char *name)
{
    MYSQL_RES *res;
    int ret, count, **u_id_list;
    unsigned int i, new_id;

    ret = _sql_qc_count_categories(&new_id);
    if (ret == -1)
	return -1;

    for (i = 0; i < MAXQUADS; i++) {
	ret = mono_sql_query(&res,
	    "INSERT INTO %s (cat_name, cat_id, f_id) VALUES ('%s', %u, %u)",
			     QC_TABLE, name, new_id, i);
	if (ret == -1) {
	    printf("%s%s",
		   "\n\nSomething went horribly wrong adding this category.",
		   "\nDelete it immediately!\n\n");
	    return -1;
	} else
	    mono_sql_u_free_result(res);
    }

    ret = _sql_qc_get_user_id_list(&u_id_list);
    if (ret != -1) {
	for (i = 0, count = ret; i < count; i++) {
	    ret = mono_sql_query(&res,
			      "INSERT INTO %s (cat_id, id) VALUES (%u, %u)",
				 USER_QC_TABLE, new_id, *(u_id_list[i]));
	    if (ret == -1) {
		printf("%s%s",
		       "\n\nSomething went horribly wrong adding this",
		       " category.\nDelete it immediately!\n\n");
		break;
	    } else
		mono_sql_u_free_result(res);
	}

	for (i = 0; i < count; i++)
	    xfree(u_id_list[i]);
	xfree(u_id_list);
    }
    return ret;
}


/* returns -1 if no such category exists, or error */
int
_sql_qc_name2id(const char *name, unsigned int *id)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret;

    ret = mono_sql_query(&res,
		     "SELECT cat_id from %s WHERE cat_name='%s' AND f_id=0",
			 QC_TABLE, name);

    if (ret == -1)
	return -1;

    if (mysql_num_rows(res) != 1) {
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", id);

    mono_sql_u_free_result(res);
    return 0;

}

int
_sql_qc_id2name(char *name, unsigned int id)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret;

    ret = mono_sql_query(&res,
		       "SELECT cat_name from %s WHERE cat_id=%u AND f_id=0",
			 QC_TABLE, id);

    if (ret == -1)
	return -1;

    if (mysql_num_rows(res) != 1) {
	return -1;
    }
    row = mysql_fetch_row(res);
    strcpy(name, row[0]);

    mono_sql_u_free_result(res);
    return 0;

}

int
_sql_qc_rename_category(const char *name, unsigned int id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
			 "UPDATE %s SET cat_name='%s' WHERE cat_id=%u",
			 QC_TABLE, name, id);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_update_u_quotient(unsigned int id, unsigned int new_val, unsigned int user_id)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
		      "UPDATE %s SET cat_quot=%u WHERE cat_id=%u AND id=%u",
			 USER_QC_TABLE, new_val, id, user_id);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_update_f_quotient(unsigned int id, unsigned int new_val, unsigned int forum)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
		    "UPDATE %s SET cat_quot=%u WHERE cat_id=%u AND f_id=%u",
			 QC_TABLE, new_val, id, forum);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_fiddle_with_flags(unsigned int newflags, unsigned int forum)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
			 "UPDATE %s SET flags=%u WHERE f_id=%u",
			 QC_TABLE, newflags, forum);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_set_newbie_unread(unsigned int new_unread, unsigned int forum)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
			 "UPDATE %s SET newbie_r=%u WHERE f_id=%u",
			 QC_TABLE, new_unread, forum);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_delete_category(unsigned int id)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret;
    unsigned int highest_id;

    ret = mono_sql_query(&res, "SELECT MAX(cat_id) FROM %s", QC_TABLE);
    if (ret != -1) {
	row = mysql_fetch_row(res);
	sscanf(row[0], "%u", &highest_id);
	mono_sql_u_free_result(res);
    } else
	return -1;

    ret = mono_sql_query(&res, "DELETE FROM %s WHERE cat_id=%u",
			 QC_TABLE, id);
    mono_sql_u_free_result(res);
    ret = mono_sql_query(&res, "DELETE FROM %s WHERE cat_id=%u",
			 USER_QC_TABLE, id);
    mono_sql_u_free_result(res);

    if (id < highest_id) {
	ret = mono_sql_query(&res, "UPDATE %s SET cat_id=%u WHERE cat_id=%u",
			     QC_TABLE, id, highest_id);
	mono_sql_u_free_result(res);
	ret = mono_sql_query(&res, "UPDATE %s SET cat_id=%u WHERE cat_id=%u",
			     USER_QC_TABLE, id, highest_id);
	mono_sql_u_free_result(res);
    }
    return ret;
}

int
_sql_qc_get_user_id_list(int ***id_list)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret, i, alloc_count = 0, count, rows, *u_id = NULL, **u_id_list = NULL;

    ret = mono_sql_query(&res,
		  "SELECT COUNT(id) FROM %s WHERE cat_id=0", USER_QC_TABLE);
    if (ret == 0) {
	row = mysql_fetch_row(res);
	sscanf(row[0], "%u", &alloc_count);
	mono_sql_u_free_result(res);
    }
    if (ret == -1 || alloc_count < 1) {
	printf("\nSome sort of sql error.\n");
	return -1;
    }
    ret = mono_sql_query(&res,
			 "SELECT id FROM %s WHERE cat_id=0 ORDER BY id",
			 USER_QC_TABLE);
    if (ret == -1) {
	printf("\nSome sort of sql error.\n");
	return -1;
    }
    rows = mysql_num_rows(res);

    u_id_list = (int **) xmalloc(sizeof(int *) * alloc_count);
    for (i = 0; i < alloc_count; i++) {
	u_id = (int *) xmalloc(sizeof(int));
	memset(u_id, 0, sizeof(int));
	u_id_list[i] = u_id;
    }

    for (i = 0, count = 0; i < rows; i++, count++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	else if (sscanf(row[0], "%u", u_id_list[i]) == -1)
	    break;
    }
    mono_sql_u_free_result(res);

    if (count != alloc_count) {	/* error */
	cprintf("\n\1f\1rSQL error\n\1a");
	for (i = 0; i < alloc_count; i++)
	    xfree(u_id_list[i]);
	xfree(u_id_list);
	return -1;
    }
    *id_list = u_id_list;
    return count;
}


int
_sql_qc_fetch_all_quads_qc(qc_record *** qc_list)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret, i, alloc_count = 0, count, rows, quad = -1;
    unsigned int tmp, cat_count = 0;
    qc_record *rec = NULL, **rec_list = NULL;

    ret = mono_sql_query(&res,
		     "SELECT COUNT(f_id) FROM %s WHERE cat_id=0", QC_TABLE);
    if (ret == 0) {
	row = mysql_fetch_row(res);
	sscanf(row[0], "%u", &alloc_count);
	mono_sql_u_free_result(res);
    }
    /* alloc_count *should* be MAXQUADS.. but it's not an error. */
    if (ret == -1 || alloc_count < 1) {
	printf("\nSome sort of sql error at fetch_all.\n");
	return -1;
    }
    ret = mono_sql_query(&res, "SELECT f_id, newbie_r, flags, cat_id, cat_name, cat_quot FROM %s ORDER BY f_id", QC_TABLE);
    if (ret == -1) {
	printf("\nSome sort of sql error at fetch_all.\n");
	return -1;
    }
    rows = mysql_num_rows(res);

    rec_list = (qc_record **) xmalloc(sizeof(qc_record *) * alloc_count);
    for (i = 0; i < alloc_count; i++) {
	rec = (qc_record *) xmalloc(sizeof(qc_record));
	memset(rec, 0, sizeof(qc_record));
	rec_list[i] = rec;
    }

    for (i = 0, count = 0; i < rows; i++) {

	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	else if (sscanf(row[0], "%u", &tmp) == -1)
	    break;

	if (tmp != quad) {
	    count++;
	    cat_count = 0;
	    rec_list[tmp]->id = quad = tmp;
	    sscanf(row[1], "%d", &(rec_list[tmp]->newbie_r));
	    sscanf(row[2], "%u", &(rec_list[tmp]->flags));
	}
	sscanf(row[3], "%u", &(rec_list[tmp]->cat_id[cat_count]));
	strcpy(rec_list[tmp]->cat_name[cat_count], row[4]);
	sscanf(row[5], "%u", &(rec_list[tmp]->cat_quot[cat_count]));
	cat_count++;
    }
    mono_sql_u_free_result(res);

    if (count != alloc_count) {	/* error */
	cprintf("\n\1f\1rSQL error at _sql_qc_fetch_all()\n\1a");
	for (i = 0; i < alloc_count; i++)
	    xfree(rec_list[i]);
	xfree(rec_list);
	return -1;
    }
    *qc_list = rec_list;
    return count;
}

int
_sql_qc_fetch_quad_qc(unsigned int quad, qc_record * qc)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret, i, rows, tempint;
    unsigned int tempuint;

    ret = mono_sql_query(&res,
		  "SELECT cat_id, cat_name, cat_quot, newbie_r, flags FROM "
			 QC_TABLE " WHERE f_id=%u ORDER BY cat_id", quad);
    if (ret == -1) {
	printf("\nSome sort of sql error at fetch.\n");
	return -1;
    }
    rows = mysql_num_rows(res);

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", &tempuint) == -1)
	    continue;
	qc->cat_id[i] = tempuint;
	strcpy(qc->cat_name[i], row[1]);
	sscanf(row[2], "%u", &tempuint);
	qc->cat_quot[i] = tempuint;
	if (!i) {
	    sscanf(row[3], "%d", &tempint);
	    qc->newbie_r = tempint;
	    sscanf(row[4], "%u", &tempuint);
	    qc->flags = tempuint;
	}
    }

    mono_sql_u_free_result(res);
    return 0;

}

int
_sql_qc_get_f_max_and_slot(unsigned int *c_id, unsigned int forum, unsigned int *max)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int rows, i;

    if (mono_sql_query(&res, "SELECT MAX(cat_quot) FROM %s WHERE f_id=%u",
		       QC_TABLE, forum) == -1)
	return -1;

    rows = mysql_num_rows(res);
    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", max) == -1)
	    continue;
    }
    mono_sql_u_free_result(res);

    if (*max == 0)
	return 0;

    if (mono_sql_query(&res,
		       "SELECT cat_id FROM %s WHERE f_id=%u AND cat_quot=%d",
		       QC_TABLE, forum, *max) == -1)
	return -1;

    rows = mysql_num_rows(res);
    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;

	if (sscanf(row[0], "%u", c_id) == -1)
	    continue;
    }
    mono_sql_u_free_result(res);

    return 0;
}

int
_sql_qc_user_on_file(unsigned int u_id, int *rows)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
			 "SELECT id FROM % s WHERE cat_id = 0 AND id = %u ",
			 USER_QC_TABLE, u_id);
    *rows = mysql_num_rows(res);
    mono_sql_u_free_result(res);
    return ret;
}

int
_sql_qc_zero_forum_categories(unsigned int forum)
{
    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res,
			 "UPDATE %s SET cat_quot=0 WHERE f_id = %u ",
			 QC_TABLE, forum);
    mono_sql_u_free_result(res);
    return ret;
}


int
_sql_qc_fetch_user_qc(unsigned int u_id, qc_record * qc)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int ret, i, rows;

    ret = mono_sql_query(&res,
			 "SELECT DISTINCT uqc.id, uqc.cat_id, uqc.cat_quot, \
		fqc.cat_name FROM %s, %s WHERE fqc.cat_id=uqc.cat_id \
		AND uqc.id=%u GROUP BY fqc.cat_id",
			 USER_QC_TABLE, QC_TABLE, u_id);

    if (ret == -1) 
	return -1;
    
    rows = mysql_num_rows(res);

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	if (sscanf(row[0], "%u", &(qc->id)) == -1)
	    break;
	sscanf(row[1], "%u", &(qc->cat_id[i]));
	sscanf(row[2], "%u", &(qc->cat_quot[i]));
	strcpy(qc->cat_name[i], row[3]);
    }
    mono_sql_u_free_result(res);

    if (i != rows)
	return -1;

    return 0;
}

#ifdef QC_TIME_FUNCTIONS

int
_sql_qc_get_u_last_mod(unsigned int id, unsigned long *timeval)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (mono_sql_query(&res,
		       "SELECT MAX(mod_date) from %s WHERE id=%u",
		       USER_QC_TABLE, id) == -1)
	return -1;
    row = mysql_fetch_row(res);
    sscanf(row[0], "%lu", timeval);
    mono_sql_u_free_result(res);
    return 0;
}

int
_sql_qc_get_f_last_mod(unsigned long *timeval)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (mono_sql_query(&res,
		       "SELECT MAX(mod_time) from %s WHERE t_id=0",
		       QC_TABLE) == -1)
	return -1;
    row = mysql_fetch_row(res);
    sscanf(row[0], "%lu", timeval);
    mono_sql_u_free_result(res);
    return 0;
}

int
_sql_qc_get_t_last_mod(unsigned long *timeval)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (mono_sql_query(&res,
		       "SELECT MAX(mod_time) from %s WHERE t_id>0",
		       QC_TABLE) == -1)
	return -1;
    row = mysql_fetch_row(res);
    sscanf(row[0], "%lu", timeval);
    mono_sql_u_free_result(res);
    return 0;
}

#endif

/*----------------------------------------------------------------------------*/

void
qc_add_category(void)
{
    int i, ret;
    char category[L_CONTENT_CATEGORY];

    cprintf("\n\1f\1gNew category name or \1w<\1rret\1w>\1g exits: \1a");
    xgetline(category, L_CONTENT_CATEGORY - 1, 1);
    if (strlen(category) == 0)
	return;

    ret = _sql_qc_count_categories(&i);
    if (ret == -1) {
	cprintf("\n\1f\1r_sql_qc_count_categories() returned error.\1a");
	return;
    } else if (i > 0) {
	i = -1;
	ret = _sql_qc_name2id(category, &i);
	if (i != -1) {
	    cprintf("\n\1f\1gThat name has already been used. \1a");
	    return;
	}
    }
    cprintf("\n\n\1f\1gNew category name: \1y%s%s",
	    category, "\1g  Save? \1w(\1rY\1w/\1rn\1w) \1a");
    if (yesno_default(YES) == NO)
	return;

    ret = _sql_qc_add_category(category);
    if (ret == 0)
	cprintf("\n\1f\1g\nDone. \1a");
    else
	cprintf("\n\1f\1r\n_sql_qc_add_category() returned error. \1a");

    cprintf("\1a\1f\1gPress a key. \1a");
    inkey();

    return;
}

/*----------------------------------------------------------------------------*/

void
qc_delete_category(void)
{
    int cat_slot = 0, ret;
    char category[L_CONTENT_CATEGORY + 1];

    if ((cat_slot = qc_get_category_slot()) == -1)
	return;

    ret = _sql_qc_id2name(category, cat_slot);
    if (ret == -1) {
	cprintf("\n\1f\1r_sql_qc_id2name() returned error.\1a\n");
	return;
    }
    cprintf("\n\1f\1rWARNING\1w: This will delete category  \1r%s%s%s",
	    category, "\1w in ALL rooms.\n\n",
	    "\1gAre you sure? \1w(\1ry\1w/\1rN\1w) \1a");

    if ((yesno_default(NO)) == YES) {
	ret = _sql_qc_delete_category(cat_slot);
	cprintf("\n\1f%s  Press a key..\1a",
	      (ret == 0) ? "\1gDeleted." : 
		"\1r_sql_qc_delete_category() returned error.\1g");
	inkey();
    }
    return;
}

/*----------------------------------------------------------------------------*/

void
qc_rename_category(void)
{
    int cat_slot = 0, ret, i = -1;
    char category[L_CONTENT_CATEGORY], old_category[L_CONTENT_CATEGORY];

    if ((cat_slot = qc_get_category_slot()) == -1)
	return;
    if ((_sql_qc_id2name(old_category, cat_slot)) == -1) {
	cprintf("\n\1f\1r_sql_qc_id2name() returned error.\1a\n");
	return;
    }
    cprintf("\n\1f\1gRename category \1y%s\1g, to: \1a\n\n",
	    old_category);
    xgetline(category, L_CONTENT_CATEGORY - 1, 1);
    if (strlen(category) == 0)
	return;

    _sql_qc_name2id(category, &i);
    if (i != -1 && i != cat_slot) {
	cprintf("\n\1f\1gThat name is already used.\1a ");
	return;
    }
    cprintf("%s%s%s%s%s",
	    "\n\1f\1g   Old category name\1w:\1y ", old_category,
	    "\n\1g   New category name\1w: \1y", category,
	    "\n\n\1gSave this, Dave?  \1w(\1rY\1w/\1rn\1w) \1a");

    if ((yesno_default(YES)) == NO)
	return;

    ret = _sql_qc_rename_category(category, cat_slot);

    cprintf("\n\1f%s  Press a key..\1a",
     	(ret == 0) ? "\1gCategory renamed." : 
	"\1r_sql_qc_rename_category() returned error.\1g");
    inkey();
    return;
}


/*----------------------------------------------------------------------------*/

void
qc_edit_room(void)
{
    register char editcmd = '\0';

    for (;;) {
	cprintf("%s%s",
		"\n\1f\1w<\1rf\1w>\1glags \1w<\1rs\1w>\1get values ",
		"\1w<\1r?\1w> <\1rq\1w>\1guit\n\1rCommand\1w: \1a");
	editcmd = get_single_quiet("fsq? \r\n");
	switch (editcmd) {
	    case 'f':
		cprintf("\1a\1f\1rFlags\1w: \1a\n");
		qc_set_flags();
		break;
	    case '?':
		more(QC_F_EDIT_DOC, 0);
		break;
	    case 's':
		cprintf("\1a\1f\1rSet qc value\1w: \1a");
		qc_set_values(FORUM_EDIT);
		break;
	    case ' ':
	    case 'q':
	    default:
		return;
	}
    }
}


int
qc_set_values(int mode)
{
    MENU_DECLARE;
    char *tempstr, tempstr1[5], tempstr2[10];
    qc_record qc_rec;
    int ret = -1, i, count;
    unsigned int id;

    void _set_qc_f_value(unsigned int, long, void *);
    void _set_qc_u_value(unsigned int, long, void *);

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       (mode & FORUM_EDIT) ?
	       "\n\n\1f\1w[\1gQuadrant Rating Menu\1w]\n\n" :
	       "\n\n\1f\1w[\1gReading Interests Ratings Menu\1w]\n\n");

	memset(&qc_rec, 0, sizeof(qc_record));

	if (mode & FORUM_EDIT) {
	    id = curr_rm;
	    ret = _sql_qc_fetch_quad_qc(id, &qc_rec);
	} else {
	    id = usersupp->usernum;
	    ret = _sql_qc_fetch_user_qc(id, &qc_rec);
	}

	if (ret == -1) {
	    cprintf("\n\1f\1r%s\n\1a", (mode & FORUM_EDIT) ?
		"_sql_qc_fetch_quad_qc() returned error." :
		"_sql_qc_fetch_user_qc() returned error.");
	    return -1;
	}
	if (_sql_qc_count_categories(&count) == -1) {
	    cprintf("\n\1f\1r_sql_qc_count_categories() returned error.\n\1a");
	    return -1;
	}
	for (i = 0; i < count; i++) {
	    sprintf(tempstr1, "%u", qc_rec.cat_id[i]);
	    sprintf(tempstr2, " %s%u", (qc_rec.cat_quot[i]) ?
		    ((qc_rec.cat_quot[i] == CONTENT_SCALE) ? "\1y" : "\1r")
		    : "\1b", qc_rec.cat_quot[i]);

	    tempstr = (char *) xmalloc(strlen(qc_rec.cat_name[i]) + 1);
	    sprintf(tempstr, "%s", qc_rec.cat_name[i]);

	    if (mode & FORUM_EDIT)
		MENU_ADDITEM(_set_qc_f_value,
			     qc_rec.cat_id[i], qc_rec.cat_quot[i], 
			     (char *) tempstr,
			     "tiv", qc_rec.cat_name[i], tempstr1, tempstr2);
	    else
		MENU_ADDITEM(_set_qc_u_value,
			     qc_rec.cat_id[i], qc_rec.cat_quot[i], 
			     (char *) tempstr,
			     "tiv", qc_rec.cat_name[i], tempstr1, tempstr2);
	}

	MENU_ADDITEM(do_nothing, 0, 0, NULL, "tiv", "-----------", "", "\1g--");

        if (mode & FORUM_EDIT) {
	    tempstr = (char *) xmalloc(strlen(QC_F_EDIT_DOC) + 1);
	    strcpy(tempstr, QC_F_EDIT_DOC);
	} else {
	    tempstr = (char *) xmalloc(strlen(QC_U_EDIT_DOC) + 1);
	    strcpy(tempstr, QC_U_EDIT_DOC);
        }
	    
	MENU_ADDITEM(more_wrapper, 1, 1, tempstr,
		     "ti", "Help", "?");

	the_menu_format.auto_columnize = 1;
	the_menu_format.no_boolean_values = 1;
	the_menu_format.value_color = 'y';
	strcpy(the_menu_format.menu_prompt,
	       "\1f\1gCategory \1w<\1rnumber\1w>\1g to edit");

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
    return 0;
}


void
_set_qc_f_value(unsigned int category_id, long val, void *category_name)
{
    int ret, new_val;
    unsigned int max, max_at_id, forum;

    cprintf("\n\1f\1gCategory:\1y %s\n", (char *) category_name);
    cprintf("\1gEnter content score on a scale of 0 to %d: \1y[%u] \1c",
	    CONTENT_SCALE, val);

    new_val = qc_get_pos_int('\0', 1);
    if (new_val > CONTENT_SCALE)
	new_val = CONTENT_SCALE;
    if (new_val == val || new_val < 0)
	return;

    forum = curr_rm;

    if (new_val == CONTENT_SCALE) {
	if (_sql_qc_get_f_max_and_slot(&max_at_id, forum, &max) == -1)
	    cprintf("\n\1f\1r_sql_qc_get_f_max_and_slot() returned error.");

	if (max == CONTENT_SCALE && max_at_id != category_id) {
	    cprintf("\n\1f\1rCategory %d%s%s%s%s", max_at_id,
		    " is already rated as this forum's major category.\n\n",
		    "\1gReduce it's rating by at least 1 before making\1y ",
		    (char *) category_name,
		  "\1g the\nmajor category for this forum.\nPress a key..");
	    inkey();
	    return;
	}
    }
    ret = _sql_qc_update_f_quotient(category_id, new_val, forum);
    if (ret == -1)
	cprintf("\n\1f\1r_sql_qc_update_f_quotient() returned error.\n\1a");
}


void
_set_qc_u_value(unsigned int category_id, long val, void *category_name)
{
    int ret;
    unsigned int new_val;

    cprintf("\n\1f\1gRate your interest in \1y%s \1gon a scale of 0 to %d: \1y[%u] \1c",
	    (char *) category_name, CONTENT_SCALE, val);

    new_val = qc_get_pos_int('\0', 1);
    if (new_val > CONTENT_SCALE)
	new_val = CONTENT_SCALE;
    if (new_val == val || new_val < 0)
	return;

    ret = _sql_qc_update_u_quotient(category_id, new_val, usersupp->usernum);
    if (ret == -1)
	cprintf("\n\1f\1r_sql_qc_update_u_quotient() returned error.\n\1a");
}
/*----------------------------------------------------------------------------*/

void
qc_set_flags(void)
{
    MENU_DECLARE;
    char tempstr[40];
    qc_record qc_rec;
    int ret = -1;
    unsigned int id;

    void _toggle_qc_flag(unsigned int, long, void *);
    void _set_qc_newbie_unread(unsigned int, long, void *);

    for (;;) {
	MENU_INIT;
	strcpy(the_menu_format.menu_title,
	       "\n\1f\1w[\1gQC Flags -n- Stuff Menu\1w]\n\n");

	memset(&qc_rec, 0, sizeof(qc_record));

	id = curr_rm;
	ret = _sql_qc_fetch_quad_qc(id, &qc_rec);

	if (ret == -1) {
	    cprintf("\n\1f\1r_sql_qc_fetch_quad_qc() returned error.\n\1a");
	    return;
	}
	sprintf(tempstr, "Newbie unread posts \1w[\1y%u\1w]", qc_rec.newbie_r);
	MENU_ADDITEM(_set_qc_newbie_unread,
		     qc_rec.newbie_r, 0, NULL,
		     "t", tempstr);

	MENU_ADDITEM(_toggle_qc_flag,
		     qc_rec.flags, QC_AUTOZAP, NULL,
	  "tv", "Newbie Auto-Zap", (qc_rec.flags & QC_AUTOZAP) ? "1" : "0");

	the_menu_format.auto_columnize = 1;
	the_menu_format.gen_1_idx = 1;

	MENU_PROCESS_INTERNALS;
	MENU_DISPLAY(2);
	if (!MENU_EXEC_COMMAND)
	    break;
	MENU_DESTROY;
    }
    MENU_DESTROY;
    return;
}

void
_toggle_qc_flag(unsigned int curr_flags, long mask, void *bar)
{
    int ret;
    unsigned int newflags;

    newflags = curr_flags;
    newflags ^= (unsigned int) mask;

    ret = _sql_qc_fiddle_with_flags(newflags, curr_rm);
    if (ret == -1)
	cprintf("\n\1f\1r_sql_qc_fiddle_with_flags() returned error.");

}

void
_set_qc_newbie_unread(unsigned int curr_unread, long foo, void *bar)
{
    int ret, new_unread;

    cprintf("\n\1f\1g%s%s%s%s",
	    "I need a (reasonable) positive integer number of posts",
	    " to leave unread in\nthis quad for a new user.  Setting",
	    " this value to \1y0\1g marks this entire\nquad as read ",
	    "for a new user. (which is sometimes a good thing)\n\n");

    cprintf("New Value from \1y0 \1gto \1y100\1w: [\1y%d\1w] \1a", curr_unread);

    if ((new_unread = qc_get_pos_int('\0', 3)) == -1)
	return;

    if ((new_unread < 0) || (new_unread > 100)) {
	cprintf("\1f\1g\nI did say reasonable, no?   Setting to \1w[\1y%d\1w]\1g.\1a\n",
		NEWBIE_NUM_UNREAD);
	new_unread = NEWBIE_NUM_UNREAD;
	sleep(1);
    }
    ret = _sql_qc_set_newbie_unread(new_unread, curr_rm);
    if (ret == -1)
	cprintf("\n\1f\1r_sql_qc_set_newbie_unread() returned error.");
}

/*----------------------------------------------------------------------------*/
void
qc_admin_menu(void)
{
    register char menucmd = '\0';

    for (;;) {
	cprintf("%s%s%s%s%s%s",
		"\n\n\1f\1w<\1re\1w>\1gdit", " \1w<\1rl\1w>\1gock menu",
		" \1w<\1rc\1w>\1gategories", " l\1w<\1ri\1w>\1gsts",
		" \1w<\1rq\1w>\1guit", "\nQC main menu command: \1a");
	menucmd = get_single_quiet("elcquiw ");

	switch (menucmd) {
	    case 'e':
		cprintf("\1a\1f\1rEdit\1a\n");
		qc_edit_room();
		break;
	    case 'c':
		qc_categories_menu();
		break;
	    case 'u':
		cprintf("\1a\1f\1rUser Menu\1a\n");
		qc_user_menu(0);
		break;
	    case 'l':
		qc_lock_menu();
		break;

	    case 'i':
		cprintf("%s%s",
			"\1f\1r\nOn todo list..  prolly should list quads",
		  "\nand topics with all zero quotients at the minimum\1a\n");
		break;
	    case ' ':
	    case 'q':
		cprintf("\1f\1rQuitting..\1a");
		return;
	    default:
		cprintf("\n\1f\1cHave you hugged *your* squid today?\1a\n");
		return;
	}			/* switch */
    }
}


/*----------------------------------------------------------------------------*/

void
qc_categories_menu(void)
{
    qc_record categories;
    char cmd = '\0';
    int i, ret;

    memset(&categories, 0, sizeof(qc_record));

    ret = _sql_qc_fetch_quad_qc(1, &categories);

    if (ret == 0) {
	cprintf("\n\1f\1gCategories:\n");
	for (i = 0; i < NO_OF_CATEGORIES; i++)
	    if (strlen(categories.cat_name[i]))
		cprintf("\n\1y%u\1w>\1g %s",
			categories.cat_id[i], categories.cat_name[i]);
    }
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
	default:
	    cprintf("\1f\1r  Quitting..\1a");
	    return;
    }
    qc_categories_menu();
}


/*----------------------------------------------------------------------------*/

int
qc_user_menu(int newbie)
{
    int i, ret, has_file = 0, **eval, alloc_count = -1;
    unsigned int flags = 0, delete_file = 0;
    char foo = '\0';
    room_t scratch;

    if (qc_islocked()) {
	cprintf("\n\1f\1rThis function temporarily disabled.\1a\n");
	return -1;
    }
    ret = _sql_qc_user_on_file(usersupp->usernum, &has_file);
    if (!has_file && ret != 1)
	ret = _sql_qc_add_user(usersupp->usernum);

    if (newbie)
	more_wrapper(1, 1, QC_NEWUSER_HELP);

    if (ret == -1)
	cprintf("\nSome sort of SQL error at qc_user_menu");
    else {
	qc_set_values(USER_EDIT);

	if (!has_file) {
	    cprintf("\n\1f\1gSave data option:\n%s%s%s%s%s%s%s%s%s",
		    "Interest data, if saved, is kept with the strictest",
		    " confidentiality.\nIt will not be used, sold,",
		    " transferred or disclosed to ANYBODY.\n",
		    "The option of saving this data is presented as a",
		    " convenience, since\nsome functions of the BBS will ",
		    "notify you if new subjects arise\nthat might suit your",
		    " interests, if your interest data is saved to file.",
		    "\n\nDo you want your evaluation saved for future use?",
		    "\1w (\1ry\1w/\1rn\1w) \1c");
	    if (yesno() == NO)
		delete_file = 1;
	}
	if (newbie)
	    flags |= QC_NEWUSER;

	alloc_count = qc_evaluate(flags, &eval);
    }

    if (delete_file)
	_sql_qc_delete_user(usersupp->usernum);

    if (alloc_count == -1) {
	cprintf("\n\n\1f\1r%s%s%s",
	    "Argh!  Something really bad happened with the selection code.",
	 "\nThe programmer obviously should have slept before he wrote it.",
		"\nPress any key to send him a porcupine.. \1a");
	inkey();
	return -1;
    }
    for (i = 0; i < MAXQUADS; i++) {
	if (*eval[i] == -5)	/* skip these, regardless */
	    continue;
	if (newbie) {
	    if ((*eval[i] == -1) || (*eval[i] == -2)) {	/* stuff to zap */
		scratch = readquad(i);
		usersupp->forget[i] = scratch.generation;
		usersupp->generation[i] = -1;
	    } else
		leave_n_unread_posts(i, *eval[i]);
        }
    }

    if (newbie) {
	writeuser(usersupp, 0);
	cprintf("\n\1f\1g");
	more(QC_NEWUSER_DONE, 0);
    } else {

	cprintf("\n\n\1f\1gDo you want to: \1w<\1rv\1w>\1giew or \1w<\1rm\1w>\1gail yourself the results? ");
	foo = '\0';
	foo = get_single_quiet("vmVM");
	switch (foo) {
	    case 'm':
	    case 'M':
		cprintf("\1cMailing..");
		if (qc_mail_results(eval) == -1)
		    cprintf("\n\nProblems mailing results, please report this bug. \1a");
		else
		    cprintf("\1g    Done.\n\n");
		break;
	    case 'v':
	    case 'V':
		cprintf("\1cView\n\n");
		qc_show_results(eval);
		cprintf("\n\n");
		break;
	}

    }
    for (i = 0; i < alloc_count; i++)
	xfree(eval[i]);
    xfree(eval);

    more(QC_USERDOC_DISCLAIMER, 0);
    return 0;
}

/*----------------------------------------------------------------------------*/
void
qc_show_results(int **eval)
{
    int i, items_left = 0;
    room_t scratch;
    char *string, line[200];

    string = (char *) xmalloc(sizeof(char) * 2);
    strcpy(string, "");

    for (i = 0; i < MAXQUADS; i++) {
	if (*eval[i] == -1) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] != scratch.generation) {
		if (!items_left)
		    string = m_strcat(string, "\n\1f\1gSelection algorithm suggests zapping the following quads according to\nyour interests..\1a\n\n");
		sprintf(line, "\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		string = m_strcat(string, line);
		items_left++;
	    }
	}
    }

    items_left = 0;
    for (i = 0; i < MAXQUADS; i++) {
	if (*eval[i] > 0) {
	    scratch = readquad(i);
	    if (usersupp->forget[i] == scratch.generation) {
		if (!items_left)
		    string = m_strcat(string, "\n\1f\1gSelection algorithm suggests unzapping the following quads according to\nyour interests..\1a\n\n");
		sprintf(line, "\1f\1y%d\1w> \1g%s\n", i, scratch.name);
		string = m_strcat(string, line);
		items_left++;
	    }
	}
    }

    more_string(string);
    xfree(string);

    return;
}

/*----------------------------------------------------------------------------*/

int
qc_mail_results(int **eval)
{
    mail_myself_quadcont_results(eval);
    return 1;
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
	if ((cat_slot >= 0) && (cat_slot < NO_OF_CATEGORIES))
	    break;
	cprintf("\1f\1gInvalid input.\1a");
    }
    return cat_slot;
}

/*----------------------------------------------------------------------------*/


void
qc_lock_menu(void)
{
    char cmd = '\0';

    for (;;) {
	qc_show_lock_status();

	cprintf("\1f\n\1w<\1rt\1w>\1goggle lockout \1w<\1r?\1w>%s",
		" <\1rq\1w>\1guit\1r : \1a");
	cmd = get_single_quiet("tq?\r\n ");
	switch (cmd) {
	    case 't':
		if (qc_islocked())
		    qc_clear_lockout();
		else
		    qc_set_lockout();
		break;
	    case '?':
		more(QC_LOCK_DOC, 0);
		break;
	    case ' ':
	    case 'q':
	    case '\r':
	    case '\n':
		cprintf("\n\1f\1rQuitting..\1a");
		return;
	}
    }
}

int
qc_islocked(void)
{
    return fexists(QC_LOCKOUT);
}

/*----------------------------------------------------------------------------*/

void
qc_show_lock_status(void)
{
    char name[L_USERNAME + 1];

    cprintf("\n\n\1a\1f\1r-- Lock Status:  \1g");
    if (qc_islocked())
	cprintf("QC user functions are \1rlocked\1g by \1c%s\1g.\1a\n",
		qc_who_locked_it(name));
    else
	cprintf("No QC lock in place.\1a\n");
}

/*----------------------------------------------------------------------------*/

int
qc_set_lockout(void)
{
    FILE *lockoutPtr;

    if ((lockoutPtr = fopen(QC_LOCKOUT, "w")) == NULL) {
	cprintf("\1f\1cFailed to create lockout file.\n");
	return 0;
    }
    fprintf(lockoutPtr, "%s", usersupp->username);
    fclose(lockoutPtr);
    return 1;
}

/*----------------------------------------------------------------------------*/

char *
qc_who_locked_it(char *wholocked_it)
{
    FILE *fp;
    char name[L_USERNAME + 1];
    int i;

    if ((fp = fopen(QC_LOCKOUT, "r")) == NULL)
	strcpy(wholocked_it, "error.");
    else {
	for (i = 0; !feof(fp); i++)
	    name[i] = fgetc(fp);
	fclose(fp);
	name[i - 1] = '\0';
	strcpy(wholocked_it, name);
    }
    return wholocked_it;
}

void
qc_clear_lockout(void)
{
    if (fexists(QC_LOCKOUT))
	unlink(QC_LOCKOUT);
}

/*----------------------------------------------------------------------------*/

/* int qc_evaluate(const unsigned int eval_flags, int *** eval_list)
 * returns 0 on success.  note that eval list is malloc'd and needs to be
 * free'd from calling function, both it's index, and it's pointers.
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
 * linear.  have fun with that.. (:  i may split these blocks off into
 * separate functions at some point.
 */

int
qc_evaluate(unsigned int eval_flags, int ***eval_list)
{
    int user_q[NO_OF_CATEGORIES], read_q[NO_OF_CATEGORIES], **kabong = NULL,
       *kazam;
    int i, j, temp_int, list_count, index, alloc_count = 0;
    int s_accept, s_reject, zero_set, w_accept, w_reject;

    qc_record **list, user_rec;

    list_count = _sql_qc_fetch_all_quads_qc(&list);
    if (list_count < 1)
	return -1;

    if (_sql_qc_fetch_user_qc(usersupp->usernum, &user_rec) == -1) {
	for (i = 0; i < list_count; i++)
	    xfree(list[i]);
	xfree(list);
	return -1;
    }
/* put user quotients into local array for coder's sanity */

    for (j = 0; j < NO_OF_CATEGORIES; j++)
	if (strlen(user_rec.cat_name[j]) == 0)
	    user_q[j] = 0;
	else
	    user_q[j] = user_rec.cat_quot[j];

/* evaluate all quads:  i */

    for (i = 0; i < MAXQUADS; i++) {
	kabong = (int **) realloc(kabong, sizeof(int *) * (++alloc_count));
	kazam = (int *) xmalloc(sizeof(int));
	kabong[i] = kazam;
	*kazam = -5;

	if (i < 6 || i == 148 || !i_may_read_forum(i))
	    continue;

/* find quad id in list */

	for (index = 0; index < MAXQUADS; index++)
	    if (list[index]->id == i)
		break;
	if (index >= MAXQUADS)	/* no record in list */
	    continue;

/* skip quads that have all 0 content ratings */

	for (j = 0; j < NO_OF_CATEGORIES; j++)
	    if (list[index]->cat_quot[j] > 0)
		break;
	if (j >= NO_OF_CATEGORIES)
	    continue;

/* newbie stuff */

	if ((list[index]->flags & QC_AUTOZAP) && (eval_flags & QC_NEWUSER)) {
	    *kazam = -2;
	    continue;
	} else if ((i <= 20) && (eval_flags & QC_NEWUSER)) {
	    *kazam = list[index]->newbie_r;
	    continue;
	}
	s_accept = s_reject = zero_set = w_accept = w_reject = 0;

/* read quad's 'quotients' into local array for coder's sanity */

	for (j = 0; j < NO_OF_CATEGORIES; j++) {
	    if (strlen(list[index]->cat_name[j]) == 0) {
		read_q[j] = 0;
		continue;
	    } else
		read_q[j] = list[index]->cat_quot[j];

/* user zero of major category */

	    if ((!user_q[j]) && (read_q[j] == CONTENT_SCALE)) {
		*kazam = -1;
		break;

/* user max of major category */

	    } else if (user_q[j] == CONTENT_SCALE &&
		       read_q[j] == CONTENT_SCALE) {
		*kazam = list[index]->newbie_r * 2;
		break;

/* skip a category evaluated zero */

	    } else if (!read_q[j])
		continue;

/* ok, now the less obvious stuff */

	    else {

/* set initial weights */

		temp_int = ((user_q[j] + read_q[j]) * (user_q[j] + read_q[j])) -
		    ((user_q[j] - read_q[j]) * (user_q[j] - read_q[j]));
		temp_int = (int) (sqrt(temp_int) * 100);

/* fuzzy code for fuzzy sets */

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

	if (*kazam == -5) {
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

	    if (s_accept && !s_reject)
		*kazam = list[index]->newbie_r;
	    else if (!s_accept && s_reject)
		*kazam = -1;
	    else if (!(s_accept || s_reject)) {	/* all zero */
		if (!(w_accept || w_reject))
		    *kazam = ((zero_set) ? 0 : -1);
		else
		    *kazam = ((w_accept) ? list[index]->newbie_r : -1);
            }

	}			/* if (*kazam == -5) */
    }				/* i */

    for (i = 0; i < list_count; i++)
	xfree(list[i]);
    xfree(list);

    *eval_list = kabong;
    return alloc_count;
}

    /* eof */
