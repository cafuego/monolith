/* $Id$ */

/* prototypes */
extern int check_remote_user (const char *name, const char *rbbs);
extern void menu_inter( void );
extern int parse_inter_address( const char *, char *, char * );
extern int port_connect (const char *bbsname, unsigned int tries);
extern int dexi_wholist (const char *bbsname );
extern int dexi_profile( const char *, const char * );
extern int remote_express( const char * );
extern void print_inter_hosts(void);
