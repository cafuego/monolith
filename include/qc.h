/* $Id$ */

#define QC_ENABLE 

#define QC_TABLE "fqc"
#define USER_QC_TABLE "uqc"

#define QC_LOCKOUT BBSDIR "save/forums/qc.lockout"  /*lockout file */

  /*docs*/
#define QC_NEWUSER_HELP BBSDIR "share/quadcont/newuser.doc"
#define QC_NEWUSER_DONE "share/quadcont/newuser.done"
#define QC_U_EDIT_DOC BBSDIR "share/quadcont/userdoc.doc"
#define QC_USERDOC_DISCLAIMER "share/quadcont/userdoc.done"
#define QC_F_EDIT_DOC BBSDIR "share/quadcont/quadedit"
#define QC_LOCK_DOC BBSDIR "share/quadcont/lockhelp"
 
#define USER_EDIT	0
#define FORUM_EDIT	1

#define QC_AUTOZAP	1
#define QC_NEWUSER	2

#define NEWBIE_NUM_UNREAD 10  /* default # of messages to be left unread */
#define L_CONTENT_CATEGORY 24 /* max strlen of category name */
#define CONTENT_SCALE   5     /* room evaluation scale: 0 - CONTENT_SCALE*/
#define NO_OF_CATEGORIES  24
                               
typedef struct quad_content_quotient {
    int  quad_number;
    char category_name[24][L_CONTENT_CATEGORY + 1];
    int  content_quotient[24];
    int  flags[4];
    char last_mod_by[22];  /* L_USERNAME + 1, in case somebody changes */  
    time_t last_mod_on;
} qc_thingme;

typedef struct {
    unsigned int id;
    unsigned int cat_id[NO_OF_CATEGORIES];
    char cat_name[NO_OF_CATEGORIES][L_CONTENT_CATEGORY + 1];
    unsigned int cat_quot[NO_OF_CATEGORIES];
    unsigned int flags;
    unsigned int newbie_r;
} qc_record;

/* prototypes */

extern void new_qc(void);
extern int _sql_qc_zero_forum_categories(const unsigned int);
extern void qc_edit_room(void);
extern int qc_user_menu(int);

