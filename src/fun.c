/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
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
#include "libshix.h"

static void cthulhu( void );
static int _cthulhu_bad(int i, char *str);

void
random_goto()
{
    char *thegoto;
    int ret;
    int timescalled;

    if((rand() % 10) == 1) {
	thegoto = mono_sql_random_goto();
        if(strlen(thegoto) && thegoto != NULL)
            cprintf("\1f\1g%s\1a", thegoto);
	else
            cprintf(_("\1f\1gNo unread messages.\1a") );
	xfree(thegoto);

    } else if ((rand() % 1000) == 666 ) {
        ret = mono_sql_u_get_login_count( usersupp->usernum, &timescalled );
        if ( ret == 0 && timescalled > 99 ) 
            cthulhu();
	else
        cprintf(_("\1f\1gNo unread messages.\1a"));
    } else
        cprintf(_("\1f\1gNo unread messages.\1a"));

    return;
}


/*
 * Improved cthulhu now knows regular expressions :)
 */
static char *munchies[] = {
    "cookie", "petit four", "biscuit", "waffle",
    "piece of pie", "kiss", "biscotti", "butterfinger",
    "cute little kitten"
};
static char *munchmatch[] = {
    "[Cc][Oo][Kk][Ii][Ee]", "[Pp][Ee][Tt][Ii][Tt] [Ff][Oo][Uu][Rr]",
    "[Bb][Ii][Ss][Cc][Uu][Ii][Tt]", "[Ww][Aa][Ff][Ff][Ll][Ee]",
    "[Pp][Ii][Ee][Cc][Ee] [Oo][Ff] [Pp][Ii][Ee]", "  ",
    "[Bb][Ii][Ss][Cc][Oo][Tt][Tt][Ii]", "[Bb][Uu][Tt][Tt][Ee][Rr][Ff][Ii][Nn][Gg][Ee][Rr]",
    "[Cc][Uu][Tt][Ee] [Ll][Ii][Tt][Tt][Ll][Ee] [Kk][Ii][Tt][Tt][Ee][Nn]"
};

static char *no = "[Nn][Oo]";
static char *why = "[Ww][Hh][Yy][?. ]";
static char *question = ".*[?]$";

/*
 * Getting scarier by the hour today.
 */
static void
cthulhu()
{
    char april_fools[65];
    int food = 0;

    food = rand() % 7;
    strcpy(april_fools, "");

    /*
     * Do some scary matching in the while() loop
     * to make sure we get what we need. :)
     */
    do {
        cprintf("\n\1w666.\1yCthulhu\1w> \1rGimme a %s! \1w", munchies[food]);
        getline(april_fools, 64, 0);
    } while( _cthulhu_bad(food, april_fools));

    april_fools[ strlen(april_fools) ] = '\0';

    if(food == 5) 
        cprintf(_("\n\1gI love you too! ;)\n"));
    else if(food == 8) 
        cprintf(_("\n\1y*MEOW* :-)\n"));
    else if( shix_strmatch(april_fools, why) )
        cprintf(_("\n\1y*whine* Coz I need one! )~:\n"));
    else if( shix_strmatch(april_fools, no))
        cprintf(_("\n\1g*pout*  ):\n"));
    else
        cprintf(_("\n\1g*buurrRp*  (:\n"));

    return;
}

static int
_cthulhu_bad( int i, char *str )
{

    switch(i) {
        case 5:		/* Kiss! */
            if (shix_strmatch(str,".*[Ss][Mm][Oo][Oo][Cc][Hh].*")) return FALSE;
            if (shix_strmatch(str,".*[Kk][Ii][Ss][Ss]")) return FALSE;
            return TRUE;
            break;

        default:	/* food */
            if ((shix_strmatch(str,no)) && (shix_strmatch(str,why))) return TRUE;
            if (shix_strmatch(str,no)) return FALSE;
            if (shix_strmatch(str,why)) return FALSE;
            if (shix_strmatch(str,munchmatch[i]) && (!shix_strmatch(str,question)) && (!shix_strmatch(str,no))) return FALSE;
            return TRUE;
            break;
    }
    return FALSE;

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
