/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#include "yawc.h"

#include "btmp.h"

int 
main()
{
    int i;

    connect_shm();

    for (i = 0; i < MAXROOMS; i++) {
	shm->rooms[i].lowest = shm->rooms[i].highest - shm->rooms[i].maxmsg;
    }
    detach_shm();
}
