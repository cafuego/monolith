/*
 * $Id$
 */
int mono_sql_express_add(unsigned int user_id, express_t *);
int mono_sql_express_list_xlog(unsigned int , xlist_t **);
int mono_sql_express_expire(unsigned int );

#define X_TABLE "express"
