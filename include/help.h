/* $Id$ */

/* defines */
#define LITDIR		BBSDIR "/share/messages/literature/"
#define SHAKEDIR	BBSDIR "/share/messages/shakespeare/"
#define HELPDIR		BBSDIR "/share/help/"

/* prototypes */
void help_topics( void );
void literature_menu( void );
int help_commands( void );
void online_help(char);
void online_help_wrapper(unsigned int, long, void *);
