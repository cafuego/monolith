/* $Id$ */

#include "extra.h"

#define BS		8	/* char: backspace			*/
#define CR		13	/* char: carriage return		*/
#define CTRL_A		1
#define CTRL_D		4	
#define CTRL_E		5
#define CTRL_R		18
#define CTRL_U		21
#define CTRL_W		23
#define CTRL_X		24
#define DEL		127
#define ESC		27
#define FALSE		0
#define FF		255
#define LF		'\n'	/* char: linefeed			*/
#define MAXNAME		40	/* maximum roomnamelength		*/
#define MAXUSERS	75	/* ABSOLUTE(!) maxusers online		*/
#define MAXQUEUED       10	/* number of maxslots in Queue		*/
#define MAXCHATROOMS	10	/* max number of chat rooms */
#define NL		LF	/* char: newline			*/
#define NO		0
#define PURGEDAY	90	/* purge inactive users			*/
#define SP		32	/* char: space				*/
#define TAB		9	/* tab is 9 in ascii			*/
#define TIMEOUT		24	/* timeout&hangup after X mins		*/
#define TIMEOUTCLIENT	39	/* timeout for all client-users		*/
#define TRUE		1
#define X_LENGTH	7	/* length in lines of x-message		*/
#define XLIMIT		60	/* how many X's to store in the XLog	*/
#define X_MARGIN	85	/* 80 characters / row plus some extra	*/
#define X_BUFFER	X_MARGIN * X_LENGTH
#define YES		1

	/********************/

#define B_XDISABLED	1	/* disabled from X's			*/
#define B_GUIDEFLAGGED	2	/* has the guideflag on			*/
#define B_TWOMINIDLE	4	/* is idle for >= 2 mins -> '#'		*/
#define B_REALIDLE	8	/* is idle fot >= 5 mins -> "Idle for.."*/
#define B_POSTING	16	/* is posting		 -> '+'		*/
#define B_CLIENTUSER	32	/* is using the CLient	 -> '-'		*/
#define B_DONATOR       64      /* is a donator  ->  '$'                */
/* #define B_UNUSED    128       * is cloaked (emps only)               */
#define B_LOCK		256     /* this user has locked their terminal  */
#define B_AWAY	       512	/* this user is afk */
#define B_FRIENDS_DISABLED 1024     /* this user has friends disabled  */
#define B_INFOUPDATED	2048	/* profile was changed within last 2 days */

	/********************/

/* #define C_UNUSED	1	 * look above [NOTUSED]			*/
#define C_XDISABLED	2	/* used for me to "know" I'm disabled	*/
#define C_ROOMLOCK	4	/* used for <m>isc <r>oomlock's		*/
#define C_NOSILC	8	/* receive no SILC messages 		*/
#define C_AWAY         16	/* is away, (same as B_AWAY) 		*/
#define C_LOCK         32	/* is locked, (same as B_LOCK) 		*/

	/********************/

#define L_FRIEND	1	/* this user is a friend */
#define L_ENEMY		2	/* this user is an enemy */
#define L_INFORM	4	/* receive login/out notifes about this user */

	/********************/

#define MES_NORMAL      65      /* Normal message                       */
#define MES_ANON        66      /* "*****" header                       */
#define MES_AN2         67      /* "Anonymous" header                   */
#define MES_DESC        68      /* Room description "message"		*/
#define MES_FORCED	69	/* forced message (yell)		*/
#define MES_ROOMAIDE    70      /* RoomAide(writer)                     */
#define MES_TECHNICIAN	71	/* Technician(writer)			*/
#define MES_SYSOP       72      /* Sysop(writer)                        */
#define MES_WIZARD      73      /* Wizard(writer)                       */
#define MES_GAMEMASTER	74	/* Game master rolls dice for roleplaying */
#define MES_SYSTEM	75	/* system post, by the computer		*/

	/********************/

#define PRIV_DELETED	1	/* Deleted				*/
#define PRIV_TWIT	2	/* Twit					*/
#define PRIV_DEGRADED	4	/* Degraded user ("unvalidated")	*/
#define PRIV_VALIDATED  8       /* this user is validated */
#define PRIV_CHATMODE	16	/* Allowed in ChatMode			*/
#define PRIV_PREFERRED	32      /* preferred user */
#define PRIV_GUIDE	64	/* made guide a priv flag */ /* not used*/
#define PRIV_ROOMAIDE	128	/* roomaide flag */          /* not used*/
#define PRIV_NEWBIE	256
#define PRIV_UNUSED4	512
#define PRIV_UNUSED5	1024
#define PRIV_MINISYSOP	2048	/* MiniSysop	[NOTUSED]		*/
#define PRIV_TECHNICIAN	4096	/* Technician				*/
#define PRIV_SYSOP	8192	/* Sysop				*/
#define PRIV_WIZARD	16384	/* Wizard				*/
#define PRIV_EMPEROR	16384	/* Emperor / Wizard			*/
#define PRIV_EMPRESS	32768	/* Empress				*/
#define PRIV_RETIRED	65536   /* retired emperor/empress		*/

#define PRIV_ALLUNVALIDATED	(PRIV_DEGRADED | PRIV_TWIT | PRIV_DELETED)

	/********************/

#define QR_BUSY		1	/* This would be a nice addition :-)	*/
#define QR_INUSE	2	/* Set if in use, clear if available	*/
#define QR_PRIVATE	4	/* Set for any type of private room	*/
#define QR_READONLY	8	/* Restrict posting to aides?		*/
#define QR_GUESSNAME	16	/* Set if it's a guessname room		*/
#define QR_NOZAP	64	/* This room is unzappable		*/
#define QR_UNUSED2	128	/*		Unused flag		*/
#define QR_ALIASNAME	256	/* Allowed to put a name on anonposts	*/
#define QR_ANONONLY	512	/* Anonymous-Only room			*/
#define QR_ANON2	1024	/* Anonymous-Option room		*/
#define QR_SUBJECTLINE	2048	/* Set if subject-lines are allowed	*/
#define QR_PREFONLY	4096	/* Preferred status needed to enter	*/
#define QR_DESCRIBED    8192    /* this room has a description file	*/
#define QR_UNUSED4      16384   /*		Unused flag		*/

	/********************/

#define SHM_BIGBTMP	300	/* all queue'rs (not queers) after 300	*/

	/********************/

#define ULOG_NORMAL	0	/* normal logout			*/
#define ULOG_DROP	1	/* drop carrier				*/
#define ULOG_SLEEP	2	/* sleeping				*/
#define ULOG_OFF	3	/* typed "off"				*/
#define ULOG_PROBLEM	4	/* problems of some sort		*/
#define ULOG_NEW	5	/* new user's first call		*/
#define ULOG_KICKED	6	/* got kicked				*/
#define ULOG_DENIED	7	/* user got denied to login		*/

	/********************/

#define US_xxxxx	1			/* unused */
#define US_BEEP		2			/* User gets X-beeps			*/
#define US_PERM		4			/* Permanent user			*/
#define US_ANSI		8			/* User has colors enabled		*/
#define US_LASTOLD	16			/* Print last old message with new	*/
#define US_EXPERT	32			/* Experienced user			*/
#define US_PAUSE	64			/* pause after screenlength lines	*/
#define US_NOPROMPT	128			/* Don't prompt after each message	*/
#define US_NOHIDE       256			/* hide reginfo from regular users	*/
#define US_IPWHOLIST	512	 		/* Show IP numbers in the wholist	*/
#define US_REGIS	1024			/* Registered user			*/
#define US_NOFLASH      2048			/* sees no flashing ansi-colors		*/
#define US_IAMBAD       4096			/* is BAD: XLogged.			*/
#define US_GUIDE	8192			/* is a SysGuide			*/
#define US_ROOMAIDE	16384			/* is a RoomAide			*/
#define US_NOBOLDCOLORS	32768			/* bold-colors disabled			*/
#define US_XOFF		65536			/* X's disabled automatically at login	*/
#define US_COOL  	131072			/* Cools as ice!     			*/
#define US_LAG		262144			/* Not very cool                        */
#define US_DONATOR	524288			/* I am happy donator !                 */
#define US_CLOAK	1048576			/* this user is cloaked         	*/
#define US_NOTIFY_FR	2097152			/* this user wants to be notified of logins*/
#define US_NOTIFY_ALL	4194304			/* this user is invisible		*/
#define US_SHIX		8388608			/* this user uses mash 			*/
#define US_AWAY		16777216 		/* user is marked as afk		*/
#define US_HIDDENHOST	33554432 		/* user has hostname hidden in profile	*/
#define US_STATUSBAR	67108864 		/* user has statusbar enabled		*/
#define US_NOINTERXS	134217728 		/* user has interbbs access disabled	*/
#define US_CLIENT	268435456 		/* user uses the CLient			*/
#define US_ADMINHELP	536870912 		/* user has extra admin help enabled	*/
#define US_NOCMDHELP	1073741824 		/* user has command line help disabled	*/
#define US_USEALIAS	2147483648 		/* user uses the standard alias		*/

#define CO_SHOWFRIENDS	1			/* Show friends-online upon login	*/
#define CO_USEALIAS	2			/* User wants to use standard alias	*/
#define CO_WHAKFILTER	4			/* general post filter */
#define CO_NEATMESSAGES 8			/* empty lines around messages */
#define CO_EXPANDHEADER 16	/* expanded (long) headers */
#define CO_LONGDATE	32	/* long date format */
#define CO_EUROPEANDATE	64	/* european (as opposed to US) date format */
#define CO_DELETEDINFO	128	/* Notify user when encountyering deleted msg */
#define CO_MONOHEADER	256	/* traditional mono-style message header */
/*
 * rest is unused sofar
 */
#define CO_UNUSED8      256
#define CO_UNUSED9	512
#define CO_UNUSED10	1024
#define CO_UNUSED11	2048
#define CO_UNUSED12	4096
#define CO_UNUSED13	8192
#define CO_UNUSED14	16384
#define CO_UNUSED15	32768
#define CO_UNUSED16	65536
#define CO_UNUSED17  	131072
#define CO_UNUSED18	262144
#define CO_UNUSED19	524288
#define CO_UNUSED20	1048576
#define CO_UNUSED21	2097152
#define CO_UNUSED22	4194304
#define CO_UNUSED23	8388608
#define CO_UNUSED24	16777216
#define CO_UNUSED25	33554432
#define CO_UNUSED26	67108864
#define CO_UNUSED27	134217728
#define CO_UNUSED28	268435456
#define CO_UNUSED29	536870912
#define CO_UNUSED30	1073741824
#define CO_UNUSED31	2147483648

	/********************/

/* idea of status taken from ICQ */
#define STATUS_ONLINE 	0			/* normal mode */
#define STATUS_DND	1			/* do not disturb */
#define STATUS_AWAY	2			/* away for short time */
#define STATUS_NA	3			/* not available (long away) */
#define STATUS_OCCUP	4			/* occupied */
#define STATUS_INVIS	5			/* invisible, NOT USED */
#define STATUS_BUSY	6			/* busy? */

	/********************/

#define IFSYSOP		if (usersupp->priv >= PRIV_SYSOP)
#define IFNSYSOP	if (usersupp->priv < PRIV_SYSOP)
#define IFWIZARD	if (usersupp->priv & PRIV_WIZARD)  
#define IFEMPEROR	if (usersupp->priv & PRIV_EMPEROR)
#define IFEMPRESS	if (usersupp->priv & PRIV_EMPRESS)
#define IFNWIZARD	if((usersupp->priv & PRIV_WIZARD)==0)
#define IFTWIT		if (usersupp->priv & PRIV_TWIT)
#define IFNEWBIE	if (usersupp->priv & PRIV_NEWBIE)

#define IFUNVALID	if (!(usersupp->priv & PRIV_VALIDATED)) 
#define IFDELETED	if (usersupp->priv & PRIV_DELETED)
#define IFDEGRADED      if (usersupp->priv & PRIV_DEGRADED)
#define IFGUEST		if (strcmp( usersupp->username, "Guest" )==0)

#define IFEXPERT	if (usersupp->flags & US_EXPERT)
#define IFNEXPERT	if ((usersupp->flags & US_EXPERT)==0)
#define IFANSI		if (usersupp->flags & US_ANSI)

#define UPPER		0
#define LOWER		1
#define STATUS		2

#define EQ(a,b)         (strcasecmp(a,b)==0)

