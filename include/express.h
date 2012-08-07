/* $Id$ */

/* overrides */
#define OR_PING         'P' /* catchx() ping() */
#define OR_T_GUIDE      'G' /* catchx() express()  -  marked as SysGuide     */
#define OR_T_TECH       's' /* catchx() express()  -  marked as Technician   */
#define OR_T_ADMIN      'S' /* catchx() express()  -  marked as Admin        */
#define OR_QUESTION     'Q' /* catchx() express()  -  question to a SysGuide */
#define OR_FEEL         'f' /* catchx() express()  -  this is a feeling      */
#define OR_EMOTE        '0' /* catchx() express()  -  this is an emote       */
#define OR_SHIX         'M' /* etc */
#define OR_INTER        'i' /* inter bbs x */
#define OR_WEB	        'W' /* web x */

/* broadcast overrides */
#define OR_IMPORTANT    'I'
#define OR_BROADCAST    'B'
#define OR_COUNTDOWN    'Z'
#define OR_NOWHERE      'v'
#define OR_SILENT       'b' 
#define OR_LLAMA	'J'
#define OR_FISH		'K'
#define OR_SHOUT        'H'

/* pseudo-broadcast overrides, uses the broadcast code: */
#define OR_LOGIN        'L' 
#define OR_LOGOUT       'l'
#define OR_KICKOFF	'k'
#define OR_SILC         'C' 
#define OR_CHAT		'N' /* newstyle chat */

/* other overrides */
#define OR_ENABLED      'e' /* on enable list */
#define OR_ENABLE_FORCE 'E' /* used to override enemylists by >= sys_analysts */
#define OR_NO_PERMS     'D' /* no permissions to send x */

#define IS_WEB		(override == 'W')
#define IS_BROADCAST	(strchr( "IBZvbJKHLlkCN" , override ))
#define QUEUED		(!strchr( "IZ", override ))

#define OR_NORMAL       ' ' /* sendx() express() ping() */
#define OR_FREE         255 /* sendx() catchx() setup_express() */

/* Ack. values                      Where used  */
#define NO_ACK           ' ' /* several functions */
#define ACK_RECEIVED     'A' /* sendx() catchx() ping() */
#define ACK_PING_BUSY    'B'
#define ACK_DISABLED     'D' /* sendx() catchx() */
#define ACK_BUFFFULL     'F' /* sendx() ping() */
#define ACK_TURNEDOFF    'O' /* sendx() catchx() ping() */
#define ACK_BUSY         'W' /* sendx() catchx() ping() */
#define ACK_SHIX         'M' /* sendx() catchx()  */
#define ACK_AWAY	 'Y' /* sendx() catchx()  */
#define ACK_LOCK	 'L' /* sendx() catchx()  */
#define ACK_LOCK	 'L' /* sendx() catchx()  */
#define ACK_NOTBUSY 	 'N' /* format_ack() catchx()  */
#define ACK_OOPS	 'o' /* something went very wrong */

/* misc */
#define SENDX   'S'  /* denotes call to put_in_personal_x_log() from sendx() */
#define CATCHX  'C'  /* ditto, but from catchx() */

#define QUICKX    (X_PARAM >= 10 && X_PARAM < 20)
#define FEEL      (X_PARAM >= 20 && X_PARAM < 60)
#define REPLY     (X_PARAM == -1)
#define NORMAL    (X_PARAM == 0)
#define QUESTION  (X_PARAM == 1)
#define BROADCAST (X_PARAM == -2 || X_PARAM == -3 || X_PARAM == 3 || X_PARAM == 7)
#define EMOTE     (X_PARAM == 2)
#define NCHAT     (X_PARAM == 3)
#define NSILC     (X_PARAM == 7)
#define WEB  	  (X_PARAM == -4)

/* prototypes	*/
void single_catchx( int sig );
void multi_catchx( int sig );
char *format_express( x_struct Catchxs );
void sendx( char *to, const char *send_string, char override );
void ping( char *to );
void quoted_Xmsgs (void);
void setup_express( void );
void remove_express( void );
void are_there_held_xs( void );
void express( int );
void change_express(int how);
void old_express(void);
void emergency_boot_all_users( void );
void send_silc( void );
void add_x_to_personal_xlog(char, x_str *, char);
char *get_guide_name( char * );
char * get_xmessage_destination(char * , const int, char * );
int user_not_allowed_to_send_x(const int);
char check_x_permissions(const char *, const int, char);
void display_express_prompt(const int);
void feeling(void);
void are_there_held_web_xs(void);
void show_web_xes(wx_list_t *);

extern volatile int xmsgp;
extern express_t *xmsgb[XLIMIT];

/* eof */

