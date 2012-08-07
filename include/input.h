/* $Id$ */

/* prototypes */
char * get_name (int quit_priv);
int xgetline (char *string, int lim, int nocol);
int get_string_wr (int lim, char *result, int line, int flag);
int get_buffer (FILE * outputfp, int how, int *lines);
int get_x_lines (char *xstring, int X_PARAM);
void ColourChar (char key);
int client_input( int, int );
int editor_edit ( const char *fname);
int inkey( void );
int flush_input( void );
int netget (char skip_sync_byte);
void send_update_to_client ( void );

/* defines */

#define EDIT_NORMAL 1
#define EDIT_CTRLD  2
#define EDIT_EDITOR 3
#define EDIT_NOEDIT 4

/* eof */
