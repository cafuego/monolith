/* $Id$ */

/* defines */

#define H_REALNAME	1
#define H_ADDRESS	2
#define H_CITY		4
#define H_COUNTRY	8
#define H_PHONE        16 
#define H_EMAIL        32
#define H_URL          64 
#define H_BIRTHDAY    128
#define H_ZIP	      256

/* prototypes */
extern void dis_regis( const user_type *userdata, int override );
extern void enter_reginfo( void );
extern void enter_info( user_type *user );
extern void change_info( user_type *user, int override );
extern void toggle_hidden_info( user_type *user );
extern int is_allowed_email( const char *email );
