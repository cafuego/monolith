#define BINGME cprintf("\n\n\1f\1rBING\n\n"); /* scary debug thingme */

#define DOCKING_BAY_FORUM	0
#define MAIL_FORUM 		1
#define YELL_FORUM 		2
#define SYSOP_FORUM		3
#define EMPEROR_FORUM		4
#define GARBAGE_FORUM		5
#define LOWER_ADMIN_FORUM	6
#define POLICY_FORUM		7
#define QL_FORUM		8
#define QUOTED_X_FORUM		9
#define CURSE_FORUM		13
#define HT_FORUM		109
#define TRASH_FORUM		144
#define HANDLED_YELL_FORUM	148

#define FORUMDIR BBSDIR "/save/forums"

#define DISPLAY_NORMAL		0
#define DISPLAY_INFO		1
#define DISPLAY_2_CLIP		2
#define DISPLAY_2_TEMP_W	4
#define DISPLAY_2_TEMP_A	8

#define QUADCONT_MAIL_BANNER	524288
#define YELL_REPLY_BANNER	262144
#define YELL_BANNER		131072
#define X_2_MAIL_BANNER		65536
#define DELETE_MAIL_BANNER	32768
#define FILE_POST_BANNER	16384
#define QUADLIZARD_MAIL_BANNER	8192 
#define QUADLIZARD_BANNER	4096
#define EMP_BANNER		2048
#define SYSOP_BANNER		1024
#define ADMIN_BANNER		512
#define TECH_BANNER		256
#define SYSTEM_BANNER		128
#define CUSTOM_BANNER		64
#define INFO_BANNER     	16
#define QL_BANNER		32
#define CLIP_BANNER		16
#define FORCED_BANNER   	8
#define MAIL_BANNER		4
#define REPLY_BANNER		2
#define ANON_BANNER		1
#define NO_BANNER		0

#define AUTOMAIL_BANNER (QUADLIZARD_MAIL_BANNER | DELETE_MAIL_BANNER | X_2_MAIL_BANNER | YELL_BANNER | QUADCONT_MAIL_BANNER)

#define AUTOMAIL_NO_PERSONAL_COPY (QUADLIZARD_MAIL_BANNER | QUADCONT_MAIL_BANNER)

#define AUTOBANNER (INFO_BANNER | CLIP_BANNER | QUADLIZARD_BANNER | AUTOMAIL_BANNER | FILE_POST_BANNER)

#define REPLY_MODE	5
#define MOD_MOVE	1
#define MOD_COPY	2
#define MOD_EDIT	4
#define L_BANNER	60
#define L_RECIPIENT	100
#define L_FILENAME	128

typedef struct {
   char author[L_USERNAME + 1];
   char subject[L_SUBJECT + 1];
   char alias[L_USERNAME + 1];
   time_t date;

   unsigned int score_number;
   int score_sum;
   unsigned int line_total;

   unsigned int anonymous;

   char recipient[L_RECIPIENT + 1];
   char forum_name[L_QUADNAME + 1];
   char banner[L_BANNER + 1];
   unsigned long banner_type;

   char reply_to_author[L_USERNAME + 1];
   unsigned int reply_m_id;
   unsigned int reply_f_id;
   unsigned int reply_t_id;
   unsigned int reply_anonymous;
 
   unsigned int m_id;
   unsigned int f_id;
   unsigned int t_id;

   time_t orig_date;
   char modified_by[L_USERNAME + 1];
   char orig_forum[L_QUADNAME + 1];
   unsigned int mod_type;
   unsigned int orig_m_id;
   unsigned int orig_f_id;
   unsigned int orig_t_id;

   unsigned int quad_flags;

} message_header_t;

size_t get_filesize(const char *);
int count_dir_files(const char *);
int write_message_header(const char *, message_header_t *);
int read_message_header(const char *, message_header_t *);
char * message_filename(char *, unsigned int , unsigned int );
char * message_header_filename(char *, unsigned int , unsigned int );
char * mail_header_filename(char *, const char *, const unsigned int);
char * mail_filename(char *, const char *, const unsigned int);
char * info_filename(char *, unsigned int);
char * info_header_filename(char *, unsigned int);
int message_copy(const unsigned int, const unsigned int,
			const unsigned int, const char *, const unsigned int);
int message_move(const unsigned int, const unsigned int,
			const unsigned int, const char *);
int message_delete(const unsigned int, const unsigned int);
void init_message_header(message_header_t *); 
void save_to_sql(const message_header_t *, const char *);

#ifdef OHH_SHIT_WE_HAVE_TO_CONVERT_AGAIN_ARRRRRRGH
int convert_message_base(int);
#endif
