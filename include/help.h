/* $Id$ */

/* defines */
#define LITDIR		BBSDIR "share/messages/literature/"
#define SHAKEDIR	BBSDIR "share/messages/shakespeare/"
#define HELPDIR		BBSDIR "share/help/"

/* prototypes */
extern void help_topics( void );
extern void literature_menu( void );
extern int help_commands( void );
extern void online_help(const char);
extern void online_help_wrapper(const unsigned int, const long, const char *);
