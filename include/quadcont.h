/* $Id$ */

#define QC_ENABLE  /*  enables the database functions */
#define QC_ADMIN_ENABLE /* enables admin accessable database functions */
#define QC_USER_ENABLE /* enables normal "user" accessable database functions */

/* #define QC_DEBUG */

#define QC_FILEDIR BBSDIR "save/forums/"  
#define QC_EDIT_LOCKDIR BBSDIR "save/forums/"  
#define QC_LOCKOUT BBSDIR "save/forums/lockout.all"  /*lockout file */
  /*docs*/
#define QC_NEWUSER_HELP BBSDIR "share/quadcont/newuser.doc"
#define QC_NEWUSER_DONE "share/quadcont/newuser.done"
#define QC_USERDOC BBSDIR "share/quadcont/userdoc.doc"
#define QC_USERDOC_DISCLAIMER "share/quadcont/userdoc.done"
#define QC_QUAD_EDIT_DOC BBSDIR "share/quadcont/quadedit"
#define QC_LOCK_DOC BBSDIR "share/quadcont/lockhelp"
 

#define TEMPLATE_QUAD 1       /* set to Mail quad */
#define NEWBIE_NUM_UNREAD 10  /* default # of messages to be left unread */
#define L_CONTENT_CATEGORY 24 /* max strlen of category name */
#define CONTENT_SCALE   5     /* room evaluation scale: 0 - CONTENT_SCALE*/

#define NO_OF_CATEGORIES  24  /* NOTE: set to an EVEN number if possible. */
#define ARGH_QUAD_FLAGS 4     /* supports up to 9 ints, useage hardcoded
                               * flag [0] is reserved for newbie zap */

typedef struct quad_content_quotient {
    int  quad_number;
    char category_name[NO_OF_CATEGORIES][L_CONTENT_CATEGORY + 1];
    int  content_quotient[NO_OF_CATEGORIES];
    int  flags[ARGH_QUAD_FLAGS];
    char last_mod_by[22];  /* L_USERNAME + 1, in case somebody changes */  
    time_t last_mod_on;
} qc_thingme;

/* prototypes */

extern void qc_menu(void);
extern void qc_edit_room(void);
extern int  qc_user_menu(int);
extern void qc_categories_menu(void);
extern void qc_lists_menu(void);

extern int  qc_create_file(int);
extern qc_thingme * qc_read_file(int);
extern int  qc_write_file(qc_thingme *, int);
extern void qc_display(qc_thingme *, int );
extern void qc_show_noqcfile(void);
extern int qc_mail_results(const int *);
extern void qc_show_results(const int *);

extern void qc_add_category(void);
extern void qc_delete_category(void);
extern void qc_rename_category(void);

extern int  qc_get_category_slot(void);
extern int  qc_set_value( int);
extern int  qc_set_flags(int);

extern void qc_lock_menu(void);
extern void qc_toggle_lockout(void);
extern void qc_toggle_edit_lock(void);
extern int  qc_change_lock_status(int, int);
extern void qc_show_lock_status(void);
extern int  qc_set_lockout(void);
extern char * qc_who_locked_it(const char *, char *);
extern int  qc_set_quadlock(int);
extern void qc_clear_all_locks(void);
extern int  qc_clear_quadlock(int);
extern int  qc_clear_lockout(void);
extern int  qc_lockout(void);
extern int  qc_quadlocked(int);

extern int * qc_evaluate_quads(const qc_thingme *, int, const user_t *);
