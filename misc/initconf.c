#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "monolith.h"
#include "routines.h"
#include "sql_config.h"

#define BUFSIZE		3000
#define DELIM 		"|"
#define CONFIGURATIONS	BBSDIR "etc/configurations.def"

int
main( int argc, char *argv[])
{

    FILE *fp;
    char buffer[BUFSIZE];
    int i = 0;
    config_t *config;

    set_invocation_name(argv[0]);
    mono_setuid("guest");

    fp = xfopen(CONFIGURATIONS, "r", FALSE);
    if (fp == NULL)
	return FALSE;

    config = (config_t *) xmalloc( sizeof(config_t) );
    memset(config, 0, sizeof(config) );

    while (fgets(buffer, BUFSIZE, fp) != NULL) {

	/* don't parse comment lines */
	if ((buffer[0] == '#') || (buffer[0] == '-'))
	    continue;

	/* remove trailing '\n' */
	buffer[strlen(buffer) - 1] = '\0';

	strcpy(config->bbsname, strtok(buffer, DELIM));
	strcpy(config->forum, strtok(NULL, DELIM));
	strcpy(config->forum_pl, strtok(NULL, DELIM));
	strcpy(config->message, strtok(NULL, DELIM));
	strcpy(config->message_pl, strtok(NULL, DELIM));
	strcpy(config->express, strtok(NULL, DELIM));
	strcpy(config->x_message, strtok(NULL, DELIM));
	strcpy(config->x_message_pl, strtok(NULL, DELIM));
	strcpy(config->user, strtok(NULL, DELIM));
	strcpy(config->user_pl, strtok(NULL, DELIM));
	strcpy(config->username, strtok(NULL, DELIM));
	strcpy(config->doing, strtok(NULL, DELIM));
	strcpy(config->location, strtok(NULL, DELIM));
	strcpy(config->chatmode, strtok(NULL, DELIM));
	strcpy(config->chatroom, strtok(NULL, DELIM));
	strcpy(config->admin, strtok(NULL, DELIM));
	strcpy(config->wizard, strtok(NULL, DELIM));
	strcpy(config->sysop, strtok(NULL, DELIM));
	strcpy(config->programmer, strtok(NULL, DELIM));
	strcpy(config->roomaide, strtok(NULL, DELIM));
	strcpy(config->guide, strtok(NULL, DELIM));
	strcpy(config->idle, strtok(NULL, DELIM));

        if( (mono_sql_save_config(0, config)) == 0 )
            printf("Saved %s...\n", config->bbsname);
        else
            printf("Error saving %s!\n", config->bbsname);
    }
    fclose(fp);
    xfree(config);
    return TRUE;

}
