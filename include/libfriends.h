/* $Id$ */

/* defines */
#define ENEMY	0
#define FRIEND	1

/* prototypes */
extern int read_friends(const char *user, friend_t ** first, int param);
extern int write_friends(const char *user, friend_t * first, int param);
extern int remove_friend_from_list( const char *name, friend_t **list );
extern int add_friend_to_list( friend_t element, friend_t **list );
extern int dest_friends_list( friend_t *list );
extern int user_on_list( const char *name, friend_t *list );
extern int is_friend( const char *user1, const char *user2 );
extern int is_enemy( const char *user1, const char *user2 );
extern char * number_to_friend( int num, friend_t *list );
extern int friend_to_number( const char *name, friend_t *list );
extern int set_quickx( const char *name, int quick, friend_t *list );
