/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdbm.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNTERFILE  "/usr/bbs/www/cgi-bin/counter.dbm"
#define ERRORMSG "&lt;unknown, counter error&gt;"

void error(void);
int main(int argc, char **argv);

extern gdbm_error gdbm_errno;

int
main(int argc, char **argv)
{
    char *document, countstr[40];
    GDBM_FILE dbf;
    datum key, content;
    int count = 0;

    printf("Content-type: text/html\n\n");

    /* get key */
    document = getenv("DOCUMENT_URI");
    if (document == NULL)
	error();

    key.dptr = document;
    key.dsize = strlen(document) + 1;

    dbf = gdbm_open(COUNTERFILE, 512, GDBM_READER, 0600, 0);
    /* it could be the first time we are accessed, do not exit when
     * we don't read any data...  */
    if (dbf != NULL) {
	if (gdbm_exists(dbf, content)) {
	    printf("YES");
	}
	content = gdbm_fetch(dbf, key);

	if (content.dptr != NULL) {
	    count = atoi(content.dptr);
	    free(content.dptr);
	} else {
	    count = 0;
	    printf("[error: Couldn't read data file]");
	}
	gdbm_close(dbf);
    } else
	count = 0;
    count++;
    sprintf(countstr, "%d", count);
    content.dptr = countstr;
    content.dsize = strlen(countstr) + 1;

    dbf = gdbm_open(COUNTERFILE, 512, GDBM_WRCREAT, 0600, 0);
    if (dbf != NULL) {
	if (gdbm_store(dbf, key, content, GDBM_REPLACE) != 0) {
	    printf("[error: couldn't store data]");
	}
    } else {
	printf("[error: couldn't open file for writing]");
    }
    printf("%d", count);

    exit(0);
}

void
error()
{
    printf("%s", ERRORMSG);
    exit(0);

}

/* eof */
