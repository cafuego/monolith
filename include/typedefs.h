/* $Id$ */

#define RGnameLEN 30		/* The User's real full name              */
#define RGaddrLEN 50		/* The User's address                     */
#define RGcityLEN 30		/* The User's address                     */
#define RGstateLEN 30		/* The User's address                     */
#define RGcountryLEN 30		/* The User's address                     */
#define RGzipLEN 15		/* The User's address                     */
#define RGphoneLEN 25		/* The User's phone number                */
#define RGemailLEN 41		/* The User's email address               */
#define RGurlLEN 100		/* the user's home page  */

#define MAXCONFIGS 50   /* max number of peet's configurations */

/* mail stuff */
#define MAIL_QUOTA     /* turns mail quota (purge) on for users */
#define MAX_USER_MAILS 200

typedef struct topic_struct {
	char name[L_TOPICNAME+1]; /* name of topic */
        unsigned int id;	/* number of topic */
        unsigned int highest; /* highest post number */
	unsigned int forum; /* the forum that `owns' this thread */
        unsigned int posttype;
} topic_t;

typedef struct date_struct {
    int day;
    int month;
    int year;
} date_t;

/* a short list with username/numbers. mainly to be returned 
   when we need a list of users. (used in userforum for example) */
typedef struct userlist userlist_t;
struct userlist {
    char name[L_USERNAME+1];
    unsigned int usernum;
    userlist_t *next;
};

/* a short list with forumname/numbers. mainly to be returned 
   when we need a list of forums. (used in userforum for example) */
typedef struct forumlist forumlist_t;
struct forumlist {
    char name[L_QUADNAME+1];
    unsigned int forum_id;
    forumlist_t *next;
};

/* a short list with message numbers. To be used when reading
   messages in a forum. */
typedef struct mlist mlist_t;
struct mlist {
    unsigned int id;
    mlist_t *next;
    mlist_t *prev;
};

typedef struct {
    unsigned int num;
    unsigned int topic;
    unsigned int forum;
    unsigned int author;
    char alias[L_USERNAME+1];
    char subject[L_SUBJECT];
    time_t date;
    char content[100];
    char type[8];
    char priv[8];
    char deleted;
    int score;
} message_t;

typedef struct xfriend friend_t;
struct xfriend {
    char name[L_USERNAME + L_BBSNAME + 1];
    unsigned int usernum;
    unsigned int flags;
    int quickx;
    friend_t *next;
};

/*  -- BBS look and feel in shm!! -- */
typedef struct {
    char bbsname[20];
    char forum[20];
    char forum_pl[22];
    char message[12];
    char message_pl[14];
    char express[12];
    char x_message[12];
    char x_message_pl[14];
    char user[12];
    char user_pl[14];
    char username[12];
    char doing[20];
    char location[16];
    char chatmode[20];
    char chatroom[20];
    char admin[20];
    char wizard[20];
    char sysop[20];
    char programmer[20];
    char roomaide[20];
    char guide[20];
    char idle[18];
} config_t;

/* typedef struct usersupp {	/ * User record */
typedef struct {	/* User record */
    char username[L_USERNAME + 1];	/* The User's UserName          */
    char password[14];		/* The User's Password                  */
    unsigned int usernum;	/* The User's UserNumber                */
    unsigned priv;		/* The User's Access level: PRIV_       */
    long lastseen[MAXQUADS];	/* Last message seen in each room       */
    char generation[MAXQUADS];	/* Generation # (for private rooms)     */
    char forget[MAXQUADS];	/* Forgotten generation number          */
    char roominfo[MAXQUADS];	/* RoomInfo-number to check if new RI   */
    long mailnum;		/* Highest mail number of user		*/
    unsigned int flags;		/* See US_flags below                   */
    unsigned int config_flags;  /* See CO_flags below  			*/
    unsigned int chat;		/* subscribed chat channels 		*/
    char alias[L_USERNAME +1];  /* users' standard aliasname		*/
    unsigned int configuration;	/* number of BBS configuration to use	*/
    unsigned int screenlength;	/* lines before MORE                    */
    int alarm_clock;		/* the Alarm-clock-time (in minutes)    */
    unsigned int timescalled;	/* Number of logins                     */
    int posted;			/* Number of messages posted (ever)     */
    int RA_rooms[5];		/* Numbers of rooms the user RA's.      */
    long validation_key;	/* the number ofthe validation key      */
    unsigned long x_s;		/* Number of X's ever received & sent   */
    time_t firstcall;		/* time of first call                   */
    time_t laston_from;		/* Last time the user called            */
    time_t laston_to;		/* time when the user logged off        */
    date_t birthday; 		/* birthday */
    long online;		/* total time the user has been online  */
    char doing[40];		/* The User's Doing-field               */
    char xtrapflag[70];		/* The User's Extra "Title-thing"       */
    char aideline[162];		/* The User's Aideline                  */
    char lasthost[80];		/* utmp name of last host on from       */
    char awaymsg[100];		/* away message */
    char RGname[RGnameLEN + 1];	/* The User's real full name            */
    char RGaddr[RGaddrLEN + 1];	/* The User's address                   */
    char RGzip[RGzipLEN + 1];	/* The User's address                   */
    char RGcity[RGcityLEN + 1];	/* The User's address                   */
    char RGstate[RGstateLEN + 1];	/* The User's address           */
    char RGcountry[RGcountryLEN+1];
    char RGphone[RGphoneLEN + 1];	/* The User's phone number      */
    char RGemail[RGemailLEN + 1];	/* The User's email address     */
    char RGurl[RGurlLEN + 1];	/* the user's home page  		*/
    int hidden_info;		/* what parts of the address are hidden */
} user_t, user_type;

/****************************************************************************/

typedef struct status {
    unsigned flags;		/* lock-flags: see the S_* below        */
}     *stat_struct;

/****************************************************************************/

typedef struct mono_queue {
    char host[L_HOSTNAME + 1];	/* hostname                     */
    pid_t pid;			/* PID of the user                      */
    unsigned flags;		/* some flags                           */
} mono_queue_t;

/****************************************************************************/

/* typedef struct btmp { */
typedef struct btmp {
    char username[L_USERNAME + 1];	/* user's name                  */
    char alias[L_USERNAME +1];		/* user's chat alias 		*/
    char remote[L_HOSTNAME + 1];	/* user's country		*/
    char ip_address[L_HOSTNAME + 1];	/* user's ip number/name	*/
    char awaymsg[100];			/* away message */
    char doing[31];		/* user's doing                         */

    char curr_room[41];		/* room the user is in right now        */
    char x_ing_to[L_USERNAME+L_BBSNAME+1]; /* the user this user is x-ing */

    time_t logintime;		/* user's logintime in unix-format      */
    time_t idletime;		/* user's idlestarttime                 */

    int chat;			/* if they are in chatmode or not       */
    int status;			/* type of away, for future use         */

    unsigned int priv;		/* user's priv.                         */
    pid_t pid;			/* user's PID                           */
    unsigned int flags;		/* different flags: look at B_* below   */
    int next;			/* next positionin the linked list      */
} btmp_t, *btmp_struct;

/****************************************************************************/

typedef struct {
    int override;		/* override char                */
    int ack;			/* acknowledgement char         */
    time_t time;		/* the time of sending          */
    char sender[L_USERNAME + L_BBSNAME + 2];	/* username of sender           */
    char recipient[L_USERNAME +L_BBSNAME + 2];	/* username of recipient        */
    unsigned int sender_priv;	/* priv of sender               */
    char message[X_BUFFER];	/* the whole x-message          */
} express_t, x_str, *x_struct;

/****************************************************************************/

typedef struct quickroom {
    char name[L_QUADNAME + 1];	/* Max. len is 40, plus null term       */
    char qls[NO_OF_QLS][L_USERNAME + 1];	/* RA's username                */
    char category[60];		/* room category */
    unsigned long highest;	/* Highest message NUMBER in room       */
    unsigned long lowest;	/* This was the dirname before. notused */
    char generation;		/* Generation number of room            */
    unsigned flags;		/* See flag values below                */
    char roominfo;		/* RoomInfo-checknumber                 */
    unsigned int maxmsg;	/* Max number of messages in a quad     */
} room_t;

/* this contains the chat channels */
typedef struct {
    char name[L_QUADNAME + 1];
    char type;
} channel_t;

/* the quad type is supposed to be a replacement for the quickroom */
typedef struct {
    char name[L_QUADNAME + 1];	/* name of quadrant */
    char ql[L_USERNAME + 1][NO_OF_QLS];		/* name of qls */
    unsigned int number;	/* number of the room */
    unsigned long type;		/* type of room */
    unsigned long post_type;	/* type of allowed posts */
    unsigned long allowed;	/* who are allowed in room? */
    unsigned int maxposts;	/* max number of posts */
    unsigned int lastpost;	/* last posted */
    unsigned int generation;	/* generation number */
    unsigned int roominfo;	/* roominfo check */
} mono_quad_t;

typedef struct {
    unsigned int user_count;
    int first; /* array index to first user in wholist */
    btmp_t wholist[MAXUSERS];
    express_t xslot[MAXUSERS];

    express_t broadcast;

    unsigned int queue_count;
    mono_queue_t queue[MAXQUEUED];

    channel_t holodeck[MAXCHATROOMS];

    struct quickroom rooms[MAXQUADS];

    long status;
    config_t config[MAXCONFIGS];

} bigbtmp_t;

typedef struct {
    char author[L_USERNAME+1];		/* name of poster */
    char subject[L_SUBJECT+1];		/* subject */
    time_t date;			/* time of posting */
    char alias[L_USERNAME+1];		/* alias, if applicable */
       /* NOTE!  post.type is cast to an int in messages.c */     
    long type;				/* type of post */
    char recipient[400];		/* recipient of mail */
    int lines;				/* number of lines in message */
    int quad;				/* quad */
    long num;				/* number */

    long ref;				/* number of post this is a follwup to  */
    char refauthor[L_USERNAME+1];	/* name of author you are referring to */
    long reftype;			/* type of post you replied to */

    long orignum;			/* original number */
    char modifier[L_USERNAME+1];
    char origroom[L_QUADNAME+1];	/* name of original quad */
    time_t moddate;			/* time of modification (move) */
} post_t;
