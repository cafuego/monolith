/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

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

#include "ext.h"
#include "input.h"
#include "inter.h"
#include "help.h"
#include "routines2.h"

#define extern
#include "friends.h"		/* friends prototypes */
#undef extern

static void menu_friend_add(int param);
char *menu_friend_list(int param);
static void menu_friend_remove(int param);
static void menu_friend_quick_add(void);

friend_t *friend_cache = NULL;

/*
 * ------------------------------------------------ 
 * this fn checks if person2 is an enemy of person1 
 * return true if so, else false                    
 * ------------------------------------------------ 
 */

int
is_my_enemy(const char *person)
{
    return is_cached_enemy(person);
}

int
is_my_friend(const char *person)
{
    return is_cached_friend(person);
}
/*
void
update_friends_cache(void)
{
    if (friend_cache != NULL)
        dest_friends_list(friend_cache);
	
    mono_sql_uu_read_list(usersupp->usernum, &friend_cache, L_FRIEND);
}

int
is_cached_friend(const char *name)
{
    friend_t *p;

    p = friend_cache;
    while(p) {
        if (strlen(p->name))
	    if (!strcasecmp(name, p->name))
	        return 1;
	p = p->next;
    }
    return 0;
}

char *
cached_x_to_name(const int slot)
{
    friend_t *p;
    
    p = friend_cache;
    while(p) {
        if (p->quickx == slot)
            return (strlen(p->name)) ? p->name : "";
        p = p->next;
    }
    return "";
}

int
cached_name_to_x(const char * bobs_name)
{
    friend_t *p;
    
    p = friend_cache;
    while(p) {
	if (!strcasecmp(p->name, bobs_name))
	    return p->quickx;
	p = p->next;
    }
    return -1;
}
*/
void
friends_online()
{
    unsigned int j = 0, k = 0;
    friend_t *p;
    int i;

    cprintf("\n");

    mono_sql_uu_read_list(usersupp->usernum, &p, L_FRIEND);

    while (p) {
	k = 0;
	if (mono_return_pid(p->name) != -1) {
	    k = 1;
	}
	if (k == 1) {
	    mono_sql_uu_user2quickx(usersupp->usernum, p->usernum, &i);
	    if (i != -1)
		cprintf("\1f\1w<\1g%d\1w> \1c%-35s%c", i, p->name, j++ % 2 ? '\n' : ' ');
	    else
		cprintf("\1f    \1c%-35s%c", p->name, j++ % 2 ? '\n' : ' ');
	}
	p = p->next;
    }

    if (j % 2)
	cprintf("\n");
    cprintf("\n");

    dest_friends_list(p);

    return;
}

void
menu_friend(int param)
{
    register char cmd = '\0';
    char *str = NULL;

    nox = 1;
    while (1) {
	IFNEXPERT
	{
	    if (param == FRIEND)
		more(MENUDIR "/menu_friend", 1);
	    else
		more(MENUDIR "/menu_enemy", 1);
	}

	if (!(usersupp->flags & US_NOCMDHELP))
	    cprintf("%s\n", (param == FRIEND) ? FRIEND_CMDS : ENEMY_CMDS);

	cprintf("\1f\1g%s \1w-> \1c", (param == FRIEND) ? "Edit X-Friends" : "Edit X-Enemies");

	cmd = get_single_quiet("Aacrtdqls\n\r ?");

	switch (cmd) {

	    case '?':
		cprintf("Help!\n");
		if (param == FRIEND)
		    online_help('f');
//                  more(MENUDIR "/menu_friend", 1);
		else
		    online_help('e');
//                  more(MENUDIR "/menu_enemy", 1);
		break;

	    case '\n':
	    case '\r':
	    case ' ':
	    case 'q':
		cprintf("\1f\1gQuit.\n");
//		update_friends_cache();
		start_user_cache(usersupp->usernum);
		return;
		break;

	    case 'A':
		if (param == FRIEND) {
		    cprintf("\1f\1gAdd Quick-X Friend.\n");
		    menu_friend_quick_add();
		    break;
		}
		break;

	    case 'a':
		cprintf("\1f\1gAdd user to %slist.\n", (param == FRIEND) ? "friends" : "enemy");
		menu_friend_add(param);
		break;

	    case 'c':
		cprintf("\1f\1gAre you sure you want to clear your %slist (y/N)? ", (param == FRIEND) ? "friends" : "enemy");
		if (yesno_default(NO) == NO)
		    break;
		cprintf("\1f\1g%s \1w-> \1c", (param == FRIEND) ? "Edit X-Friends" : "Edit X-Enemies");
		cprintf("\1f\1gClearing %slist.\n", (param == FRIEND) ? "friends" : "enemy");
		if (param == FRIEND) {
		    mono_sql_uu_clear_list_by_type(usersupp->usernum, L_FRIEND);
		} else {
		    mono_sql_uu_clear_list_by_type(usersupp->usernum, L_ENEMY);
		}
		break;

	    case 'd':
	    case 'r':
		cprintf("\1f\1gRemove user from %slist.\n", (param == FRIEND) ? "friends" : "enemy");
		menu_friend_remove(param);
		break;

	    case 'l':
	    case 's':
		cprintf("\1f\1gList %s.\n", (param == FRIEND) ? "friends" : "enemies");
		str = menu_friend_list(param);
		more_string(str);
		xfree(str);
		break;

	    case 't':
		if (param == ENEMY)
		    break;
		usersupp->flags ^= US_NOTIFY_FR;
		cprintf("\1f\1gToggle logon notifications %s.\n", (usersupp->flags & US_NOTIFY_FR) ? "on" : "\1roff\1g");
		break;

	    default:
		cprintf("\1f\1rEEK, HAMSTERS\n");
		break;
	}			/* switch */
    }				/* while */
    return;
}

void
menu_friend_add(int param)
{

    char *name;
#ifdef FRIENDS_ENEMIES_LISTS_WORK_WITH_SQL
    char user[L_USERNAME + 1], bbs[L_BBSNAME + 1];
#endif
    unsigned int flag, flag2;
    unsigned int id2;

    nox = 1;
    cprintf("\1f\1gEnter username\1w: \1c");
    flag = (param == FRIEND) ? L_FRIEND : L_ENEMY;
    flag2 = (param == FRIEND) ? L_ENEMY : L_FRIEND;

    name = get_name(5);

    if (strlen(name) == 0)
	return;

    if (strchr(name, '@') != NULL) {
	cprintf("\1f\1rInterBBS names aren't supported on friends-enemieslists.\n");
	return;
#ifdef FRIENDS_ENEMIES_LISTS_WORK_WITH_SQL
	parse_inter_address(name, user, bbs);
	if (check_remote_user(user, bbs) == 'X') {
	    cprintf("\1f\1rUser \1y%s\1r does not exist at \1y%s\1r.\n", user, bbs);
	    return;
	} else {
	    /* reformat name, so we get the full bbs name if they used an abbrev. */
	    sprintf(name, "%s@%s", user, bbs);
	}
#endif
    } else if (mono_sql_u_name2id(name, &id2) == -1) {
	cprintf("\1f\1rNo such user.\n");
	return;
    }
    if (mono_sql_uu_is_on_list(usersupp->usernum, id2, flag) == TRUE) {
	cprintf("\1f\1y%s \1ris already on your %slist.\n", name,
		(param == FRIEND) ? "friends" : "enemy");
	return;
    }
    if (mono_sql_uu_is_on_list(usersupp->usernum, id2, flag2) == TRUE) {
	cprintf("\1f\1y%s \1rshould be removed from your %slist first.\n", name,
		(param == FRIEND) ? "enemy" : "friends");
	return;
    }
    mono_sql_uu_add_entry(usersupp->usernum, id2, flag);

    cprintf("\1f\1y%s \1ghas been added to your %slist.\n", name,
	    (param == FRIEND) ? "friends" : "enemy");
    return;
}

static void
menu_friend_quick_add()
{

    char command;
    char *name;
    char old[L_USERNAME + 1];
    friend_t f;
    int i;
    unsigned int id2;

    nox = 1;
    menu_friend_list(FRIEND);

    cprintf("\1gWhich slot do you want to edit\1w: \1c");
    command = get_single("0123456789 \r\n");

    if (command == '\r' || command == ' ' || command == '\n' || command == 'q')
	return;

    i = command - '0';
    if (mono_sql_uu_quickx2user(usersupp->usernum, i, &id2) == -1) {
	strcpy(old, "");
    } else if (mono_sql_u_id2name(id2, old) == -1) {
	strcpy(old, "");
    }
    if (strlen(old) != 0) {
	cprintf("\1gDo you want to remove \1y%s \1gfrom slot \1w(\1g%d\1w)\1g ?\1c ", old, i);
	if (yesno() == NO)
	    return;
	mono_sql_uu_remove_quickx(usersupp->usernum, id2);
    }
    /* ask new name! */

    cprintf("\1f\1gEnter username\1w: \1c");
    name = get_name(5);

    if (strlen(name) == 0)
	return;

    if (strchr(name, '@') != NULL) {
	cprintf("\1f\1rSorry, InterBBS names are not allowed.\n");
	return;
    } else if (mono_sql_u_name2id(name, &id2) == -1) {
	cprintf("\1f\1rNo such user.\n");
	return;
    }
    if (!is_my_friend(name)) {
	strcpy(f.name, name);
	f.quickx = i;
	mono_sql_uu_add_entry(usersupp->usernum, id2, L_FRIEND);
    } else
	mono_sql_uu_add_quickx(usersupp->usernum, id2, i);

    cprintf("\1f\1gUser \1y%s \1gwas added to slot \1w(\1g%d\1w)\1g.\n", name, i);

    return;
}

void
menu_friend_remove(int param)
{
    char *name;
    unsigned int id2;
    int flag;

    nox = 1;
    flag = (param == FRIEND) ? L_FRIEND : L_ENEMY;

    cprintf("\1f\1gEnter username\1w: \1c");
    name = get_name(5);

    if (strlen(name) == 0)
	return;

    mono_sql_u_name2id(name, &id2);

    if (mono_sql_uu_is_on_list(usersupp->usernum, id2, flag) == FALSE) {
	cprintf("\1f\1y%s \1gis not on your %slist.\n", name,
		(param == FRIEND) ? "friends" : "enemy");
	return;
    }
    mono_sql_uu_remove_entry(usersupp->usernum, id2);
    cprintf("\1f\1y%s \1ghas been removed from your %slist.\n", name,
	    (param == FRIEND) ? "friends" : "enemy");
    return;
}

char *
menu_friend_list(int param)
{
    unsigned int j = 0;
    friend_t *p;
    unsigned int flag;
    char line[100], *q;

    q = (char *) xmalloc(100 * 100);
    strcpy(q, "");

    flag = (param == FRIEND) ? L_FRIEND : L_ENEMY;

    mono_sql_uu_read_list(usersupp->usernum, &p, flag);

    sprintf(q, "\n\1g\1f");
    while (p) {
	if (p->quickx == -1)
	    sprintf(line, "     %-20s", p->name);
	else
	    sprintf(line, " \1w<\1g%d\1w> \1g%-20s", p->quickx, p->name);
	p = p->next;
	if ((j++ % 3) == 2)
	    strcat(line, "\n");
	strcat(q, line);
    }
    strcat(q, "\n");
    if (j % 3 != 0)
	strcat(q, "\n");

    dest_friends_list(p);

    return q;
}
