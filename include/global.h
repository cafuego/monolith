/* $Id$ */

#define QUICKROOM 		BBSDIR "/save/speedyforum"

#define ADMINLIST 		BBSDIR "/etc/UBL/adminlist"
#define ADMINLISTSYS 		BBSDIR "/etc/UBL/adminsyslist"
#define CHANGEPW 		BBSDIR "/share/messages/changepw"
#define CHATFILE		BBSDIR "/etc/chatfile"
#define CHATDOWNFILE 		BBSDIR "/etc/chatdown"
#define DELETEDDENY 		BBSDIR "/share/messages/msg_delusrdeny"
#define DEGRADEDMSG		BBSDIR "/share/messages/degraded"
#define DONATORLIST     	BBSDIR "/etc/UBL/donatorlist"
#define DOWNFILE 		BBSDIR "/etc/front.DOWN"
#define ENTERMESSAGE 		BBSDIR "/share/messages/entermsg"
#define GOODBYE 		BBSDIR "/share/messages/goodbye"
#define GUESTMSG		BBSDIR "/share/messages/guestmsg"
#define HELLODIR		BBSDIR "/share/loginscreens"
#define HELPTOPICS 		BBSDIR "/share/help/topics"
#define LOCKOUTFILE 		BBSDIR "/etc/lockouts"
#define LOGDIR			BBSDIR "/log"
#define MENUDIR			BBSDIR "/share/menuhelp"
#define NEWUSERLIST 		BBSDIR "/etc/UBL/newuserlist"
#define NEWUSERMSG 		BBSDIR "/share/messages/newuser"
#define NONEWMSG 		BBSDIR "/share/messages/nonew"
#define NOTEBOOKDIR 		BBSDIR "/etc/NoteBooks/"
#define NOTVALID 	 	BBSDIR "/share/messages/notvalid"
#define PROBLEMFILE 		BBSDIR "/etc/problems"
#define PROHIBNEWMSG 		BBSDIR "/share/messages/prohibnew"
#define PURGELIST 		BBSDIR "/etc/UBL/purgelist"
#define REGISTER 		BBSDIR "/share/newuser/registration"
#define SLEEPINGMSG 		BBSDIR "/share/messages/sleeping"
#define VICUNA			BBSDIR "/share/messages/vicuna"
#define YELLINFO		BBSDIR "/share/messages/yellmenu"
#define SYSOPLOGDIR 		BBSDIR "/log/sysop/"
#define TABOONAMES 		BBSDIR "/etc/taboonames"
#define TWITLIST 		BBSDIR "/etc/UBL/twitlist"
#define TWITMSG 		BBSDIR "/share/messages/twituserdeny"
#define UNVALIDMSG 		BBSDIR "/share/messages/unvalid"
#define USERDIR 		BBSDIR "/save/users"
#define QUOTEDIR		BBSDIR "/share/quotes"
#define QUADRANT		BBSDIR "/share/commandhelp/quadrant"
#define HELPTERM		BBSDIR "/share/commandhelp/helpterm"
#define VALID 			BBSDIR "/share/messages/valid"
#define WKRDIR 			BBSDIR "/save/WKR/"

#define HOSTNAME_IF_HIDDEN	"              "

/* The MAXQUOTE is the number of quotes in share/quotes.
 * quotenumber is randomly generated from 1 to MAXQUOTE in main.c
 */

#define MAXQUOTE		61

/* These are the current help prompts
 * Short prompt:
 * <w>ho <C>hat e<x>press <j>ump (Press ? for commands or h for help)
 *
 * Long prompt:
 * <w>ho <n>ext <j>ump m<e>ssage <s>top (Press ? for commands or h for help)
 */

#define SHORT_HELPPROMPT	"\1f\1w<\1rw\1w>\1gho \1w<\1rc\1w>\1ghat e\1w<\1rx\1w>\1gpress \1w<\1rj\1w>\1gump  \1w(\1gPress \1r?\1g for online help\1w)"
#define LONG_HELPPROMPT		"\1f\1w<\1rw\1w>\1gho \1w<\1rn\1w>\1gext \1w<\1rj\1w>\1gump m\1w<\1re\1w>\1gssage \1w<\1rs\1w>\1gtop \1w(\1gPress \1r?\1g for online help\1w)"
#define FRIEND_CMDS		"\1f\1w<\1ra\1w>\1gdd \1w<\1rd\1w>\1gelete \1w<\1rl\1w>\1gist \1w<\1rq\1w>\1guit \1w(\1gPress \1r?\1g for online help\1w)"
#define ENEMY_CMDS		"\1f\1w<\1ra\1w>\1gdd \1w<\1rd\1w>\1gelete \1w<\1rl\1w>\1gist \1w<\1rq\1w>\1guit \1w(\1gPress \1r?\1g for online help\1w)"
