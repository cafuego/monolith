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

int main(void);

int
main()
{

    int fd1, i, j, h;
    room_t old;
    user_t u;
    fd1 = open( BBSDIR "save/fastquadrant", O_RDONLY);


    mono_sql_connect();

    for (i = 0; i < 150; i++) {

	read(fd1, &old, sizeof(room_t));
        for( j = 0; j < 5; j++ ) {
            if(check_user(old.qls[i])) {
                u = readuser(old.qls[j]);
                h = u.usernum;
                mono_sql_query("INSERT INTO userforum VALUES(%d,%d,'y','normal','0')", i, h);
            }
        }
       
    }
    mono_sql_detach();
    close(fd1);
    return 0;
}
