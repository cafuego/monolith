/* $Id */

/* defines */

#define SAVE_CURSOR     cprintf( "7" );
#define RESTORE_CURSOR  cprintf( "8" );
#define OUTPUT_WINDOW   cprintf( "^[[%d;1f", usersupp->screenlength - 2)
#define INPUT_WINDOW    cprintf( "^[[%d;1f", usersupp->screenlength)

/* prototypes */

extern void statusbar(char *outstr);
extern void status_bar_on( void );
extern void status_bar_off( void );
extern void holodeck_statusbar_on( void );
extern void holodeck_statusbar_off( void );
extern void switch_window( int how );
