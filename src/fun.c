/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#ifdef SUPERHERO
#include "superhero.h"
#endif

#include "monolith.h"
#include "libmono.h"
#include "ext.h"
#include "setup.h"

#define extern
#include "fun.h"
#undef extern

#include "input.h"
#include "sql_goto.h"
#include "routines2.h"

void
random_goto()
{

    char *thegoto, april_fools[65];
  
    if((rand() % 10) == 1) {
	thegoto = mono_sql_random_goto();
        if(strlen(thegoto))
            cprintf("\1f\1g%s\1a", thegoto);
	else
            cprintf("\1f\1gNo unread %s.\1a", config.message_pl );
	xfree(thegoto);

    } else if ((rand() % 1000) == 666 && usersupp->timescalled > 99) { 
        strcpy(april_fools, "");
        while ( strstr(april_fools, "cookie") == NULL) {
            cprintf("\n\1w666.\1yCthulhu\1w> \1rGimme a cookie! \1w");
            getline(april_fools, 64, 1);
        }
        if((strstr(april_fools, "no ") == NULL) &&
            (strstr(april_fools, "not") == NULL) )
            cprintf("\n\1g*buurrRp*  (:\n");
        else
            cprintf("\n\1g*pout*  ):\n");

    } else
        cprintf("\1f\1gNo unread %s.\1a", config.message_pl );

    return;
}

void
add_goto()
{

    char mygoto[200];

    IFNSYSOP
        return;

    cprintf("\n\1f\1gEnter goto\1f\1w: \1c");
    getline(mygoto,100,TRUE);

    if( strlen(mygoto) > 2 ) {
        cprintf("\1f\1gAdd this goto to the database? \1w(\1gy\1w/\1gn\1w)\1c ");
        if( yesno() == YES) {
            (void) mono_sql_add_goto(mygoto);
            cprintf("\1f\1gSaved.\1a");
        } else {
            cprintf("\1f\1r Not saved.\1a");
        }
    }

    return;
}

int
roulette()
{

    unsigned int i = 0;

    cprintf("\n\1f\1b\n*** \1yWelcome to %s Russian Roulette! \1b***\1a\n\n", BBSNAME);
    fflush(stdout);
    sleep(2);
    cprintf("\1f\1gSo, you don't know if you want to log off, eh?");
    fflush(stdout);
    sleep(3);
    cprintf("\n\1f\1gI wonder...");
    fflush(stdout);
    sleep(4);
    cprintf("\1f\1gdo you feel lucky");
    fflush(stdout);
    sleep(1);
    cprintf(", punk?");
    fflush(stdout);
    sleep(2);
    cprintf("\n\1f\1gMake my day ... ");
    fflush(stdout);
    sleep(2);
    IFNEXPERT {
	i = (((int) random() % 2) + 1);
    } else {
	i = (((int) random() % 6) + 1);
    }
    if (i == 1) {
	cprintf("\1f\1rBLAM!!! ");
	fflush(stdout);
	sleep(2);
	cprintf("\1f\1r... see ya on the other side...\1a\n\n");
	fflush(stdout);
	return 1;
    } else {
	cprintf("\1f\1w*click* ");
	fflush(stdout);
	sleep(2);
	cprintf("\1f\1w... guess it's your lucky day then...\1a\n\n");
	fflush(stdout);
	return -1;
    }
}

/* eof */

void
crap(int flag, const char *type, int arg)
{

    char command[100];

    cprintf("\n\n\1f\1g");
    fflush(stdout);
    switch (flag) {

	case 1:
	    sprintf(command, "/usr/games/fortune fortunes/%s", type);
	    break;
	case 2:
	    sprintf(command, "/usr/local/bin/rant %d", arg);
	    break;
#ifdef SUPERHERO
	case 3:
	    generate_hero();
	    return;
#endif
    }

    cprintf("\n");
    system(command);
    fflush(stdout);
    return;
}

#ifdef SUPERHERO

void
generate_hero()
{

    cprintf("\1f\1gName           \1w: \1yThe %s %s%s.\n", heroadjective[rand() % ADJNUM], herotitle[rand() % TITNUM], heroname[rand() % NAMNUM]);
    cprintf("\1f\1gSource of power\1w: \1y%s.\n", heropower[rand() % POWNUM]);
    cprintf("\1f\1gWeapon         \1w: \1y%s %s.\n", herotype[rand() % TYPNUM], heroweapon[rand() % WEANUM]);
    cprintf("\1f\1gTransportation \1w: \1y%s.\n", herovehicle[rand() % VEHNUM]);

    return;
}

#endif /* SUPERHERO */

void
roll_the_bones(void)
{
    unsigned int sum = 0;
    int i, sides = 0, throws = 0;
    char boner[10];
    char *bonehead = NULL;

    cprintf("\n\n\1f\1rRolling the bones!  \1w<\1r?\1w>\1g for help : \1y");
    getline(boner, 7, 1);

    for (i = 0; i < strlen(boner); i++)		/* idiot check */
	if (((strlen(boner) > 2) || (boner[0] == '?')) &&
	  ((isdigit(boner[i])) || (boner[i] == '\n') || (boner[0] == '?') ||
	   (boner[i] == '\r') || (boner[i] == 'd')))
	    continue;
	else {
	    cprintf("\nHmmm..  can't roll them bones! \1a\n");
	    return;
	}
    /* ok..  parse */

    if (boner[0] != '?') {

	if ((bonehead = strtok(boner, "d")) != NULL)
	    if (strlen(bonehead))
		throws = atoi(bonehead);
	if ((bonehead = strtok(NULL, "d")) != NULL)
	    if (strlen(bonehead))
		sides = atoi(bonehead);

	if ((throws <= 0) || (throws > 99) || (sides <= 0) || (sides > 999)) {
	    cprintf("\nHmmm..  can't roll them bones! \1a\n");
	    return;
	}
	for (i = 0; i < throws; i++)
	    sum += ((rand() % sides) + 1);
	cprintf("\n\nResult: \1c%d\1a\n\n", sum);
	return;
    } else {
	cprintf("\n\n\1gThe format is \1w<\1y# of dice to throw\1w><\1yd\1w><\1y# of sides\1w>\1g.  For\n");
	cprintf("example, \1y3d6\1g would roll a 6 sided die three times.\n");
	roll_the_bones();
	return;
    }
}
/* eof */
