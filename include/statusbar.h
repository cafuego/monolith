/* $Id */

/* defines */

#define SAVE_CURSOR     cprintf( "7" );
#define RESTORE_CURSOR  cprintf( "8" );
#define OUTPUT_WINDOW   cprintf( "^[[%d;1f", usersupp->screenlength - 2)
#define INPUT_WINDOW    cprintf( "^[[%d;1f", usersupp->screenlength)

/* prototypes */

void statusbar(char *outstr);
void status_bar_on( void );
void status_bar_off( void );
void holodeck_statusbar_on( void );
void holodeck_statusbar_off( void );
void switch_window( int how );
