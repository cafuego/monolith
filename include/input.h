/* $Id$ */

/* prototypes */
extern char * get_name (int quit_priv);
extern int getline (char *string, int lim, int nocol);
extern int get_string_wr (int lim, char *result, int line, int flag);
extern int get_buffer (FILE * outputfp, int how, int *lines);
extern int get_x_lines (char *xstring, int X_PARAM);
extern void ColourChar (char key);
extern int client_input( int, int );
extern int editor_edit ( const char *fname);
extern int inkey( void );
extern int flush_input( void );
extern int netget (char skip_sync_byte);
extern void send_update_to_client ( void );

/* defines */

#define EDIT_NORMAL 1
#define EDIT_CTRLD  2
#define EDIT_EDITOR 3
#define EDIT_NOEDIT 4

/* eof */
