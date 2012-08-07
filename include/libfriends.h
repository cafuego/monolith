/* $Id$ */

/* defines */
#define ENEMY	0
#define FRIEND	1

/* prototypes */
int read_friends(const char *user, friend_t ** first, int param);
int write_friends(const char *user, friend_t * first, int param);
int remove_friend_from_list( const char *name, friend_t **list );
int add_friend_to_list( friend_t element, friend_t **list );
int dest_friends_list( friend_t *list );
int user_on_list( const char *name, friend_t *list );
int is_friend( const char *user1, const char *user2 );
int is_enemy( const char *user1, const char *user2 );
char * number_to_friend( int num, friend_t *list );
int friend_to_number( const char *name, friend_t *list );
int set_quickx( const char *name, int quick, friend_t *list );
