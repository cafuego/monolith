/* $Id$ */
/* the purpose of this file changes, it should from now on contain
 * a sort of `cache' for users. with much-used info, such as usernumbers
 * quickx, etc 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "monolith.h"

#include "routines.h"
#include "userfile.h"
#include "libfriends.h"

int
add_friend_to_list(friend_t element, friend_t ** list)
{
    friend_t *p, *q;

    p = (friend_t *) xmalloc(sizeof(friend_t));
    if (p == NULL)
	return -1;

    /* copy, and set as last element */
    *p = element;
    p->next = NULL;

    /* find last element in list */
    q = *list;
    if (q == NULL) {		/* empty list */
	*list = p;		/* put that in as first element */
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 0;
}

int
remove_friend_from_list(const char *name, friend_t ** list)
{
    friend_t *p, *q;

    if (*list == NULL)
	return -1;		/* empty list */

    p = q = *list;
    while (p != NULL) {
	if (EQ(name, p->name)) {
	    if (p == *list) {
		*list = p->next;
	    } else {
		q->next = p->next;
	    }
	    xfree(p);
	    return 0;
	}
	q = p;
	p = p->next;
    }
    return -1;
}

int
dest_friends_list(friend_t * list)
{
    friend_t *p, *q;

    p = list;
    while (p) {
	q = p->next;
	xfree(p);
	p = q;
    }
    return 0;
}

/* checks if user2 is a friend of user1 */
int
is_friend(const char *user1, const char *user2)
{
    unsigned int id1, id2;

    if (mono_sql_u_name2id(user1, &id1) == -1)
	return FALSE;
    if (mono_sql_u_name2id(user2, &id2) == -1)
	return FALSE;

    return mono_sql_uu_is_on_list(id1, id2, L_FRIEND);
}

/* checks if user2 is an enemy of user1 */
int
is_enemy(const char *user1, const char *user2)
{
    unsigned int id1, id2;

    if (mono_sql_u_name2id(user1, &id1) == -1)
	return FALSE;
    if (mono_sql_u_name2id(user2, &id2) == -1)
	return FALSE;

    return mono_sql_uu_is_on_list(id1, id2, L_ENEMY);
}

int
user_on_list(const char *name, friend_t * list)
{
    friend_t *p;

    p = list;
    while (p) {
	if (EQ(p->name, name)) {
	    return TRUE;
	}
	p = p->next;
    }
    return FALSE;
}

char *
number_to_friend(int num, friend_t * list)
{
    friend_t *p;

    p = list;
    while (p) {
	if (p->quickx == num) {
	    return p->name;
	}
	p = p->next;
    }
    return NULL;
}

int
friend_to_number(const char *name, friend_t * list)
{
    friend_t *p;

    p = list;
    while (p) {
	if (EQ(p->name, name)) {
	    return p->quickx;
	}
	p = p->next;
    }
    return -2;
}

int
set_quickx(const char *name, int quick, friend_t * list)
{
    friend_t *p;

    p = list;
    while (p) {
	if (EQ(p->name, name)) {
	    p->quickx = quick;
	}
	p = p->next;
    }
    return -2;
}
