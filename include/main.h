/* $Id$ */

extern RETSIGTYPE dropcarr(int sig);
extern RETSIGTYPE sleeping(int sig);
extern RETSIGTYPE segfault( int sig);
extern RETSIGTYPE updateself(int sig);
extern RETSIGTYPE kickoutmyself(int sig);
extern void change_passwd(user_t *user);
extern void do_changepw(void);
extern void getwindowsize(int sig);
extern void enter_name( char *usernm);
extern int enter_passwd(user_t *user);
extern void init_system(void);
extern void logoff(int code);
extern void mailcheck(void);
extern int main(int argc, char *argv[] );
extern void print_login_banner(time_t laston);
extern int user_terminate(void);
extern void admin_info( void );
extern void check_profile_updated( void );
