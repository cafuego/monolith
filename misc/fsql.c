/* forums to sql */
/* a conversiion program */

#include <sys/types.h>
#include <stdio.h>

#include <mysql.h>

#include "monolith.h"
#include "libmono.h"


int
main()
{
    unsigned int i;
    int ret;

    mono_sql_connect();
    mono_connect_shm();

    for (i = 0; i < MAXQUADS; i++) {

      ret=   mono_sql_f_write_quad( i, &shm->rooms[i] );
      if ( ret == -1 ) {
          fprintf( stderr," error!\n" );
          break;
      }

    }
    mono_sql_detach();
    mono_detach_shm();

    return 0;
}
