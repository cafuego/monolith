/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <mysql.h>

#include "monolith.h"
#include "sql_utils.h"
#include "sql_user.h"

int main(void);

int
main()
{

// #define FIRST_RUN

#ifdef FIRST_RUN
    unsigned int y, z, highest_usernum;
    char bing[L_USERNAME + 1];
    MYSQL_ROW row;
#else
    int i, j, h;
    size_t ret;
    room_t old;
#endif

    int fd1;
    MYSQL_RES *res;

    fd1 = open(BBSDIR "save/fastquadrant", O_RDONLY);

    if (fd1 == -1) {
	fprintf(stderr, "can't open fastquadrant");
	exit(1);
    }
    mono_sql_connect();

#ifndef FIRST_RUN

    printf("got here\n");
    fflush(stdout);

    for (i = 0; i < MAXQUADS; i++) {
	printf("reading quad %d...", i);
	fflush(stdout);

	ret = read(fd1, &old, sizeof(room_t));
	if (ret != sizeof(room_t)) {
	    fprintf(stderr, "can't read from fastquadrant");
	    exit(1);
	}
	printf("read..");
	fflush(stdout);
	for (j = 0; j < 5; j++) {
	    if (old.qls[j] && strlen(old.qls[j]))
		if (1) {
		    printf("ql %d exists: ", j);
		    fflush(stdout);
		    if (mono_sql_u_name2id(old.qls[j], &h) != -1) {
			printf("id %u, ", h);
		        mono_sql_query(&res, "UPDATE userforum SET host='y' WHERE user_id=%u AND forum_id=%u", h, i);
		    }
		}
	}



	printf("done\n");
	fflush(stdout);

    } 

#endif
#ifdef FIRST_RUN

    if (mono_sql_query(&res, "SELECT MAX(id) from user") == -1)
	highest_usernum = 0;
    else {
        row = mysql_fetch_row(res);
	if (row)
            sscanf(row[0], "%u", &highest_usernum);
    }

    for (z = 0; z < MAXQUADS; z++) 
        for (y = 1; highest_usernum && y <= highest_usernum; y++)
            if (mono_sql_u_id2name(y, bing) == -1)
                continue;
            else if (mono_sql_query(&res, 
                    "INSERT INTO userforum (user_id,forum_id) VALUES (%u,%u)"
                    , y, z) == -1)
                printf("\nInsert error");
#endif

    mono_sql_detach();
    close(fd1);
    return 0;
}

