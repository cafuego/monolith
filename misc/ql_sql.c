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

#include <mysql.h>

#include "monolith.h"

int main(void);

int
main()
{

    int fd1, i, j, h;
    size_t ret;
    room_t old;
    user_t u;
    MYSQL_RES *res;
    fd1 = open(BBSDIR "save/fastquadrant", O_RDONLY);

    if (fd1 == -1) {
	fprintf(stderr, "can't open fastquadrant");
	exit(1);
    }
    mono_sql_connect();

    printf( "got here\n" );fflush(stdout);

    for (i = 0; i < MAXQUADS ; i++) {
        printf( "reading quad %d...", i );fflush(stdout);

	ret = read(fd1, &old, sizeof(room_t));
        if ( ret != sizeof( room_t )  ) {
              fprintf( stderr, "can't read from fastquadrant" );
            exit (1 );
       }
        printf( "read.." );fflush(stdout);
	for (j = 0; j < 5; j++) {
	    if (old.qls[j] && strlen(old.qls[j]))
		if (check_user(old.qls[j])) {
                    printf( "ql %d exists: ", j ); fflush(stdout);
		    if (mono_sql_u_name2id(old.qls[j], &h) != -1)
   		        printf( "id %u, ",  h );
			mono_sql_query( &res, "UPDATE userforum SET host='y' WHERE user_id=%u AND forum_id=%u", h, i);
		}
	}
        printf( "done\n" );fflush(stdout);

    }
    mono_sql_detach();
    close(fd1);
    return 0;
}
