/*
 * $Id$
 */

extern int mono_sql_ll_add_mlist_to_list(mlist_t entry, mlist_t ** list);
extern void mono_sql_ll_free_mlist(mlist_t *list);
extern int mono_sql_ll_add_srlist_to_list(sr_list_t entry, sr_list_t ** list);
extern void mono_sql_ll_free_sr_list(sr_list_t *list);
extern int mono_sql_ll_add_xlist_to_list(xlist_t x, xlist_t ** list);
extern void mono_sql_ll_free_xlist(xlist_t *list);
extern int mono_sql_ll_add_wulist_to_list(wu_list_t x, wu_list_t ** list);
extern void mono_sql_ll_free_wulist(wu_list_t *list);
extern int mono_sql_ll_add_wxlist_to_list(wx_list_t x, wx_list_t ** list);
extern void mono_sql_ll_free_wxlist(wx_list_t *list);

/* eof */
