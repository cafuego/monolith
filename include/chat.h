/* $Id$ */

/* defines			*/

#define MAXCHATCHAMBERS 8
#define CHATOP 1
#define MAXCHATUSERS 99

/* prototypes			*/

extern int is_chat_subscribed( int chat,  const char *channel );
extern void chat_subscribe(void);

extern int chat_command( void );
extern int send_chatmsg(void);
extern void read_holodecks( int how );
extern void change_holodeck(void);
extern void lock_function(int semaphore, int key);
extern void change_type( unsigned int number );
extern void op_command(void);
extern void in_chat( void );
extern int get_alias( void );
extern int is_allowed( unsigned int );
extern void edit_programme( void );
