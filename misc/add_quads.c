/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "monolith.h"

typedef struct {
    char name[L_QUADNAME + 1];	/* Max. len is 40, plus null term       */
    char qls[NO_OF_QLS][L_USERNAME + 1];	/* RA's username                */
    char category[60];		/* room category */
    long highest;		/* Highest message NUMBER in room       */
    long lowest;		/* This was the dirname before. notused */
    char generation;		/* Generation number of room            */
    unsigned flags;		/* See flag values below                */
    char roominfo;		/* RoomInfo-checknumber                 */
    int maxmsg;			/* Max number of messages in a quad     */
} new_room_t;

int main(void);
new_room_t convert_room(room_t);

int
main()
{

    int fd1, fd2, i;
    room_t old;
    new_room_t new;
    fd1 = open( BBSDIR "save/fastquadrant", O_RDONLY);
    fd2 = open( BBSDIR "save/fastquadrant.new", O_WRONLY | O_CREAT, 0600);


    for (i = 0; i < 150; i++) {

	read(fd1, &old, sizeof(room_t));
/*        printf("Reading room %d (%s).\n", i, old.name); */
	new = convert_room(old);
	write(fd2, &new, sizeof(new_room_t));
    }
    close(fd1);
    close(fd2);
    return 0;
}


new_room_t
convert_room(room_t old)
{

    int i = 0;
    new_room_t new;

    memset(&new, 0, sizeof(new_room_t));

    strcpy(new.name, old.name);
    for (i = 0; i < 5; i++) {
	strcpy(new.qls[i], old.qls[i]);
	if (strlen(new.qls[i]))
	    printf("Moving %s.QL[%d] (%s) to new.\n", new.name, i, new.qls[i]);
	fflush(stdout);
    }
    strcpy(new.category, "");
    new.highest = old.highest;
    new.lowest = old.lowest;
    new.generation = old.generation;
    new.flags = old.flags;
    new.roominfo = old.roominfo;
    new.maxmsg = old.maxmsg;

    return new;
}
