#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"
#include "sql_user.h"
#include "sql_useruser.h"

#define extern
#include "libcache.h"
#undef extern

static int destroy_user_cache(void);
static int add_to_user_cache(user_cache_t);
static int remove_from_user_cache(user_cache_t *);
static int remove_stale_cache_entries(void);
static char * cached_user_id2name(const unsigned int );
static unsigned int cached_user_name2id(const char *);
static void update_user_cache(const unsigned int );

user_cache_t *user_cache = NULL;  /* global user cache */

/*
 * -  starting the cache:  caches the friends and enemies list
 * -  cache should be updated with any user info that sql encounters, this
 *    is done by usernumber..
 * -  starting the cache dumps everything, re-loads the friend/enemy lists
 * -  DO NOT remove friends or enemies from the cache!  That information is 
 *    only to be reloaded any time the friendslist is updated.
 * -  By design, friends are loaded first, then enemies, then anything else.
 * -  internally, friends are stored as friend = 1 in the user_cache_t struct
 * -  enemies are stored as friend = -1.  
 * -  anything else..  friend = 0;
 * -  quickx should always be either a quickx slot, or -1.
 *
 * WARNING!  this code assumes usernames and numbers don't change during
 *           the course of a login.  If namechanges are implemented, 
 *           all cache information has to be reloaded.. for everybody!
 *           could perhaps be added to updateself().
 */

void
flush_user_cache(void)
{
    if (user_cache != NULL)
	destroy_user_cache();
    user_cache = NULL;
}

void
update_user_cache(const unsigned int num)
{
    user_cache_t the_user;
    static int update_ctr = 0;

    if (is_cached_usernumber(num))	/* no dups, please! */
	return;

    update_ctr++;
    if (!(update_ctr % 10))
	remove_stale_cache_entries();

    if (mono_sql_u_id2name(num, the_user.name) == -1)
	return;

    the_user.user_number = num;
    the_user.friend = 0;
    the_user.quickx = -1;
    the_user.next = NULL;
    add_to_user_cache(the_user);
}

void
start_user_cache(const unsigned int the_number)
{
    friend_t *tmp_flist = NULL, *frPtr;
    user_cache_t the_user;

    if (user_cache != NULL)	/* flush any existing cache to /dev/null */
	flush_user_cache();

/* load friends */
    if (mono_sql_uu_read_list(the_number, &tmp_flist, L_FRIEND) != -1) {
	frPtr = tmp_flist;
	while (frPtr) {
	    the_user.user_number = frPtr->usernum;
	    the_user.friend = 1;
	    the_user.quickx = frPtr->quickx;
	    strcpy(the_user.name, frPtr->name);
	    the_user.next = NULL;
	    add_to_user_cache(the_user);
	    frPtr = frPtr->next;
	}
	dest_friends_list(tmp_flist);
    }
    tmp_flist = NULL;

/* load enemies */
    if (mono_sql_uu_read_list(the_number, &tmp_flist, L_ENEMY) != -1) {
	frPtr = tmp_flist;
	while (frPtr) {
	    the_user.user_number = frPtr->usernum;
	    the_user.friend = the_user.quickx = -1;
	    strcpy(the_user.name, frPtr->name);
	    the_user.next = NULL;
	    add_to_user_cache(the_user);
	    frPtr = frPtr->next;
	}
	dest_friends_list(tmp_flist);
    }
}

int
is_cached_username(const char *name)
{
    user_cache_t *p;

    p = user_cache;
    while (p) {
	if (strlen(p->name))
	    if (!strcasecmp(name, p->name))
		return 1;
	p = p->next;
    }
    return 0;
}

int
is_cached_usernumber(const unsigned int number)
{
    user_cache_t *p;

    p = user_cache;
    while (p) {
	if (p->user_number == number)
	    return 1;
	p = p->next;
    }
    return 0;
}

/* static wrapper for mono_sql_u_id2name */
int
mono_cached_sql_u_id2name(const unsigned int the_id, char * the_name)
{

    if (is_cached_usernumber(the_id)) {
	strcpy(the_name, cached_user_id2name(the_id));
	return 0;
    } 

    if (mono_sql_u_id2name(the_id, the_name) == -1)
	return -1;

    update_user_cache(the_id);
    return 0;
}

char *
cached_user_id2name(const unsigned int number)
{
    user_cache_t *p;

	p = user_cache;
	while (p) {
	    if (p->user_number == number)
		return (strlen(p->name)) ? p->name : "";
	    p = p->next;
	}
    return "";
}

unsigned int
cached_user_name2id(const char *bobs_name)
{
    user_cache_t *p;

	p = user_cache;
	while (p) {
	    if (!strcasecmp(p->name, bobs_name))
		return p->user_number;
	    p = p->next;
	}
	return -1;
}

/* static wrapper for mono_sql_u_name2id */
int
mono_cached_sql_u_name2id(const char * the_name, unsigned int * the_id)
{

    assert(the_name != NULL);

    if (is_cached_username(the_name)) {
	*the_id = cached_user_name2id(the_name);
	return 0;
    } 

    if (mono_sql_u_name2id(the_name, the_id) == -1)
	return -1;

    update_user_cache(*the_id);
    return 0;
}

int
remove_stale_cache_entries(void)
{
    user_cache_t *p;

    if (user_cache == NULL)
	return 0;
    p = user_cache;
    while (p)
	if (p->friend)		/* don't remove friends! */
	    p = p->next;
	else if (mono_return_pid(p->name) == -1) {
	    remove_from_user_cache(p);
	    p = user_cache;
	} else
	    p = p->next;
    return 1;
}

int
remove_from_user_cache(user_cache_t * element)
{
    user_cache_t *p, *q;

    if (!element)
	return 0;
    if (user_cache == NULL)
	return 0;

    q = p = user_cache;
    while (p) {
	if (p->user_number == element->user_number)
	    break;
	q = p;
	p = p->next;
    }
    if (p) {
	if (p == q)
	    user_cache = p->next;
	else
	    q->next = p->next;
	xfree(p);
    }
    return 1;
}

int
add_to_user_cache(user_cache_t element)
{
    user_cache_t *p, *q;

    if (!strlen(element.name))
	return 0;

    p = (user_cache_t *) xmalloc(sizeof(user_cache_t));
    if (p == NULL)
	return 0;
    p->user_number = element.user_number;
    strcpy(p->name, element.name);
    p->friend = element.friend;
    p->quickx = element.quickx;
    p->next = NULL;

    q = user_cache;

    if (q == NULL) {		/* empty list */
	user_cache = p;
    } else {
	while (q->next != NULL)
	    q = q->next;
	q->next = p;
    }
    return 1;
}

int
destroy_user_cache(void)
{
    user_cache_t *p;

    p = user_cache;
    while (user_cache) {
	user_cache = p->next;
	xfree(p);
	p = user_cache;
    }
    return 0;
}

/* from friends.c, now lives here..                          */

int
is_cached_friend(const char *name)
{
    user_cache_t *p;

    p = user_cache;
    while (p && p->friend == 1) {
	if (strlen(p->name))
	    if (!strcasecmp(name, p->name) && p->friend == 1)
		return 1;
	p = p->next;
    }
    return 0;
}

int
is_cached_enemy(const char *name)
{
    user_cache_t *p;
    p = user_cache;
    while (p && p->friend) {
	if (strlen(p->name))
	    if (!strcasecmp(name, p->name) && p->friend == -1)
		return 1;
	p = p->next;
    }
    return 0;
}

char *
cached_x_to_name(const int slot)
{
    user_cache_t *p;

    p = user_cache;
    while (p && p->friend == 1) {
	if (p->quickx == slot)
	    return (strlen(p->name)) ? p->name : "";
	p = p->next;
    }
    return "";
}

int
cached_name_to_x(const char *bobs_name)
{
    user_cache_t *p;

    p = user_cache;
    while (p && p->friend == 1) {
	if (!strcasecmp(p->name, bobs_name))
	    return p->quickx;
	p = p->next;
    }
    return -1;
}

/*  debugging dump of cache to screen */

void
show_user_cache(void)
{
    user_cache_t *p;
    int i;

    p = user_cache;
    for (i = 1; p; i++) {
	cprintf("\n%d> name: %15s  id: %4d", i, p->name, p->user_number);
	if (p->friend)
	    cprintf("  (%s)  quickx(%d)", (p->friend == 1) ?
		    "friend" : "enemy ", p->quickx);
	p = p->next;
    }
}
