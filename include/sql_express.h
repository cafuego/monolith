/*
 * $Id$
 */
int mono_sql_express_add(unsigned int user_id, express_t *x);
int mono_sql_express_list_xlog(unsigned int user_id, xlist_t **list);
int mono_sql_express_expire(unsigned int user_id);

#define X_TABLE "express"
