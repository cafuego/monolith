/* $Id$ */

/* prototypes */
extern void key_menu( void );
extern void enter_key( void );
extern int send_key( const user_type *user );
extern void generate_new_key( user_type *user );

/* defines */
#define VALIDATION_EMAIL BBSDIR "/share/messages/validation.txt"
