/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <string.h>

#ifdef HAVE_MYSQL_H
#undef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#else
#ifdef HAVE_MYSQL_MYSQL_H
#undef HAVE_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif
 
#include "sql_goto.h"

#define HERO_GENERATOR 1

#ifdef HERO_GENERATOR

char *heroname[] =
{
    "Alien",
    "America",
    "Android",
    "Angel",
    "Ant",
    "Ape",
    "Armadillo",
    "Arrow",
    "Atom",
    "Avenger",
    "Baby",
    "Bat",
    "Beast",
    "Bee",
    "Beetle",
    "Bird",
    "Blade",
    "Blur",
    "Bolt",
    "Boy",
    "Brain",
    "Bug",
    "Bullet",
    "Canary",
    "Cavalier",
    "Centurion",
    "Chameleon",
    "Champion",
    "Chimp",
    "Claw",
    "Condor",
    "Dancer",
    "Dazzler",
    "Defender",
    "Demon",
    "Devil",
    "Dog",
    "Dragon",
    "Droid",
    "Duke",
    "Eagle",
    "Emperor",
    "Empress",
    "Enigma",
    "Eye",
    "Falcon",
    "Fang",
    "Finger",
    "Flame",
    "Flash",
    "Flea",
    "Flower",
    "Fly",
    "Fox",
    "Freak",
    "Frog",
    "Fury",
    "F|ghter",
    "F|re",
    "Gal",
    "Ghost",
    "Girl",
    "Glory",
    "Gol|ath",
    "Gorilla",
    "Guardian",
    "Guy",
    "G|ant",
    "Hammer",
    "Hawk",
    "Hornet",
    "Inferno",
    "Jack",
    "Jet",
    "Kid",
    "King",
    "Kn|ght",
    "Lad",
    "Lady",
    "Lantern",
    "Lass",
    "Lord",
    "L|ght",
    "L|ghtning",
    "L|on",
    "Man",
    "Mane",
    "Manhunter",
    "Mariner",
    "Marvel",
    "Mask",
    "Master",
    "Mistress",
    "Mouse",
    "Ninja",
    "Nova",
    "Panther",
    "Phantom",
    "Platypus",
    "Prince",
    "Princess",
    "Protector",
    "Prowler",
    "Punisher",
    "Queen",
    "Racer",
    "Racoon",
    "Ranger",
    "Rat",
    "Robot",
    "Rocket",
    "R|der",
    "Samurai",
    "Shadow",
    "Shield",
    "Shrike",
    "Singer",
    "Skier",
    "Slayer",
    "Soarer",
    "Sorcerer",
    "Sorceress",
    "Spear",
    "Specter",
    "Sp|der",
    "Squid",
    "Squirrel",
    "Stalker",
    "Star",
    "Storm",
    "Surfer",
    "Sword",
    "S|dekick",
    "Torpedo",
    "T|ger",
    "T|tan",
    "Victory",
    "Viking",
    "Vision",
    "Walker",
    "Warlock",
    "Warrior",
    "Wave",
    "Wing",
    "Witch",
    "Wizard",
    "Wolf",
    "Woman",
    "Wombat",
    "Wonder",
    "Worm",
    "X",
    "Yak"
};
char *herotitle[] =
{
    "Air ",
    "Alien ",
    "Alpha ",
    "Ambush ",
    "Android ",
    "Animal ",
    "Ant ",
    "Aqua-",
    "Arch-",
    "Armadillo ",
    "Astro-",
    "Atomic ",
    "Azure ",
    "B'Wana ",
    "Bat ",
    "Bee ",
    "Beta ",
    "Bionic ",
    "Black ",
    "Blood ",
    "Blue ",
    "Bronze ",
    "Bullet ",
    "Caped ",
    "Captain ",
    "Cat ",
    "Chameleon ",
    "Commander ",
    "Commodore ",
    "Composite ",
    "Compu-",
    "Copper ",
    "Cosmic ",
    "Crimson ",
    "Cyber-",
    "Dare-",
    "Dark ",
    "Dawn ",
    "Death ",
    "Delta ",
    "Detective ",
    "Doc ",
    "Doctor ",
    "Dog ",
    "Dragon ",
    "Dream ",
    "Duke ",
    "Dynamo ",
    "Earth ",
    "Elasti-",
    "Element ",
    "Emerald ",
    "Fly ",
    "Flying ",
    "Frog ",
    "Future ",
    "F|ghting ",
    "F|re ",
    "Gamma ",
    "Gaseous ",
    "General ",
    "Ghost ",
    "Gold ",
    "Golden ",
    "Gorilla ",
    "Green ",
    "Grey ",
    "G|ant ",
    "Hawk ",
    "Human ",
    "Hyper-",
    "Ice ",
    "Insect ",
    "Invisible ",
    "Iron ",
    "Jade ",
    "Karate ",
    "Laser ",
    "Lieutenant ",
    "Living ",
    "L|ght ",
    "L|ghtning ",
    "L|on ",
    "Machine ",
    "Mad ",
    "Magna-",
    "Magnetic ",
    "Major ",
    "Martian ",
    "Masked ",
    "Mighty ",
    "Miss ",
    "Mister ",
    "Moon ",
    "M|cro-",
    "M|nd ",
    "N|ght ",
    "Omega ",
    "Orange ",
    "Phantom ",
    "Platinum ",
    "Platypus ",
    "Power ",
    "Prince ",
    "Princess ",
    "Professor ",
    "Purple ",
    "Racoon ",
    "Rainbow ",
    "Rat ",
    "Red ",
    "Ring ",
    "Robot ",
    "Rocket ",
    "Ruby ",
    "Samurai ",
    "Sand ",
    "Scarlet ",
    "Screaming ",
    "Sea ",
    "Seagoing ",
    "Secret ",
    "Sergeant ",
    "Shadow ",
    "Shatter ",
    "She-",
    "Shrinking ",
    "Sh|ning ",
    "Silver ",
    "Sky ",
    "Snow ",
    "Space ",
    "Speed ",
    "Sp|der ",
    "Squirrel ",
    "Star ",
    "Steel ",
    "Sub-",
    "Suic|de ",
    "Sun ",
    "Super ",
    "Techni-",
    "Terra-",
    "Thunder ",
    "Tomorrow ",
    "T|me ",
    "Ultra ",
    "Valiant ",
    "V|olet ",
    "Warrior ",
    "Water ",
    "Wh|te ",
    "Wind ",
    "Wing ",
    "Winged ",
    "Winter ",
    "Wolf ",
    "Wombat ",
    "Wonder ",
    "X-",
    "Yak ",
    "Yellow "
};
char *heroadjective[] =
{
    "$6 million", "adjectiveless", "amazing", "astonishing",
    "astounding", "athletic", "bewildering", "bizarre",
    "courteous", "dazzling", "electrifying", "enigmatic",
    "ever-loving, blue-eyed", "famous", "fantastic", "fearless",
    "friendly, neighborhood", "grim 'n' gritty", "impossible",
    "incredible", "indescribable", "intangible", "invincible",
    "invulnerable", "irresistable", "kool", "marvelous", "mighty",
    "morphin'", "noble", "perplexing", "polite", "savage",
    "sensational", "spectacular", "spellbinding", "strange",
    "stupendous", "super-intelligent", "unbeatable", "uncanny",
    "valiant", "valorous", "weird", "wonderous", "evil"
};
char *herovehicle[] =
{
    "Bike", "Boat", "Broom", "Buggy", "Car", "Chariot", "Cycle",
    "Dragon", "Glider", "Horse", "Hovercraft", "Hydrofoil", "Jet",
    "Plane", "Pogo Stick", "Rocket", "Rollerblades", "Saucer",
    "Ship", "Shuttle", "Skates", "Skis", "Sled", "Starship",
    "Stilts", "Submarine", "Surfboard", "Van", "Wings"
};
char *herotype[] =
{
    "Admantium", "Air", "Anti-matter", "Cosmic", "Energy", "Ether",
    "Flash", "Foam", "Force", "Glue", "Gravity", "Laser", "Light",
    "Magic", "Magnetic", "Mystic", "Radiation", "Sonic", "Star",
    "Stellar", "Strobe", "Trick", "Vibranium", "Web"
};
char *heroweapon[] =
{
    "Armor", "Arrows", "Battleaxe", "Blaster", "Boomerang", "Bow",
    "Bullets", "Cannon", "Chain", "Claws", "Crossbow", "Dagger",
    "Discs", "Gun", "Hammer", "Knife", "Lance", "Lasso", "Needles",
    "Neutralizer", "Nunchucks", "Pistol", "Rod", "Sai", "Shield",
    "Shooter", "Shotgun", "Spear", "Staff", "Sword", "Thorns",
    "Throwing Stars"
};
char *heropower[] =
{
    "Chemical", "Chronal", "Cybernetics", "Extra-dimensional",
    "Extra-terrestrial", "Gadgets", "Magical", "Meditation",
    "Metahuman", "Mutagen", "Mutant", "Mystical", "Mythical God(ess)",
    "None", "Paranormal", "Psychic", "Radiation", "Solar", "Spinach",
    "Super-goobers", "Supernatural", "Technological", "Training",
    "Unknown", "Quaaludes"
};

#endif /* HERO_GENERATOR */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"
#include "ext.h"
#include "setup.h"

#define extern
#include "fun.h"
#undef extern

#include "input.h"
#include "routines2.h"

void
random_goto()
{

    char *thegoto;
  
    if((rand() % 10) == 1) {
	thegoto = mono_sql_random_goto();
        if(strlen(thegoto))
            cprintf("\1f\1g%s\1a", thegoto);
	else
            cprintf("\1f\1gNo unread %s.\1a", config.message_pl );
	xfree(thegoto);
    } else {
        cprintf("\1f\1gNo unread %s.\1a", config.message_pl );
    }

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
	case 3:
	    generate_hero();
	    return;
    }

    cprintf("\n");
    system(command);
    fflush(stdout);
    return;
}

void
generate_hero()
{

    cprintf("\1f\1gName           \1w: \1yThe %s %s%s.\n", heroadjective[rand() % ADJNUM], herotitle[rand() % TITNUM], heroname[rand() % NAMNUM]);
    cprintf("\1f\1gSource of power\1w: \1y%s.\n", heropower[rand() % POWNUM]);
    cprintf("\1f\1gWeapon         \1w: \1y%s %s.\n", herotype[rand() % TYPNUM], heroweapon[rand() % WEANUM]);
    cprintf("\1f\1gTransportation \1w: \1y%s.\n", herovehicle[rand() % VEHNUM]);

    return;
}

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
