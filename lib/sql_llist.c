/*
 * $Id$
 *
 * Contains low-level linked list functions for use with sql system.
 * And yes, I still HATE linked lists. :P
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "monolith.h"

#ifdef extern
#include "sql_llist.h"
#endif
 
/*
 * Add mlist_t to linked list.
 * Used for storing list of messages to read in forum or topic.
 *
 */
int
mono_sql_ll_add_mlist_to_list(mlist_t entry, mlist_t ** list)
{
    mlist_t *p, *q;

    /*
     * Note mono_sql_ll_free_mlist()
     */
    p = (mlist_t *) xmalloc(sizeof(mlist_t));

    if (p == NULL)
	return -1;

    memset(&p, 0, sizeof(mlist_t));

    *p = entry;
    p->next = NULL;
    p->prev = NULL;

    q = *list;
    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
        p->prev = q;
    }
    return 0;
}

void
mono_sql_ll_free_mlist(mlist_t *list)
{
    mlist_t *ref;

    /*
     * rewind list, just in case...
     */
    while (list->prev != NULL)
        list = list->prev;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->message->content);
        (void) xfree(list->message);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/*
 * Add sr_list_t to linked list.
 * Used for storing list of search results after forum search.
 */
int
mono_sql_ll_add_srlist_to_list(sr_list_t entry, sr_list_t ** list)
{
    sr_list_t *p, *q;

    /*
     * Note mono_sql_ll_free_sr_list()
     */
    p = (sr_list_t *) xmalloc(sizeof(sr_list_t));

    if (p == NULL)
	return -1;

    memset(&p, 0, sizeof(sr_list_t));

    *p = entry;
    p->next = NULL;
    p->prev = NULL;

    q = *list;
    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
        p->prev = q;
    }
    return 0;
}

void
mono_sql_ll_free_sr_list(sr_list_t *list)
{
    sr_list_t *ref;

    /*
     * rewind list, just in case...
     */
    while (list->prev != NULL)
        list = list->prev;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->result);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/* eof */
