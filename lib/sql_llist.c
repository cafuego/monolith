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
#include "routines.h"

#define extern
#include "sql_llist.h"
#undef extern
 
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

    *p = entry;
    p->next = NULL;
    p->prev = NULL;

    q = *list; if (q == NULL) { *list = p; } else { while (q->next != NULL) q = q->next; q->next = p; p->prev = q; }
    return 0;
}

void
mono_sql_ll_free_mlist(mlist_t *list)
{
    mlist_t *ref;

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

    p = (sr_list_t *) xmalloc(sizeof(sr_list_t));

    if (p == NULL)
	return -1;

    *p = entry;
    p->next = NULL;
    p->prev = NULL;

    q = *list;

    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL) {
	    q = q->next;
        }
	q->next = p;
        p->prev = q;
    }

    return 0;
}

void
mono_sql_ll_free_sr_list(sr_list_t *list)
{
    sr_list_t *ref;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->result);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/*
 * Add xlist_t to linked list.
 * Used for storing list of express messages.
 *
 */
int
mono_sql_ll_add_xlist_to_list(xlist_t x, xlist_t ** list)
{
    xlist_t *p, *q;

    /*
     * Note mono_sql_ll_free_xlist()
     */
    p = (xlist_t *) xmalloc(sizeof(xlist_t));

    if (p == NULL)
	return -1;

    *p = x;
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
mono_sql_ll_free_xlist(xlist_t *list)
{
    xlist_t *ref;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->x);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/*
 * Add wu_list_t to linked list.
 */
int
mono_sql_ll_add_wulist_to_list(wu_list_t x, wu_list_t ** list)
{
    wu_list_t *p, *q;

    /*
     * Note mono_sql_ll_free_wulist()
     */
    p = (wu_list_t *) xmalloc(sizeof(wu_list_t));

    if (p == NULL)
	return -1;

    *p = x;
    p->next = NULL;

    q = *list;
    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 0;
}

void
mono_sql_ll_free_wulist(wu_list_t *list)
{
    wu_list_t *ref;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->user);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/*
 * Add wx_list_t to linked list.
 */
int
mono_sql_ll_add_wxlist_to_list(wx_list_t x, wx_list_t ** list)
{
    wx_list_t *p, *q;

    /*
     * Note mono_sql_ll_free_wxlist()
     */
    p = (wx_list_t *) xmalloc(sizeof(wx_list_t));

    if (p == NULL)
	return -1;

    *p = x;
    p->next = NULL;

    q = *list;
    if (q == NULL) {
	*list = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 0;
}

void
mono_sql_ll_free_wxlist(wx_list_t *list)
{
    wx_list_t *ref;

    while (list != NULL) {
        ref = list->next;
        (void) xfree(list->x->message);
        (void) xfree(list->x);
        (void) xfree(list);
        list = ref;
    }
    return;
}

/* eof */

/* eof */
