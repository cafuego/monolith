/*
 * btmp.c 
 * $Id$ 
 *
 * Have to make all pointers relative to the shm->first.
 * Because different process have other offsets
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>

#include "monolith.h"

#define extern
#include "btmp.h"
#undef extern

#include "friends.h"
#include "log.h"
#include "libuid.h"
#include "registration.h"
#include "routines.h"

#include "sql_forum.h"
#include "sql_online.h"

#include "telnet.h"

#define WHO_LOCK 1
#define WHO_UNLOCK 0

#define SHMKEY BBSDIR "/etc/geheugensleutel"
#define WHOLISTMODE     0664

#define S_BUSYWHOLIST	1	/* set when the wholist is "busy"       */
#define S_BUSYQUEUE	2	/* set when the queue is "busy"         */
#define S_BUSYROOMS	4	/* set when roomslist is busy */

static int _mono_initialize_shm(void);
static int _mono_remove_from_linked_list(const char *user);
static int _mono_add_to_linked_list(btmp_t user);
static int _mono_initialize_holodeck(void);
static int _mono_guide_ok(btmp_t *guide);

/* -------------------------------------------------------------------- */
/* return_pid() */
pid_t
mono_return_pid(const char *name)
{
    int p;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    mono_lock_shm(WHO_LOCK);
    p = shm->first;
    while (p != -1) {
	if (EQ(name, shm->wholist[p].username)) {
	    mono_lock_shm(WHO_UNLOCK);
	    return shm->wholist[p].pid;
	}
	p = shm->wholist[p].next;
    }
    mono_lock_shm(WHO_UNLOCK);
    return -1;
}

/* -------------------------------------------------------------------- */
/* add_loggedin() this adds user `user' to the wholist */
int
mono_add_loggedin(const user_t * user)
{
    btmp_t newb;

    if (!shm)
	return -1;

    memset(&newb, 0, sizeof(btmp_t));
    strcpy(newb.username, user->username);
    newb.logintime = time(NULL);
    newb.chat = user->chat;
    newb.priv = user->priv;
    strcpy(newb.doing, user->doing);
    newb.pid = getpid();

    newb.flags = 0;

    if (user->flags & US_CLIENT)
	newb.flags |= B_CLIENTUSER;
    if (user->priv & PRIV_ALLUNVALIDATED)
	newb.flags |= B_XDISABLED;
    if (user->flags & US_DONATOR)
	newb.flags |= B_DONATOR;
    if (user->flags & US_GUIDE)
	newb.flags |= B_GUIDEFLAGGED;

    strcpy(newb.curr_room, " * Hasn't left the Docking Bay yet *");
    strcpy(newb.x_ing_to, "");

    if (user->hidden_info & H_COUNTRY) {
	strncpy(newb.remote, HOSTNAME_IF_HIDDEN, 17);
    } else if (strlen(user->RGstate) && strlen(user->RGcountry)) {
	snprintf(newb.remote, 20, "%s, %s", user->RGstate, user->RGcountry);
    } else if (strlen(user->RGstate)) {
	snprintf(newb.remote, 20, "%s", user->RGstate);
    } else {
	snprintf(newb.remote, 20, "%s", user->RGcountry);
    }

    newb.remote[17] = '\0';
    if (user->flags & US_HIDDENHOST) {
	strncpy(newb.ip_address, HOSTNAME_IF_HIDDEN, 17);
    } else {
	strncpy(newb.ip_address, user->lasthost, 20);
    }
    newb.ip_address[16] = '\0';

    _mono_add_to_linked_list(newb);

    return 0;
}

/*************************************************
* remove_loggedin()
*
* how: 1 -> remove me and the ghosts.
*      0 -> don't remove me; just the ghosts.
*************************************************/

int
mono_remove_loggedin(const char *user)
{

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    _mono_remove_from_linked_list(user);

    return 0;
}

/*************************************************
* change_online()
*
* ch:
*	 1 -> user has new name or priv, update.
*	 2 -> user is idle; add time to wholist.
*	 3 -> user is not idle anymore.
*	 4 -> user changes his/her HOSTNAME 
*	 5 -> user updates the roomname-field.
*	 6 -> user is "twominutesidle"	       [#]
*
*        7 -> user is guideflagged	       [%]
*	-7 -> user is not guideflagged
*	 8 -> user goes posting		       [+]
*	-8 -> user stops posting
*	 9 -> user is xdisabled		       [*]
*	-9 -> user is xenabled
*       11 -> user goes to chatmode            [!]
*      -11 -> user leaves chatmode
*       13 -> toggle away status
*	14 -> new flying
*       15 -> new user we're x-ing to 
*       16 -> change away message
*       17 -> change lock status
*       10 -> xdisable all
*      -10 -> turn off xdisable all
*       12 -> user has updated profile.
*      -12 -> profile outdated again.
*************************************************/

int
mono_change_online(const char *user, const char *tmp_string, int ch)
{

    btmp_t *p;
    int q;
    time_t now;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    q = shm->first;
    while (q != -1) {
	p = &(shm->wholist[q]);

	if (EQ(user, p->username)) {

	    mono_lock_shm(WHO_LOCK);

	    switch (ch) {


		case 1:
                    sscanf( tmp_string, "%ul", &(p->priv) );
		    /* p->priv = atoi(tmp_string); */
		    break;

		case 2:
		    now = time(NULL);
		    p->idletime = now - 300;	/*
						 * * * 5 minutes ago 
						 */
		    p->flags |= B_REALIDLE;	/*
						 * * * means idle 
						 */
		    break;

		case 3:
		    p->flags &= ~B_TWOMINIDLE;
		    p->flags &= ~B_REALIDLE;
		    break;

		case 4:
		    strncpy(p->remote, tmp_string, 20);
		    p->remote[20] = '\0';
		    break;

		case 5:
		    strncpy(p->curr_room, tmp_string, 40);
		    p->curr_room[40] = '\0';
		    break;

		case 6:
		    p->flags |= B_TWOMINIDLE;
		    break;

		case 7:
		    p->flags |= B_GUIDEFLAGGED;
		    break;

		case -7:
		    p->flags &= ~B_GUIDEFLAGGED;
		    break;

		case 8:
		    p->flags |= B_POSTING;
		    break;

		case -8:
		    p->flags &= ~B_POSTING;
		    break;

		case 9:
		    p->flags |= B_XDISABLED;
		    break;

		case -9:
		    p->flags &= ~B_XDISABLED;
		    break;

#ifdef ALLX
		case -10:
		    p->flags &= ~B_ALLXDISABLED;
		    break;

		case 10:
		    p->flags |= B_ALLXDISABLED;
		    break;
#endif /* ALLX */

		case 11:
		    sscanf(tmp_string, "%d", &(p->chat));
/*                  p->chat = atoi(tmp_string); */
		    break;

		case -11:
		    p->chat = 0;
		    break;

                case 12:
                    p->flags |= B_INFOUPDATED;
                    break;
                case -12:
		    p->flags &= ~B_INFOUPDATED;
                    break;

		case 13:
		    p->flags ^= B_AWAY;
		    break;

		case 14:
		    strncpy(p->doing, tmp_string, 29);
		    p->doing[30] = '\0';
		    break;

		case 15:
		    strncpy(p->x_ing_to, tmp_string, L_USERNAME);
		    p->x_ing_to[L_USERNAME] = '\0';
		    break;

		case 16:
		    strncpy(p->awaymsg, tmp_string, 99);
		    p->awaymsg[99] = '\0';
		    break;

		case 17:
		    p->flags ^= B_LOCK;
		    break;

		default:
		    break;

	    }			/* switch */
	    mono_lock_shm(WHO_UNLOCK);
	    return 0;
	}			/* if */
	q = shm->wholist[q].next;
    }				/* while */
    return -1;
}

/*************************************************
* send_signal_to_the_queue()
*
* tell the first person in the queue it's time to
* enter the bbs
*************************************************/

int
mono_send_signal_to_the_queue()
{
    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    if (shm->queue_count == 0)
	return 0;

    kill(shm->queue[0].pid, SIGALRM);
    return 0;
}
/*************************************************
* mono_lock_shm()
*
* 1 -> wait until unlocked, then LOCK
* 2 -> UNLOCK
*
* it waits for 5 seconds, trying ten times every
* second, and if it is still locked, it doesn't
* pay any attention to it.
*************************************************/ int
mono_lock_shm(int key)
{
    unsigned int a;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    if (key == WHO_LOCK) {
	for (a = 0; (a < 50 && (shm->status & S_BUSYWHOLIST)); a++)
	    usleep(100000);

	shm->status |= S_BUSYWHOLIST;	/* set the lock-flag */
    } else {
	shm->status &= ~S_BUSYWHOLIST;	/* unset the lock-flag */
    }
    return 0;
}

/* ------------------ */
int
mono_lock_queue(int key)
{
    unsigned int a;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    if (key == WHO_LOCK) {
	for (a = 0; (a < 50 && (shm->status & S_BUSYQUEUE)); a++)
	    usleep(100000);

	shm->status |= S_BUSYQUEUE;	/* set the lock-flag */
    } else {
	shm->status &= ~S_BUSYQUEUE;	/* unset the lock-flag */
    }
    return 0;
}

int
mono_lock_rooms(int key)
{
    unsigned int a;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    if (key == WHO_LOCK) {
	for (a = 0; (a < 50 && (shm->status & S_BUSYROOMS)); a++)
	    usleep(100000);

	shm->status |= S_BUSYROOMS;	/* set the lock-flag */
    } else {
	shm->status &= ~S_BUSYROOMS;	/* unset the lock-flag */
    }
    return 0;
}


/**********************************************
* read_btmp()
*
* remember to free() the structure.
**********************************************/

btmp_t *
mono_read_btmp(const char *name)
{

    btmp_t *record = NULL;
    int i;

    if (!shm) {
	mono_errno = E_NOSHM;
	return NULL;
    }
    mono_lock_shm(WHO_LOCK);

    i = shm->first;
    while (i != -1) {
	if (EQ(shm->wholist[i].username, name)) {
	    record = (btmp_t *) xcalloc(1, sizeof(btmp_t));
	    *record = shm->wholist[i];
	    break;
	}
	i = shm->wholist[i].next;
    }
    mono_lock_shm(WHO_UNLOCK);
    return record;
}


/*
 * connect_shm()
 * connect global variables to the shared memory.
 */
int
mono_connect_shm()
{
    int shmid = 0;
    int readonly = FALSE;
    FILE *fp;
    char bing[10];

    if (geteuid() != BBSUID) {
	fprintf(stderr, "Incorrect euid for IPC connect.\n");
	fflush(stderr);
	log_it("shmlog", "Attempted connect with euid %d.", geteuid());
	readonly = TRUE;
/*	exit(EXIT_FAILURE); */
    }
    if (fexists(SHMKEY) == TRUE) {
	fp = xfopen(SHMKEY, "r", TRUE);
	fgets(bing, 9, fp);
	shmid = atoi(bing);
	fclose(fp);
        if ( readonly == TRUE ) {
	    shm = (bigbtmp_t *) shmat(shmid, 0, SHM_RDONLY );
        } else {
    	    shm = (bigbtmp_t *) shmat(shmid, 0, 0);
        }
	if (-1 == (int) shm) {
	    fprintf(stderr, "Couldn't connect to Monolith IPC Services.\n");
	    fflush(stderr);
	    log_it("shmlog", "Unable to connect to IPC Services.");
	    exit(EXIT_FAILURE);
	}
    } else {
	if ((shmid = shmget(IPC_PRIVATE, sizeof(bigbtmp_t),
			    IPC_CREAT | WHOLISTMODE)) == -1) {
	    fprintf(stderr, "Couldn't connect to Monolith IPC Services.");
	    fflush(stderr);
	    log_it("shmlog", "Unable to connect to IPC Services.");
	    exit(EXIT_FAILURE);
	}
	fp = xfopen(SHMKEY, "w", TRUE);
	fprintf(fp, "%d\n", shmid);
	fclose(fp);
	log_it("shmlog", "New shm ID %d created.", shmid);
	shm = (bigbtmp_t *) shmat(shmid, 0, 0);

	if (-1 == (int) shm) {
	    fprintf(stderr, "Couldn't connect to Monolith IPC Services\n");
	    fflush(stderr);
	    log_it("shmlog", "Unable to connect to IPC Services.");
	    exit(EXIT_FAILURE);
	}
	_mono_initialize_shm();
	mono_sql_f_fix_quickroom();
    }
    if (shm == NULL) {
	exit(EXIT_FAILURE);
    }
    return 0;
}

/* detach_shm(), detached shared memory */
int
mono_detach_shm()
{
    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    if (shmdt((char *) shm) == -1)
	return -1;
    return 0;
}

int
_mono_initialize_shm()
{
    int fd;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    memset(shm, 0, sizeof(bigbtmp_t));
    shm->status = 0;
    shm->first = -1;
    shm->user_count = 0;
    shm->queue_count = 0;

    /* Initialize holodeck room names and types */
    _mono_initialize_holodeck();

    fd = xopen(QUICKROOM, O_RDONLY, FALSE);

    if (fd == -1) {
	mono_errno = E_NOFILE;
	return -1;
    }
    if (read(fd, shm->rooms, sizeof(room_t) * MAXQUADS) == -1) {
	close(fd);
	return -1;
    }
    close(fd);

    return 0;
}

/*
 * search_guide()
 * warning: returns a pointer to self allocated memory, needs to be freed.
 */

btmp_t *
mono_search_guide()
{

    btmp_t *record = NULL, *p = NULL;
    int i, count = 0, total_sgnumber = 0, sysguide_number = 0;

    if (!shm) {
	mono_errno = E_NOSHM;
	return NULL;
    }
    i = shm->first;
    while (i != -1) {
	p = &(shm->wholist[i]);
	if (_mono_guide_ok(p))
	    total_sgnumber++;
	i = p->next;
    }

    if (total_sgnumber < 1)
	return NULL;

    sysguide_number = rand() % total_sgnumber;

    i = shm->first;
    while (i != -1) {
       p = &(shm->wholist[i]);
          if ( _mono_guide_ok(p)) {
              if( count == sysguide_number ) {
   	          record = (btmp_t *) xcalloc(1, sizeof(btmp_t));
	          memcpy(record, p, sizeof(btmp_t));
	          return record;
              }
              count++;
	  }
	  i = p->next;
     }
     return NULL;
}

static int
_mono_guide_ok(btmp_t *guide) {

    if ((guide->flags & B_GUIDEFLAGGED)
        && ((guide->flags & B_XDISABLED) == 0) &&
        (guide->pid != getpid()))
        return 1;
    return 0;
}

/* --------------------------------------------------------------------- */
/* this function returns a pointer to a certain users x-slot */
express_t *
mono_find_xslot(const char *name)
{
    int p;

    if (!shm) {
	mono_errno = E_NOSHM;
	return NULL;
    }
    p = shm->first;
    while (p != -1) {
	if (EQ(name, shm->wholist[p].username))
	    return &(shm->xslot[p]);
	p = shm->wholist[p].next;
    }
    return NULL;
}

static int
_mono_add_to_linked_list(btmp_t user)
{
    int i, p, q;
    char *my_name;

    mono_lock_shm(WHO_LOCK);

    /* first find an empty slot. */
    for (i = 0, p = -1; i < MAXUSERS; i++)
	if (strlen(shm->wholist[i].username) == 0) {	/* empty slot */
	    p = i;
	    break;
	}
    if (p == -1) {
	mono_lock_shm(WHO_UNLOCK);
	return -1;		/* couldn't find empty slot */
    }
    /* copy into slot */
    shm->wholist[p] = user;
    shm->wholist[p].next = -1;

    /* change list pointers */
    /* find last element in list first  */

    if (shm->first == -1)
	shm->first = p;
    else {
	q = shm->first;
	while (shm->wholist[q].next != -1)
	    q = shm->wholist[q].next;
	shm->wholist[q].next = p;
    }
    /* done */
    shm->user_count++;
    mono_lock_shm(WHO_UNLOCK);

    my_name = (char *) xmalloc(sizeof(char) * (L_USERNAME + 1));
    strcpy(my_name, user.username);
    who_am_i(my_name);
    xfree(my_name);
    return 0;
}

char *
who_am_i(const char * name)
{
    static char myname[L_USERNAME + 1];
    static int i_am_me = 0;

    if (!i_am_me) {  /* heh, not feeling like myself today.. */
	strcpy(myname, name);
	i_am_me = 1;
    }

    return myname;
}
    
    
static int
_mono_remove_from_linked_list(const char *user)
{
    int p, q;

    mono_lock_shm(WHO_LOCK);

    p = q = shm->first;
    while (p != -1) {
	if (EQ(shm->wholist[p].username, user)) {
	    strcpy(shm->wholist[p].username, "");
	    shm->user_count--;
	    if (p == shm->first)
		shm->first = shm->wholist[p].next;
	    else
		shm->wholist[q].next = shm->wholist[p].next;
	    mono_lock_shm(WHO_UNLOCK);
	    return 0;
	}
	q = p;
	p = shm->wholist[p].next;
    }
    mono_lock_shm(WHO_UNLOCK);
    return -1;
}

int
mono_remove_ghosts()
{
    int p, q;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    p = q = shm->first;
    while (p != -1) {
	if (kill(shm->wholist[p].pid, 0) == -1) {
	    strcpy(shm->wholist[p].username, "");
	    if (p == shm->first)
		shm->first = shm->wholist[p].next;
	    else
		shm->wholist[q].next = shm->wholist[p].next;
	    shm->user_count--;
	}
	q = p;
	p = shm->wholist[p].next;
    }
    return 0;
}

static int
_mono_initialize_holodeck()
{
    FILE *fp;
    char buffer[51];
    int i = 0;

    fp = xfopen(CHATFILE, "r", FALSE);
    if (fp == NULL) {
	mono_errno = E_NOFILE;
	return -1;
    }
    while (fgets(buffer, 50, fp) != NULL) {
	buffer[strlen(buffer) - 1] = '\0';
	strcpy(shm->holodeck[i].name, strtok(buffer, "|"));
	if (i < 4)
	    shm->holodeck[i].type = 'N';
	else if (i == 7)
	    shm->holodeck[i].type = 'P';
	else
	    shm->holodeck[i].type = 'A';
	i++;
    }
    fclose(fp);
    return 0;
}

int
mono_find_x_ing(const char *name, char *xer)
{
    int i;

    if (!shm) {
	mono_errno = E_NOSHM;
	return -1;
    }
    mono_lock_shm(WHO_LOCK);

    strcpy(xer, "");

    i = shm->first;
    while (i != -1) {
	if (EQ(shm->wholist[i].x_ing_to, name) || EQ(shm->wholist[i].x_ing_to, "Everybody")) {
	    strcpy(xer, shm->wholist[i].username);
	    break;
	}
	i = shm->wholist[i].next;
    }
    mono_lock_shm(WHO_UNLOCK);
    if (*xer)
	return 0;
    else
	return -1;

}
/* eof */
