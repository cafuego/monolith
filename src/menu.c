#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

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

#include "menu.h"
#include "routines2.h"

/* static prototypes */

static M_LNptr linked_list_insertNode(M_LNptr, M_item_t *);
static void init_menu_item(M_item_t *);
static int linked_list_itemtotal(M_LN_t *);
static M_LN_t *column_sort_linked_list(M_LN_t *, int);
static int cnt_cc(char *);
static void set_subcolumn_emax(M_display_t *, M_LN_t *);
static int opt_autocolumn(M_display_t *);
static void build_valid_input(M_LN_t *, M_display_t *);
static M_LN_t *menu_generate_indices(M_LN_t *, M_display_t *);
static void menu_command_prompt(M_display_t *);

/* code */

M_LNptr
linked_list_insertNode(M_LNptr startPtr, M_item_t * menu_item)
{
    M_LNptr newPtr, prevPtr, currPtr;

    if ((newPtr = (M_LNptr) malloc(sizeof(M_LN_t))) != NULL) {
	newPtr->mi_ptr = menu_item;
	prevPtr = newPtr->nextPtr = NULL;
	currPtr = startPtr;

	while (currPtr != NULL) {
	    prevPtr = currPtr;
	    currPtr = currPtr->nextPtr;
	}

	if (prevPtr == NULL) {
	    newPtr->nextPtr = startPtr;
	    startPtr = newPtr;
	} else {
	    prevPtr->nextPtr = newPtr;
	    newPtr->nextPtr = currPtr;
	}
    } else
	cprintf("\nmalloc() fail at linked_list_insertNode()\n");

    return startPtr;
}


void
test_dlist_code(void)
{
//  #define _DEBUG_MENU_CODE
#ifdef _DEBUG_MENU_CODE
    MENU_DECLARE;

    MENU_INIT;
//    the_menu_format.gen_1_idx = 1;
the_menu_format.auto_columnize = 1;
    strcpy(the_menu_format.menu_title, "\1f\1yBing's Bongs:\n");
    MENU_ADDITEM(do_bing, 256, 777, (char *) "smoke porcupine",
	"tiv", "tastes like chicken(do_bing)", "\"", "1");
    MENU_PROCESS_INTERNALS;
    MENU_DISPLAY(1);
    MENU_EXEC_COMMAND;
    MENU_DESTROY;
#endif
    return;
}

void 
process_menu_internals(M_LN_t * the_menu, M_display_t * m_fmt)
{
    if (m_fmt->gen_0_idx || m_fmt->gen_1_idx)
	the_menu = menu_generate_indices(the_menu, m_fmt);

    set_subcolumn_emax(m_fmt, the_menu);
    strcat(m_fmt->menu_valid_input, m_fmt->quit_command);
    build_valid_input(the_menu, m_fmt);
}

void 
menu_command_prompt(M_display_t * m_fmt)
{
    char prompt[200], *p;

    strcpy(prompt, "\n\1f\1g ");
    if (strlen(m_fmt->menu_prompt))
	strcat(prompt, m_fmt->menu_prompt);

    if (!strlen(m_fmt->quit_command))
	strcat(prompt, "\1w:\1c ");
    else
	strcat(prompt, " \1w[\1gor");

    for (p = m_fmt->quit_command; *p; p++) {
	switch (*p) {
	    case ' ':
		strcat(prompt, " \1w<\1rspace\1w>");
		break;
	    case '\r':
	    case '\n':
		if (!strstr(prompt, "enter"))
		    strcat(prompt, " \1w<\1renter\1w>");
		break;
	    default:
		if (isalnum(p)) {
		    strcat(prompt, " \1w<\1r");
		    strcat(prompt, p);
		    strcat(prompt, "\1w>");
		}
		break;
	}
    }
    if (strlen(m_fmt->quit_command))
	strcat(prompt, "\1g exits\1w] :\1c ");
    cprintf("%s", prompt);

}
int 
menu_command(M_LN_t * the_menu, M_display_t * m_fmt)
{
    char command = '\0';
    char cmdstr[10];
    int bing;

    menu_command_prompt(m_fmt);
    strcpy(cmdstr, "");
    if (m_fmt->gen_0_idx || m_fmt->gen_1_idx) {
	command = get_single_quiet(m_fmt->menu_valid_input);
	bing = qc_get_pos_int(command, m_fmt->idx_emax);
	if (bing > -1)
	    snprintf(cmdstr, sizeof(cmdstr), "%d", bing);
    } else {
	if (m_fmt->idx_emax > 1) {
	    command = get_single_quiet(m_fmt->menu_valid_input);
	    if (isdigit(command)) {
	        bing = qc_get_pos_int(command, m_fmt->idx_emax);
	        if (bing > -1)
	            snprintf(cmdstr, sizeof(cmdstr), "%d", bing);
	    } else
		sprintf(cmdstr, "%c", command);
        } else {
	    command = get_single(m_fmt->menu_valid_input);
	    sprintf(cmdstr, "%c", command);
	}
    }

    fflush(stdout);

    if (strlen(cmdstr) && (!strpbrk(cmdstr, m_fmt->quit_command)))
	exec_menu_cmd(the_menu, cmdstr);
    else
	return 0;
    return 1;
}

void 
exec_menu_cmd(M_LN_t * the_menu, const char *cmd)
{
    M_LN_t *linkPtr;

    while (the_menu != NULL) {
	linkPtr = the_menu->nextPtr;
	if (strcmp(the_menu->mi_ptr->MI_idx, cmd) == 0)
	    break;
	the_menu = linkPtr;
    }
    if (the_menu != NULL)
	the_menu->mi_ptr->vfunc(the_menu->mi_ptr->ui_arg,
			  the_menu->mi_ptr->l_arg, the_menu->mi_ptr->obj_arg);
}
void
do_bing(const unsigned int the_int, const long the_long, void *the_string)
{
    cprintf("\nunsigned int:%ud  long: %ld  string: %s\n", the_int, the_long, the_string);
}

M_LN_t *
destroy_menu(M_LN_t * the_menu, M_display_t * m_fmt )
{
    M_LN_t *linkPtr;

    while (the_menu != NULL) {

#ifdef _DEBUG_MENU_CODE
	cprintf("\ndestroying link.");
#ifdef _DEBUG_MENU_TEXT_VPTR
	cprintf("\ndestroying text object: %s", (char *) the_menu->mi_ptr->obj_arg);
#endif
#endif
	linkPtr = the_menu->nextPtr;
	xfree(the_menu->mi_ptr->MI_txt);
	xfree(the_menu->mi_ptr->MI_idx);
	xfree(the_menu->mi_ptr->MI_ival);
	if (m_fmt->destroy_void_object) 
	    if (the_menu->mi_ptr->obj_arg != NULL) 
	        xfree(the_menu->mi_ptr->obj_arg);
	xfree(the_menu->mi_ptr);
	xfree(the_menu);
	the_menu = linkPtr;
#ifdef _DEBUG_MENU_CODE
	cprintf("\nlink destroyed.");
#endif
    }
    return the_menu;
}

void
build_valid_input(M_LN_t * the_menu, M_display_t * m_fmt)
{
    M_LN_t *linkPtr;

    while (the_menu != NULL) {
	linkPtr = the_menu->nextPtr;
	strcat(m_fmt->menu_valid_input, the_menu->mi_ptr->MI_idx);
	the_menu = linkPtr;
    }
}

void
set_menu_defaults(M_display_t * mformPtr)
{
    strcpy(mformPtr->menu_title, "");
    strcpy(mformPtr->menu_valid_input, "");
    strcpy(mformPtr->quit_command, "\n\r ");
    strcpy(mformPtr->menu_prompt, "Command");
    mformPtr->gen_0_idx = 0;
    mformPtr->gen_1_idx = 0;
    mformPtr->no_boolean_values = 0;
    mformPtr->auto_columnize = 0;

    mformPtr->text_color = 'g';
    mformPtr->bracket_color = 'w';
    mformPtr->value_color = 'r';
    mformPtr->index_color = 'r';

    mformPtr->flag_char = '*';
    mformPtr->idx_emax = 0;
    mformPtr->ival_emax = 0;
    mformPtr->txt_emax = 0;
   
    mformPtr->destroy_void_object = 1;
}

void 
do_nothing(const unsigned int foo, const long footoo, void *bar)
{
}

void
init_menu_item(M_item_t * m_item)
{
    m_item->MI_txt = m_item->MI_idx = m_item->MI_ival = NULL;
    m_item->vfunc = do_nothing;
    m_item->ui_arg = m_item->l_arg = 0;
    m_item->obj_arg = NULL;
}

M_LNptr
add_menu_item(M_LNptr the_menu, void (*vfunction) (const unsigned int, const long, void *), unsigned int unsigned_arg, long long_arg, void *void_arg, char *fmt,...)
{
    va_list ap;
    M_item_t *m_item;
    char temp_str[200];

    m_item = (M_item_t *) xmalloc(sizeof(M_item_t));
    if (m_item == NULL) {
	cprintf("\nmalloc() fail at add_menu_item()\n");
	return the_menu;
    }
    init_menu_item(m_item);
    m_item->vfunc = vfunction;

    if (fmt) {
	va_start(ap, fmt);

	for (; *fmt; ++fmt) {
	    switch (*fmt) {
		case 't':
		    strcpy(temp_str, va_arg(ap, char *));
		    m_item->MI_txt = (char *) xmalloc(strlen(temp_str) + 1);
		    strcpy(m_item->MI_txt, temp_str);
		    break;
		case 'i':
		    strcpy(temp_str, va_arg(ap, char *));
		    m_item->MI_idx = (char *) xmalloc(strlen(temp_str) + 1);
		    strcpy(m_item->MI_idx, temp_str);
		    break;
		case 'v':
		    strcpy(temp_str, va_arg(ap, char *));
		    m_item->MI_ival = (char *) xmalloc(strlen(temp_str) + 1);
		    strcpy(m_item->MI_ival, temp_str);
		    break;
	    }
	}
	va_end(ap);
    }
    m_item->ui_arg = unsigned_arg;
    m_item->l_arg = long_arg;
    m_item->obj_arg = void_arg;
    
    return the_menu = linked_list_insertNode(the_menu, m_item);
}

int
linked_list_itemtotal(M_LN_t * the_menu)
{
    M_LN_t *itemPtr;
    int i = 0;

    for (itemPtr = the_menu; itemPtr != NULL; itemPtr = itemPtr->nextPtr)
	i++;

    return i;
}

M_LN_t *
column_sort_linked_list(M_LN_t * src_menu, int columns)
{
    M_LN_t *dest_menu = NULL;
    M_LNptr colPtr[4];
    int row_total, i, j;

    row_total = linked_list_itemtotal(src_menu);
    while (row_total % columns) {	/* append empty items to end of list */
	src_menu = add_menu_item(src_menu, do_nothing, 0, 0, NULL, "tiv", "", "", "");
	row_total++;
    }

    row_total /= columns;
    if (row_total == 1 || columns < 1 || columns > 4)
	return src_menu;

    colPtr[0] = src_menu;
    for (i = 1; i < columns; i++) {
	colPtr[i] = colPtr[i - 1];
	for (j = 1; colPtr[i] != NULL; j++) {
	    colPtr[i] = colPtr[i]->nextPtr;
	    if (!(j % row_total))
		break;
	}
    }

    for (j = 0; j < row_total; j++)
	for (i = 0; i < columns; i++) {
	    dest_menu = linked_list_insertNode(dest_menu, colPtr[i]->mi_ptr);
	    colPtr[i] = colPtr[i]->nextPtr;
	}

    /* destroy the source linked list, NOT the data it pointed to, as dest now
     * points to that data, in proper [columnized] order */

    for (colPtr[0] = src_menu; colPtr[0] != NULL; colPtr[0] = colPtr[1]) {
	colPtr[1] = colPtr[0]->nextPtr;
	xfree(colPtr[0]);
    }

    return dest_menu;
}

/* sets _menu_display_format_shit struct internals..  
 * namely, the effective max display length for a subcolumn
 * (i.e.  strlen that disregards colorcodes, they don't take up 
 * any 'space' in the display.  */

void
set_subcolumn_emax(M_display_t * m_fmt, M_LN_t * the_menu)
{
    M_LNptr some_listnode;
    int ctr;
    char *p;

    some_listnode = the_menu;
    while (some_listnode != NULL) {
	if ((p = some_listnode->mi_ptr->MI_txt) != NULL) {
	    ctr = strlen(p) - cnt_cc(p);
	    if (ctr > m_fmt->txt_emax)
		m_fmt->txt_emax = ctr;

	}
	if ((p = some_listnode->mi_ptr->MI_idx) != NULL) {
	    ctr = strlen(p) - cnt_cc(p);
	    if (ctr > m_fmt->idx_emax)
		m_fmt->idx_emax = ctr;
	}
	if ((p = some_listnode->mi_ptr->MI_ival) != NULL) {
	    ctr = strlen(p) - cnt_cc(p);
	    if (ctr > m_fmt->ival_emax)
		m_fmt->ival_emax = ctr;
	}
	some_listnode = some_listnode->nextPtr;
    }
}

/*******************************************************/

M_LN_t *
mono_display_menu_ified(M_LN_t * the_menu, M_display_t * m_fmt, int columns)
{

    char *more_output;
    M_LNptr someNode;

    int i, k;
    unsigned int ctr, col_width;
    char tmpstr[200], tmpstr1[100];

    if (the_menu == NULL) {
	cprintf("\nerr: NULL menu");
	return the_menu;
    }
/* autocolumnize ? */
    if (m_fmt->auto_columnize)
	columns = opt_autocolumn(m_fmt);

    if (columns < 1 || columns > 4)
	return the_menu;

    the_menu = column_sort_linked_list(the_menu, columns);
    ctr = linked_list_itemtotal(the_menu);

/* some formatting math */

    col_width = (80 / columns);
    col_width -= (col_width * columns > 80) ? 1 : 0;

    col_width -= (m_fmt->idx_emax) ? m_fmt->idx_emax + 3 : 0;
    col_width -= (m_fmt->ival_emax) ?
	((m_fmt->no_boolean_values) ? m_fmt->ival_emax + 1 : 2)
	: 0;

/* setup for more_string() */

    more_output = (char *) xmalloc(strlen(m_fmt->menu_title) + 1);
    strcpy(more_output, m_fmt->menu_title);

/* build the output string */

    someNode = the_menu;
    for (ctr = 1; someNode != NULL; someNode = someNode->nextPtr) {
	/* index subcolumn */
	strcpy(tmpstr, "");
	if (someNode->mi_ptr->MI_idx != NULL) {
	    if (!strlen(someNode->mi_ptr->MI_idx))
		strcat(tmpstr, "  ");
	    else
		snprintf(tmpstr, sizeof(tmpstr), "\1f\1%c<\1%c%s\1%c>",
		     m_fmt->bracket_color,
		     m_fmt->index_color, someNode->mi_ptr->MI_idx,
		     m_fmt->bracket_color);
        }
	if (m_fmt->idx_emax) {
	    for (k = strlen(tmpstr) - cnt_cc(tmpstr); k <= m_fmt->idx_emax + 2; k++)
		strcat(tmpstr, " ");
	    more_output = (char *) realloc(more_output,
				  strlen(more_output) + strlen(tmpstr) + 1);
	    strcat(more_output, tmpstr);
	}
	/* value subcolumn */
	strcpy(tmpstr, "");
	if (someNode->mi_ptr->MI_ival != NULL) {
	    snprintf(tmpstr, sizeof(tmpstr), "\1%c", m_fmt->value_color);
	    if (m_fmt->no_boolean_values)
		snprintf(tmpstr1, sizeof(tmpstr), "\1%c%s", m_fmt->value_color,
			 someNode->mi_ptr->MI_ival);
	    else
		snprintf(tmpstr1, sizeof(tmpstr), "\1%c%c", m_fmt->value_color,
			 (strcmp(someNode->mi_ptr->MI_ival, "1")) ? ' ' :
			 m_fmt->flag_char);
	    strcat(tmpstr, tmpstr1);
	}
	if (m_fmt->ival_emax) {
	    i = (m_fmt->no_boolean_values) ? m_fmt->ival_emax : 1;
	    for (k = strlen(tmpstr) - cnt_cc(tmpstr); k <= i; k++)
		strcat(tmpstr, " ");
	    more_output = (char *) realloc(more_output, strlen(more_output)
					   + strlen(tmpstr) + 1);
	    strcat(more_output, tmpstr);
	}
	/* text subcolumn */
	strcpy(tmpstr, "");
	if (someNode->mi_ptr->MI_txt != NULL) {

	    snprintf(tmpstr, sizeof(tmpstr), "\1f\1%c%s", m_fmt->text_color,
		     someNode->mi_ptr->MI_txt);

	    if ((strlen(tmpstr) - cnt_cc(tmpstr)) >= col_width) {
		for (i = k = 0; (i <= col_width); i++) {
		    if (tmpstr[k] == '\0')
			break;
		    if (tmpstr[k] == '\1' && tmpstr[k + 1] != '\0')
			k++;
		    k++;
		}
		tmpstr[k] = '\0';
	    }
	    if (ctr % columns)
		for (k = strlen(tmpstr) - cnt_cc(tmpstr); k < col_width; k++)
		    strcat(tmpstr, " ");
	    else
		strcat(tmpstr, "\n");

	    more_output = (char *) realloc(more_output, strlen(more_output)
					   + strlen(tmpstr) + 1);
	    strcat(more_output, tmpstr);
	}
	ctr++;

    }
    more_string(more_output);
    xfree(more_output);
    return the_menu;
}

/* counts colorcodes..  returns double the # of '\1' chars in string */
int
cnt_cc(char *s)
{
    char *p;
    int ctr = 0;

    if (s)
	for (p = s; *p != '\0'; p++)
	    if (*p == '\1')
		ctr += 2;
    return ctr;
}

/* autocolumnize formatting, returns # of columns */
static int 
opt_autocolumn(M_display_t * m_fmt)
{
    int tot_emax, i;

    tot_emax = m_fmt->txt_emax;
    tot_emax += ((m_fmt->idx_emax) ? m_fmt->idx_emax + 3 : 0);
    tot_emax += ((m_fmt->ival_emax) ? m_fmt->ival_emax + 1 : 0);

    for (i = 4; (tot_emax * i > 80) && (i > 1); i--) ;
    return i;
}

static M_LN_t *
menu_generate_indices(M_LN_t * the_menu, M_display_t * m_fmt)
{
    M_LNptr someNode;

    int i;
    char tmpstr[20];

    if (the_menu != NULL) {
	someNode = the_menu;
	for (i = 0; someNode != NULL; i++) {
	    snprintf(tmpstr, sizeof(tmpstr), "%d",
		     (m_fmt->gen_0_idx) ? i : i + 1);
	    someNode->mi_ptr->MI_idx =
		(char *) realloc(someNode->mi_ptr->MI_idx, strlen(tmpstr) + 1);
	    strcpy(someNode->mi_ptr->MI_idx, tmpstr);
	    someNode = someNode->nextPtr;
	}
    }
    return the_menu;
}
