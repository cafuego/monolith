/* forums to sql */
/* a conversiion program */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <sys/types.h>
#include <stdio.h>

#include MYSQL_HEADER

#include "monolith.h"
#include "libmono.h"


int
main()
{
    unsigned int i;
    int ret;
    mono_connect_shm();
    for (i = 0; i < MAXQUADS; i++) {
       printf("%u: %lu\n", i, shm->rooms[i].highest );
    }
    mono_detach_shm();

    return 0;
}
