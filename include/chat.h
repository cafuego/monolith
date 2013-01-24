
/* defines			*/

#define MAXCHATCHAMBERS 8
#define CHATOP 1
#define MAXCHATUSERS 99

/* prototypes			*/

int is_chat_subscribed( int chat,  const char *channel );
void chat_subscribe(void);

int chat_command( void );
int send_chatmsg(void);
void read_holodecks( int how );
void change_holodeck(void);
void lock_function(int semaphore, int key);
void change_type( unsigned int number );
void op_command(void);
void in_chat( void );
int get_alias( void );
int is_allowed( unsigned int );
void edit_programme( void );
