int mono_sql_rat_add_rating(unsigned int, unsigned int, unsigned int, int);
float mono_sql_rat_get_rating(unsigned int, unsigned int);
int mono_sql_rat_check_rating(unsigned int, unsigned int, unsigned int);
int mono_sql_rat_erase_forum(unsigned int forum);

#define R_TABLE	"rating"
