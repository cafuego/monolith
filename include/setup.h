/* $Id$ */

/* name definitions */
#define BBSNAME			"Monolith BBS"	/* full name of the BBS */
#define FORUM			"Quadrant"	/* name for a forum */
#define FORUM_PL		"Quadrants"	/* plural */
#define MESSAGE			"message"	/* name for a post */
#define MESSAGE_PL		"messages"	/* plural */
#define EXPRESS			"eXpress"	/* title for express */
#define X_MESSAGE		"message"	/* name for express */
#define X_MESSAGE_PL		"messages"	/* plural */
#define USER			"Alien"		/* title for user */
#define USER_PL			"Aliens"	/* plural */
#define USERNAME		"species"	/* used in the wholist header in combination with USER */
#define DOING			"Flying"	/* name for doing */
#define LOCATION		"Planet"	/* name for location in wholist */
#define CHATMODE		"Holodeck"	/* name for chatmode */
#define CHATROOM		"programme"	/* name for chatroom */

/* title definitions */
#define ADMIN			"Admin"
#define WIZARD			"Emperor"
#define SYSOP			"Fleet Commander"
#define PROGRAMMER		"Systems Analist"
#define ROOMAIDE		"Quadrant Leader"
#define GUIDE			"Help Terminal"

/* title definitions for profiles */
#define EXTADMINTITLE		BOLD ADMINCOL "(** " ADMIN " **)"
#define EXTWIZARDTITLE		BOLD WIZARDCOL "(*** " WIZARD " ***)"
#define EXTSYSOPTITLE		BOLD SYSOPCOL "(** " SYSOP " **)"
#define EXTPROGRAMMERTITLE	BOLD PROGRAMMERCOL "(* " PROGRAMMER " *)"
#define EXTROOMAIDETITLE	BOLD ROOMAIDECOL "(* " ROOMAIDE " *)"
#define EXTGUIDETITLE		BOLD GUIDECOL "( " GUIDE " )"

/* title definitions for titled posts  and eXpress messages */
#define ADMINTITLE		BOLD WHITE "( " ADMINCOL ADMIN WHITE " )"
#define WIZARDTITLE		BOLD WHITE "( " WIZARDCOL WIZARD WHITE " )"
#define SYSOPTITLE		BOLD WHITE "( " SYSOPCOL SYSOP WHITE " )"
#define PROGRAMMERTITLE		BOLD WHITE "( " PROGRAMMERCOL PROGRAMMER WHITE " )"
#define ROOMAIDETITLE		BOLD WHITE "( " ROOMAIDECOL ROOMAIDE WHITE " )"
#define GUIDETITLE		BOLD WHITE "( " GUIDECOL GUIDE WHITE " )"

/* colour definitions */
#define ADMINCOL		WHITE
#define WIZARDCOL		WHITE
#define SYSOPCOL		PURPLE
#define PROGRAMMERCOL		BLUE
#define ROOMAIDECOL		RED
#define GUIDECOL		CYAN
#define USERCOL			GREEN
#define FRIENDCOL		CYAN		/* colour of friends in the wholist */

/* forum colours */
#define NORMAL_COL		GREEN		/* normal forum */
#define ANON_COL		PURPLE		/* anon only/option forum */
#define PRIVATE_COL		RED		/* invite inly forum */

/* colours -- don't change this */
#define BOLD			"\1f"
#define WHITE			"\1w"
#define CYAN			"\1c"
#define YELLOW			"\1y"
#define GREEN			"\1g"
#define BLUE			"\1b"
#define PURPLE			"\1p"
#define RED			"\1r"
#define DARK			"\1d"
#define NONE			"\1a"
