/* $Id$ */

void dropcarr(int sig);
void sleeping(int sig);
void segfault( int sig);
void updateself(int sig);
void kickoutmyself(int sig);
void change_passwd(user_t *user);
void do_changepw(void);
void getwindowsize(int sig);
void enter_name( char **username);
int enter_passwd(const char *username);
void init_system(void);
void logoff(int code);
void mailcheck(void);
int main(int argc, char *argv[] );
void print_login_banner(time_t laston);
int user_terminate(void);
void admin_info( void );
void check_profile_updated( void );
